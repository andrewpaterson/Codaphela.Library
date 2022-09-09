#ifndef __FAT_STRUCTURE_H__
#define __FAT_STRUCTURE_H__


// misc
#define FAT_MAX_PATH					260
#define FAT_FIRST_LFN_ENTRY				0x40
#define FAT_MAX_FILENAME				255


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
			uint8 lfn_sequence;
			uint8 lfn_chars_1[10];
			uint8 lfn_attributes;
			uint8 lfn_type;
			uint8 lfn_checksum;
			uint8 lfn_chars_2[12];
			uint16 lfn_first_cluster;
			uint8 lfn_chars_3[4];
		} sFatRawLongFileName;
	} uEntry;
};
#pragma pack(pop)


// FAT entry data type
typedef uint32 FatEntry;


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
	uint32					sector_addr;
	uint16					sector_offset;
	SFatRawDirectoryEntry	raw;
};


#endif // !__FAT_STRUCTURE_H__

