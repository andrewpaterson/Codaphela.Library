#ifndef __FAT_COMMON_H__
#define __FAT_COMMON_H__
#include <time.h>
#include "FatStructure.h"


// Defines the maximun sector size (in bytes) that this library should
// support. An attempt to mount a volume with a sector size larger than this
// value will fail.
#define MAX_SECTOR_LENGTH				0x200


#define INT16_BYTE0		0
#define INT16_BYTE1		1
#define INT32_BYTE0		0
#define INT32_BYTE1		1
#define INT32_BYTE2		2
#define INT32_BYTE3		3
#define INT32_WORD0		0
#define INT32_WORD1		1


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



// Holds the internal state of a directory query.
struct SFatQueryState
{
	uint8						Attributes;
	uint16						current_sector;
	uint32						current_cluster;
	SFatRawDirectoryEntry*		current_entry_raw;
	uint8*						uiBuffer;

	SFatRawDirectoryEntry*		first_entry_raw;

	// LFN support members
	uint16						long_filename[256];
	uint8						lfn_sequence;
	uint8						lfn_checksum;

	// buffer (MUST ALWAYS BE LAST!!!)
	uint8						buff[MAX_SECTOR_LENGTH];
};


// holds the state of a read or write operation
struct SFatOperationState
{
	uint32					pos;
	uint16					bytes_remaining;
	uint32					sector_addr;
	uint32*					bytes_read;
	uint16					length;
	uint16					storage_state;
	uint8*					end_of_buffer;
	uint8*					uiBuffer;
	uint8					internal_state;
};


// Holds the state of a directory query.
struct SFatFileSystemQuery
{
	SFatDirectoryEntry	current_entry;
	SFatQueryState		state;
};


#endif // ! __FAT_COMMON_H__

