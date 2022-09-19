#ifndef __FAT_COMMON_H__
#define __FAT_COMMON_H__
#include <time.h>
#include "BaseLib/PrimitiveTypes.h"


#define FAT_MAX_PATH					260
#define FAT_FIRST_LFN_ENTRY				0x40
#define FAT_MAX_FILENAME				255


// FAT entry data type
typedef uint32 FatEntry;


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

		struct SFatRawLongFileName
		{
			uint8	uiSequence;
			uint8	auiChars1[10];
			uint8	uiAttributes;
			uint8	uiType;
			uint8	uiChecksum;
			uint8	auiChars2[12];
			uint16	uiFirstCluster;
			uint8	auiChars3[4];
		} sFatRawLongFileName;
	} uEntry;
};
#pragma pack(pop)


// FAT32 FSInfo structure
#pragma pack(push, 1)
struct SFatFileSystemInfo
{
	uint32	TrailSig;
	uint8	Reserved2[12];
	uint32	Nxt_Free;
	uint32	Free_Count;
	uint32	StructSig;
	uint8	Reserved1[480];
	uint32	LeadSig;
};
#pragma pack(pop)


// MBR partition entry structure
#pragma pack(push, 1)
struct SFatPartitionEntry
{
	uint8	status;
	uint8	chs_first_sector[3];
	uint8	partition_type;
	uint8	chs_last_sector[3];
	uint32	lba_first_sector;
	uint32	total_sectors;
};
#pragma pack(pop)


// BPB structure ( 224 bits/28 bytes )
#pragma pack(push, 1)
struct SFatBIOSParameterBlock
{
	uint8	BS_jmpBoot[3];					/* 0  */
	char	BS_OEMName[8];					/* 3  */
	uint16	BPB_BytsPerSec;					/* 11 */
	uint8	BPB_SecPerClus;					/* 13 */
	uint16	BPB_RsvdSecCnt;					/* 14 */
	uint8	BPB_NumFATs;					/* 16 */
	uint16	BPB_RootEntCnt;					/* 17 */
	uint16	BPB_TotSec16;					/* 19 */
	uint8	BPB_Media;						/* 21 */
	uint16	BPB_FATSz16;					/* 22 */
	uint16	BPB_SecPerTrk;					/* 24 */
	uint16	BPB_NumHeads;					/* 26 */
	uint32	BPB_HiddSec;					/* 28 */
	uint32	BPB_TotSec32;					/* 32 */
	union
	{
		struct SFat16BPB
		{
			uint8	BS_DrvNum;
			uint8	BS_Reserved1;
			uint8	BS_BootSig;
			uint32	BS_VolID;
			char	BS_VolLab[11];
			char	BS_FilSysType[8];
			char	Pad1[8];
			uint32	Pad2;
			uint8	Pad3;
			uint8	ExtraPadding[15];
		} sFat16;

		struct SFat32BPB
		{
			uint32	BPB_FATSz32;
			uint16	BPB_ExtFlags;
			uint16	BPB_FSVer;
			uint32	BPB_RootClus;
			uint16	BPB_FSInfo;
			uint16	BPB_BkBootSec;
			uint8	BPB_Reserved[12];
			uint8	BS_DrvNum;
			uint8	BS_Reserved1;
			uint8	BS_BootSig;
			uint32	BS_VolID;
			char	BS_VolLab[11];
			char	BS_FilSysType[8];
		} sFat32;
	} uFatEx;
};
#pragma pack(pop)


// Stores information about directory entries.
struct SFatDirectoryEntry
{
	uint8					name[FAT_MAX_FILENAME + 1];
	uint8					attributes;
	time_t					create_time;
	time_t					modify_time;
	time_t					access_time;
	uint32					size;
	uint32					uiSectorAddress;
	uint16					uiSectorOffset;
	SFatRawDirectoryEntry	raw;
};


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


// seek modes
enum EFatSeek
{
	FAT_SEEK_START = 0x1,
	FAT_SEEK_CURRENT = 0x2,
	FAT_SEEK_END = 0x3
};


// return codes (first 32 codes are reserved)
enum EFatCode
{
	FAT_SUCCESS							= 0x0,
	FAT_UNKNOWN_ERROR					= 0x20 + 0x1,
	FAT_CANNOT_READ_MEDIA				= 0x20 + 0x2,
	FAT_CANNOT_WRITE_MEDIA				= 0x20 + 0x3,
	FAT_NOT_A_DIRECTORY					= 0x20 + 0x4,
	FAT_INVALID_FILENAME				= 0x20 + 0x5,
	FAT_FILENAME_ALREADY_EXISTS			= 0x20 + 0x6,
	FAT_INVALID_PATH					= 0x20 + 0x7,
	FAT_CORRUPTED_FILE					= 0x20 + 0x8,
	FAT_ILLEGAL_FILENAME				= 0x20 + 0x9,
	FAT_FILENAME_TOO_LONG				= 0x20 + 0xA,
	FAT_NOT_A_FILE						= 0x20 + 0xB,
	FAT_FILE_NOT_FOUND					= 0x20 + 0xC,
	FAT_DIRECTORY_DOES_NOT_EXIST		= 0x20 + 0xD,
	FAT_INSUFFICIENT_DISK_SPACE			= 0x20 + 0xE,
	FAT_FEATURE_NOT_SUPPORTED			= 0x20 + 0xF,
	FAT_OP_IN_PROGRESS					= 0x20 + 0x10,
	FAT_SECTOR_SIZE_NOT_SUPPORTED		= 0x20 + 0x11,
	FAT_LFN_GENERATED					= 0x20 + 0x12,
	FAT_SHORT_LFN_GENERATED				= 0x20 + 0x13,
	FAT_SEEK_FAILED						= 0x20 + 0x14,
	FAT_FILE_NOT_OPENED_FOR_WRITE_ACCESS= 0x20 + 0x15,
	FAT_INVALID_HANDLE					= 0x20 + 0x16,
	FAT_INVALID_CLUSTER					= 0x20 + 0x17,
	FAT_INVALID_FAT_VOLUME				= 0x20 + 0x18,
	FAT_INVALID_VOLUME_LABEL			= 0x20 + 0x19,
	FAT_INVALID_FORMAT_PARAMETERS		= 0x20 + 0x1A,
	FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED	= 0x20 + 0x1B,
	FAT_DIRECTORY_LIMIT_EXCEEDED		= 0x20 + 0x1C,
	FAT_INVALID_PARAMETERS				= 0x20 + 0x1D,
	FAT_FILE_HANDLE_IN_USE				= 0x20 + 0x1E,
	FAT_FILE_BUFFER_NOT_SET				= 0x20 + 0x1F,
	FAT_MISALIGNED_IO					= 0x20 + 0x20,
	FAT_AWAITING_DATA					= 0x20 + 0x21,
	FAT_BUFFER_TOO_BIG					= 0x20 + 0x22
};

// these are the bits set on the reserved fields of a directory
// entry to indicate that a SFN entry is actually a LFN entry with
// a filename conforming to the 8.3 convention but with a lowercase
// base name or extension (or both). This library will display these
// entries correctly, but for compatibility reasons when creating
// entries an LFN entry will be created for this type of files.
#define FAT_LOWERCASE_EXTENSION			0x10
#define FAT_LOWERCASE_BASENAME			0x08



// Holds the internal state of a directory query.
struct SFatQueryState
{
	uint8						Attributes;
	uint16						uiCurrentSector;
	uint32						uiCurrentCluster;
	SFatRawDirectoryEntry*		current_entry_raw;
	uint8*						uiBuffer;

	SFatRawDirectoryEntry*		first_entry_raw;

	// LFN support members
	uint16						long_filename[256];
	uint8						uiSequence;
	uint8						uiChecksum;

	// buffer (MUST ALWAYS BE LAST!!!)
	uint8						buff[MAX_SECTOR_LENGTH];
};


// Holds the state of a directory query.
struct SFatFileSystemQuery
{
	SFatDirectoryEntry	current_entry;
	SFatQueryState		state;
};


enum EFatFileSystemType
{
	FAT_FS_TYPE_UNSPECIFIED,
	FAT_FS_TYPE_FAT12,
	FAT_FS_TYPE_FAT16,
	FAT_FS_TYPE_FAT32
};


#define FAT12_EOC						( 0x0FFF )
#define FAT16_EOC						( 0xFFFF )
#define FAT32_EOC						( 0x0FFFFFFF )
#define FAT12_BAD_CLUSTER				( 0x0FF7 )
#define FAT16_BAD_CLUSTER				( 0xFFF7 )
#define FAT32_BAD_CLUSTER				( 0x0FFFFFF7 )
#define FAT16_CLEAN_SHUTDOWN			( 0x8000 )
#define FAT32_CLEAN_SHUTDOWN			( 0x08000000 )
#define FAT16_HARD_ERROR				( 0x4000 )
#define FAT32_HARD_ERROR				( 0x04000000 )
#define FAT32_CUTOVER					( 1024 )
#define FREE_FAT						( 0x0000 )
#define ILLEGAL_CHARS_COUNT 			( 0x10 )
#define BACKSLASH						( 0x5C )
#define FAT_OPEN_HANDLE_MAGIC			( 0x4B )
#define FAT_DELETED_ENTRY				( 0xE5 )
#define FAT_UNKNOWN_SECTOR				( 0xFFFFFFFF )


// macros for checking if a directory entry is free
// and if it's the last entry on the directory
#define IS_FREE_DIRECTORY_ENTRY(entry) (*(entry)->uEntry.sFatRawCommon.name == 0xE5 || *(entry)->uEntry.sFatRawCommon.name == 0x0)
#define IS_LAST_DIRECTORY_ENTRY(entry) (*(entry)->uEntry.sFatRawCommon.name == 0x0)


 // min and max macros
#define MAX(a, b)		(( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define MIN(a, b)		(( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define LO8(word)		((uint8) (word))
#define HI8(word)		((uint8) ((word) >> 8 ))
#define LO16(dword)		((uint16) (dword))
#define HI16(dword)		((uint16) ((dword) >> 16 ))


// table of illegal filename chars.
static const char ILLEGAL_CHARS[] = { 0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D, 0x7C };


struct SFatQueryStateInternal
{
	uint8						Attributes;
	uint16						uiCurrentSector;
	uint32						uiCurrentCluster;
	SFatRawDirectoryEntry*		current_entry_raw;
	uint8*						pvBuffer;

	SFatRawDirectoryEntry* first_entry_raw;

	// LFN support members
	uint16						long_filename[256];
	uint8						uiSequence;
	uint8						uiChecksum;

};


#endif // ! __FAT_COMMON_H__

