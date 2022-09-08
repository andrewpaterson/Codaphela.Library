#ifndef __FAT_STRUCTURE_H__
#define __FAT_STRUCTURE_H__


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


#endif // !__FAT_STRUCTURE_H__

