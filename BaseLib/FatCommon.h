#ifndef __FAT_COMMON_H__
#define __FAT_COMMON_H__
#include <time.h>
#include "PrimitiveTypes.h"
#include "FatInfoSectorCache.h"

#define FAT_MAX_PATH					260
#define FAT_FIRST_LFN_ENTRY				0x40
#define FAT_MAX_FILENAME				255
#define FAT_MAX_SECTOR_QUERY_LOCKS		32
#define FAT_MAX_DIRECTORY_ENTRIES		0xFFFF
#define FAT_LONG_NAME_END_WCHAR			0xFFFF

// FAT entry data type
typedef uint32 fatEntry;


// fat 32-byte directory entry structure
#pragma pack(push, 1)
struct SFatRawDirectoryEntry
{
	union
	{
		struct SFatRawCommon
		{
			uint8	szShortName[11];
			uint8	uiAttributes;
			uint8	uiReserved;
			uint8	uiCreateTimeTenths;
			uint16	uiCreateTime;
			uint16	uiCreateDate;
			uint16	uiAccessDate;
			uint16	uiFirstClusterHighWord;
			uint16	uiModifyTime;
			uint16	uiModifyDate;
			uint16	uiFirstClusterLowWord;
			uint32	uiSize;
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
	uint32	uiTrailSignature;
	uint8	uiReserved2[12];
	uint32	uiNextFreeCluster;
	uint32	uiNumFreeClusters;
	uint32	uiStructSignature;
	uint8	uiReserved1[480];
	uint32	uiLeadSignature;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct SFatPartitionEntry
{
	uint8	uiStatus;
	uint8	uiFirstCylinderHeadSectorSector[3];
	uint8	uiPartitionType;
	uint8	uiLastCylinderHeadSectorSector[3];
	uint32	uiLBAFirstSector;
	uint32	uiTotalSectors;
};
#pragma pack(pop)


#pragma pack(push, 1)
struct SFatBIOSParameterBlock
{
	uint8	uiJumpInstrustion[3];	
	char	szOEMName[8];			
	uint16	uiBytsPerSector;		
	uint8	uiSectorsPerCluster;	
	uint16	uiReservedSectorCount;	
	uint8	uiNumFileAllocationTables;
	uint16	uiRootEntryCount;		
	uint16	uiTotalSectorsFat16;	
	uint8	uiMediaType;			
	uint16	uiFATSzFat16;			
	uint16	uiSectorsPerTrack;		
	uint16	uiNumHeads;				
	uint32	uiNumHiddenSectors;		
	uint32	uiTotalSectorsFat32;	
	union
	{
		struct SFat16BPB
		{
			uint8	uiDriverNumber;
			uint8	uiReserved1;
			uint8	uiBootSignature;
			uint32	uiVolumeID;
			char	szVolumeLabel[11];
			char	szFileSystemType[8];
			char	acPad1[8];
			uint32	uiPad2;
			uint8	uiPad3;
			uint8	auiExtraPadding[15];
		} sFat16;

		struct SFat32BPB
		{
			uint32	uiFATSzFat32;
			uint16	uiExtendedFlags;
			uint16	uiFileSystemVersion;
			uint32	uiRootCluster;
			uint16	uiFileSystemInformation;
			uint16	uiBackupBootSector;
			uint8	auiReserved[12];
			uint8	uiDriverNumber;
			uint8	uiReserved1;
			uint8	uiBootSignature;
			uint32	uiVolumeID;
			char	szVolumeLabel[11];
			char	szFileSystemType[8];
		} sFat32;
	} uFatEx;
};
#pragma pack(pop)


struct SFatDirectoryEntry
{
	uint8					szName[FAT_MAX_FILENAME + 1];  //Converting this to a char causes a failure with sign extension ... somewhere.
	uint8					uiAttributes;
	time_t					tCreateTime;
	time_t					tModifyTime;
	time_t					tAccessTime;
	uint32					uiSize;
	uint32					uiSectorAddress;
	uint16					uiSectorOffset;
	SFatRawDirectoryEntry	sRaw;
};


uint32	GetFirstClusterFromFatEntry(SFatRawDirectoryEntry* psRawEntry, bool bFat32Volume);
char*	FatEntryToString(SFatDirectoryEntry* psFatEntry, bool bFat32Volume);
char*	FatAttributeToString(uint8 uiAttribute);


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
	FAT_INVALID_FAT_CACHE				= 0x20 + 0x1F,
	FAT_NO_CACHE_LOCK					= 0x20 + 0x20,
	FAT_SHORT_NAME_FOUND				= 0x20 + 0x21,
	FAT_NOT_ALL_CLUSTERS_ALLOCATED		= 0x20 + 0x22,
	FAT_LAST_ERROR_ENUM					= FAT_SHORT_NAME_FOUND
};


char* FatCodeToString(EFatCode eCode);


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
	uint8					uiAttributes;
	uint16					uiCurrentSector;
	uint32					uiCurrentCluster;
	SFatRawDirectoryEntry*	psCurrentEntryRaw;
	SFatRawDirectoryEntry*	psFirstEntryRaw;

	uint16					auiLongFilename[FAT_MAX_FILENAME + 1];
	uint8					uiSequence;
	uint8					uiChecksum;

	SFatCache				sBuffer;
	uint32					uiLockedSectors[FAT_MAX_SECTOR_QUERY_LOCKS];

	void	Init(void);
	bool	Kill(CFatInfoSectorCache* pcCache);
	bool	Lock(uint32 uiSector);
};


// Holds the state of a directory query.
struct SFatFileSystemQuery
{
	SFatDirectoryEntry	sCurrentEntry;
	SFatQueryState		sQuery;

	void	Init(void);
	void	Kill(CFatInfoSectorCache* pcCache);
};


enum EFatFileSystemType
{
	FAT_FS_TYPE_UNSPECIFIED,
	FAT_FS_TYPE_FAT12,
	FAT_FS_TYPE_FAT16,
	FAT_FS_TYPE_FAT32
};


#define FAT12_EOC					0x00000FFF
#define FAT16_EOC					0x0000FFFF
#define FAT32_EOC					0x0FFFFFFF
#define FAT12_BAD_CLUSTER			0x00000FF7
#define FAT16_BAD_CLUSTER			0x0000FFF7
#define FAT32_BAD_CLUSTER			0x0FFFFFF7
#define FAT16_CLEAN_SHUTDOWN		0x8000
#define FAT32_CLEAN_SHUTDOWN		0x08000000
#define FAT16_HARD_ERROR			0x4000
#define FAT32_HARD_ERROR			0x04000000
#define FREE_FAT					0x0000
#define ILLEGAL_CHARS_COUNT 		0x10
#define FAT_OPEN_HANDLE_MAGIC		0x4B
#define FAT_KANJI_SKIP_ENTRY		0x05
#define FAT_DELETED_ENTRY			0xE5
#define FAT12_CLUSTER_MASK			0x00000FFF
#define FAT16_CLUSTER_MASK			0x0000FFFF
#define FAT32_CLUSTER_MASK			0x0FFFFFFF
#define FAT12_MAX_CLUSTERS			4084
#define FAT16_MAX_CLUSTERS			65524


#define RETURN_ON_FAT_FAILURE(result) 	if (eResult != FAT_SUCCESS)	{ return result; }


// table of illegal filename chars.
static const char ILLEGAL_CHARS[] = { 0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D, 0x7C };


#endif // ! __FAT_COMMON_H__

