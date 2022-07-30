//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
// DESCRIPTION:
//	Main program, simply calls D_DoomMain high level loop.
//

//#include "config.h"

#include <stdio.h>

//#include "doomtype.h"
//#include "i_system.h"
#include "m_argv.h"

//
// D_DoomMain()
// Not a globally visible function, just included for source reference,
// calls all startup code, parses command line options.
//

void D_DoomMain (void);

void M_FindResponseFile(void);

void dg_Create();

static void init_lookup_tables(void)
{
	extern void info_init(void);
	extern void R_DrawSegsInit(void);
	extern void R_MainBufferInit(void);
	extern void R_PlaneInit(void);
	extern void tables_init(void);

	info_init();
	R_DrawSegsInit();
	R_MainBufferInit();
	R_PlaneInit();
	tables_init();
}

int doom_main(int argc, char **argv)
{
	init_lookup_tables();

    // save arguments

    myargc = argc;
    myargv = argv;

    M_FindResponseFile();

    // start doom
    printf("Starting D_DoomMain\r\n");
    
	dg_Create();

	D_DoomMain();

    return 0;
}

