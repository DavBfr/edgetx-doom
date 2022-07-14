#include "doomgeneric.h"

#include "display.h"
#include "model_init.h"
#include "opentx.h"
#include "stdio.h"

#include "board.h"
#include "doomkeys.h"
#include "doomtype.h"

#ifdef SIMU
#include <sys/time.h>
#include <unistd.h>
#endif

#ifdef SIMU
const uint8_t keyboardMap[] = {KEY_ESCAPE,   KEY_USE,       KEY_ENTER,
                               KEY_UPARROW,  KEY_DOWNARROW, KEY_RIGHTARROW,
                               KEY_LEFTARROW};
#else
const uint8_t keyboardMap[] = {KEY_DOWNARROW,  KEY_UPARROW, KEY_ENTER,
                               KEY_RIGHTARROW, KEY_ESCAPE,  KEY_USE,
                               KEY_LEFTARROW};
#endif

rotenc_t oldRotencValue;

void dg_Create() {
  pwrOn();
#if defined(SPORT_UPDATE_PWR_GPIO)
  SPORT_UPDATE_POWER_INIT();
#endif
  DG_Init();
  if (!sdMounted())
    sdInit();
  generalDefault();
  g_eeGeneral.inactivityTimer = 0;
  setModelDefaults();
  logsInit();
#if defined(DEBUG) && defined(STM32) && !defined(SIMU)
  initSerialPorts();
  if (usbPlugged()) {
    setSelectedUsbMode(USB_SERIAL_MODE);
    serialInit(SP_VCP, serialGetMode(SP_VCP));
    usbStart();
  }
#endif
  // loadRadioSettings();
  resetBacklightTimeout();
  WDG_ENABLE(WDG_DURATION);
  startPulses();
  oldRotencValue = rotencValue;
}

int AD_RV = 0;
int AD_RH = 0;

#define deadzone (32767 * 0.065)

int map_values(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void button_update_loop() {
  // getADC();
  // evalInputs(e_perout_mode_notrainer);
  // AD_RV = map_values(calibratedAnalogs[2], -RESX, RESX, -32767, 32767);
  // AD_RH = map_values(calibratedAnalogs[3], -RESX, RESX, 32767, -32767);

  // if (AD_RV < deadzone && AD_RV > -deadzone) {
  //   AD_RV = 0;
  // }

  // if (AD_RH < deadzone && AD_RH > -deadzone) {
  //   AD_RH = 0;
  // }

  if (pwrPressed()) {
    boardOff();
  }
}

void DG_SleepMs(uint32_t ms) {
#ifndef SIMU
  delay_ms(ms);
#else
  usleep(ms * 1000);
#endif
}

uint32_t DG_GetTicksMs() {
  return RTOS_GET_MS();
}

int DG_GetKey(int* pressed, unsigned char* key) {
  extern boolean menuactive;
  static uint32_t oldKeys = 0;
  auto keys = readKeys();

  for (auto i = 0; i < sizeof(keyboardMap); i++) {
    uint32_t k = 1 << i;
    if ((keys & k) && !(oldKeys & k)) {
      *key = keyboardMap[i];
      if (!menuactive && *key == KEY_ENTER) {
        *key = KEY_FIRE;
      }
      *pressed = 1;
      oldKeys |= k;
      return 1;
    }
    if (!(keys & k) && (oldKeys & k)) {
      *key = keyboardMap[i];
      if (!menuactive && *key == KEY_ENTER) {
        *key = KEY_FIRE;
      }
      *pressed = 0;
      oldKeys = oldKeys & (~k);

      return 1;
    }
  }

  return 0;
}
