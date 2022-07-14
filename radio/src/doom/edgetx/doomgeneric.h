#ifndef DOOM_GENERIC
#define DOOM_GENERIC

// #include <stdlib.h>
#include <stdint.h>
#include "debug.h"

#if defined(__cplusplus)
extern "C" {
#endif

void dg_Create();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int* pressed, unsigned char* key);
void button_update_loop();

#define DOOM_LOG debugPrintf

#if defined(__cplusplus)
}
#endif

#endif //DOOM_GENERIC
