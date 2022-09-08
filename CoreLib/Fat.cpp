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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Fat.h"
#include "FatInternals.h"

 

 // function pointer for rtc access routing
 #if !defined(FAT_USE_SYSTEM_TIME)
typedef struct TIMEKEEPER
{
	FAT_GET_SYSTEM_TIME fat_get_system_time;
}
TIMEKEEPER;
static TIMEKEEPER timekeeper;
#endif

/*
// TODO:
// 1. Optimize fat_file_seek
//
// 2. Update fat_file_alloc to allocated clusters based on the # of bytes
//		needed and not the clusters needed to allocate those bytes. Right now
//		it's overallocating under some circumstances. The extra clusters get
//		freed when the file is closed anyways but it'll still be more efficient
//		that way.
*/


// initialize fat driver
void fat_init()
{
	fat_shared_buffer_sector = FAT_UNKNOWN_SECTOR;
}


/*
// registers the function that gets the system time
*/
#if !defined(FAT_USE_SYSTEM_TIME)
void fat_register_system_time_function(FAT_GET_SYSTEM_TIME system_time)
{
	timekeeper.fat_get_system_time = system_time;
}
#endif

/*
// finds the first file in a directory
*/
uint16 fat_find_first_entry(SFatVolume* volume, char* parent_path, uint8 attributes, SFatDirectoryEntry** dir_entry, SFatFileSystemQuery* q) 
{
	uint16 uiResult;
	SFatDirectoryEntry parent_entry;
	FAT_FILESYSTEM_QUERY_INNER* query = (FAT_FILESYSTEM_QUERY_INNER*)q;

	// make sure the query has a buffer
	if (!q->state.buffer)
	{
		q->state.buffer = q->state.buff;
	}

	// if the path starts with a backlash then advance to
	// the next character
	if (parent_path)
	{
		if (*parent_path == '\\')
		{
			parent_path++;
		}
	}

	// if a parent was specified...
	if (parent_path != NULL)
	{
		// try to get the entry for the parent
		uiResult = fat_get_file_entry(volume, parent_path, &parent_entry);

		// if we were unable to get the parent entry
		// then return the error that we received from
		// fat_get_file_entry
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// try to get the 1st entry of the
		// query results
		uiResult = fat_query_first_entry(volume, &parent_entry.raw, attributes, &query->state, 0);
	}
	// if the parent was not supplied then we
	// submit the query without it
	else
	{
		uiResult = fat_query_first_entry(volume, 0, attributes, &query->state, 0);
	}

	// if we cant get the 1st entry then return the
	// error that we received from fat_Query_First_entry
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if there are no more entries
	if (query->state.current_entry_raw == 0) 
	{
		*query->current_entry.name = 0;
		return FAT_SUCCESS;
	}

	// fill the current entry structure with data from
	// the current raw entry of the query
	fat_fill_directory_entry_from_raw(&query->current_entry, query->state.current_entry_raw);

	// calculate the sector address of the entry - if
	// query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is
	// different
	if (query->state.current_cluster == 0x0) 
	{
		query->current_entry.sector_addr = volume->no_of_reserved_sectors + 
											(volume->no_of_fat_tables * volume->uiFatSize) +
											query->state.current_sector;
	}
	else
	{
		query->current_entry.sector_addr = FIRST_SECTOR_OF_CLUSTER(volume, query->state.current_cluster) + query->state.current_sector;
	}

	// calculate the offset of the entry within it's sector
	query->current_entry.sector_offset = (uint16)((uintptr_t)query->state.current_entry_raw) - ((uintptr_t)query->state.buffer);
	// store a copy of the original FAT directory entry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	query->current_entry.raw = *query->state.current_entry_raw;

	// if long filenames are enabled copy the filename to the entry
	if (*query->current_entry.name != 0)
	{
		if (*query->state.long_filename != 0)
		{
			for (uiResult = 0; uiResult < 256; uiResult++)
			{
				query->current_entry.name[uiResult] = (uint8)query->state.long_filename[uiResult];
				if (query->state.long_filename[uiResult] == 0)
				{
					break;
				}
			}
		}
	}


	/*
	// if the user provided a pointer-to-pointer to the
	// result set it to the current entry.
	*/
	if (dir_entry)
	{
		*dir_entry = &query->current_entry;
	}

	/*
	// return success code
	*/
	return FAT_SUCCESS;
}

/*
// finds the next file in the directory
*/
uint16 fat_find_next_entry(	SFatVolume* volume, SFatDirectoryEntry** dir_entry, SFatFileSystemQuery* q) 
{
	uint16 uiResult;
	FAT_FILESYSTEM_QUERY_INNER* query = (FAT_FILESYSTEM_QUERY_INNER*)q;

	/*
	// try to get the next entry of the query
	*/
	uiResult = fat_query_next_entry(volume, &query->state, 0, 0);
	/*
	// if we received an error from fat_query_next_entry
	// then we return the error code to the calling function
	*/
	if (uiResult != FAT_SUCCESS)
		return uiResult;
	/*
	// if there are no more entries
	*/
	if (query->state.current_entry_raw == 0)
	{
		/*
		// set the filename of the current entry
		// to 0
		*/
		*query->current_entry.name = 0;
		/*
		// return success
		*/
		return FAT_SUCCESS;
	}
	/*
	// fill the current entry structure with data from
	// the current raw entry of the query
	*/
	fat_fill_directory_entry_from_raw(&query->current_entry, query->state.current_entry_raw);
	/*
	// calculate the sector address of the entry - if
	// query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is
	// different
	*/
	if (query->state.current_cluster == 0x0)
	{
		query->current_entry.sector_addr = volume->no_of_reserved_sectors + 
											(volume->no_of_fat_tables * volume->uiFatSize) +
											query->state.current_sector;
	}
	else
	{
		query->current_entry.sector_addr = FIRST_SECTOR_OF_CLUSTER(volume, query->state.current_cluster) + query->state.current_sector;
	}
	/*
	// calculate the offset of the entry within it's sector
	*/
	query->current_entry.sector_offset = (uint16)((uintptr_t)query->state.current_entry_raw) - ((uintptr_t)query->state.buffer);
	/*
	// store a copy of the original FAT directory entry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	*/
	query->current_entry.raw = *query->state.current_entry_raw;

	if (*query->current_entry.name != 0)
	{
		if (*query->state.long_filename != 0)
		{
			for (uiResult = 0; uiResult < 256; uiResult++)
			{
				query->current_entry.name[uiResult] = (uint8)query->state.long_filename[uiResult];
				if (query->state.long_filename[uiResult] == 0)
					break;
			}
		}
	}

	if (dir_entry)
	{
		*dir_entry = &query->current_entry;
	}
	/*
	// return success code
	*/
	return FAT_SUCCESS;
}

/*
// fills a directory entry structure with data taken
// from a raw directory entry
*/
void fat_fill_directory_entry_from_raw(
	SFatDirectoryEntry* entry, SFatRawDirectoryEntry* raw_entry) {

	/*
	// copy the filename and transform the filename
	// from the internal structure to the public one
	*/
	fat_get_short_name_from_entry(entry->name, raw_entry->uEntry.sFatRawCommon.name);
	/*
	// copy other data from the internal entry structure
	// to the public one
	*/
	entry->attributes = raw_entry->uEntry.sFatRawCommon.attributes;
	entry->size = raw_entry->uEntry.sFatRawCommon.size;
	entry->create_time = fat_decode_date_time(raw_entry->uEntry.sFatRawCommon.create_date, raw_entry->uEntry.sFatRawCommon.create_time);
	entry->modify_time = fat_decode_date_time(raw_entry->uEntry.sFatRawCommon.modify_date, raw_entry->uEntry.sFatRawCommon.modify_time);
	entry->access_time = fat_decode_date_time(raw_entry->uEntry.sFatRawCommon.access_date, 0);
	entry->raw = *raw_entry;
}


// creates a directory
uint16 fat_create_directory(SFatVolume* volume, char* directory)
{
	uint16 uiResult;
	SFatDirectoryEntry entry;
	/*
	// check that we got a valid pathname
	*/
	if (!directory || strlen(directory) > FAT_MAX_PATH)
		return FAT_INVALID_FILENAME;
	/*
	// try get the file entry
	*/
	uiResult = fat_get_file_entry(volume, directory, &entry);
	if (uiResult != FAT_SUCCESS)
		return uiResult;
	/*
	// if we don'tfind a file or directory by that name
	// we can create it, otherwise return file already exists error
	*/
	if (*entry.name == 0)
	{
		/*
		// allocate memory for the file path
		*/
		size_t path_len;
		char* path_scanner;
		char file_path[FAT_MAX_PATH + 1];
		SFatDirectoryEntry parent_entry;
		/*
		// get the name of the file path including
		// the filename
		*/
		path_len = strlen(directory);
		/*
		// set the pointer that will be used to scan
		// filename to the end of the filename
		*/
		path_scanner = directory + (path_len - 0x1);
		/*
		// if the filename ends with a backslash then it
		// is an invalid filename ( it's actually a directory
		// path )
		*/
		if (*path_scanner == BACKSLASH)
			return FAT_INVALID_FILENAME;
		/*
		// scan the filename starting at the end until
		// a backslash is found - when we exit this loop
		// path_scanner will point to the last character
		// of the filepath
		*/
		while (*(--path_scanner) != BACKSLASH);	/*scan file backwords until we get to */
		/*
		// calculate the length of the path part of the
		// filename
		*/
		path_len = (size_t)(path_scanner - directory);
		/*
		// copy the path part of the filename to
		// the file_path buffer
		*/
		memcpy(file_path, directory, path_len);
		/*
		// set the null terminator of the file_path buffer
		*/
		file_path[path_len] = 0x0;
		/*
		// increase pointer to the beggining of the filename
		// part of the path
		*/
		path_scanner++;
		/*
		// try to get the entry for the parent directory
		*/
		uiResult = fat_get_file_entry(volume, file_path, &parent_entry);
		/*
		// if fat_get_file_entry returned an error
		// then we return the error code to the calling
		// function
		*/
		if (uiResult != FAT_SUCCESS)
			return uiResult;
		/*
		// if the parent directory does not exists
		*/
		if (*parent_entry.name == 0)
			return FAT_DIRECTORY_DOES_NOT_EXIST;
		/*
		// try to create the directory entry
		*/
		return fat_create_directory_entry(volume,
			&parent_entry.raw, path_scanner, FAT_ATTR_DIRECTORY, 0, &entry);
	}
	/*
	// if we get here it means that a file or
	// directory with that name already exists.
	*/
	return FAT_FILENAME_ALREADY_EXISTS;
}


// gets a SFatDirectoryEntry by it's full path
uint16 fat_get_file_entry(SFatVolume* volume, char* path, SFatDirectoryEntry* entry)
{
	uint16 uiResult;
	char match;
	uint8 target_file[13];
	uint8* pLevel;
	SFatRawDirectoryEntry* current_entry;
	SFatQueryStateInternal query;
	/* SFatQueryState query; */

	char using_lfn;
	/* char using_lfn_and_short; */
	uint16 target_file_long[FAT_MAX_PATH + 1];	/* stores the utf16 long filename */
	uint8 current_level[FAT_MAX_PATH + 1];

	query.buffer = fat_shared_buffer;

	/*
	// if the path starts with a backlash then advance to
	// the next character
	*/
	if (*path == '\\')
	{
		path++;
	}

	if (*path != 0)
	{
		// set current_entry to 0, in this state
		// it represents the root directory of the
		// volume
		current_entry = 0;
	}
	// if the caller did not supply a path then the
	// request is for the root directory, since there's
	// no physical entry for the root directory we must
	// create a fake one
	else
	{
		/*
		// copy the file name to the entry and the raw
		// entry in their respective formats
		*/
		strcpy((char*)entry->name, "ROOT");
		get_short_name_for_entry(entry->raw.uEntry.sFatRawCommon.name, entry->name, 1);
		/*
		// set the general fields of the entry
		*/
		entry->attributes = entry->raw.uEntry.sFatRawCommon.attributes = FAT_ATTR_DIRECTORY;
		entry->size = entry->raw.uEntry.sFatRawCommon.size = 0x0;
		/*
		// since the entry does not physically exist the
		// address fields are set to zero as well
		*/
		entry->sector_addr = 0x0;
		entry->sector_offset = 0x0;

		/*
		// set the location of the root directory
		*/
		if (volume->fs_type == FAT_FS_TYPE_FAT32)
		{
			/*
			// if this is a FAT32 volume then the root
			// directory is located on the data section just like
			// any other directory
			*/
			entry->raw.uEntry.sFatRawCommon.first_cluster_lo = LO16(volume->root_cluster);
			entry->raw.uEntry.sFatRawCommon.first_cluster_hi = HI16(volume->root_cluster);
		}
		else
		{
			/*
			// if the volume is FAT12/16 we set the cluster
			// address to zero and when time comes to get the
			// directory we'll calculate the address right after
			// the end of the FATs
			*/
			entry->raw.uEntry.sFatRawCommon.first_cluster_lo = 0x0;
			entry->raw.uEntry.sFatRawCommon.first_cluster_hi = 0x0;
		}
		/*
		// return success code
		*/
		return FAT_SUCCESS;
	}
	/*
	// mark the cached sector as unknown
	*/
	fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);
	/*
	// for each level on the path....
	*/
	do
	{
		/*
		// set the pointer current level name buffer
		*/
		pLevel = current_level;
		/*
		// if the path starts with a backlash then advance to
		// the next character
		*/
		if (*path == '\\')
			path++;
		/*
		// copy the name of the current level entry
		*/
		uiResult = 0;
		while (*path != 0x0 && *path != '\\')
		{
			if (uiResult++ > FAT_MAX_PATH)
			{
				return FAT_INVALID_FILENAME;
			}
			*pLevel++ = *path++;
		}
		*pLevel = 0x0;
		/*
		// try to find the first entry
		*/
		uiResult = fat_query_first_entry(volume, current_entry, 0, (SFatQueryState*)&query, 1);
		/*
		// if we could not find the entry then
		// return an error code
		*/
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}
		/*
		// if the output of fat_query_first_entry indicates that
		// there are no entries available...
		*/
		if (*query.current_entry_raw->uEntry.sFatRawCommon.name == 0x0)
		{
			/*
			// set the name of the entry to 0
			*/
			*entry->name = 0;
			/*
			// return success
			*/
			return FAT_SUCCESS;
		}
		/*
		// get an LFN version of the filename
		*/
		using_lfn = 0;
		/*/using_lfn_and_short = 0;*/
		/*
		// format the current level filename to the 8.3 format
		// if this is an invalid 8.3 filename try to get the LFN
		// once we get a valid filename (either short or LFN) compare
		// it to the one on the current query entry
		*/
		if (get_short_name_for_entry(target_file, current_level, 1) == FAT_INVALID_FILENAME)
		{
			if (get_long_name_for_entry(target_file_long, current_level) == FAT_INVALID_FILENAME)
			{
				return FAT_INVALID_FILENAME;
			}
			using_lfn = 1;
			match = fat_compare_long_name(target_file_long, query.long_filename)
				|| fat_compare_short_name(target_file, query.current_entry_raw->uEntry.sFatRawCommon.name);
		}
		else
		{
			match = fat_compare_short_name(target_file, query.current_entry_raw->uEntry.sFatRawCommon.name);
		}

		/*
		// if the file doesn't match then get the
		// next file
		*/
		while (!match)
		{
			/*
			//  try to get the next file
			*/
			uiResult = fat_query_next_entry(volume, (SFatQueryState*)&query, 1, 0);
			/*
			// if we received an error message then return
			// it to the calling function
			*/
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}
			/*
			// if the output of fat_query_first_entry indicates that
			// there are no entries available then set the entry name to 0
			// and return success
			*/
			if (IS_LAST_DIRECTORY_ENTRY(query.current_entry_raw))
			{
				*entry->name = 0;
				return FAT_SUCCESS;
			}
			/*
			// match the filename against the next entry
			*/
			if (using_lfn)
			{
				match = fat_compare_long_name(target_file_long, query.long_filename)
					|| fat_compare_short_name(target_file, query.current_entry_raw->uEntry.sFatRawCommon.name);
			}
			else
			{
				match = fat_compare_short_name(target_file, query.current_entry_raw->uEntry.sFatRawCommon.name);
			}
		}
		/*
		// set the current entry to the entry
		// that we've just found
		*/
		current_entry = query.current_entry_raw;
	} while (*path != 0x0);
	/*
	// copy the filename and transform the filename
	// from the internal structure to the public one
	*/
	fat_get_short_name_from_entry(entry->name, query.current_entry_raw->uEntry.sFatRawCommon.name);
	/*
	// copy other data from the internal entry structure
	// to the public one
	*/
	entry->attributes = query.current_entry_raw->uEntry.sFatRawCommon.attributes;
	entry->size = query.current_entry_raw->uEntry.sFatRawCommon.size;
	entry->create_time = fat_decode_date_time(query.current_entry_raw->uEntry.sFatRawCommon.create_date, query.current_entry_raw->uEntry.sFatRawCommon.create_time);
	entry->modify_time = fat_decode_date_time(query.current_entry_raw->uEntry.sFatRawCommon.modify_date, query.current_entry_raw->uEntry.sFatRawCommon.modify_time);
	entry->access_time = fat_decode_date_time(query.current_entry_raw->uEntry.sFatRawCommon.access_date, 0);
	/*
	// calculate the sector address of the entry - if
	// query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is
	// different
	*/
	if (query.current_cluster == 0x0)
	{
		entry->sector_addr =
			volume->no_of_reserved_sectors + (volume->no_of_fat_tables * volume->uiFatSize) +
			query.current_sector;
	}
	else
	{
		entry->sector_addr =
			FIRST_SECTOR_OF_CLUSTER(volume, query.current_cluster) +
			query.current_sector; /*  + volume->NoOfSectorsPerCluster; */
	}
	/*
	// calculate the offset of the entry within it's sector
	*/
	entry->sector_offset = (uint16)((uintptr_t)query.current_entry_raw) - ((uintptr_t)query.buffer);

	/*
	// store a copy of the original FAT directory entry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	*/
	entry->raw = *query.current_entry_raw;
	/*
	// return success.
	*/
	return FAT_SUCCESS;
}


/*
// initializes a query of a set of directory
// entries
*/
uint16 fat_query_first_entry(SFatVolume* volume, SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, char buffer_locked)
{
	bool	bSuccess;
	uint32	first_sector;

	/*
	// make sure the long filename is set to an empty string
	*/
	query->long_filename[0] = 0;
	/*
	// if the directory entry has the cluster # set to
	// zero it is the root directory so we need to
	// get the cluster # accordingly
	*/
	if (directory)
	{
		if (directory->uEntry.sFatRawCommon.first_cluster_hi == 0 && directory->uEntry.sFatRawCommon.first_cluster_lo == 0)
		{
			directory = 0;
		}
	}
	/*
	// if no directory entry was provided
	// we'll use the root entry of the volume
	*/
	if (directory == 0)
	{
		/*
		// calculate the cluster # from the
		*/
		if (volume->fs_type == FAT_FS_TYPE_FAT32)
		{
			query->current_cluster = volume->root_cluster;
			first_sector = FIRST_SECTOR_OF_CLUSTER(volume, query->current_cluster);
		}
		else
		{
			query->current_cluster = 0x0;
			first_sector = volume->no_of_reserved_sectors + (volume->no_of_fat_tables * volume->uiFatSize);
		}
	}
	/*
	// if a directory entry was provided
	*/
	else
	{
		/*
		// if the entry provided is not a directory
		// entry return an error code
		*/
		if (!(directory->uEntry.sFatRawCommon.attributes & FAT_ATTR_DIRECTORY))
			return FAT_NOT_A_DIRECTORY;
		/*
		// set the CurrentCluster field of the query
		// state structure to the values found on the
		// directory entry structure
		*/
		((uint16*)&query->current_cluster)[INT32_WORD0] = directory->uEntry.sFatRawCommon.first_cluster_lo;
		/*
		// read the upper word of the cluster address
		// only if this is a FAT32 volume
		*/
		if (volume->fs_type == FAT_FS_TYPE_FAT32)
		{
			((uint8*)&query->current_cluster)[INT32_BYTE2] = LO8(directory->uEntry.sFatRawCommon.first_cluster_hi);
			((uint8*)&query->current_cluster)[INT32_BYTE3] = HI8(directory->uEntry.sFatRawCommon.first_cluster_hi);
		}
		else
		{
			((uint16*)&query->current_cluster)[INT32_WORD1] = 0;
		}
		/*
		// get the 1st sector of the directory entry
		*/
		first_sector =
			FIRST_SECTOR_OF_CLUSTER(volume, query->current_cluster);
	}

	/*
	// read the sector into the query
	// state buffer
	*/
	bSuccess = volume->device->Read(first_sector, query->buffer);
	if (!bSuccess)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	/*
	// set the 1st and current entry pointers
	// on the query state to the 1st entry of the
	// directory
	*/
	query->Attributes = attributes;
	query->current_sector = 0;

	query->first_entry_raw = (SFatRawDirectoryEntry*)query->buffer;
	query->current_entry_raw = (SFatRawDirectoryEntry*)query->buffer;

	/*
	// find the 1st entry and return it's result code
	*/
	return fat_query_next_entry(volume, query, buffer_locked, 1);
}
/*
// moves a query to the next entry
*/
uint16 fat_query_next_entry(SFatVolume* volume, SFatQueryState* query, char buffer_locked, char first_entry)
{
	char	pass;
	bool	bSuccess;
	uint32	sector_address;

	do
	{
		/*
		// if the current entry is the last entry of
		// the sector...
		*/
		if (!first_entry)
		{
			if (((uintptr_t)query->current_entry_raw - (uintptr_t)query->first_entry_raw) == volume->no_of_bytes_per_serctor - 0x20)
			{
				/*
				// if the current sector is the last of the current cluster then we must find the next
				// cluster... if CurrentCluster == 0 then this is the root directory of a FAT16/FAT12 volume, that
				// volume has a fixed size in sectors and is not allocated as a cluster chain so we don't do this
				*/
				if (query->current_cluster > 0 &&/*query->current_sector > 0x0 &&*/ query->current_sector == volume->no_of_sectors_per_cluster - 1)
				{
					FAT_ENTRY fat;

					/*
					// get the fat structure for the current cluster
					// and return UNKNOWN_ERROR if the operation fails
					*/
					if (fat_get_cluster_entry(volume, query->current_cluster, &fat) != FAT_SUCCESS)
					{
						return FAT_UNKNOWN_ERROR;
					}

					/*
					// if this is the last cluster of the directory...
					*/
					if (fat_is_eof_entry(volume, fat))
					{
						/*
						// set the current entry to 0
						*/
						*query->current_entry_raw->uEntry.sFatRawCommon.name = 0;
						/*
						// and return success
						*/
						return FAT_SUCCESS;
					}
					/*
					// set the current cluster to the next
					// cluster of the directory entry
					*/
					query->current_cluster = fat;
					/*
					// reset the current sector
					*/
					query->current_sector = 0x0;
					/*
					// calculate the address of the next sector
					*/
					sector_address =
						FIRST_SECTOR_OF_CLUSTER(volume, query->current_cluster) + query->current_sector;
				}
				/*
				// if there are more sectors on the current cluster then
				*/
				else
				{
					query->current_sector++;

					// if this is the root directory of a FAT16/FAT12
					// volume and we have passed it's last sector then
					// there's no more entries...
					if (query->current_cluster == 0x0)
					{
						if (query->current_sector == volume->root_directory_sectors)
						{
							*query->current_entry_raw->uEntry.sFatRawCommon.name = 0;
							return FAT_SUCCESS;
						}
						sector_address =
							(volume->no_of_reserved_sectors + (volume->no_of_fat_tables * volume->uiFatSize)) + query->current_sector;
					}
					else
					{
						/*
						// calculate the address of the next sector
						*/
						sector_address = FIRST_SECTOR_OF_CLUSTER(volume, query->current_cluster) + query->current_sector;
					}
				}
				/*
				// read the next sector into the query buffer
				*/
				bSuccess = volume->device->Read(sector_address, query->buffer);
				if (!bSuccess)
				{
					return FAT_CANNOT_READ_MEDIA;
				}
				/*
				// set the 1st and current entry pointers
				// on the query state to the 1st entry of the
				// directory
				*/
				query->first_entry_raw = (SFatRawDirectoryEntry*)query->buffer;
				query->current_entry_raw = (SFatRawDirectoryEntry*)query->buffer;
			}
			/*
			// if there are more entries on the current sector...
			*/
			else
			{
				/*
				// simply increase the current entry pointer
				*/
				query->current_entry_raw++;
			}
		}
		else
		{
			first_entry = 0;
		}
		/*
		// if this is a long filename entry...
		*/
		if (query->current_entry_raw->uEntry.sFatRawCommon.attributes == FAT_ATTR_LONG_NAME && !IS_FREE_DIRECTORY_ENTRY(query->current_entry_raw))
		{
			/*
			// if this enntry is marked as the 1st LFN entry
			*/
			if (query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_sequence & FAT_FIRST_LFN_ENTRY)
			{
				query->lfn_sequence = (query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_sequence ^ FAT_FIRST_LFN_ENTRY) + 1;
				query->lfn_checksum = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_checksum;
				/*
				// insert null terminator at the end of the long filename
				*/
				((uint8*)&query->long_filename[((query->lfn_sequence - 2) * 13) + 0xD])[INT16_BYTE0] = 0;
				((uint8*)&query->long_filename[((query->lfn_sequence - 2) * 13) + 0xD])[INT16_BYTE1] = 0;
			}
			/*
			// if this is the LFN that we're expecting then
			// process it, otherwise we'll have to wait for
			// another 1st LFN entry otherwise read the LFN
			// chrs and save them on the query state struct
			*/
			if (query->lfn_checksum == query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_checksum &&
				(query->lfn_sequence == (query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_sequence & (0xFF ^ FAT_FIRST_LFN_ENTRY)) + 1))
			{
				query->lfn_sequence = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_sequence & (0xFF ^ FAT_FIRST_LFN_ENTRY);
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x0])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[0];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x0])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[1];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x1])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[2];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x1])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[3];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x2])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[4];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x2])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[5];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x3])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[6];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x3])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[7];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x4])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[8];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x4])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[9];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x5])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[0];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x5])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[1];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x6])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[2];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x6])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[3];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x7])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[4];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x7])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[5];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x8])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[6];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x8])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[7];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x9])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[8];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x9])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[9];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xA])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[10];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xA])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[11];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xB])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_3[0];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xB])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_3[1];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xC])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_3[2];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xC])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_3[3];
			}
			else
			{
				query->lfn_checksum = 0;
			}

			/*
			// make sure we never return this entry
			*/
			pass = (query->Attributes == FAT_ATTR_LONG_NAME);
		}
		else
		{
			/*
			// check that the current entry passes the query
			// attributes check
			*/
			pass =
				(!(query->current_entry_raw->uEntry.sFatRawCommon.attributes & FAT_ATTR_HIDDEN) || (query->Attributes & FAT_ATTR_HIDDEN)) &&
				(!(query->current_entry_raw->uEntry.sFatRawCommon.attributes & FAT_ATTR_SYSTEM) || (query->Attributes & FAT_ATTR_SYSTEM)) &&
				(!(query->current_entry_raw->uEntry.sFatRawCommon.attributes & FAT_ATTR_VOLUME_ID) || (query->Attributes & FAT_ATTR_VOLUME_ID)) &&
				(!(query->current_entry_raw->uEntry.sFatRawCommon.attributes & FAT_ATTR_LONG_NAME) || (query->Attributes & FAT_ATTR_LONG_NAME));
		}
	}
	/*
	// repeat the process until we find a valid entry
	// that matches the attributes given
	*/
	while (!pass || *query->current_entry_raw->uEntry.sFatRawCommon.name == 0xE5);
	/*
	// if we found an entry we need to check it's LFN checksum
	// to make sure that the long filename that we've associated
	// with it belongs to it. If it doesn't clear it.
	*/
	if (*query->current_entry_raw->uEntry.sFatRawCommon.name != 0x0)
	{
		if (query->lfn_checksum != fat_long_entry_checksum((uint8*)query->current_entry_raw->uEntry.sFatRawCommon.name))
		{
			query->long_filename[0] = 0x0;
		}
	}
	/*
	// if this entry doesn't have an LFN entry but its marked as having
	// a lowercase name or extension then fill the long filename with the
	// lowercase version
	*/
	if (query->long_filename[0] == 0 &&
		(query->current_entry_raw->uEntry.sFatRawCommon.reserved & (FAT_LOWERCASE_EXTENSION | FAT_LOWERCASE_BASENAME)))
	{
		int i = 0;
		for (uint16 uiResult = 0; uiResult < 8; uiResult++)
		{
			if (query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult] != 0x20)
			{
				if (query->current_entry_raw->uEntry.sFatRawCommon.reserved & FAT_LOWERCASE_BASENAME)
				{
					query->long_filename[i++] = tolower(query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult]);
				}
				else
				{
					query->long_filename[i++] = query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult];
				}
			}
		}

		if (query->current_entry_raw->uEntry.sFatRawCommon.name[8] != 0x20)
		{
			query->long_filename[i++] = '.';

			for (uint16 uiResult = 8; uiResult < 11; uiResult++)
			{
				if (query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult] != 0x20)
				{
					if (query->current_entry_raw->uEntry.sFatRawCommon.reserved & FAT_LOWERCASE_EXTENSION)
					{
						query->long_filename[i++] = tolower(query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult]);
					}
					else
					{
						query->long_filename[i++] = query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult];
					}
				}
			}
		}
		query->long_filename[i] = 0x0;
	}

	return FAT_SUCCESS;
}

// creates a FAT directory entry
uint16 fat_create_directory_entry(SFatVolume* volume, SFatRawDirectoryEntry* parent, char* name, uint8 attribs, uint32 entry_cluster, SFatDirectoryEntry* new_entry)
{
	uint16							uiResult;
	int16							char_index;
	uint16							illegal_char;
	uint16							entries_count = 0;
	uint32							sector;
	uint32							first_sector_of_cluster = 0;
	uintptr_t						last_entry_address;
	FAT_ENTRY						fat;
	FAT_ENTRY						last_fat;
	SFatRawDirectoryEntry*		parent_entry;

	int								no_of_lfn_entries_needed = 0;
	int								no_of_lfn_entries_found;
	char							lfn_checksum;

	uint8* buffer = fat_shared_buffer;

	/*
	// get the length of the filename
	*/
	uiResult = (uint16)strlen(name);

	/*
	// check that the character is a valid 8.3 filename, the
	// file is invalid if:
	//
	//	- name part is more than 8 chars (char_index > 8)
	//	- extension part is more than 3 (uiResult - char_index > 4)
	//	- it has more than one dot (indexof('.', name, char_index + 1) >= 0)
	*/
	if (uiResult > 255)
	{
		return FAT_FILENAME_TOO_LONG;
	}

	/*
	// all names are also invalid if they start or end with
	// a dot
	*/
	char_index = indexof('.', name, 0x0);

	if (char_index == 0 || char_index == (uiResult - 1))
	{
		return FAT_INVALID_FILENAME;
	}

	for (char_index = 0x0; char_index < uiResult; char_index++)
	{
		/*
		// if the character is less than 0x20 with the
		// exception of 0x5 then the filename is illegal
		*/
		if (name[char_index] < 0x1F)
		{
			return FAT_ILLEGAL_FILENAME;
		}

		/*
		// compare the character with a table of illegal
		// characters, if a match is found then the filename
		// is illegal
		*/
		for (illegal_char = 0x0; illegal_char < ILLEGAL_CHARS_COUNT; illegal_char++)
		{
			if (name[char_index] == ILLEGAL_CHARS[illegal_char] && name[char_index] != '.')
			{
				return FAT_ILLEGAL_FILENAME;
			}
		}
	}

	// initialize the raw entry
	// todo: check if no other functions are initializing
	// new_entry and initialize the whole thing
	memset(&new_entry->raw, 0, sizeof(new_entry->raw));
	/*
	// attempt to format the filename provided
	// to the format required by the directory entry
	// and copy it to it's field
	*/
	uiResult = get_short_name_for_entry(new_entry->raw.uEntry.sFatRawCommon.name, (uint8*)name, 0);
	/*
	// if the above operation failed then the filename
	// is invalid
	*/
	if (uiResult != FAT_SUCCESS && uiResult != FAT_LFN_GENERATED)
	{
		return FAT_INVALID_FILENAME;
	}
	/*
	// if this is going to be an lfn entry we need to make
	// sure that the short filename is available
	*/
	if (uiResult == FAT_LFN_GENERATED)
	{
		SFatQueryState query;
		uint16 name_suffix = 0;
		char is_valid_entry;
		char name_suffix_str[6];
		uint8 i, c;

		do
		{
			is_valid_entry = 1;

			memset(&query, 0, sizeof(query));
			query.buffer = query.buff;
			uiResult = fat_query_first_entry(volume, parent, 0, &query, 0);
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}

			sprintf(name_suffix_str, "~%i", name_suffix);

			for (i = 0; i < 8 - (char)strlen(name_suffix_str); i++)
				if (new_entry->raw.uEntry.sFatRawCommon.name[i] == 0x20)
					break;

			for (c = 0; c < (char)strlen(name_suffix_str); c++)
				new_entry->raw.uEntry.sFatRawCommon.name[i++] = name_suffix_str[c];
			/*
			// loop through all entries in the parent directory
			// and if we find one with the same name as hours mark the name
			// as invalid
			*/
			while (*query.current_entry_raw->uEntry.sFatRawCommon.name != 0)
			{
				if (memcmp(query.current_entry_raw->uEntry.sFatRawCommon.name, new_entry->raw.uEntry.sFatRawCommon.name, 11) == 0)
				{
					is_valid_entry = 0;
					break;
				}
				uiResult = fat_query_next_entry(volume, &query, 0, 0);
				if (uiResult != FAT_SUCCESS)
				{
					return uiResult;
				}
			}
			/*
			// if the filename is taken we need to compute a new one
			*/
			if (!is_valid_entry)
			{
				/*
				// create the filename suffix and append it after
				// the last char or replace the end of the filename
				// with it.
				*/
				sprintf(name_suffix_str, "~%i", name_suffix++);

				for (i = 0; i < 8 - (char)strlen(name_suffix_str); i++)
					if (new_entry->raw.uEntry.sFatRawCommon.name[i] == 0x20)
						break;

				for (c = 0; c < (char)strlen(name_suffix_str); c++)
					new_entry->raw.uEntry.sFatRawCommon.name[i++] = name_suffix_str[c];
			}
		} while (!is_valid_entry);
		/*
		// calculate the # of entries needed to store the lfn
		// including the actual entry
		*/
		no_of_lfn_entries_needed = ((strlen(name) + 12) / 13) + 1;
		no_of_lfn_entries_found = 0;
	}

	/*
	// if the new entry is a directory and no cluster was supplied
	// by the calling function then allocate a new cluster
	*/
	if (entry_cluster == 0 && (attribs & FAT_ATTR_DIRECTORY))
	{
		entry_cluster = fat_allocate_directory_cluster(volume, parent, &uiResult);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}
	}

	// set the entry attributes
	strcpy((char*)new_entry->name, name);
	new_entry->attributes = attribs;
	new_entry->size = 0x0;
	new_entry->raw.uEntry.sFatRawCommon.attributes = attribs;
	new_entry->raw.uEntry.sFatRawCommon.reserved = 0;
	new_entry->raw.uEntry.sFatRawCommon.size = 0x0;
	new_entry->raw.uEntry.sFatRawCommon.first_cluster_lo = LO16(entry_cluster);
	new_entry->raw.uEntry.sFatRawCommon.first_cluster_hi = HI16(entry_cluster);
	new_entry->raw.uEntry.sFatRawCommon.create_time_tenth = 0x0;
	new_entry->raw.uEntry.sFatRawCommon.create_date = rtc_get_fat_date();
	new_entry->raw.uEntry.sFatRawCommon.create_time = rtc_get_fat_time();
	new_entry->raw.uEntry.sFatRawCommon.modify_date = new_entry->raw.uEntry.sFatRawCommon.create_date;
	new_entry->raw.uEntry.sFatRawCommon.modify_time = new_entry->raw.uEntry.sFatRawCommon.create_time;
	new_entry->raw.uEntry.sFatRawCommon.access_date = new_entry->raw.uEntry.sFatRawCommon.create_date;
	new_entry->create_time = fat_decode_date_time(new_entry->raw.uEntry.sFatRawCommon.create_date, new_entry->raw.uEntry.sFatRawCommon.create_time);
	new_entry->modify_time = fat_decode_date_time(new_entry->raw.uEntry.sFatRawCommon.modify_date, new_entry->raw.uEntry.sFatRawCommon.modify_time);
	new_entry->access_time = fat_decode_date_time(new_entry->raw.uEntry.sFatRawCommon.access_date, 0);

	/*
	// there's no fat entry that points to the 1st cluster of
	// a directory's cluster chain but we'll create a
	// fake fat entry from the 1st cluster data on the
	// directory entry so that we can handle the 1st
	// cluster with the same code as all other clusters
	// in the chain
	*/
	if (parent && (parent->uEntry.sFatRawCommon.first_cluster_lo != 0x0 || parent->uEntry.sFatRawCommon.first_cluster_hi != 0x0))
	{
		/*
		// read the low word of the cluster address
		// and read the high word of the 1st cluster address
		// ONLY if the file system type is FAT32
		*/
		((uint16*)&fat)[INT32_WORD0] = parent->uEntry.sFatRawCommon.first_cluster_lo;
		((uint16*)&fat)[INT32_WORD1] = (volume->fs_type == FAT_FS_TYPE_FAT32) ? parent->uEntry.sFatRawCommon.first_cluster_hi : 0x0;
	}
	/*
	// if no parent was specified then we create
	// the fake fat entry from the root directory's
	// cluster address found on the volume structure
	*/
	else
	{
		if (volume->fs_type == FAT_FS_TYPE_FAT32)
		{
			fat = volume->root_cluster;
		}
		else
		{
			fat = last_fat = 0x0;
			first_sector_of_cluster =
				volume->no_of_reserved_sectors + (volume->no_of_fat_tables * volume->uiFatSize);
		}
	}

	/*
	// mark the cached sector as unknown
	*/
	fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);
	/*
	// for each cluster allocated to the parent
	// directory entry
	*/
	do
	{
		/*
		// calculate the address of the 1st sector
		// of the cluster - skip this step if uiResult equals
		// 1, this means that this is the 1st sector of the
		// root entry which doesn't start at the beggining
		// of the cluster
		*/
		if (fat != 0x0)
		{
			first_sector_of_cluster = FIRST_SECTOR_OF_CLUSTER(volume, fat);
		}
		/*
		// set the current sector to the first
		// sector of the cluster
		*/
		sector = first_sector_of_cluster;
		/*
		// calculate the address of the last directory
		// entry on a sector when the sector is loaded
		// into sec_buff
		*/
		last_entry_address =
			((uintptr_t)buffer + volume->no_of_bytes_per_serctor) - 0x20;
		/*
		// for each sector in the cluster
		*/
		while (fat == 0 || sector < (first_sector_of_cluster + volume->no_of_sectors_per_cluster))
		{
			/*
			// read the current sector to RAM
			*/
			bool bSuccess;
			bSuccess = volume->device->Read(sector, buffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_READ_MEDIA;
			}
			/*
			// set the parent entry pointer to the 1st
			// entry of the current sector
			*/
			parent_entry = (SFatRawDirectoryEntry*)buffer;

			/*
			// for each directory entry in the sector...
			*/
			while ((uintptr_t)parent_entry <= last_entry_address)
			{
				/*
				// make sure we don't exceed the limit of 0xFFFF entries
				// per directory
				*/
				if ((entries_count + no_of_lfn_entries_needed) == 0xFFFF)
				{
					return FAT_DIRECTORY_LIMIT_EXCEEDED;
				}
				/*
				// increase the count of directory entries
				*/
				entries_count++;
				/*
				// if the directory entry is free
				*/
				if (IS_FREE_DIRECTORY_ENTRY(parent_entry))
				{
					/*
					// we've found a free entry
					*/
					no_of_lfn_entries_found++;
					/*
					// if this is the last directory entry or if we've
					// found all the entries that we need then let's get
					// ready to write them
					*/
					if (IS_LAST_DIRECTORY_ENTRY(parent_entry) || no_of_lfn_entries_found == no_of_lfn_entries_needed)
					{
						/*
						// if there where any free entries before this
						// one then we need to rewind a bit
						*/
						while (no_of_lfn_entries_found-- > 1)
						{
							if ((uintptr_t)parent_entry > (uintptr_t)buffer)
							{
								parent_entry--;
							}
							else
							{
								/*
								// if the last entry is on the same cluster we
								// can just decrease the sector number, otherwise we
								// need to get the sector address for the last cluster
								*/
								if (sector > first_sector_of_cluster)
								{
									sector--;
								}
								else
								{
									if (last_fat == 0)
									{
										first_sector_of_cluster =
											volume->no_of_reserved_sectors + (volume->no_of_fat_tables * volume->uiFatSize);
									}
									else
									{
										fat = last_fat;
										first_sector_of_cluster = FIRST_SECTOR_OF_CLUSTER(volume, fat);
									}
									sector = first_sector_of_cluster + volume->no_of_sectors_per_cluster;
								}

								/*
								// read the last sector to the cache, calculate the last
								// entry address and set our pointer to it
								*/
								bSuccess = volume->device->Read(sector, buffer);
								if (!bSuccess)
								{
									return FAT_CANNOT_READ_MEDIA;
								}
								last_entry_address = ((uintptr_t)buffer + volume->no_of_bytes_per_serctor) - 0x20;
								parent_entry = (SFatRawDirectoryEntry*)last_entry_address;
							}
						}
						/*
						// compute the checksum for this entry
						*/
						lfn_checksum = fat_long_entry_checksum((uint8*)new_entry->raw.uEntry.sFatRawCommon.name);
						/*
						// now we can start writting
						*/
						no_of_lfn_entries_found = no_of_lfn_entries_needed;
						while (no_of_lfn_entries_found--)
						{
							if (no_of_lfn_entries_found)
							{
								uint16 i, c;
								/*
								// set the required fields for this entry
								*/
								parent_entry->uEntry.sFatRawLongFileName.lfn_sequence = (uint8)no_of_lfn_entries_found;
								parent_entry->uEntry.sFatRawLongFileName.lfn_checksum = lfn_checksum;
								parent_entry->uEntry.sFatRawCommon.attributes = FAT_ATTR_LONG_NAME;
								parent_entry->uEntry.sFatRawLongFileName.lfn_first_cluster = 0;
								parent_entry->uEntry.sFatRawLongFileName.lfn_type = 0;
								/*
								// mark entry as the 1st entry if it is so
								*/
								if (no_of_lfn_entries_found == no_of_lfn_entries_needed - 1)
									parent_entry->uEntry.sFatRawLongFileName.lfn_sequence = parent_entry->uEntry.sFatRawLongFileName.lfn_sequence | FAT_FIRST_LFN_ENTRY;
								/*
								// copy the lfn chars
								*/
								c = (uint16)strlen(name);
								i = ((no_of_lfn_entries_found - 1) * 13);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x0] = LO8((i + 0x0 > c) ? 0xFFFF : (uint16)name[i + 0x0]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x1] = HI8((i + 0x0 > c) ? 0xFFFF : (uint16)name[i + 0x0]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x2] = LO8((i + 0x1 > c) ? 0xFFFF : (uint16)name[i + 0x1]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x3] = HI8((i + 0x1 > c) ? 0xFFFF : (uint16)name[i + 0x1]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x4] = LO8((i + 0x2 > c) ? 0xFFFF : (uint16)name[i + 0x2]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x5] = HI8((i + 0x2 > c) ? 0xFFFF : (uint16)name[i + 0x2]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x6] = LO8((i + 0x3 > c) ? 0xFFFF : (uint16)name[i + 0x3]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x7] = HI8((i + 0x3 > c) ? 0xFFFF : (uint16)name[i + 0x3]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x8] = LO8((i + 0x4 > c) ? 0xFFFF : (uint16)name[i + 0x4]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x9] = HI8((i + 0x4 > c) ? 0xFFFF : (uint16)name[i + 0x4]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x0] = LO8((i + 0x5 > c) ? 0xFFFF : (uint16)name[i + 0x5]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x1] = HI8((i + 0x5 > c) ? 0xFFFF : (uint16)name[i + 0x5]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x2] = LO8((i + 0x6 > c) ? 0xFFFF : (uint16)name[i + 0x6]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x3] = HI8((i + 0x6 > c) ? 0xFFFF : (uint16)name[i + 0x6]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x4] = LO8((i + 0x7 > c) ? 0xFFFF : (uint16)name[i + 0x7]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x5] = HI8((i + 0x7 > c) ? 0xFFFF : (uint16)name[i + 0x7]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x6] = LO8((i + 0x8 > c) ? 0xFFFF : (uint16)name[i + 0x8]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x7] = HI8((i + 0x8 > c) ? 0xFFFF : (uint16)name[i + 0x8]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x8] = LO8((i + 0x9 > c) ? 0xFFFF : (uint16)name[i + 0x9]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x9] = HI8((i + 0x9 > c) ? 0xFFFF : (uint16)name[i + 0x9]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0xA] = LO8((i + 0xA > c) ? 0xFFFF : (uint16)name[i + 0xA]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0xB] = HI8((i + 0xA > c) ? 0xFFFF : (uint16)name[i + 0xA]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_3[0x0] = LO8((i + 0xB > c) ? 0xFFFF : (uint16)name[i + 0xB]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_3[0x1] = HI8((i + 0xB > c) ? 0xFFFF : (uint16)name[i + 0xB]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_3[0x2] = LO8((i + 0xC > c) ? 0xFFFF : (uint16)name[i + 0xC]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_3[0x3] = HI8((i + 0xC > c) ? 0xFFFF : (uint16)name[i + 0xC]);

								/*
								// continue to next entry
								*/
								if ((uintptr_t)parent_entry < (uintptr_t)last_entry_address)
								{
									parent_entry++;
								}
								else
								{
									/*
									// flush this sector to the storage device and
									// load the next sector
									*/
									bSuccess = volume->device->Write(sector, buffer);
									if (!bSuccess)
									{
										return FAT_CANNOT_WRITE_MEDIA;
									}

									if (fat == 0 || sector < first_sector_of_cluster + volume->no_of_sectors_per_cluster - 1)
									{
										sector++;
										/*
										// make sure that we don't overflow the root directory
										// on FAT12/16 volumes
										*/
										if (!fat)
										{
											if (sector > first_sector_of_cluster + volume->root_directory_sectors)
											{
												return FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED;
											}
										}
									}
									else
									{
										/*
										// get the next cluster, we'll remember the last one so
										// we can update it bellow if it's the eof cluster
										*/
										last_fat = fat;
										uiResult = fat_get_cluster_entry(volume, fat, &fat);
										if (uiResult != FAT_SUCCESS)
										{
											return uiResult;
										}
										/*
										// if this is the end of the FAT chain allocate
										// a new cluster to this folder and continue
										*/
										if (fat_is_eof_entry(volume, fat))
										{
											FAT_ENTRY newfat = fat_allocate_data_cluster(volume, 1, 1, &uiResult);
											if (uiResult != FAT_SUCCESS)
											{
												return uiResult;
											}

											uiResult = fat_set_cluster_entry(volume, last_fat, newfat);
											if (uiResult != FAT_SUCCESS)
											{
												return uiResult;
											}
											fat = newfat;
										}
										/*
										// mark the loaded sector as unknown
										*/
										fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);
										/*
										// continue working on the new cluster
										*/
										sector = FIRST_SECTOR_OF_CLUSTER(volume, fat);
									}
									/*
									// load the next sector
									*/
									bSuccess = volume->device->Read(sector, buffer);
									if (!bSuccess)
									{
										return FAT_CANNOT_READ_MEDIA;
									}
									parent_entry = (SFatRawDirectoryEntry*)buffer;
								}
							}
							else
							{
								*parent_entry = new_entry->raw;
							}
						}
						/*
						// flush this sector to the storage device and
						// load the next sector
						*/
						bSuccess = volume->device->Write(sector, buffer);
						if (!bSuccess)
						{
							return FAT_CANNOT_WRITE_MEDIA;
						}
						new_entry->sector_addr = sector;
						new_entry->sector_offset = (uintptr_t)parent_entry - (uintptr_t)buffer;
						/*
						// we're done!!!!!
						*/
						return FAT_SUCCESS;
					}
				}
				else
				{
					no_of_lfn_entries_found = 0;
				}

				/*
				// move the parent entry pointer to
				// the next entry in the sector
				*/
				parent_entry++;
			}

			/*
			// move to the next sector in the cluster
			*/
			sector++;

			/*
			// make sure that we don't overflow the root directory
			// on FAT12/16 volumes
			*/
			if (!fat)
			{
				if (sector > first_sector_of_cluster + volume->root_directory_sectors)
				{
					return FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED;
				}
			}
		}

		/*
		// get the next cluster, we'll remember the last one in
		// case we need to rewind to write lfn entries
		*/
		last_fat = fat;
		uiResult = fat_get_cluster_entry(volume, fat, &fat);
		if (uiResult != FAT_SUCCESS)
			return uiResult;
		/*
		// if this is the end of the FAT chain allocate
		// a new cluster to this folder and continue
		*/
		if (fat_is_eof_entry(volume, fat))
		{
			FAT_ENTRY newfat;
			/*
			// allocate the cluster
			*/
			newfat = fat_allocate_data_cluster(volume, 1, 1, &uiResult);
			if (uiResult != FAT_SUCCESS)
				return uiResult;
			/*
			// link it to the cluster chain
			*/
			uiResult = fat_set_cluster_entry(volume, last_fat, newfat);
			if (uiResult != FAT_SUCCESS)
				return uiResult;
			fat = newfat;
		}

		/*
		// mark the loaded sector as unknown
		*/
		fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);
	} while (1);
}


// converts a 8.3 filename from the internal
// filesystem format to the user friendly convention
void fat_get_short_name_from_entry(uint8* dest, const uint8* src)
{
	/*
	// copy the 1st character - 0xE5 is used on the
	// 1st character of the entry to indicate an unused
	// entry but it is also a valid KANJI lead byte for the
	// character set used in Japan. The special value 0x5 is
	// used so that this special file name case for Japan
	// can be handled properly and not cause the FAT code
	// to think that the entry is free.
	*/
	if (*src != 0x5)
		*dest++ = *src++;
	else *dest++ = 0xE5;
	/*
	// if there's a second character...
	*/
	if (*src != 0x20)
	{
		*dest++ = *src++;
		if (*src != 0x20)
		{
			*dest++ = *src++;
			if (*src != 0x20)
			{
				*dest++ = *src++;
				if (*src != 0x20)
				{
					*dest++ = *src++;
					if (*src != 0x20)
					{
						*dest++ = *src++;
						if (*src != 0x20)
						{
							*dest++ = *src++;
							if (*src != 0x20)
							{
								*dest++ = *src++;
							}
							else src++;
						}
						else src += 0x2;
					}
					else src += 0x3;
				}
				else src += 0x4;
			}
			else src += 0x5;
		}
		else src += 0x6;
	}
	else src += 0x7;
	/*
	// if there's an extension append it to
	// the output
	*/
	if (*src != 0x20)
	{
		*dest++ = '.';
		*dest++ = *src++;
		if (*src != 0x20)
		{
			*dest++ = *src++;
			if (*src != 0x20)
			{
				*dest++ = *src;
			}
		}
	}
	*dest = 0x0;
}

/*
// converts the filename to it's Unicode UTF16 representation
*/
char get_long_name_for_entry(uint16* dst, uint8* src)
{
	register int i;
	for (i = 0; i < (int)strlen((char*)src); i++)
	{
		dst[i] = (uint16)src[i];
	}
	dst[i] = 0x0;
	/*
	// todo: check that this is a valid filename
	// and that it only uses ASCII chars since we don't
	// support unicode at this time
	*/
	return FAT_SUCCESS;
}


/*
// compares two short names after they
// have been formatted by get_short_name_for_entry
// returns 1 if both name are equal and 0 otherwise
*/
char fat_compare_short_name(uint8* name1, uint8* name2)
{
	return memcmp(name1, name2, 11) == 0;
}

/*
// performs an ASCII comparison on two UTF16 strings
*/
char fat_compare_long_name(uint16* name1, uint16* name2)
{
	register short i;
	for (i = 0; i < 256; i++)
	{
		if (toupper((char)name1[i]) != toupper((char)name2[i]))
			return 0;
		if ((char)name1[i] == 0x0)
			return 1;
	}
	return 1;
}


/*
// converts an 8.3 filename to the format required
// by the FAT directory entry structure
*/
uint16 get_short_name_for_entry(uint8* dest, uint8* src, char lfn_disabled)
{

	char tmp[13];
	char has_uppercase = 0;
	uint16 dot_index;
	uint16 length;
	uint16 i;

	/*
	// check that the name is actually a long filename
	// before processing it as such
	*/
	if (!lfn_disabled)
	{
		uint8 c;
		char is_lfn = 0;
		length = (uint16)strlen((char*)src);
		dot_index = indexof('.', (char*)src, 0x0);
		/*
		// if the file hs no extension and is longer than 8 chars
		// or if the name part has more than 8 chars or the extension more than 8
		// or if it has more than one dot then we need to handle it as a lfn
		*/
		if (dot_index < 0 && length > 8) is_lfn = 1;
		if (dot_index >= 0)
		{
			if (dot_index > 7 || (length - dot_index/*, 0*/) > 4) is_lfn = 1;
			if (dot_index >= 0) if (indexof('.', (char*)src, 1) >= 0) is_lfn = 1;
		}
		else
		{
			if (length > 8) is_lfn = 1;
		}
		/*
		// if it has spaces or lowercase letters we must also
		// handle it as a long filename
		*/
		if (!is_lfn)
		{
			for (i = 0; i < length; i++)
			{
				if (src[i] == 0x20 || src[i] != toupper(src[i]))
				{
					is_lfn = 1;
					break;
				}
			}
		}
		if (is_lfn)
		{
			/*
			// first we find the location of the LAST dot
			*/
			dot_index = length;
			for (i = length - 1; i; i--)
			{
				if (src[i] == '.')
				{
					dot_index = i;
					break;
				}
			}
			/*
			// now we copy the first 8 chars of the filename
			// excluding dots and spaces and we pad it with
			// spaces
			*/
			c = 0;
			for (i = 0; i < 8; i++)
			{
				while (c < dot_index)
				{
					if (src[c] == 0x20 || src[c] == '.')
					{
						c++;
					}
					else
					{
						break;
					}
				}
				if (c < dot_index)
				{
					tmp[i] = toupper(src[c++]);
				}
				else
				{
					tmp[i] = 0x20;
				}
			}
			/*
			// do the same for the extension
			*/
			c = dot_index + 1;
			for (i = 8; i < 11; i++)
			{
				while (c < length)
				{
					if (src[c] == 0x20 || src[c] == '.')
					{
						c++;
					}
					else
					{
						break;
					}
				}
				if (c < length)
				{
					tmp[i] = toupper(src[c++]);
				}
				else
				{
					tmp[i] = 0x20;
				}
			}
			/*
			// now we copy it to the callers buffer and we're done
			*/
			for (i = 0; i < 11; i++)
				*dest++ = tmp[i];
			/*
			// return special code so the caller knows
			// to store the long name
			*/
			return FAT_LFN_GENERATED;
		}
	}

	/*
	// trim-off spaces - if the result is
	// greater than 12 it will return an empty
	// string
	*/
	strtrim(tmp, (char*)src, 12);
	/*
	// if the name length was invalid return
	// error code
	*/
	if (*tmp == 0 || strlen(tmp) > 12)
		return FAT_INVALID_FILENAME;
	/*
	// find the location of the dot
	*/
	dot_index = (uintptr_t)strchr(tmp, (int)'.');

	/*
	// strchr gave us the address of the dot, we now
	// convert it to a 1-based index
	*/
	if (dot_index)
		dot_index -= (uintptr_t)tmp - 0x1;
	/*
	// get the length of the input string
	*/
	length = (uint16)strlen(tmp);
	/*
	// check that this is a valid 8.3 filename
	*/
	if ((length > 0x9 &&
		(dot_index == 0x0 || (dot_index) > 0x9)) ||
		(dot_index > 0x0 && (length - dot_index) > 0x5))
		return FAT_INVALID_FILENAME;
	/*
	// copy the 1st part of the filename to the
	// destination buffer
	*/
	for (i = 0x0; i < 0x8; i++)
	{
		if (dot_index == 0x0)
		{
			if (i < length)
			{
				if (lfn_disabled && (tmp[i] != toupper(tmp[i])))
					has_uppercase = 1;

				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
		else
		{
			if (i < dot_index - 0x1)
			{
				if (lfn_disabled && (tmp[i] != toupper(tmp[i])))
					has_uppercase = 1;

				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
	}
	/*
	// if there's not extension fill the extension
	// characters with spaces
	*/
	if (dot_index == 0x0)
	{
		for (i = 0x0; i < 0x3; i++)
			*dest++ = 0x20;
	}
	/*
	// if there is an extension...
	*/
	else
	{
		/*
		// copy the extension characters to the
		// destination buffer
		*/
		for (i = dot_index; i < dot_index + 0x3; i++)
		{
			if (i < length)
			{
				if (lfn_disabled && (tmp[i] != toupper(tmp[i])))
					has_uppercase = 1;
				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
	}
	/*
	// return success code
	*/
	return has_uppercase ? FAT_INVALID_FILENAME : FAT_SUCCESS;
}


/*
// computes the short filename checksum
*/
uint8 fat_long_entry_checksum(uint8* filename)
{
	uint16 len;
	uint8 sum = 0;
	for (len = 11; len != 0; len--)
	{
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *filename++;
	}
	return sum;
}


int indexof(char chr, char* str, int index)
{
	int i = 0;
	str = str + index;

	do
	{
		if (str[i] == chr)
		{
			return i;
		}
		i++;
	} while (str[i]);

	return -1;
}


uint16 rtc_get_fat_date()
{
#if defined(FAT_USE_SYSTEM_TIME)
	time_t now;
	struct tm* timeinfo;
	time(&now);
	timeinfo = localtime(&now);
	return FAT_ENCODE_DATE(timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_year + 1900);
#else
	if (timekeeper.fat_get_system_time)
	{
		time_t now = timekeeper.fat_get_system_time();
		struct tm* timeinfo = localtime(&now);
		return FAT_ENCODE_DATE(timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_year + 1900);
	}
	else
	{
		return FAT_ENCODE_DATE(8, 2, 1985);
	}
#endif
}


uint16 rtc_get_fat_time()
{
#if defined(FAT_USE_SYSTEM_TIME)
	time_t now;
	struct tm* timeinfo;
	time(&now);
	timeinfo = localtime(&now);
	return FAT_ENCODE_TIME(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
#else
	if (timekeeper.fat_get_system_time)
	{
		time_t now;
		struct tm* timeinfo;
		now = timekeeper.fat_get_system_time();
		timeinfo = localtime(&now);
		return FAT_ENCODE_TIME(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	}
	else
	{
		return FAT_ENCODE_TIME(12, 5, 35);
	}
#endif
}


time_t fat_decode_date_time(uint16 date, uint16 time) 
{
	//YYYYYYYMMMMDDDDD
	//0000000000011111
	//0000000111100000
	//1111111000000000
	//HHHHHMMMMMMSSSSS
	//0000000000011111
	//0000011111100000
	//1111100000000000
	//struct tm datetime;
	//time_t now;

	struct tm tm;
	tm.tm_year = ((date) >> 9) + 80;
	tm.tm_mon = (((date) & 0x1E0) >> 5) - 1;
	tm.tm_mday = (date) & 0x1F;
	tm.tm_hour = (time) >> 11;
	tm.tm_min = ((time) & 0x7E0) >> 5;
	tm.tm_sec = ((time) & 0x1F) << 1;
	return mktime(&tm);
}


// treams leading and trailing spaces. If the result
// exceeds the max length the destination will be set
// to an empty string
// todo: clean it up a bit
void strtrim(char* dest, char* src, size_t max) {

	uint32 max_length;
	uint32 lead_spaces = 0x0;
	uint32 last_char = 0x0;
	uint32 i;
	char* dst = dest;

	max_length = strlen(src);
	/*
	// count the lead spaces
	*/
	for (i = 0; i < max_length && src[i] == 0x20; i++)
		lead_spaces++;
	/*
	// if the whole string is full of spaces
	// return an empty string
	*/
	if (max_length == lead_spaces) {
		*dest = 0x0;
		return;
	}
	/*
	// calculate the index of the last non-space
	// character
	*/
	for (last_char = max_length - 1;
		last_char > 0 && (src[last_char] == 0x20); last_char--);

	/*
	// copy the non-space characters to the
	// destination buffer
	*/
	for (i = lead_spaces; i <= last_char; i++)
	{
		*dest++ = src[i];
		if (!max--)
		{
			*dst = 0x0;
			return;
		}
	}
	/*
	// set the null terminator
	*/
	*dest = 0x0;
}

void fat_parse_path(char* path, char* path_part, char** filename_part)
{
	*filename_part = path + strlen(path);
	while (*--(*filename_part) != '\\' && (*filename_part) != path);

	while (path != *filename_part)
		*path_part++ = *path++;
	*path_part = 0;
	(*filename_part)++;
}

