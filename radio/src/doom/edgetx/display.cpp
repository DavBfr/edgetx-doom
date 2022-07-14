
#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include "doomgeneric.h"
#include "lcd.h"
#include "opentx.h"

uint16_t* LCDFrameBuffer;

uint16_t display_get_width(void) {
  return LCD_W;
}

uint16_t display_get_height(void) {
  return LCD_H;
}

void DG_Init() {
  lcdInitDisplayDriver();
  BACKLIGHT_ENABLE();
}

void DG_StartFrame() {
  lcdInitDirectDrawing();
  LCDFrameBuffer = lcd->getData();
}

void DG_EndFrame() {
  lcdRefresh();
}

void DG_ShowError(const char* message) {
  LED_ERROR_BEGIN();
  auto n = 12;

  while (--n > 0) {
    lcdInitDirectDrawing();
    lcd->clear(COLOR_THEME_PRIMARY2);
    lcd->drawText(LCD_W / 2, 10, "DOOM", FONT(XL) | CENTERED);
    lcd->drawText(LCD_W / 2, LCD_H / 2, message, CENTERED);
    lcd->drawNumber(10, LCD_H - 30, n, 0, 0, "Power OFF in ");
    lcdRefresh();
    WDG_RESET();
    DG_SleepMs(1000);
  }

#ifndef SIMU
  boardOff();
#else
  exit(1);
#endif
}
