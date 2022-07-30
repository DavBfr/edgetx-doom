/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "opentx.h"
#include "mixer_scheduler.h"
#include "timers_driver.h"

#ifdef WITH_DOOM
#include "doom_main.h"
#endif

RTOS_TASK_HANDLE menusTaskId;
#ifndef WITH_DOOM
RTOS_DEFINE_STACK(menusStack, MENUS_STACK_SIZE);
#endif

RTOS_TASK_HANDLE mixerTaskId;
RTOS_DEFINE_STACK(mixerStack, MIXER_STACK_SIZE);

RTOS_TASK_HANDLE audioTaskId;
#ifndef WITH_DOOM
RTOS_DEFINE_STACK(audioStack, AUDIO_STACK_SIZE);
#endif


RTOS_MUTEX_HANDLE audioMutex;
RTOS_MUTEX_HANDLE mixerMutex;

void stackPaint()
{
  mixerStack.paint();
#ifndef WITH_DOOM
  menusStack.paint();
  audioStack.paint();
 #endif
#if defined(CLI)
  cliStack.paint();
#endif
}

volatile uint16_t timeForcePowerOffPressed = 0;

bool isForcePowerOffRequested()
{
  if (pwrOffPressed()) {
    if (timeForcePowerOffPressed == 0) {
      timeForcePowerOffPressed = get_tmr10ms();
    }
    else {
      uint16_t delay = (uint16_t)get_tmr10ms() - timeForcePowerOffPressed;
      if (delay > 1000/*10s*/) {
        return true;
      }
    }
  }
  else {
    resetForcePowerOffRequest();
  }
  return false;
}

void sendSynchronousPulses(uint8_t runMask)
{
#if defined(HARDWARE_INTERNAL_MODULE)
  if (runMask & (1 << INTERNAL_MODULE)) {
    if (setupPulsesInternalModule())
      intmoduleSendNextFrame();
  }
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  if (runMask & (1 << EXTERNAL_MODULE)) {
    if (setupPulsesExternalModule())
      extmoduleSendNextFrame();
  }
#endif
}

constexpr uint8_t MIXER_FREQUENT_ACTIONS_PERIOD = 5 /*ms*/;
constexpr uint8_t MIXER_MAX_PERIOD = MAX_REFRESH_RATE / 1000 /*ms*/;

void execMixerFrequentActions()
{
#if defined(SBUS_TRAINER)
  // SBUS trainer
  processSbusInput();
#endif

#if defined(IMU)
  gyro.wakeup();
#endif

#if defined(BLUETOOTH)
  bluetooth.wakeup();
#endif
}

TASK_FUNCTION(mixerTask)
{
  s_pulses_paused = true;

  mixerSchedulerInit();
  mixerSchedulerStart();

  while (true) {
    int timeout = 0;
    for (; timeout < MIXER_MAX_PERIOD; timeout += MIXER_FREQUENT_ACTIONS_PERIOD) {

      // run periodicals before waiting for the trigger
      // to keep the delay short
      execMixerFrequentActions();

      // mixer flag triggered?
      if (!mixerSchedulerWaitForTrigger(MIXER_FREQUENT_ACTIONS_PERIOD)) {
        break;
      }
    }

#if defined(DEBUG_MIXER_SCHEDULER)
    GPIO_SetBits(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
    GPIO_ResetBits(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
#endif

    // re-enable trigger
    mixerSchedulerEnableTrigger();

#if defined(SIMU)
    if (pwrCheck() == e_power_off) {
      TASK_RETURN();
    }
#else
    if (isForcePowerOffRequested()) {
      boardOff();
    }
#endif

    if (!s_pulses_paused) {
      uint16_t t0 = getTmr2MHz();

      DEBUG_TIMER_START(debugTimerMixer);
      RTOS_LOCK_MUTEX(mixerMutex);

      doMixerCalculations();
      sendSynchronousPulses((1 << INTERNAL_MODULE) | (1 << EXTERNAL_MODULE));
      doMixerPeriodicUpdates();

      DEBUG_TIMER_START(debugTimerMixerCalcToUsage);
      DEBUG_TIMER_SAMPLE(debugTimerMixerIterval);
      RTOS_UNLOCK_MUTEX(mixerMutex);
      DEBUG_TIMER_STOP(debugTimerMixer);

#if defined(STM32) && !defined(SIMU)
      if (getSelectedUsbMode() == USB_JOYSTICK_MODE) {
        usbJoystickUpdate();
      }
#endif

      if (heartbeat == HEART_WDT_CHECK) {
        WDG_RESET();
        heartbeat = 0;
      }

      t0 = getTmr2MHz() - t0;
      if (t0 > maxMixerDuration)
        maxMixerDuration = t0;
    }
  }
}


#define MENU_TASK_PERIOD_TICKS         (50 / RTOS_MS_PER_TICK)    // 50ms

#if defined(COLORLCD) && defined(CLI)
bool perMainEnabled = true;
#endif

TASK_FUNCTION(menusTask)
{
#if defined(LIBOPENUI)
  LvglWrapper::instance();
#endif

#if defined(SPLASH) && !defined(STARTUP_ANIMATION)
  if (!UNEXPECTED_SHUTDOWN()) {
    drawSplash();
    TRACE("drawSplash() completed");
  }
#endif

#if defined(HARDWARE_TOUCH) && !defined(PCBFLYSKY) && !defined(SIMU)
  touchPanelInit();
#endif

#if defined(IMU)
  gyroInit();
#endif
  
  opentxInit();

#if defined(PWR_BUTTON_PRESS)
  while (true) {
    uint32_t pwr_check = pwrCheck();
    if (pwr_check == e_power_off) {
      break;
    }
    else if (pwr_check == e_power_press) {
      RTOS_WAIT_TICKS(MENU_TASK_PERIOD_TICKS);
      continue;
    }
#else
  while (pwrCheck() != e_power_off) {
#endif
    uint32_t start = (uint32_t)RTOS_GET_TIME();
    DEBUG_TIMER_START(debugTimerPerMain);
#if defined(COLORLCD) && defined(CLI)
    if (perMainEnabled) {
      perMain();
    }
#else
    perMain();
#endif
    DEBUG_TIMER_STOP(debugTimerPerMain);
    // TODO remove completely massstorage from sky9x firmware
    uint32_t runtime = ((uint32_t)RTOS_GET_TIME() - start);
    // deduct the thread run-time from the wait, if run-time was more than
    // desired period, then skip the wait all together
    if (runtime < MENU_TASK_PERIOD_TICKS) {
      RTOS_WAIT_TICKS(MENU_TASK_PERIOD_TICKS - runtime);
    }

    resetForcePowerOffRequest();
  }

#if defined(PCBX9E)
  toplcdOff();
#endif

#if defined(PCBHORUS)
  ledOff();
#endif

  drawSleepBitmap();
  opentxClose();
  boardOff(); // Only turn power off if necessary

  TASK_RETURN();
}


#if defined(WITH_DOOM)
#define DOOM_STACK_SIZE     8192

RTOS_TASK_HANDLE doomTaskId;
RTOS_DEFINE_STACK(doomStack, DOOM_STACK_SIZE);

TASK_FUNCTION(doomTask)
{
  char* argv[] = {(char*)"doom", (char*)"-iwad", (char*)DOOM_PATH"/DOOM1.WAD", NULL};
  int argc = sizeof(argv) / sizeof(argv[0]) - 1;

  doom_main(argc, argv);
  boardOff();
}
#endif


void tasksStart()
{
  RTOS_CREATE_MUTEX(audioMutex);
  RTOS_CREATE_MUTEX(mixerMutex);

#if defined(CLI)
  cliStart();
#endif

  RTOS_CREATE_TASK(mixerTaskId, mixerTask, "mixer", mixerStack,
                   MIXER_STACK_SIZE, MIXER_TASK_PRIO);
#ifdef WITH_DOOM
  RTOS_CREATE_TASK(doomTaskId, doomTask, "doom", doomStack,
                   DOOM_STACK_SIZE, MIXER_TASK_PRIO);
#else
  RTOS_CREATE_TASK(menusTaskId, menusTask, "menus", menusStack,
                   MENUS_STACK_SIZE, MENUS_TASK_PRIO);

#if !defined(SIMU)
  RTOS_CREATE_TASK(audioTaskId, audioTask, "audio", audioStack,
                   AUDIO_STACK_SIZE, AUDIO_TASK_PRIO);
#endif
#endif // WITH_DOOM


  RTOS_START();
}
