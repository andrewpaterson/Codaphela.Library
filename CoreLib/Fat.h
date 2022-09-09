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


//  Initializes the FAT library.</summary>
void fat_init(void);


/**
 * <summary>
 * Deletes a file.
 * </summary>
 * <param name="volume">A pointer to the volume handle (CFatVolume structure).</param>
 * <param name="filename">The full path and filename of the file to delete.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_delete(CFatVolume* volume, char* filename);


/**
 * <summary>
 * Renames a file.
 * </summary>
 * <param name="volume">A pointer to the volume handle (CFatVolume structure).</param>
 * <param name="original_filename">The full path and original filename of the file to be renamed.</param>
 * <param name="new_filename">The full path and new filename for the file.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_rename(CFatVolume* volume, char* original_filename, char* new_filename);


/**
 * <summary>
 * Opens or create a file.
 * </summary>
 * <param name="volume">A pointer to the volume handle (CFatVolume structure).</param>
 * <param name="filename">The full path and filename of the file to open.</param>
 * <param name="access_flags">An ORed list of one or more of the access flags defined in fat.h</param>
 * <param name="file">A pointer to a file handle SFatFile structure.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_open(CFatVolume* volume, char* filename, uint8 access_flags, SFatFile* file);
uint16 fat_open_file_by_entry(CFatVolume* volume, SFatDirectoryEntry* entry, SFatFile* handle, uint8 access_flags);

/**
 * <summary>
 * Sets an external buffer for this file handle.
 * </summary>
*/
uint16 fat_file_set_buffer(SFatFile* file, uint8* buffer);


/**
 * <summary>
 * Gets the unique identifier of the file.
 * </summary>
 * <param name="file">An open file handle.</param>
 */
uint32 fat_file_get_unique_id(SFatFile* file);


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
uint16 fat_file_alloc
(
	SFatFile* file,
	uint32 bytes
);


/**
 * <summary>
 * Moves the file cursor to a new position within the file.
 * </summary>
 * <param name="file">A pointer to the file handle SFatFile structure.</param>
 * <param name="offset">The offset of the new cursor position relative to the position specified by the mode param.</param>
 * <param name="mode">One of the supported seek modes: FAT_SEEK_START, FAT_SEEK_CURRENT, or FAT_SEEK_END.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
 * <remarks>
 * If FAT_SEEK_END is specified the offset must be 0 or the function will fail.
 * </remarks>
*/
uint16 fat_file_seek
(
	SFatFile* file,
	uint32 offset,
	char mode
);


/**
 * <summary>
 * Writes the specified number of bytes to the current position on an opened file.
 * </summary>
 * <param name="file">A pointer to a file handle SFatFile structure.</param>
 * <param name="buffer">A buffer containing the bytes to be written.</param>
 * <param name="length">The amount of bytes to write.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_write
(
	SFatFile* file,
	uint8* buffer,
	uint32 length
);


/**
 * <summary>
 * Reads the specified number of bytes from the current position on an opened file.
 * </summary>
 * <param name="handle">A pointer to a file handle SFatFile structure.</param>
 * <param name="buffer">A buffer where the bytes will be copied to.</param>
 * <param name="length">The amount of bytes to read.</param>
 * <param name="bytes_read">A pointer to a 32 bit integer where the amount of bytes read will be written to.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_read
(
	SFatFile* handle,
	uint8* buffer,
	uint32 length,
	uint32* bytes_read
);


/**
 * <summary>
 * Flushes file buffers and updates directory entry.
 * </summary>
 * <param name="file">A pointer to the file handle (SFatFile) structure.</param>
*/
uint16 fat_file_flush
(
	SFatFile* file
);


uint16 fat_format_volume(uint8 fs_type, char* const volume_label, uint32 uiNoOfSectorsPerCluster, CFileDrive* device);


/**
 * <summary>
 * Closes an open file.
 * </summary>
 * <param name="handle">A pointer to the file handle SFatFile structure.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_close
(
	SFatFile* handle
);


#endif // __FAT_32_H__

