// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

//static const char
//rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include "config.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_video.h"
#include "z_zone.h"

#include "tables.h"
#include "doomkeys.h"

#include "../../Display/display.h"

#include "doomgeneric.h"

#include <stdbool.h>
#include <stdlib.h>

#include <fcntl.h>

#include <stdarg.h>

#include <sys/types.h>

//#define CMAP256

struct FB_BitField
{
	uint32_t offset;			/* beginning of bitfield	*/
	uint32_t length;			/* length of bitfield		*/
};

struct FB_ScreenInfo
{
	uint32_t xres;			/* visible resolution		*/
	uint32_t yres;
	uint32_t xres_virtual;		/* virtual resolution		*/
	uint32_t yres_virtual;

	uint32_t bits_per_pixel;		/* guess what			*/
	
							/* >1 = FOURCC			*/
	struct FB_BitField red;		/* bitfield in s_Fb mem if true color, */
	struct FB_BitField green;	/* else only length is significant */
	struct FB_BitField blue;
	struct FB_BitField transp;	/* transparency			*/
};

static struct FB_ScreenInfo s_Fb;
int fb_scaling = 1;
int usemouse = 0;

struct color {
    uint32_t b:8;
    uint32_t g:8;
    uint32_t r:8;
    uint32_t a:8;
};

static boolean should_rescale = false;

void I_GetEvent(void);

// The screen buffer; this is modified to draw things to the screen

byte *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = 2.0;
int mouse_threshold = 10;

// Gamma correction level to use

int usegamma = 0;

typedef struct
{
	byte r;
	byte g;
	byte b;
} col_t;

// Palette converted to RGB565

static uint16_t rgb565_palette[256];

void I_InitGraphics (void)
{
    int i;

	memset(&s_Fb, 0, sizeof(struct FB_ScreenInfo));
	s_Fb.xres = display_get_width();
	s_Fb.yres = display_get_height();
	s_Fb.xres_virtual = s_Fb.xres;
	s_Fb.yres_virtual = s_Fb.yres;
	s_Fb.bits_per_pixel = 16;

	s_Fb.blue.length = 5;
	s_Fb.green.length = 6;
	s_Fb.red.length = 5;
	s_Fb.transp.length = 0;

	s_Fb.blue.offset = 0;
	s_Fb.green.offset = 5;
	s_Fb.red.offset = 11;
	s_Fb.transp.offset = 16;

	should_rescale = !((s_Fb.xres == SCREENWIDTH) && (s_Fb.yres == SCREENHEIGHT));

    printf("I_InitGraphics: framebuffer: x_res: %lu, y_res: %lu, x_virtual: %lu, y_virtual: %lu, bpp: %lu\n",
            s_Fb.xres, s_Fb.yres, s_Fb.xres_virtual, s_Fb.yres_virtual, s_Fb.bits_per_pixel);

    printf("I_InitGraphics: framebuffer: RGBA: %lu%lu%lu%lu, red_off: %lu, green_off: %lu, blue_off: %lu, transp_off: %lu\n",
            s_Fb.red.length, s_Fb.green.length, s_Fb.blue.length, s_Fb.transp.length, s_Fb.red.offset, s_Fb.green.offset, s_Fb.blue.offset, s_Fb.transp.offset);

    printf("I_InitGraphics: DOOM screen size: w x h: %d x %d\n", SCREENWIDTH, SCREENHEIGHT);


    i = M_CheckParmWithArgs("-scaling", 1);
    if (i > 0) {
        i = atoi(myargv[i + 1]);
        fb_scaling = i;
        printf("I_InitGraphics: Scaling factor: %d\n", fb_scaling);
    } else {
        fb_scaling = s_Fb.xres / SCREENWIDTH;
        if (s_Fb.yres / SCREENHEIGHT < fb_scaling)
            fb_scaling = s_Fb.yres / SCREENHEIGHT;
        printf("I_InitGraphics: Auto-scaling factor: %d\n", fb_scaling);
    }


    /* Allocate screen to draw to */
	I_VideoBuffer = (byte*)Z_Malloc (SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);  // For DOOM to draw on

	screenvisible = true;

    extern int I_InitInput(void);
    I_InitInput();
}

void I_ShutdownGraphics (void)
{
	Z_Free (I_VideoBuffer);
}

void I_StartFrame (void)
{

}

void I_StartTic (void)
{
	I_GetEvent();
}

void I_UpdateNoBlit (void)
{
}

static inline void I_CopyFrameBufferRGB565(void)
{
	const int screen_height = display_get_height();
	const int screen_width  = display_get_width();
	const int screen_pixels = screen_height * screen_width;

	for(int y = 0; y < SCREENHEIGHT; y++)
	{
		for(int x = 0; x < SCREENWIDTH; x++)
		{
			int offset = SCREENWIDTH * y + x;
			LCDFrameBuffer[screen_pixels-offset]
						   = rgb565_palette[I_VideoBuffer[offset]];
		}
	}
}

static inline void I_ScaleFBNearestNeighbourRGB565(void)
{
	const int screen_height = display_get_height();
	const int screen_width  = display_get_width();
	const int screen_pixels = screen_height * screen_width;

	const int x_ratio = ((SCREENWIDTH << 16) / screen_width)+1;
	const int y_ratio = ((SCREENHEIGHT << 16) / screen_height)+1;
	for (int y = 0; y < screen_height; y++)
	{
		for (int x = 0; x < screen_width; x++)
		{
			const int xn = ((x*x_ratio)>>16) ;
            const int yn = ((y*y_ratio)>>16) ;
			LCDFrameBuffer[screen_pixels-(screen_width * y + x)]
						   = rgb565_palette[I_VideoBuffer[(SCREENWIDTH*yn)+xn]];
		}
	}
}

//
// I_FinishUpdate
//

void I_FinishUpdate (void)
{
	if(should_rescale)
	{
		I_ScaleFBNearestNeighbourRGB565();
	}
	else
	{
		I_CopyFrameBufferRGB565();
	}

	DG_DrawFrame();
}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
#define GFX_RGB565(r, g, b)			((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))
#define GFX_RGB565_R(color)			((0xF800 & color) >> 11)
#define GFX_RGB565_G(color)			((0x07E0 & color) >> 5)
#define GFX_RGB565_B(color)			(0x001F & color)

void I_SetPalette (byte* palette)
{
	for (int i = 0; i < 256; i++)
	{
		col_t* c = (col_t*)palette;

		rgb565_palette[i] = GFX_RGB565(gammatable[usegamma][c->r],
									   gammatable[usegamma][c->g],
									   gammatable[usegamma][c->b]);

		palette += 3;
	}

//	for (int i = 0; i < 256; ++i )
//	{
//		colors[i].a = 0;
//		colors[i].r = gammatable[usegamma][*palette++];
//		colors[i].g = gammatable[usegamma][*palette++];
//		colors[i].b = gammatable[usegamma][*palette++];
//	}
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex (int r, int g, int b)
{
    int best, best_diff, diff;
    int i;
    col_t color;

    printf("I_GetPaletteIndex\n");

    best = 0;
    best_diff = INT_MAX;

    for (i = 0; i < 256; ++i)
    {
    	color.r = GFX_RGB565_R(rgb565_palette[i]);
    	color.g = GFX_RGB565_G(rgb565_palette[i]);
    	color.b = GFX_RGB565_B(rgb565_palette[i]);

        diff = (r - color.r) * (r - color.r)
             + (g - color.g) * (g - color.g)
             + (b - color.b) * (b - color.b);

        if (diff < best_diff)
        {
            best = i;
            best_diff = diff;
        }

        if (diff == 0)
        {
            break;
        }
    }

    return best;
}

void I_BeginRead (void)
{
}

void I_EndRead (void)
{
}

void I_SetWindowTitle (char *title)
{
	DG_SetWindowTitle(title);
}

void I_GraphicsCheckCommandLine (void)
{
}

void I_SetGrabMouseCallback (grabmouse_callback_t func)
{
}

void I_EnableLoadingDisk(void)
{
}

void I_BindVideoVariables (void)
{
}

void I_DisplayFPSDots (boolean dots_on)
{
}

void I_CheckIsScreensaver (void)
{
}
