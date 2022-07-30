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
//	Created by a sound utility.
//	Kept as a sample, DOOM2 sounds.
//


#include <stdlib.h>


#include "doomtype.h"
#include "sounds.h"

//
// Information about all the music
//

#define MUSIC(name) \
    { name, 0, NULL, NULL }

musicinfo_t S_music[] =
{
    MUSIC(NULL),
};


//
// Information about all the sfx
//

#define SOUND(name, priority) \
  { NULL, name, priority, NULL, -1, -1, 0, 0, -1, NULL }
#define SOUND_LINK(name, priority, link_id, pitch, volume) \
  { NULL, name, priority, &S_sfx[link_id], pitch, volume, 0, 0, -1, NULL }

sfxinfo_t S_sfx[] =
{
  // S_sfx[0] needs to be a dummy for odd reasons.
  SOUND("none",   0),
};

