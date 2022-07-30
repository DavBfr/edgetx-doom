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
//	WAD I/O functions.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#include "doomtype.h"
#include "m_argv.h"

#include "w_file.h"

#include "../../umm_malloc/umm_malloc.h"
#include "fatfs.h"

/*
#ifdef _WIN32
extern wad_file_class_t win32_wad_file;
#endif
*/

#ifdef HAVE_MMAP
extern wad_file_class_t posix_wad_file;
#endif 

wad_file_t wad_file;


wad_file_t *W_OpenFile(char *path)
{
	FIL handle;
	FRESULT result;
	wad_file_t* file;
	FILINFO file_info;

	result = f_open(&handle, path, FA_READ);

	if(result != FR_OK)
	{
		return NULL;
	}

	result = f_stat(path, &file_info);

	if(result != FR_OK)
	{
		return NULL;
	}

	file = calloc(1, sizeof(wad_file_t));

	file->file_handle = handle;
	file->length = file_info.fsize;

    return file;
}

void W_CloseFile(wad_file_t *wad)
{
	f_close(&wad->file_handle);

	free(wad);
}

size_t W_Read(wad_file_t *wad, unsigned int offset,
              void *buffer, size_t buffer_len)
{
	FRESULT result;
	UINT read_bytes;

	result = f_lseek(&wad->file_handle, offset);

	if(result != FR_OK)
	{
		return 0;
	}

	result = f_read(&wad->file_handle, buffer, buffer_len, &read_bytes);

	if(result != FR_OK)
	{
		return 0;
	}

    return read_bytes;
}
