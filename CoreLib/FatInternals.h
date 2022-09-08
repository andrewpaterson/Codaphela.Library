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

#ifndef __FAT_INTERNALS__
#define __FAT_INTERNALS__
#include "Fat.h"


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


 // date/time macros
#define FAT_ENCODE_DATE(month, day, year)			((((uint16)((year) - 1980)) << 9) | ((uint16)(month) << 5) | (uint16)(day))
#define FAT_DECODE_DATE(date, month, day, year)		(year) = ((date) >> 9) + 1980);(month) = ((date & 0x1E0) >> 5);(day) = (date & 0x1F)
#define FAT_ENCODE_TIME(hour, minute, second)		(((uint16)(hour) << 11) | ((uint16)(minute) << 5) | ((uint16)(second) >> 1))
#define FAT_DECODE_TIME(time, hour, minute, second)	(hour) = ((time) >> 11); (minute) = (((time) & 0x7E0) >> 5); (secs) = (((time) & 0x1F) << 1)


 // min and max macros
#define MAX(a, b)		(( ( a ) > ( b ) ) ? ( a ) : ( b ) )
#define MIN(a, b)		(( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define LO8(word)		((uint8) (word))
#define HI8(word)		((uint8) ((word) >> 8 ))
#define LO16(dword)		((uint16) (dword))
#define HI16(dword)		((uint16) ((dword) >> 16 ))


// table of illegal filename chars.
static const char ILLEGAL_CHARS[] = {
	0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B,
	0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D, 0x7C
};


struct SFatQueryStateInternal
{
	uint8						Attributes;
	uint16						current_sector;
	uint32						current_cluster;
	SFatRawDirectoryEntry*		current_entry_raw;
	uint8*						buffer;

	SFatRawDirectoryEntry*		first_entry_raw;

	// LFN support members
	uint16						long_filename[256];
	uint8						lfn_sequence;
	uint8						lfn_checksum;

};


// prototypes
uint16 fat_get_cluster_entry(SFatVolume* volume, uint32 cluster, FatEntry* fat_entry);
uint16 fat_set_cluster_entry(SFatVolume* volume, uint32 cluster, FatEntry fat_entry);
uint16 fat_free_cluster_chain(SFatVolume* volume, uint32 cluster);
uint32 fat_allocate_data_cluster(SFatVolume* volume, uint32 count, char zero, uint16* result);
uint16 fat_create_directory_entry(SFatVolume* volume, SFatRawDirectoryEntry* parent, char* name, uint8 attribs, uint32 entry_cluster, SFatDirectoryEntry* entry);
char fat_increase_cluster_address(SFatVolume* volume, uint32 current_cluster, uint16 count, uint32* value);
char fat_is_eof_entry(SFatVolume* volume, FatEntry fat);

uint8 fat_long_entry_checksum(uint8* filename);
uint16 get_short_name_for_entry(uint8* dest, uint8* src, char lfn_disabled);
uint32 fat_allocate_directory_cluster(SFatVolume* volume, SFatRawDirectoryEntry* parent, uint16* result);
uint16 fat_query_first_entry(SFatVolume* volume, SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, char buffer_locked);
uint16 fat_query_next_entry(SFatVolume* volume, SFatQueryState* query, char buffer_locked, char first_entry);
uint16 fat_open_file_by_entry(SFatVolume* volume, SFatDirectoryEntry* entry, SFatFile* handle, uint8 access_flags);

int indexof(char chr, char* str, int index);

void fat_get_short_name_from_entry(uint8* dest, const uint8* src);
char fat_compare_short_name(uint8* name1, uint8* name2);
void fat_fill_directory_entry_from_raw(SFatDirectoryEntry* entry, SFatRawDirectoryEntry* raw_entry);
uint16 rtc_get_fat_date();
uint16 rtc_get_fat_time();
time_t fat_decode_date_time(uint16 date, uint16 time);
void strtrim(char* dest, char* src, size_t max);
void fat_parse_path(char* path, char* path_part, char** filename_part);

#if defined(FAT_OPTIMIZE_FOR_FLASH)
uint32 fat_allocate_data_cluster_ex(SFatVolume* volume, uint32 count, char zero, uint32 page_size, uint16* result);
#endif

char fat_compare_long_name(uint16* name1, uint16* name2);
char get_long_name_for_entry(uint16* dst, uint8* src);

#endif
