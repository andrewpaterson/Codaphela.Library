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


  /* #################################
  // compile options
  // ################################# */

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


  // Defines the maximun sector size (in bytes) that this library should
  // support. An attempt to mount a volume with a sector size larger than this
  // value will fail.
#define MAX_SECTOR_LENGTH				0x200
  

// Defines that the fat_format_volume function should be included in the library.
// If you don't need format support you can save a few lines of code by commenting
// this out.
#define FAT_FORMAT_UTILITY


// this is the interval in sectors written at which an
// open file will be flushed 0x800 = 1 MiB with 512 bytes
// sectors
#define FAT_FLUSH_INTERVAL				0x800
#define FAT_OPTIMIZE_FOR_FLASH


// FAT file system types
#define FAT_FS_TYPE_UNSPECIFIED			0x0		// used for formatting
#define FAT_FS_TYPE_FAT12				0x1
#define FAT_FS_TYPE_FAT16				0x2
#define FAT_FS_TYPE_FAT32				0x3

// File Attributes
#define FAT_ATTR_READ_ONLY				0x1
#define FAT_ATTR_HIDDEN					0x2
#define FAT_ATTR_SYSTEM					0x4
#define FAT_ATTR_VOLUME_ID				0x8
#define FAT_ATTR_DIRECTORY				0x10
#define FAT_ATTR_ARCHIVE				0x20 
#define FAT_ATTR_LONG_NAME				(FAT_ATTR_READ_ONLY |	\
											FAT_ATTR_HIDDEN |	\
											FAT_ATTR_SYSTEM |	\
											FAT_ATTR_VOLUME_ID)

// file access flags
#define FAT_FILE_ACCESS_READ					0x1
#define FAT_FILE_ACCESS_WRITE					0x2
#define FAT_FILE_ACCESS_APPEND					(0x4)
#define FAT_FILE_ACCESS_OVERWRITE				(0x8)
#define FAT_FILE_ACCESS_CREATE					(0x10) 
#define FAT_FILE_ACCESS_CREATE_OR_OVERWRITE		(FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE)
#define FAT_FILE_ACCESS_CREATE_OR_APPEND		(FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_APPEND)
#define FAT_FILE_FLAG_NO_BUFFERING				(0x20)
#define FAT_FILE_FLAG_OPTIMIZE_FOR_FLASH		(0x40)


// seek modes
#define FAT_SEEK_START							0x1
#define FAT_SEEK_CURRENT						0x2
#define FAT_SEEK_END							0x3


// streaming IO responses
#define FAT_STREAMING_RESPONSE_STOP				STORAGE_MULTI_SECTOR_RESPONSE_STOP
#define FAT_STREAMING_RESPONSE_SKIP				STORAGE_MULTI_SECTOR_RESPONSE_SKIP
#define FAT_STREAMING_RESPONSE_READY			STORAGE_MULTI_SECTOR_RESPONSE_READY


// return codes (first 32 codes are reserved)
#define FAT_SUCCESS								( 0x0 )
#define FAT_UNKNOWN_ERROR						( 0x20 + 0x1 )
#define FAT_CANNOT_READ_MEDIA					( 0x20 + 0x2 )
#define FAT_CANNOT_WRITE_MEDIA					( 0x20 + 0x3 )
#define FAT_NOT_A_DIRECTORY						( 0x20 + 0x4 )
#define FAT_INVALID_FILENAME					( 0x20 + 0x5 )
#define FAT_FILENAME_ALREADY_EXISTS				( 0x20 + 0x6 )
#define FAT_INVALID_PATH						( 0x20 + 0x7 )
#define FAT_CORRUPTED_FILE						( 0x20 + 0x8 )
#define FAT_ILLEGAL_FILENAME					( 0x20 + 0x9 )
#define FAT_FILENAME_TOO_LONG					( 0x20 + 0xA )
#define FAT_NOT_A_FILE							( 0x20 + 0xB )
#define FAT_FILE_NOT_FOUND						( 0x20 + 0xC )
#define FAT_DIRECTORY_DOES_NOT_EXIST			( 0x20 + 0xD )
#define FAT_INSUFFICIENT_DISK_SPACE				( 0x20 + 0xE )
#define FAT_FEATURE_NOT_SUPPORTED				( 0x20 + 0xF )
#define FAT_OP_IN_PROGRESS						( 0x20 + 0x10 )
#define FAT_SECTOR_SIZE_NOT_SUPPORTED			( 0x20 + 0x11 )
#define FAT_LFN_GENERATED						( 0x20 + 0x12 )
#define FAT_SHORT_LFN_GENERATED					( 0x20 + 0x13 )
#define FAT_SEEK_FAILED							( 0x20 + 0x14 )
#define FAT_FILE_NOT_OPENED_FOR_WRITE_ACCESS	( 0x20 + 0x15 )
#define FAT_INVALID_HANDLE						( 0x20 + 0x16 )
#define FAT_INVALID_CLUSTER						( 0x20 + 0x17 )
#define FAT_INVALID_FAT_VOLUME					( 0x20 + 0x18 )
#define FAT_INVALID_VOLUME_LABEL				( 0x20 + 0x19 )
#define FAT_INVALID_FORMAT_PARAMETERS			( 0x20 + 0x1A )
#define FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED		( 0x20 + 0x1B )
#define FAT_DIRECTORY_LIMIT_EXCEEDED			( 0x20 + 0x1C )
#define FAT_INVALID_PARAMETERS					( 0x20 + 0x1D )
#define FAT_FILE_HANDLE_IN_USE					( 0x20 + 0x1E )
#define FAT_FILE_BUFFER_NOT_SET					( 0x20 + 0x1F )
#define FAT_MISALIGNED_IO						( 0x20 + 0x20 )
#define FAT_AWAITING_DATA						( 0x20 + 0x21 )
#define FAT_BUFFER_TOO_BIG						( 0x20 + 0x22 )


// these are the bits set on the reserved fields of a directory
// entry to indicate that a SFN entry is actually a LFN entry with
// a filename conforming to the 8.3 convention but with a lowercase
// base name or extension (or both). This library will display these
// entries correctly, but for compatibility reasons when creating
// entries an LFN entry will be created for this type of files.
#define FAT_LOWERCASE_EXTENSION			0x10
#define FAT_LOWERCASE_BASENAME			0x8


// misc
#define FAT_MAX_PATH					260
#define FAT_FIRST_LFN_ENTRY				0x40
#define FAT_MAX_FILENAME				255


//Function pointer to get the system time
typedef time_t(*FAT_GET_SYSTEM_TIME)(void);


 // This structure is the volume handle. All the fields in the structure are
 // reserved for internal use and should not be accessed directly by the
 // developer.
struct SFatVolume
{
	uint32			id;
	uint32			fat_size;
	uint32			root_cluster;
	uint32			first_data_sector;
	uint32			no_of_sectors;
	uint32			no_of_data_sectors;
	uint32			no_of_clusters;
	uint32			no_of_reserved_sectors;
	uint32			next_free_cluster;
	uint32			total_free_clusters;
	uint32			fsinfo_sector;
	uint16			root_directory_sectors;
	uint16			no_of_bytes_per_serctor;
	uint16			no_of_sectors_per_cluster;
	char			use_long_filenames;
	uint8			fs_type;
	uint8			no_of_fat_tables;
	char			label[12];
	CFileDrive*		device;
};


 //* This is the callback function for an asynchronous operation.
 //* <param name="context">The context pointer of the asynchronous operation.</param>
 //* <param name="result">A pointer to the result of the asynchronous operation.</param>
typedef void (*STORAGE_CALLBACK)(void* context, uint16* result);


 //* This structure holds the callback function pointer and
 //* the callback context and is passed by the file system driver
 //* as a parameter to the driver's asynchronous IO functions.
struct SStorageCallbackInfo
{
	 //* <summary>The callback function for an asynchronous IO function.</summary>
	STORAGE_CALLBACK	Callback;
	 //* <summary>The callback context for an asynchronous IO function.</summary>
	void*				Context;
};


// fat 32-byte directory entry structure
#pragma pack(push, 1)
struct SFatRawDirectoryEntry
{
	union
	{
		struct SFatRawCommon
		{
			uint8 name[11];
			uint8 attributes;
			uint8 reserved;
			uint8 create_time_tenth;
			uint16 create_time;
			uint16 create_date;
			uint16 access_date;
			uint16 first_cluster_hi;
			uint16 modify_time;
			uint16 modify_date;
			uint16 first_cluster_lo;
			uint32 size;
		} sFatRawCommon;
		struct LFN
		{
			uint8 lfn_sequence;
			uint8 lfn_chars_1[10];
			uint8 lfn_attributes;
			uint8 lfn_type;
			uint8 lfn_checksum;
			uint8 lfn_chars_2[12];
			uint16 lfn_first_cluster;
			uint8 lfn_chars_3[4];
		} LFN;
	} ENTRY;
};
#pragma pack(pop)


/*!
 * <summary>
 * Stores information about directory entries.
 * </summary>
*/
struct SFatDirectoryEntry
{
	/*!
	 * <summary>The name of the file.</summary>
	 */
	uint8 name[FAT_MAX_FILENAME + 1];
	/*!
	 * <summary>The list of file attributes ORed together.</summary>
	*/
	uint8 attributes;
	/*!
	 * <summary>The creation timestamp of the file.</summary>
	*/
	time_t create_time;
	/*!
	 * <summary>The modification timestamp of the file.</summary>
	 */
	time_t modify_time;
	/*!
	 * <summary>The access timestamp of the file.</summary>
	*/
	time_t access_time;
	/*!
	 * <summary>The size of the file.</summary>
	*/
	uint32 size;
	/*!
	 * <summary>Reserved for internal use.</summary>
	*/
	uint32 sector_addr;
	/*!
	 * <summary>Reserved for internal use.</summary>
	*/
	uint16 sector_offset;
	/*!
	 * <summary>Reserved for internal use.</summary>
	*/
	SFatRawDirectoryEntry raw;
};


/*
// Holds the internal state of a directory query.
*/
struct SFatQueryState
{
	uint8						Attributes;
	uint16						current_sector;
	uint32						current_cluster;
	SFatRawDirectoryEntry*		current_entry_raw;
	uint8*						buffer;

	SFatRawDirectoryEntry*		first_entry_raw;
	/*
	// LFN support members
	*/
	uint16						long_filename[256];
	uint8						lfn_sequence;
	uint8						lfn_checksum;
	/*
	// buffer (MUST ALWAYS BE LAST!!!)
	*/
	uint8						buff[MAX_SECTOR_LENGTH];
};


/*!
 * <summary>Callback function for asynchronous IO.</summary>
*/
typedef void FAT_ASYNC_CALLBACK(void* context, uint16* state);


/*!
 * <summary>Callback function for asynchronous STREAMING IO.</summary>
 */
typedef void FAT_STREAM_CALLBACK(void* context, uint16* state, uint8** buffer, uint16* response);

/*
 * holds the state of a read or write operation
 */
typedef struct FAT_OP_STATE
{
	uint32					pos;
	uint16					bytes_remaining;
	uint32					sector_addr;
	uint16*					async_state;
	uint32*					bytes_read;
	uint16					length;
	uint16					storage_state;
	uint8*					end_of_buffer;
	uint8*					buffer;
	uint8					internal_state;

	SStorageCallbackInfo	storage_callback_info;
	FAT_ASYNC_CALLBACK*		callback;
	void*					callback_context;
}
FAT_OP_STATE;

/*!
 * <summary>
 * This is the file handle structure. All the fields in this structure
 * are reserved for internal use and should not be accessed directly by the
 * developer.
 * </summary>
 */
typedef struct FAT_FILE
{
	SFatVolume*				volume;
	SFatDirectoryEntry		directory_entry;
	uint32					current_size;
	uint32					current_clus_addr;
	uint32					current_clus_idx;
	uint32					current_sector_idx;
	uint32					no_of_clusters_after_pos;
	uint16					no_of_sequential_clusters;
	uint8*					buffer_head;
	char					buffer_dirty;
	char					busy;
	uint8					magic;
	uint8					access_flags;
	FAT_OP_STATE			op_state;
	uint8*					buffer;
	uint8					buffer_internal[MAX_SECTOR_LENGTH];
} FAT_FILE;

/*!
 * <summary>Holds the state of a directory query.</summary>
*/
typedef struct FILESYSTEM_QUERY
{
	SFatDirectoryEntry current_entry;
	SFatQueryState state;
}
FAT_FILESYSTEM_QUERY;

/*
// declare shared buffer
*/
extern uint8 fat_shared_buffer[MAX_SECTOR_LENGTH];
extern uint32 fat_shared_buffer_sector;


#if !defined(FAT_USE_SYSTEM_TIME)
/*!
// <summary>Registers the function that gets the system time.</summary>
*/
void fat_register_system_time_function(FAT_GET_SYSTEM_TIME system_time);
#endif


/*!
 * <summary>Initializes the FAT library.</summary>
 */
void fat_init(void);


/*!
 * <summary>
 * Mounts a FAT volume.
 * </summary>
 * <param name="volume">A pointer to a volume handle structure (SFatVolume).</param>
 * <param name="device">A pointer to the storage device driver STORAGE_DEVICE structure.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
 */
uint16 fat_mount_volume(SFatVolume* volume, CFileDrive* device);


/**
 * <summary>
 * Dismounts a FAT volume.
 * </summary>
 * <param name="volume">The handle of the volume to dismount.</param>
 */
uint16 fat_unmount_volume(SFatVolume* volume);


/**
 * <summary>Gets the sector size of a volume in bytes.</summary>
 * <param name="volume">A pointer to the volume handle.</param>
 */
uint16 fat_get_sector_size(SFatVolume* volume);


/**
 * <summary>
 * Gets the directory entry of a file. This function should be used
 * to get information about a file such as file size, timestamps, and
 * attributes.
 * </summary>
 * <param name="volume">The handle of the volume containing the file.</param>
 * <param name="path">The path of the file within the volume.</param>
 * <param name="entry">A pointer to a SFatDirectoryEntry structure where the
 * details about the file will be stored.
 * </param>
 * <returns>
 * If successful it will return FAT_SUCCESS, otherwise it will return one of the
 * result codes defined in fat.h
 * </result>
 */
uint16 fat_get_file_entry(SFatVolume* volume, char* path, SFatDirectoryEntry* entry);


/**
 * <summary>
 * Finds the first entry in a directory.
 * </summary>
 * <param name="volume">A pointer to the volume handle (SFatVolume).</param>
 * <param name="path">The path of the directory to query.</param>
 * <param name="attributes">An ORed list of file attributes to filter the query.</param>
 * <param name="dir_entry">
 * A pointer-to-pointer to a SFatDirectoryEntry structure.
 * When this function returns the pointer will be set to to point to the directory entry.
 * </param>
 * <param name="query">A pointer to a FAT_FILESYSTEM_QUERY that will be initialized as the query handle.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_find_first_entry(SFatVolume* volume, char* path, uint8 attributes, SFatDirectoryEntry** dir_entry, FAT_FILESYSTEM_QUERY* query);


/**
 * <summary>
 * Finds the next entry in a directory.
 * </summary>
 * <param name="volume">A pointer to the volume handle (SFatVolume).</param>
 * <param name="dir_entry">
 * A pointer-to-pointer to a SFatDirectoryEntry structure.
 * When this function returns the pointer will be set to point the the directory entry.
 * </param>
 * <param name="query">The file system query object.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_find_next_entry(SFatVolume* volume, SFatDirectoryEntry** dir_entry, FAT_FILESYSTEM_QUERY* query);


/**
 * <summary>
 * Creates a new directory on the volume.
 * </summary>
 * <param name="volume">A pointer to the volume handle (SFatVolume).</param>
 * <param name="filename">The full path to the new directory.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_create_directory(SFatVolume* volume, char* filename);


/**
 * <summary>
 * Deletes a file.
 * </summary>
 * <param name="volume">A pointer to the volume handle (SFatVolume structure).</param>
 * <param name="filename">The full path and filename of the file to delete.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_delete(SFatVolume* volume, char* filename);


/**
 * <summary>
 * Renames a file.
 * </summary>
 * <param name="volume">A pointer to the volume handle (SFatVolume structure).</param>
 * <param name="original_filename">The full path and original filename of the file to be renamed.</param>
 * <param name="new_filename">The full path and new filename for the file.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_rename(SFatVolume* volume, char* original_filename, char* new_filename);


/**
 * <summary>
 * Opens or create a file.
 * </summary>
 * <param name="volume">A pointer to the volume handle (SFatVolume structure).</param>
 * <param name="filename">The full path and filename of the file to open.</param>
 * <param name="access_flags">An ORed list of one or more of the access flags defined in fat.h</param>
 * <param name="file">A pointer to a file handle FAT_FILE structure.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_open(SFatVolume* volume, char* filename, uint8 access_flags, FAT_FILE* file);


/**
 * <summary>
 * Sets an external buffer for this file handle.
 * </summary>
*/
uint16 fat_file_set_buffer(FAT_FILE* file, uint8* buffer);


/**
 * <summary>
 * Gets the unique identifier of the file.
 * </summary>
 * <param name="file">An open file handle.</param>
 */
uint32 fat_file_get_unique_id(FAT_FILE* file);


/**
 * <summary>
 * Allocates disk space to an open file.
 * </summary>
 * <param name="file">A pointer to a file handle FAT_FILE structure.</param>
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
	FAT_FILE* file,
	uint32 bytes
);


/**
 * <summary>
 * Moves the file cursor to a new position within the file.
 * </summary>
 * <param name="file">A pointer to the file handle FAT_FILE structure.</param>
 * <param name="offset">The offset of the new cursor position relative to the position specified by the mode param.</param>
 * <param name="mode">One of the supported seek modes: FAT_SEEK_START, FAT_SEEK_CURRENT, or FAT_SEEK_END.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
 * <remarks>
 * If FAT_SEEK_END is specified the offset must be 0 or the function will fail.
 * </remarks>
*/
uint16 fat_file_seek
(
	FAT_FILE* file,
	uint32 offset,
	char mode
);


/**
 * <summary>
 * Writes the specified number of bytes to the current position on an opened file.
 * </summary>
 * <param name="file">A pointer to a file handle FAT_FILE structure.</param>
 * <param name="buffer">A buffer containing the bytes to be written.</param>
 * <param name="length">The amount of bytes to write.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_write
(
	FAT_FILE* file,
	uint8* buffer,
	uint32 length
);


/**
 * <summary>
 * Reads the specified number of bytes from the current position on an opened file.
 * </summary>
 * <param name="handle">A pointer to a file handle FAT_FILE structure.</param>
 * <param name="buffer">A buffer where the bytes will be copied to.</param>
 * <param name="length">The amount of bytes to read.</param>
 * <param name="bytes_read">A pointer to a 32 bit integer where the amount of bytes read will be written to.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_read
(
	FAT_FILE* handle,
	uint8* buffer,
	uint32 length,
	uint32* bytes_read
);


/**
 * <summary>
 * Flushes file buffers and updates directory entry.
 * </summary>
 * <param name="file">A pointer to the file handle (FAT_FILE) structure.</param>
*/
uint16 fat_file_flush
(
	FAT_FILE* file
);

/**
 * <summary>
 * Closes an open file.
 * </summary>
 * <param name="handle">A pointer to the file handle FAT_FILE structure.</param>
 * <returns>One of the return codes defined in fat.h.</returns>
*/
uint16 fat_file_close
(
	FAT_FILE* handle
);

#endif // __FAT_32_H__

