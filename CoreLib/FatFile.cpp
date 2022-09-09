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
#include <stdio.h>
#include <string.h>
#include "Fat.h"
#include "FatInternals.h"


uint16 fat_file_update_sequential_cluster_count(SFatFile* file);


uint16 fat_file_update_sequential_cluster_count(SFatFile* handle)
{
	uint32 current_cluster;
	uint32 next_cluster;

	// find out how many clusters are allocated sequentially
	// to this file following the current cursor location
	handle->no_of_sequential_clusters = 0;
	current_cluster = handle->current_clus_addr;

	while (!handle->volume->FatIsEOFEntry(handle->volume, current_cluster))
	{
		handle->volume->FatGetClusterEntry(handle->volume, current_cluster, &next_cluster);

		if (next_cluster == (current_cluster + 1))
		{
			handle->no_of_sequential_clusters++;
			current_cluster = next_cluster;
			if (handle->no_of_sequential_clusters == 0xFFFF)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	return FAT_SUCCESS;
}

// opens a file
uint16 fat_file_open(CFatVolume* volume, char* filename, uint8 access_flags, SFatFile* handle)
{
	uint16					uiResult;
	SFatDirectoryEntry		file_entry;

	// use the internal buffer
	if (!(access_flags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		handle->buffer = handle->buffer_internal;
	}

	// check that the user supplied a filename
	if (filename == 0 || strlen(filename) > FAT_MAX_PATH)
	{
		return FAT_INVALID_FILENAME;
	}

	// get the file entry
	uiResult = volume->FatGetFileEntry(volume, filename, &file_entry);

	// if we could not get the file entry then return
	// the error that we received from FatGetFileEntry
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if the entry was not found...
	if (*file_entry.name == 0)
	{
		// if the create access flag was set then
		// we create the file
		if (access_flags & FAT_FILE_ACCESS_CREATE)
		{
			// allocate memory for the file path
			ptrdiff_t			path_len;
			char*				filename_scanner;
			char				file_path[FAT_MAX_PATH + 1];
			SFatDirectoryEntry	parent_entry;

			// get the name of the file path including
			// the filename
			path_len = strlen(filename);

			// set the pointer that will be used to scan
			// filename to the end of the filename
			filename_scanner = filename + (path_len - 0x1);

			// if the filename ends with a backslash then it
			// is an invalid filename ( it's actually a directory
			// path )
			if (*filename_scanner == BACKSLASH)
			{
				return FAT_INVALID_FILENAME;
			}

			// scan the filename starting at the end until
			// a backslash is found - when we exit this loop
			// path_scanner will point to the last character
			// of the filepath
			for (;;)
			{
				if (*filename_scanner == BACKSLASH)
				{
					break;
				}

				if (filename_scanner == filename)
				{
					return FAT_INVALID_PATH;
				}
				filename_scanner--;
			}

			// calculate the length of the path part of the
			// filename
			path_len = filename_scanner - filename;

			// copy the path part of the filename to
			// the file_path buffer
			memcpy(file_path, filename, path_len);

			// set the null terminator of the file_path buffer
			file_path[path_len] = 0x0;

			// increase pointer to the beggining of the filename
			// part of the path
			filename_scanner++;

			// try to get the entry for the parent directory
			uiResult = volume->FatGetFileEntry(volume, file_path, &parent_entry);

			// if FatGetFileEntry returned an error
			// then we return the error code to the calling
			// function
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}

			// if the parent directory does not exists
			if (*parent_entry.name == 0)
			{
				return FAT_DIRECTORY_DOES_NOT_EXIST;
			}

			// try to create the directory entry
			uiResult = volume->FatCreateDirectoryEntry(volume, &parent_entry.raw, filename_scanner, 0, 0, &file_entry);

			// make sure the file is opened with no append flags
			// todo: figure out why we need this and fix it
			access_flags = access_flags & (0xFF ^ FAT_FILE_ACCESS_APPEND);

			// if we were unable to create the entry for the file
			// and
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}

		}
		// if the create flag is not set then return the
		// file not found error code
		else
		{
			return FAT_FILE_NOT_FOUND;
		}
	}

	uiResult = fat_open_file_by_entry(volume, &file_entry, handle, access_flags);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if the file has no clusters allocated then allocate one
	if (handle->access_flags & FAT_FILE_ACCESS_WRITE)
	{
		fat_file_update_sequential_cluster_count(handle);
	}
	return FAT_SUCCESS;
}


/*
// opens a file given a pointer to it's
// directory entry
*/
uint16 fat_open_file_by_entry(CFatVolume* volume, SFatDirectoryEntry* entry, SFatFile* handle, uint8 access_flags)
{
	bool	bSuccess;
	uint16	uiResult;

	// set implicit access flags
	access_flags |= FAT_FILE_ACCESS_READ;
	if (access_flags & (FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_APPEND | FAT_FILE_ACCESS_OVERWRITE))
	{
		access_flags |= FAT_FILE_ACCESS_WRITE;
	}

	// if the user is trying to open a directory then
	// return an error code, otherwise it would corrupt
	// the volume
	if (entry->attributes & FAT_ATTR_DIRECTORY)
	{
		return FAT_NOT_A_FILE;
	}

	// copy the volume handle and the entry's
	// structure to the file handle
	handle->volume = volume;
	handle->directory_entry = *entry;
	handle->current_size = entry->size;
	handle->current_clus_idx = 0;
	handle->access_flags = access_flags;
	handle->magic = FAT_OPEN_HANDLE_MAGIC;
	handle->busy = 0;

	// calculate the # of clusters allocated
	handle->no_of_clusters_after_pos = (entry->size + volume->GetNoOfBytesPerSector() - 1) / volume->GetNoOfBytesPerSector();
	handle->no_of_clusters_after_pos = (handle->no_of_clusters_after_pos + volume->GetNoOfSectorsPerCluster() - 1) / volume->GetNoOfSectorsPerCluster();
	if (handle->no_of_clusters_after_pos)
	{
		handle->no_of_clusters_after_pos--;
	}

	// set the sector cache as dirty, this will
	// indicate that the current sector has not yet
	// been read to other file functions
	handle->buffer_dirty = 1;

	// read the the cluster number
	((uint16*)&handle->current_clus_addr)[INT32_WORD0] = entry->raw.uEntry.sFatRawCommon.first_cluster_lo;
	((uint16*)&handle->current_clus_addr)[INT32_WORD1] = (volume->GetFileSystemType() == FAT_FS_TYPE_FAT32) ? entry->raw.uEntry.sFatRawCommon.first_cluster_hi : 0;

	if (access_flags & FAT_FILE_ACCESS_APPEND)
	{
		// if the file is being opened for append access we
		// seek to the end of the file
		uiResult = fat_file_seek(handle, 0, FAT_SEEK_END);
		if (uiResult != FAT_SUCCESS)
		{
			handle->magic = 0;
			return uiResult;
		}
	}
	else if (access_flags & FAT_FILE_ACCESS_OVERWRITE)
	{
		// if the file is being opened with the OVERWRITE flag we must free all the clusters
		// currently allocated to the file and update it's directory entry to point to cluster 1

		// if the file is not already empty then
		// we'll empty it
		if (entry->raw.uEntry.sFatRawCommon.first_cluster_lo != 0x0 || entry->raw.uEntry.sFatRawCommon.first_cluster_hi != 0x0)
		{
			uint8* buffer = fat_shared_buffer;

			/*
			// update the entry to point to cluster 0
			*/
			entry->raw.uEntry.sFatRawCommon.first_cluster_lo = 0x0;
			entry->raw.uEntry.sFatRawCommon.first_cluster_hi = 0x0;
			entry->raw.uEntry.sFatRawCommon.size = 0x0;
			handle->directory_entry = *entry;
			handle->current_size = entry->size;
			/*
			// mark the cached sector as unknown
			*/
			fat_shared_buffer_sector = FAT_UNKNOWN_SECTOR;
			/*
			// read the sector that contains the entry
			*/
			bSuccess = volume->Read(entry->sector_addr, buffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				handle->magic = 0;
				return FAT_CANNOT_READ_MEDIA;
			}
			// copy the modified file entry to the sector buffer
			memcpy(buffer + entry->sector_offset, &entry->raw, sizeof(SFatRawDirectoryEntry));

			// write the modified entry to the media
			bSuccess = volume->Write(entry->sector_addr, buffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				handle->magic = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}

			// free the clusters allocated to the file
			volume->FatFreeClusterChain(volume, handle->current_clus_addr);
		}

		// reset the file handle to reflect an
		// empty file
		handle->current_size = 0x0;
		handle->current_sector_idx = 0x0;
		handle->current_clus_addr = 0x0;
		handle->current_clus_idx = 0;
		handle->buffer_head = handle->buffer;
		handle->no_of_clusters_after_pos = 0;
	}
	else
	{
		// if no append or overwrite flags are set then
		// simply set the handle to point to the beggining
		// of the 1st cluster
		handle->current_sector_idx = 0x0;
		handle->buffer_head = handle->buffer;
	}
	return FAT_SUCCESS;
}


// sets the buffer of the file
uint16 fat_file_set_buffer(SFatFile* file, uint8* buffer)
{
	if (file->buffer_head != file->buffer)
	{
		uint16	uiResult;
		uint32	sector_address;
		bool	bSuccess;

		sector_address = file->current_sector_idx + calculate_first_sector_of_cluster(file->volume, file->current_clus_addr);
		file->buffer = buffer;
		file->buffer_head = buffer + (uintptr_t)file->buffer_head;
		bSuccess = file->volume->Read(sector_address, file->buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			file->busy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}
		file->buffer_dirty = 0;
	}
	else
	{
		file->buffer = buffer;
		file->buffer_head = buffer;
	}

	return FAT_SUCCESS;
}

/*
// gets a unique identifier of the file (ie. first cluster)
*/
uint32 fat_file_get_unique_id(SFatFile* file)
{
	/*return file->directory_entry.raw.uEntry.sFatRawCommon.first_cluster_lo
		& (file->directory_entry.raw.uEntry.sFatRawCommon.first_cluster_hi << 16); */

	uint32 id;
	((uint16*)&id)[INT32_WORD0] = file->directory_entry.raw.uEntry.sFatRawCommon.first_cluster_lo;

	if (file->volume->GetFileSystemType() == FAT_FS_TYPE_FAT32)
	{
		((uint16*)&id)[INT32_WORD1] = file->directory_entry.raw.uEntry.sFatRawCommon.first_cluster_hi;
	}
	else
	{
		((uint16*)&id)[INT32_WORD1] = 0;
	}

	return id;
}

/*
// Deletes a file.
*/
uint16 fat_file_delete(CFatVolume* volume, char* filename)
{
	uint16					uiResult;
	uint32					first_cluster;
	SFatDirectoryEntry		entry;
	uint8					buffer[MAX_SECTOR_LENGTH];
	bool					bSuccess;

	char					path_part[256];
	char*					name_part;
	uint8					checksum;
	SFatFileSystemQuery	query;


	// get the entry for the file
	uiResult = volume->FatGetFileEntry(volume, filename, &entry);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if the entry is located go ahead and delete it.
	if (entry.name != 0)
	{
		/*
		// compute the checksum for the file
		*/
		checksum = fat_long_entry_checksum((uint8*)entry.raw.uEntry.sFatRawCommon.name);

		/*
		// make sure we're not trying to delete a directory
		*/
		if (entry.attributes & FAT_ATTR_DIRECTORY)
		{
			return FAT_NOT_A_FILE;
		}

		// find the entry's first cluster address
		((uint16*)&first_cluster)[INT32_WORD0] = entry.raw.uEntry.sFatRawCommon.first_cluster_lo;
		((uint16*)&first_cluster)[INT32_WORD1] = (volume->GetFileSystemType() == FAT_FS_TYPE_FAT32) ? entry.raw.uEntry.sFatRawCommon.first_cluster_hi : 0;
		/*
		// free the file's cluster change
		*/
		if (first_cluster)
		{
			uiResult = volume->FatFreeClusterChain(volume, first_cluster);
			if (uiResult != FAT_SUCCESS)
				return uiResult;
		}
		/*
		// mark the entry as deleted
		*/
		entry.raw.uEntry.sFatRawCommon.name[0] = FAT_DELETED_ENTRY;

		bSuccess = volume->Read(entry.sector_addr, buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
			return uiResult;

		memcpy(buffer + entry.sector_offset, &entry.raw, sizeof(entry.raw));

		bSuccess = volume->Write(entry.sector_addr, buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			return uiResult;
		}
	}

	// parse the filename
	fat_parse_path(filename, path_part, &name_part);

	// get the 1st LFN entry of the parent directory
	uiResult = volume->FatFindFirstEntry(volume, path_part, FAT_ATTR_LONG_NAME, 0, &query);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// loop through each entry.
	while (*query.current_entry.raw.uEntry.sFatRawCommon.name != 0)
	{
		if (query.current_entry.raw.uEntry.sFatRawLongFileName.lfn_checksum == checksum)
		{
			/*
			// mark the entry as deleted
			*/
			query.current_entry.raw.uEntry.sFatRawCommon.name[0] = FAT_DELETED_ENTRY;

			bSuccess = volume->Read(query.current_entry.sector_addr, buffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				return uiResult;
			}

			memcpy(buffer + query.current_entry.sector_offset, &query.current_entry.raw, sizeof(entry.raw));

			bSuccess = volume->Write(query.current_entry.sector_addr, buffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				return uiResult;
			}
		}
		/*
		// get the next LFN entry
		*/
		volume->FatFindNextEntry(volume, 0, &query);
	}

	/*
	// return success code
	*/
	return FAT_SUCCESS;
}


// renames a file
uint16 fat_file_rename(CFatVolume* volume, char* original_filename, char* new_filename)
{
	uint16					uiResult;
	uint32					entry_cluster;
	char					new_parent[256];
	char*					original_filename_part;
	SFatDirectoryEntry		original_entry;
	SFatDirectoryEntry		new_entry;
	bool					bSuccess;
	uint8					checksum = 0;
	char					original_parent[256];
	char*					new_filename_part;
	uint8*					buffer = fat_shared_buffer;

	// parse paths
	fat_parse_path(original_filename, original_parent, &original_filename_part);
	fat_parse_path(new_filename, new_parent, &new_filename_part);

	// try to get the new entry to see if it exists.
	volume->FatGetFileEntry(volume, new_filename, &new_entry);
	if (*new_entry.name != 0)
	{
		return FAT_FILENAME_ALREADY_EXISTS;
	}

	// get the directory entry
	volume->FatGetFileEntry(volume, original_filename, &original_entry);

	if (*original_entry.name != 0)
	{
		SFatDirectoryEntry parent;
		
		// compute the checksum for the file
		checksum = fat_long_entry_checksum((uint8*)original_entry.raw.uEntry.sFatRawCommon.name);

		// get the cluster # for the entry
		((uint16*)&entry_cluster)[INT32_WORD0] = original_entry.raw.uEntry.sFatRawCommon.first_cluster_lo;
		((uint16*)&entry_cluster)[INT32_WORD1] = original_entry.raw.uEntry.sFatRawCommon.first_cluster_hi;

		// get the new parent entry
		uiResult = volume->FatGetFileEntry(volume, new_parent, &parent);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// create the new entry in the parent folder
		uiResult = volume->FatCreateDirectoryEntry(volume, &parent.raw, new_filename_part, original_entry.attributes, entry_cluster, &new_entry);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// copy all info except name from the old entry to the new one
		new_entry.raw.uEntry.sFatRawCommon.access_date = original_entry.raw.uEntry.sFatRawCommon.access_date;
		new_entry.raw.uEntry.sFatRawCommon.attributes = original_entry.raw.uEntry.sFatRawCommon.attributes;
		new_entry.raw.uEntry.sFatRawCommon.create_date = original_entry.raw.uEntry.sFatRawCommon.create_date;
		new_entry.raw.uEntry.sFatRawCommon.create_time = original_entry.raw.uEntry.sFatRawCommon.create_time;
		new_entry.raw.uEntry.sFatRawCommon.create_time_tenth = original_entry.raw.uEntry.sFatRawCommon.create_time_tenth;
		new_entry.raw.uEntry.sFatRawCommon.first_cluster_hi = original_entry.raw.uEntry.sFatRawCommon.first_cluster_hi;
		new_entry.raw.uEntry.sFatRawCommon.first_cluster_lo = original_entry.raw.uEntry.sFatRawCommon.first_cluster_lo;
		new_entry.raw.uEntry.sFatRawCommon.modify_date = original_entry.raw.uEntry.sFatRawCommon.modify_date;
		new_entry.raw.uEntry.sFatRawCommon.modify_time = original_entry.raw.uEntry.sFatRawCommon.modify_time;
		new_entry.raw.uEntry.sFatRawCommon.reserved = original_entry.raw.uEntry.sFatRawCommon.reserved;
		new_entry.raw.uEntry.sFatRawCommon.size = original_entry.raw.uEntry.sFatRawCommon.size;

		// write modified entry to drive
		fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);
		bSuccess = volume->Read(new_entry.sector_addr, buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			return uiResult;
		}
		memcpy(buffer + new_entry.sector_offset, &new_entry.raw, sizeof(new_entry.raw));
		bSuccess = volume->Write(new_entry.sector_addr, buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			return uiResult;
		}

		// mark the original entry as deleted.
		*original_entry.raw.uEntry.sFatRawCommon.name = FAT_DELETED_ENTRY;
		bSuccess = volume->Read(original_entry.sector_addr, buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			return uiResult;
		}
		memcpy(buffer + original_entry.sector_offset, &original_entry.raw, sizeof(original_entry.raw));
		bSuccess = volume->Write(original_entry.sector_addr, buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			return uiResult;
		}
	}
	{
		SFatFileSystemQuery query;

		// get the 1st LFN entry of the parent directory
		uiResult = volume->FatFindFirstEntry(volume, original_parent, FAT_ATTR_LONG_NAME, 0, &query);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// loop through each entry.
		while (*query.current_entry.raw.uEntry.sFatRawCommon.name != 0)
		{
			if (query.current_entry.raw.uEntry.sFatRawLongFileName.lfn_checksum == checksum)
			{
				// mark the entry as deleted
				fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);
				query.current_entry.raw.uEntry.sFatRawCommon.name[0] = FAT_DELETED_ENTRY;
				bSuccess = volume->Read(query.current_entry.sector_addr, buffer);
				uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
				if (uiResult != STORAGE_SUCCESS)
				{
					return uiResult;
				}
				memcpy(buffer + query.current_entry.sector_offset, &query.current_entry.raw, sizeof(query.current_entry.raw));
				bSuccess = volume->Write(query.current_entry.sector_addr, buffer);
				uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
				if (uiResult != STORAGE_SUCCESS)
				{
					return uiResult;
				}
			}
			// get the next LFN entry
			volume->FatFindNextEntry(volume, 0, &query);
		}
	}

	return FAT_SUCCESS;
}

// pre-allocates disk space for a file
uint16 fat_file_alloc(SFatFile* file, uint32 bytes)
{
	uint16	uiResult;
	uint32	new_cluster;
	uint32	no_of_clusters_needed;
	bool	bSuccess;
	uint8*	buffer = fat_shared_buffer;

	// check that this is a valid handle
	if (file->magic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// check that another operation is not using the
	// handle at this time
	if (file->busy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the handle as in use
	file->busy = 1;

	// calculate how many clusters we need
	no_of_clusters_needed = (bytes + file->volume->GetNoOfBytesPerSector() - 1) / file->volume->GetNoOfBytesPerSector();
	no_of_clusters_needed = (no_of_clusters_needed + file->volume->GetNoOfSectorsPerCluster() - 1) / file->volume->GetNoOfSectorsPerCluster();
	no_of_clusters_needed = (file->no_of_clusters_after_pos > no_of_clusters_needed) ? 0 : (no_of_clusters_needed - file->no_of_clusters_after_pos);

	// if we already got all the clusters requested then thre's nothing to do
	if (no_of_clusters_needed == 0)
	{
		file->busy = 0;
		return FAT_SUCCESS;
	}

	// allocate a new cluster
	if (file->access_flags & FAT_FILE_FLAG_OPTIMIZE_FOR_FLASH)
	{
		uint32 current_cluster;
		uint32 next_cluster;
		uint32 page_size;
		uint32 start_address;
		uint32 end_address;

		page_size = file->volume->GetPageSize();

		if (page_size > file->volume->GetNoOfSectorsPerCluster())
		{
			uint32 clusters_per_page = page_size / file->volume->GetNoOfSectorsPerCluster();

			if (no_of_clusters_needed % clusters_per_page)
			{
				no_of_clusters_needed += clusters_per_page - (no_of_clusters_needed % clusters_per_page);
			}

			if ((no_of_clusters_needed % clusters_per_page) != 0)
			{
				return FAT_UNKNOWN_ERROR;
			}

			new_cluster = file->volume->FatAllocateDataClusterEx(file->volume, no_of_clusters_needed, 0, page_size, &uiResult);
			if (uiResult != FAT_SUCCESS)
			{
				file->busy = 0;
				return uiResult;
			}
		}
		else
		{
			new_cluster = file->volume->FatAllocateDataCluster(file->volume, no_of_clusters_needed, 1, &uiResult);
			if (uiResult != FAT_SUCCESS)
			{
				file->busy = 0;
				return uiResult;
			}
		}

		// find out how many clusters are allocated sequentially
		// to this file following the current cursor location
		current_cluster = new_cluster;

		while (!file->volume->FatIsEOFEntry(file->volume, current_cluster))
		{
			// calculate the start and end address the cluster
			start_address = calculate_first_sector_of_cluster(file->volume, current_cluster);
			end_address = start_address + file->volume->GetNoOfSectorsPerCluster();

			// find the last sequential sector after this address
			while (!file->volume->FatIsEOFEntry(file->volume, current_cluster))
			{
				file->volume->FatGetClusterEntry(file->volume, current_cluster, &next_cluster);

				if (next_cluster == (current_cluster + 1))
				{
					end_address += file->volume->GetNoOfSectorsPerCluster();
					current_cluster = next_cluster;
				}
				else
				{
					current_cluster = next_cluster;
					break;
				}
			}

			// pre-erase the clusters
			file->volume->Erase(start_address, end_address - 1);
		}
	}
	else
	{
		new_cluster = file->volume->FatAllocateDataCluster(file->volume, no_of_clusters_needed, 0, &uiResult);
		if (uiResult != FAT_SUCCESS)
		{
			file->busy = 0;
			return uiResult;
		}
	}

	// if this is the 1st cluster cluster allocated
	// to the file then we must modify the file's entry
	if (!file->directory_entry.raw.uEntry.sFatRawCommon.first_cluster_lo && !file->directory_entry.raw.uEntry.sFatRawCommon.first_cluster_hi)
	{
		// modify the file entry to point to the
		// new cluster
		file->directory_entry.raw.uEntry.sFatRawCommon.first_cluster_lo = LO16(new_cluster);
		file->directory_entry.raw.uEntry.sFatRawCommon.first_cluster_hi = HI16(new_cluster);

		// mark the cached sector as unknown
		fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);

		// try load the sector that contains the entry
		bSuccess = file->volume->Read(file->directory_entry.sector_addr, buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			file->busy = 0;
			return uiResult;
		}

		// copy the modified file entry to the
		// sector buffer
		memcpy(buffer + file->directory_entry.sector_offset, &file->directory_entry.raw, sizeof(SFatRawDirectoryEntry));

		// write the modified entry to the media
		bSuccess = file->volume->Write(file->directory_entry.sector_addr, buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			file->busy = 0;
			return uiResult;
		}
	}
	// if there are clusters allocated to the file update the last FAT entry
	// of the file to point to the newly allocated cluster(s)
	else
	{
		uint32 last_cluster;

		if (file->no_of_clusters_after_pos)
		{
			if (!file->volume->FatIncreaseClusterAddress(file->volume, file->current_clus_addr, file->no_of_clusters_after_pos, &last_cluster))
			{
				file->busy = 0;
				return FAT_CORRUPTED_FILE;
			}
		}
		else
		{
			last_cluster = file->current_clus_addr;
		}

		/*
		// set the FAT entry for the last cluster to the beggining of the newly
		// allocated cluster chain (ie. link them)
		*/
		uiResult = file->volume->FatSetClusterEntry(file->volume, last_cluster, (FatEntry)new_cluster);
		if (uiResult != FAT_SUCCESS)
		{
			file->busy = 0;
			return uiResult;
		}
	}
	/*
	// update the file to point to the
	// new cluster
	*/
	if (!file->current_clus_addr)
	{
		file->current_clus_addr = new_cluster;
		file->no_of_clusters_after_pos = no_of_clusters_needed - 1;
	}
	else
	{
		file->no_of_clusters_after_pos += no_of_clusters_needed;
	}
	/*
	// update the count of sequential clusters
	*/
	fat_file_update_sequential_cluster_count(file);

	file->busy = 0;
	/*
	// return success code
	*/
	return FAT_SUCCESS;
}

// moves the file cursor to the specified offset
uint16 fat_file_seek(SFatFile* file, uint32 offset, char mode)
{
	uint32	new_pos;
	uint32	sector_count;
	uint32	cluster_count;
	uint32	old_cluster;
	uint32	sector_address;
	uint16	uiResult;
	bool	bSuccess;
	/*
	// check that this is a valid handle
	*/
	if (file->magic != FAT_OPEN_HANDLE_MAGIC)
		return FAT_INVALID_HANDLE;
	/*
	// check that another operation is not using the
	// handle at this time
	*/
	if (file->busy)
		return FAT_FILE_HANDLE_IN_USE;
	/*
	// mark the handle as in use
	*/
	file->busy = 1;

	switch (mode)
	{
	case FAT_SEEK_START:
		new_pos = offset;
		break;
	case FAT_SEEK_CURRENT:
		new_pos = file->current_clus_idx * file->volume->GetNoOfSectorsPerCluster() * file->volume->GetNoOfBytesPerSector();
		new_pos += file->current_sector_idx * file->volume->GetNoOfBytesPerSector();
		new_pos += (uint32)(file->buffer_head - file->buffer);
		new_pos += offset;
		break;
	case FAT_SEEK_END:
		new_pos = file->current_size;
		if (offset)
		{
			file->busy = 0;
			return FAT_SEEK_FAILED;
		}
		break;
	default:
		file->busy = 0;
		return FAT_SEEK_FAILED;
	}
	/*
	// if the seek goes out of bounds return error
	*/
	if (new_pos > file->current_size)
	{
		file->busy = 0;
		return FAT_SEEK_FAILED;
	}
	/*
	// if the file is open in unbuffered mode make sure that
	// we're seeking to a sector boundry
	*/
	if (file->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (new_pos % file->volume->GetNoOfBytesPerSector())
		{
			file->busy = 0;
			return FAT_MISALIGNED_IO;
		}
	}

	old_cluster = file->current_clus_idx;
	file->current_clus_idx = 0;
	cluster_count = 1;
	/*
	// calculate the count of sectors being used by the file up to the desired position
	*/
	sector_count = (new_pos + file->volume->GetNoOfBytesPerSector() - 1) / file->volume->GetNoOfBytesPerSector();
	/*
	// set the 1st cluster as the current cluster, we'll seek from there
	*/
	((uint16*)&file->current_clus_addr)[INT32_WORD0] = file->directory_entry.raw.uEntry.sFatRawCommon.first_cluster_lo;
	((uint16*)&file->current_clus_addr)[INT32_WORD1] =
		(file->volume->GetFileSystemType() == FAT_FS_TYPE_FAT32) ? file->directory_entry.raw.uEntry.sFatRawCommon.first_cluster_hi : 0;
	/*
	// if the file occupies more than one cluster
	*/
	if (sector_count > file->volume->GetNoOfSectorsPerCluster())
	{
		/*
		// calculate the count of clusters occupied by the file and
		// update the ClustersAllocated value of the file
		*/
		cluster_count = (sector_count + file->volume->GetNoOfSectorsPerCluster() - 1) / file->volume->GetNoOfSectorsPerCluster();
		/*
		// set the file handle to point to the last cluster. if the file doesn't have
		// that many clusters allocated this function will return 0. if that ever happens it means
		// that the file is corrupted
		*/
		if (!file->volume->FatIncreaseClusterAddress(file->volume, file->current_clus_addr, (cluster_count - 1), &file->current_clus_addr))
		{
			file->busy = 0;
			return FAT_CORRUPTED_FILE;
		}
	}
	/*
	// calculate the last sector of the file and set the buffer
	// head to point to the byte after the last one
	*/
	if (new_pos)
	{
		file->current_sector_idx = (((new_pos + file->volume->GetNoOfBytesPerSector() - 1) / file->volume->GetNoOfBytesPerSector()) - 1) % file->volume->GetNoOfSectorsPerCluster();
		file->buffer_head = (uint8*)((uintptr_t)file->buffer) + (new_pos % file->volume->GetNoOfBytesPerSector());

		if (new_pos % file->volume->GetNoOfBytesPerSector() == 0)
		{
			file->buffer_head = (uint8*)((uintptr_t)file->buffer) + file->volume->GetNoOfBytesPerSector();
		}
	}
	else
	{
		file->current_sector_idx = 0;
		file->buffer_head = file->buffer;
	}

	sector_address = file->current_sector_idx + calculate_first_sector_of_cluster(file->volume, file->current_clus_addr);
	file->current_clus_idx = cluster_count - 1;
	/*
	// load the last sector
	*/
	if (file->buffer)
	{
		bSuccess = file->volume->Read(sector_address, file->buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			file->busy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}
		file->buffer_dirty = 0;
	}
	/*
	// maintain the count of allocated clusters
	*/
	if (old_cluster < file->current_clus_idx)
	{
		file->no_of_clusters_after_pos -= (file->current_clus_idx - old_cluster);
	}
	else if (old_cluster > file->current_clus_idx)
	{
		file->no_of_clusters_after_pos += (old_cluster - file->current_clus_idx);
	}
	file->busy = 0;
	return FAT_SUCCESS;
}


void fat_file_write_callback(SFatFile* handle, uint16* async_state_in)
{
	uint16	uiResult;
	uint16* state;
	bool	bSuccess;

	state = async_state_in;

	// loop while there are bytes to be read
	while (handle->op_state.bytes_remaining)
	{
		/*
		// if we've reached the end of the current
		// sector then we must flush it
		*/
		if (handle->buffer_head == handle->op_state.end_of_buffer)
		{
			/*
			// update the sector head
			*/
			if (handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
			{
				handle->buffer = handle->buffer_head;
				handle->op_state.end_of_buffer = handle->buffer + handle->volume->GetNoOfBytesPerSector();
			}
			else
			{
				handle->op_state.end_of_buffer = handle->buffer + handle->volume->GetNoOfBytesPerSector();
				handle->buffer_head = handle->buffer;
			}
			/*
			// write the cached sector to media
			*/
			bSuccess = handle->volume->Write(handle->op_state.sector_addr, handle->buffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;

			/*
			// if we were unable to read the sector
			// then throw an error
			*/
			if (uiResult != STORAGE_SUCCESS)
			{
				*state = FAT_CANNOT_WRITE_MEDIA;
				handle->busy = 0;
				return;
			}
			/*
			// if this sector is the last of the current cluster then
			// locate the next cluster and continue writing
			*/
			if (handle->current_sector_idx == handle->volume->GetNoOfSectorsPerCluster() - 1)
			{
				uiResult = handle->volume->FatGetClusterEntry(handle->volume, handle->current_clus_addr, &handle->current_clus_addr);
				if (uiResult != STORAGE_SUCCESS)
				{
					*state = FAT_CANNOT_WRITE_MEDIA;
					handle->busy = 0;
					return;
				}
				/*
				// reset the current sector and increase the cluster index
				*/
				handle->current_sector_idx = 0x0;
				handle->current_clus_idx++;
				handle->no_of_clusters_after_pos--;
				/*
				// calculate the sector address
				*/
				handle->op_state.sector_addr =
					calculate_first_sector_of_cluster(handle->volume, handle->current_clus_addr);
			}
			/*
			// if there are more sectors in the
			// current cluster then simply increase
			// the current sector counter and address
			*/
			else
			{
				handle->current_sector_idx++;
				handle->op_state.sector_addr++;
			}
			/*
			// if we've reached a flush boundry then flush the file
			*/
			/*if (!(handle->op_state.pos % (handle->volume->GetNoOfBytesPerSector() * FAT_FLUSH_INTERVAL)))
			{
				fat_file_flush(handle);
			}*/
		}
		if (handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
		{
			handle->buffer_head = handle->op_state.end_of_buffer;
			handle->op_state.pos += handle->volume->GetNoOfBytesPerSector();

			if (handle->op_state.pos >= handle->current_size)
			{
				handle->current_size = handle->op_state.pos;
			}
			handle->op_state.bytes_remaining -= handle->volume->GetNoOfBytesPerSector();
		}
		else
		{
			/*
			// write the next byte to the handle cache
			*/
			*handle->buffer_head++ = *handle->op_state.buffer++;
			/*
			// update the file size only if we're writting past
			// the end of the file
			*/
			if (handle->op_state.pos++ >= handle->current_size)
			{
				handle->current_size++;
			}
			handle->op_state.bytes_remaining--;
		}
	}

	*state = FAT_SUCCESS;
	handle->busy = 0;
	return;
}


uint16 fat_file_write(SFatFile* handle, uint8* buff, uint32 length)
{
	uint32 uiResult;

	// check that this is a valid handle
	if (handle->magic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}
	// check that the file is open for write access
	if (!(handle->access_flags & FAT_FILE_ACCESS_WRITE))
	{
		return FAT_FILE_NOT_OPENED_FOR_WRITE_ACCESS;
	}

	// make sure length is not larger than 16-bit
	if (length > 0xFFFF)
	{
		return FAT_BUFFER_TOO_BIG;
	}

	// if there's no clusters allocated to this file allocate
	// enough clusters for this request
	uiResult = fat_file_alloc(handle, length);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	/*
	// make sure that either a buffer is set or the file has been opened in
	// unbuffered mode.
	*/
	if (!handle->buffer && !(handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		return FAT_FILE_BUFFER_NOT_SET;
	}

	/*
	// check that another operation is not using the
	// handle at this time
	*/
	if (handle->busy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	/*
	// mark the handle as in use
	*/
	handle->busy = 1;
	/*
	// calculate current pos
	*/
	handle->op_state.pos = handle->current_clus_idx * handle->volume->GetNoOfSectorsPerCluster() * handle->volume->GetNoOfBytesPerSector();
	handle->op_state.pos += (handle->current_sector_idx) * handle->volume->GetNoOfBytesPerSector();
	handle->op_state.pos += (uintptr_t)(handle->buffer_head - handle->buffer);
	/*
	// if the file is opened in unbuffered mode make sure that
	// we're reading a multiple of the sector size
	*/
	if (handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (length % handle->volume->GetNoOfBytesPerSector())
		{
			handle->busy = 0;
			return FAT_MISALIGNED_IO;
		}
		handle->buffer = buff;
		handle->op_state.end_of_buffer = handle->buffer_head = buff;
	}
	else
	{
		/*
		// calculate the address of the end of
		// the current sector
		*/
		handle->op_state.end_of_buffer = handle->buffer + handle->volume->GetNoOfBytesPerSector();
	}
	/*
	// copy the length of the buffer to be writen
	// into the counter
	*/
	handle->op_state.bytes_remaining = (uint16)length;
	/*
	// calculate the address of the current
	// sector
	*/
	handle->op_state.sector_addr = handle->current_sector_idx +
		calculate_first_sector_of_cluster(handle->volume, handle->current_clus_addr);


	handle->op_state.internal_state = 0x0;
	handle->op_state.length = (uint16)length;
	handle->op_state.buffer = buff;

	uint16 internal_state;
	fat_file_write_callback(handle, &internal_state);
	return internal_state;
}


void fat_file_read_callback(SFatFile* handle, uint16* state)
{
	uint16	uiResult;
	bool	bSuccess;

	// reset the # of bytes read
	if (handle->op_state.bytes_read)
	{
		*handle->op_state.bytes_read = 0;
	}


	/*
	// if the sector cache is invalid
	*/
	if (handle->buffer_dirty)
	{
		/*
		// read the current sector synchronously
		*/
		bSuccess = handle->volume->Read(handle->op_state.sector_addr, handle->buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			*state = FAT_CANNOT_READ_MEDIA;
			handle->busy = 0;
			return;
		}
		/*
		// mark the cache as clean
		*/
		handle->buffer_dirty = 0;
		/*
		// if the file is unbuffered decrease the count of bytes remaining
		// by one sector and set the buffer head to the end of the buffer
		*/
		if (handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
		{
			handle->op_state.bytes_remaining -= handle->volume->GetNoOfBytesPerSector();
			handle->buffer_head += handle->volume->GetNoOfBytesPerSector();
			handle->op_state.end_of_buffer = handle->buffer_head;
			if (handle->op_state.bytes_read)
				*handle->op_state.bytes_read += handle->volume->GetNoOfBytesPerSector();

			handle->op_state.pos += handle->volume->GetNoOfBytesPerSector();

			if (handle->op_state.pos >= handle->current_size)
			{
				if (handle->op_state.bytes_read)
				{
					*handle->op_state.bytes_read -= handle->op_state.pos - handle->current_size;
				}
				handle->op_state.bytes_remaining = 0;
			}
		}
	}
	/*
	// make sure that we haven't reaced the end of the file
	*/
	if (handle->op_state.pos >= handle->current_size)
		handle->op_state.bytes_remaining = 0;
	/*
	// loop while there are bytes to be
	// read
	*/
	while (handle->op_state.bytes_remaining)
	{
		/*
		// if we've reached the end of the current
		// sector then we must load the next...
		*/
		if (handle->buffer_head == handle->op_state.end_of_buffer)
		{
			/*
			// if the file is unbuffered change the buffer to the
			// new position of the buffer head. otherwise reset the position
			// of the position of the buffer head
			*/
			if (handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
			{
				handle->buffer = handle->buffer_head;
				handle->op_state.end_of_buffer = handle->buffer_head + handle->volume->GetNoOfBytesPerSector();
			}
			else
			{
				handle->buffer_head = handle->buffer;
			}
			/*
			// if this sector is the last of the current cluster
			// we must find the next cluster
			*/
			if (handle->current_sector_idx == handle->volume->GetNoOfSectorsPerCluster() - 1)
			{
				// update the cluster address with the address of the
				// next cluster
				if (!handle->volume->FatIncreaseClusterAddress(handle->volume, handle->current_clus_addr, 1, &handle->current_clus_addr))
				{
					*state = FAT_CORRUPTED_FILE;
					handle->busy = 0;
					return;
				}

				// reset the current sector increase the current cluster
				// number and calculate the address of the first sector of the
				// cluster
				handle->current_clus_idx++;
				handle->current_sector_idx = 0x0;
				handle->op_state.sector_addr = calculate_first_sector_of_cluster(handle->volume, handle->current_clus_addr);
			}
			else
			{

				// if there are more sectors in the current cluster then
				// simply increase the current sector counter and address
				handle->current_sector_idx++;
				handle->op_state.sector_addr++;
			}

			/*
			// read the next sector into the cache
			*/
			bSuccess = handle->volume->Read(handle->op_state.sector_addr, handle->buffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				*state = FAT_CANNOT_READ_MEDIA;
				handle->busy = 0;
				return;
			}
		}
		/*
		// update the count of bytes read/remaining and if the file
		// is buffered copy data to file buffer
		*/
		if (handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
		{
			handle->buffer_head = handle->op_state.end_of_buffer;
			handle->op_state.bytes_remaining -= handle->volume->GetNoOfBytesPerSector();
			if (handle->op_state.bytes_read)
				(*handle->op_state.bytes_read) += handle->volume->GetNoOfBytesPerSector();

			handle->op_state.pos += handle->volume->GetNoOfBytesPerSector();

			if (handle->op_state.pos >= handle->current_size)
			{
				if (handle->op_state.bytes_read)
				{
					*handle->op_state.bytes_read -= (handle->op_state.pos - handle->current_size);
				}
				handle->op_state.bytes_remaining = 0;
			}
		}
		else
		{
			/*
			// copy the next byte to the buffer
			*/
			*handle->op_state.buffer++ = *handle->buffer_head++;
			/*
			// update the  count of bytes read
			*/
			if (handle->op_state.bytes_read)
				(*handle->op_state.bytes_read)++;
			/*
			// decrease the count of remaining bytes
			*/
			handle->op_state.bytes_remaining--;
			/*
			// increase the file pointer
			*/
			handle->op_state.pos++;
			/*
			// check if we've reached the end of the file
			*/
			if (handle->op_state.pos >= handle->current_size)
				handle->op_state.bytes_remaining = 0;
		}
	}
	/*
	// set the return code
	*/
	*state = FAT_SUCCESS;
	handle->busy = 0;
	return;
}


uint16 fat_file_read(SFatFile* handle, uint8* buff, uint32 length, uint32* bytes_read)
{
	// check that this is a valid handle
	if (handle->magic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}
	/*
	// make sure that either a buffer is set or the file has been
	// opened in unbuffered mode
	*/
	if (!handle->buffer && !(handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		return FAT_FILE_BUFFER_NOT_SET;
	}
	/*
	// make sure length is not larger than 16-bit
	*/
	if (length > 0xFFFF)
		return FAT_BUFFER_TOO_BIG;
	/*
	// check that another operation is not using the
	// handle at this time
	*/
	if (handle->busy)
		return FAT_FILE_HANDLE_IN_USE;
	/*
	// mark the handle as in use
	*/
	handle->busy = 1;
	/*
	// calculate current pos
	*/
	handle->op_state.pos = handle->current_clus_idx * handle->volume->GetNoOfSectorsPerCluster() * handle->volume->GetNoOfBytesPerSector();
	handle->op_state.pos += (handle->current_sector_idx) * handle->volume->GetNoOfBytesPerSector();
	handle->op_state.pos += (uintptr_t)(handle->buffer_head - handle->buffer);
	/*
	// calculate the address of the current
	// sector and the address of the end of the buffer
	*/
	handle->op_state.sector_addr = handle->current_sector_idx + calculate_first_sector_of_cluster(handle->volume, handle->current_clus_addr);
	handle->op_state.end_of_buffer = handle->buffer + handle->volume->GetNoOfBytesPerSector();
	/*
	// if the file is opened in unbuffered mode make sure that
	// we're reading a multiple of the sector size and set the buffer
	// to the one supplied by the user in this call
	*/
	if (handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (length % handle->volume->GetNoOfBytesPerSector())
		{
			return FAT_MISALIGNED_IO;
		}
		handle->buffer = buff;
		handle->op_state.end_of_buffer = handle->buffer_head = buff;
	}
	/*
	// set the async op context
	*/
	handle->op_state.bytes_remaining = (uint16)length;
	handle->op_state.internal_state = 0x0;
	handle->op_state.length = (uint16)length;
	handle->op_state.buffer = buff;
	handle->op_state.bytes_read = bytes_read;

	uint16 internal_state;
	fat_file_read_callback(handle, &internal_state);
	return internal_state;
}


// flushes file buffers
uint16 fat_file_flush(SFatFile* handle)
{
	uint16	uiResult;
	uint32	sector_address = 0;
	bool	bSuccess;
	uint8*	buffer = fat_shared_buffer;

	/*
	// check that this is a valid handle
	*/
	if (handle->magic != FAT_OPEN_HANDLE_MAGIC)
		return FAT_INVALID_HANDLE;
	/*
	// make sure that either a buffer has been set or the file was
	// opened in unbuffered mode
	*/
	if (!handle->buffer && !(handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING))
		return FAT_FILE_BUFFER_NOT_SET;
	/*
	// check that we got write access
	*/
	if (handle->access_flags & FAT_FILE_ACCESS_WRITE)
	{
		/*
		// check that another operation is not using the
		// handle at this time
		*/
		if (handle->busy)
			return FAT_FILE_HANDLE_IN_USE;
		/*
		// mark the handle as in use
		*/
		handle->busy = 1;
		/*
		// if the file is buffered then flush the buffer
		*/
		if (!(handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING))
		{
			/*
			// calculate the address of the current
			// sector
			*/
			sector_address = handle->current_sector_idx +
				calculate_first_sector_of_cluster(handle->volume, handle->current_clus_addr);
			/*
			// if the buffer is only partially filled we need to merge it
			// with the one on the drive
			*/
			if (handle->buffer_head <= handle->buffer + handle->volume->GetNoOfBytesPerSector())
			{
				uint16 i;
				uint8 buff[MAX_SECTOR_LENGTH];
				bSuccess = handle->volume->Read(sector_address, buff);
				uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
				if (uiResult != STORAGE_SUCCESS)
				{
					handle->busy = 0;
					return uiResult;
				}

				for (i = (uint16)(handle->buffer_head -
					handle->buffer); i < handle->volume->GetNoOfBytesPerSector(); i++)
				{
					handle->buffer[i] = buff[i];
				}
			}
			/*
			// write the cached sector to media
			*/
			bSuccess = handle->volume->Write(sector_address, handle->buffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				handle->busy = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}
		}

		// update the file size on the entry
		handle->directory_entry.raw.uEntry.sFatRawCommon.size = handle->current_size;
		handle->directory_entry.raw.uEntry.sFatRawCommon.modify_date = rtc_get_fat_date();
		handle->directory_entry.raw.uEntry.sFatRawCommon.modify_time = rtc_get_fat_time();
		handle->directory_entry.raw.uEntry.sFatRawCommon.access_date = handle->directory_entry.raw.uEntry.sFatRawCommon.modify_date;
		/*
		// try load the sector that contains the entry
		*/
		fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);

		bSuccess = handle->volume->Read(handle->directory_entry.sector_addr, buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			handle->busy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}

		// copy the modified file entry to the
		// sector buffer
		memcpy(buffer + handle->directory_entry.sector_offset, &handle->directory_entry.raw, sizeof(SFatRawDirectoryEntry));

		// write the modified entry to the media
		bSuccess = handle->volume->Write(handle->directory_entry.sector_addr, buffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			handle->busy = 0;
			return FAT_CANNOT_WRITE_MEDIA;
		}
		/*
		// some cards seem not to update the sector correctly if it
		// is the first sector on the page and is the only sector written
		// to so for now until we figure this out we'll write the next
		// sector too TODO: implement this on driver!!
		*/
		{
			bSuccess = handle->volume->Read(handle->directory_entry.sector_addr + 1, buffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				handle->busy = 0;
				return FAT_CANNOT_READ_MEDIA;
			}
			bSuccess = handle->volume->Write(handle->directory_entry.sector_addr + 1, buffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				handle->busy = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}
		}
		/*
		// mark the file handle as not busy
		*/
		handle->busy = 0;
	}
	/*
	// return success code
	*/
	return FAT_SUCCESS;
}


// closes an open file
uint16 fat_file_close(SFatFile* handle)
{
	uint16		uiResult;
	FatEntry	fat_entry;

	// check that this is a valid handle
	if (handle->magic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// flush the file buffers
	uiResult = fat_file_flush(handle);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	if (handle->access_flags & FAT_FILE_ACCESS_WRITE)
	{
		/*
		// clear the no buffering attribute so we can seek to a misaligned
		// position (since we want the very end of the last written, not the
		// beginning of the next one.)
		*/
		handle->access_flags &= (FAT_FILE_FLAG_NO_BUFFERING ^ 0xFF);
		/*
		// seek to the end of the file
		*/
		uiResult = fat_file_seek(handle, handle->current_size - 1, FAT_SEEK_START);
		if (uiResult != FAT_SUCCESS)
			return uiResult;
		/*
		// check that another operation is not using the
		// handle at this time
		*/
		if (handle->busy)
			return FAT_FILE_HANDLE_IN_USE;
		/*
		// mark the handle as in use
		*/
		handle->busy = 1;
		/*
		// free unused clusters
		*/
		uiResult = handle->volume->FatGetClusterEntry(handle->volume, handle->current_clus_addr, &fat_entry);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		if (!handle->volume->FatIsEOFEntry(handle->volume, fat_entry))
		{
			uiResult = handle->volume->FatFreeClusterChain(handle->volume, fat_entry);
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}

			switch (handle->volume->GetFileSystemType())
			{
			case FAT_FS_TYPE_FAT12: fat_entry = FAT12_EOC; break;
			case FAT_FS_TYPE_FAT16: fat_entry = FAT16_EOC; break;
			case FAT_FS_TYPE_FAT32: fat_entry = FAT32_EOC; break;
			}
			uiResult = handle->volume->FatSetClusterEntry(handle->volume, handle->current_clus_addr, fat_entry);
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}
		}
	}

	/*
	// invalidate the file handle
	*/
	handle->magic = 0;
	/*
	// return success
	*/
	return FAT_SUCCESS;
}

