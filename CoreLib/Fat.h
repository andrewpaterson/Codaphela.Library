/*
 * fat32lib - Portable FAT12/16/32 Filesystem Library
 * Copyright (C) 2013 Fernando Rodriguez (frodriguez.developer@outlook.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __FAT_32_H__
#define __FAT_32_H__

 /*! \file fat.h
  * \brief This is the header file for the FAT driver. It allows low level
  * access to the file system. The functions defined in smlib\sm.h allow
  * access to all the functionality provided by this header, most users
  * should use those instead. The only all users need to call on this
  * module is fat_get_filesystem_interface which is defined in filesystem_interface.h
  * and allows smlib (the volume manager) to interface with fat32lib (the
  * file system driver.)
  * <seealso cref="smlib\sm.h">smlib\sm.h</seealso>
  */

#include <time.h>
#include "BaseLib/PrimitiveTypes.h"
#include "FileDrive.h"
#include "FatCommon.h"
#include "FatVolume.h"
#include "FatFile.h"
#include "FatStructure.h"


  // #################################
  // compile options
  // #################################

  // if this option is not specified the library will only maintain 1 copy of
  // the FAT table, otherwise it will maintain all the tables in the volume
  // (usually two)

  /* #define FAT_MAINTAIN_TWO_FAT_TABLES */

  // If this option is defined the library will use the C time() function
  // to obtain the date/time used for timestamps, otherwise the application needs to
  // provide a function to get the time and register it with the library using the
  // fat_register_system_time_function function. If no such function is registered
  // the library will use 8/2/1985 @ 12:05 PM for all timestamps.
#define FAT_USE_SYSTEM_TIME


// this is the interval in sectors written at which an
// open file will be flushed 0x800 = 1 MiB with 512 bytes
// sectors
#define FAT_FLUSH_INTERVAL				0x800


//Function pointer to get the system time
typedef time_t(*FAT_GET_SYSTEM_TIME)(void);


#if !defined(FAT_USE_SYSTEM_TIME)
// Registers the function that gets the system time.
void fat_register_system_time_function(FAT_GET_SYSTEM_TIME system_time);
#endif


/**
 * <summary>
 * Allocates disk space to an open file.
 * </summary>
 * <param name="file">A pointer to a file handle SFatFile structure.</param>
 * <param name="bytes">The amount of disk space (in bytes) to be allocated.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
 * <remarks>
 * When writting large files in small chunks calling this function to pre-allocate
 * drive space significantly improves write speed. Any space that is not used will be freed
 * when the file is closed.
 *
 * This function will allocate enough disk space to write the requested amount of
 * bytes after the current poisition of the file. So if you request 5K bytes and there's
 * already 2K bytes allocated after the cursor position it will only allocate 3K bytes.
 * All disk allocations are done in multiples of the cluster size.
 * </remarks>
*/
uint16 FatFileAllocate(SFatFile*	file, uint32		bytes);


uint16 fat_format_volume(uint8 fs_type, char* const volume_label, uint32 uiNoOfSectorsPerCluster, CFileDrive* device);


#endif // __FAT_32_H__

