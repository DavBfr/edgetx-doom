#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

extern uint16_t* LCDFrameBuffer;

uint16_t display_get_width(void);

uint16_t display_get_height(void);

void DG_Init();
void DG_StartFrame();
void DG_EndFrame();
void DG_ShowError(const char * message);

#if defined(__cplusplus)
}
#endif

#endif
