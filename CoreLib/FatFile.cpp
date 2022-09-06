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

 /*
 // TODO:
 //
 // 1. Fix thread safety issues with write_stream
 // 2. Fix file system alignment issue with fat_format
 //
 //
 */
#include <stdio.h>
#include <string.h>
#include "Fat.h"
#include "FatInternals.h"


#if defined(FAT_ALLOCATE_SHARED_BUFFER)
#define FAT_SET_LOADED_SECTOR(volume, sector)		fat_shared_buffer_sector = (sector)
#elif defined(FAT_ALLOCATE_VOLUME_BUFFER)
#define FAT_SET_LOADED_SECTOR(volume, sector)		volume->sector_buffer_sector = (sector)
#else
#define FAT_SET_LOADED_SECTOR(volume, sector)	
#endif


uint16 fat_file_update_sequential_cluster_count(FAT_FILE* file);

uint16 fat_file_update_sequential_cluster_count(FAT_FILE* handle)
{
	uint32 current_cluster;
	uint32 next_cluster;
	/*
	// find out how many clusters are allocated sequentially
	// to this file following the current cursor location
	*/
	handle->no_of_sequential_clusters = 0;
	current_cluster = handle->current_clus_addr;

	while (!fat_is_eof_entry(handle->volume, current_cluster))
	{
		fat_get_cluster_entry(handle->volume, current_cluster, &next_cluster);

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

/*
// opens a file
*/
uint16 fat_file_open(FAT_VOLUME* volume, char* filename, uint8 access_flags, FAT_FILE* handle)
{
	uint16 ret;
	FAT_DIRECTORY_ENTRY file_entry;
	/*
	// use the internal buffer
	*/
#if defined(FAT_ALLOCATE_FILE_BUFFERS)
	if (!(access_flags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		handle->buffer = handle->buffer_internal;
	}
#else
	handle->buffer = 0;
#endif
	/*
	// check that the user supplied a filename
	*/
	if (filename == 0 || strlen(filename) > FAT_MAX_PATH)
	{
		return FAT_INVALID_FILENAME;
	}
	/*
	// get the file entry
	*/
	ret = fat_get_file_entry(volume, filename, &file_entry);
	/*
	// if we could not get the file entry then return
	// the error that we received from fat_get_file_entry
	*/
	if (ret != FAT_SUCCESS)
	{
		return ret;
	}
	/*
	// if the entry was not found...
	*/
	if (*file_entry.name == 0)
	{
		/*
		// if the create access flag was set then
		// we create the file
		*/
#if !defined(FAT_READ_ONLY)
		if (access_flags & FAT_FILE_ACCESS_CREATE)
		{
			/*
			// allocate memory for the file path
			*/
			size_t path_len;
			char* filename_scanner;
			char file_path[FAT_MAX_PATH + 1];
			FAT_DIRECTORY_ENTRY parent_entry;
			/*
			// get the name of the file path including
			// the filename
			*/
			path_len = strlen(filename);
			/*
			// set the pointer that will be used to scan
			// filename to the end of the filename
			*/
			filename_scanner = filename + (path_len - 0x1);
			/*
			// if the filename ends with a backslash then it
			// is an invalid filename ( it's actually a directory
			// path )
			*/
			if (*filename_scanner == BACKSLASH)
				return FAT_INVALID_FILENAME;
			/*
			// scan the filename starting at the end until
			// a backslash is found - when we exit this loop
			// path_scanner will point to the last character
			// of the filepath
			*/
			while (*(--filename_scanner) != BACKSLASH);
			/*
			// calculate the length of the path part of the
			// filename
			*/
			path_len = (size_t)(filename_scanner - filename);
			/*
			// copy the path part of the filename to
			// the file_path buffer
			*/
			memcpy(file_path, filename, path_len);
			/*
			// set the null terminator of the file_path buffer
			*/
			file_path[path_len] = 0x0;
			/*
			// increase pointer to the beggining of the filename
			// part of the path
			*/
			filename_scanner++;
			/*
			// try to get the entry for the parent directory
			*/
			ret = fat_get_file_entry(volume, file_path, &parent_entry);
			/*
			// if fat_get_file_entry returned an error
			// then we return the error code to the calling
			// function
			*/
			if (ret != FAT_SUCCESS)
				return ret;
			/*
			// if the parent directory does not exists
			*/
			if (*parent_entry.name == 0)
				return FAT_DIRECTORY_DOES_NOT_EXIST;
			/*
			// try to create the directory entry
			*/
			ret = fat_create_directory_entry(volume, &parent_entry.raw, filename_scanner, 0, 0, &file_entry);
			/*
			// make sure the file is opened with no append flags
			// todo: figure out why we need this and fix it
			*/
			access_flags = access_flags & (0xFF ^ FAT_FILE_ACCESS_APPEND);
			/*
			// if we were unable to create the entry for the file
			// and
			*/
			if (ret != FAT_SUCCESS)
				return ret;

		}
		/*
		// if the create flag is not set then return the
		// file not found error code
		*/
		else
#endif
		{
			return FAT_FILE_NOT_FOUND;
		}
	}

	/*
	// open the file
	*/
	ret = fat_open_file_by_entry(volume, &file_entry, handle, access_flags);
	if (ret != FAT_SUCCESS)
		return ret;
	/*
	// if the file has no clusters allocated then allocate one
	*/
	if (handle->access_flags & FAT_FILE_ACCESS_WRITE)
	{
		/*
		// update the count of sequential clusters
		*/
		fat_file_update_sequential_cluster_count(handle);
	}
	return FAT_SUCCESS;
}


/*
// opens a file given a pointer to it's
// directory entry
*/
uint16 fat_open_file_by_entry(FAT_VOLUME* volume, FAT_DIRECTORY_ENTRY* entry, FAT_FILE* handle, uint8 access_flags)
{
	bool	bSuccess;
	uint16	ret;

	/*
	// set implicit access flags
	*/
	access_flags |= FAT_FILE_ACCESS_READ;
	if (access_flags & (FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_APPEND | FAT_FILE_ACCESS_OVERWRITE))
		access_flags |= FAT_FILE_ACCESS_WRITE;
	/*
	// if the user is trying to open a directory then
	// return an error code, otherwise it would corrupt
	// the volume
	*/
	if (entry->attributes & FAT_ATTR_DIRECTORY)
		return FAT_NOT_A_FILE;
	/*
	// copy the volume handle and the entry's
	// structure to the file handle
	*/
	handle->volume = volume;
	handle->directory_entry = *entry;
	handle->current_size = entry->size;
	handle->current_clus_idx = 0;
	handle->access_flags = access_flags;
	handle->magic = FAT_OPEN_HANDLE_MAGIC;
	handle->busy = 0;
	/*
	// calculate the # of clusters allocated
	*/
	handle->no_of_clusters_after_pos = (entry->size + volume->no_of_bytes_per_serctor - 1) / volume->no_of_bytes_per_serctor;
	handle->no_of_clusters_after_pos = (handle->no_of_clusters_after_pos + volume->no_of_sectors_per_cluster - 1) / volume->no_of_sectors_per_cluster;
	if (handle->no_of_clusters_after_pos)
		handle->no_of_clusters_after_pos--;
	/*
	// set the sector cache as dirty, this will
	// indicate that the current sector has not yet
	// been read to other file functions
	*/
	handle->buffer_dirty = 1;
	/*
	// read the the cluster number
	*/
	((uint16*)&handle->current_clus_addr)[INT32_WORD0] = entry->raw.ENTRY.STD.first_cluster_lo;
	((uint16*)&handle->current_clus_addr)[INT32_WORD1] = (volume->fs_type == FAT_FS_TYPE_FAT32) ? entry->raw.ENTRY.STD.first_cluster_hi : 0;

#if defined(FAT_READ_ONLY)
	if (access_flags & (FAT_FILE_ACCESS_APPEND | FAT_FILE_ACCESS_OVERWRITE))
	{
		return FAT_FEATURE_NOT_SUPPORTED;
	}
#else
	if (access_flags & FAT_FILE_ACCESS_APPEND)
	{
		/*
		// if the file is being opened for append access we
		// seek to the end of the file
		*/
		ret = fat_file_seek(handle, 0, FAT_SEEK_END);
		if (ret != FAT_SUCCESS)
		{
			handle->magic = 0;
			return ret;
		}
	}
	else if (access_flags & FAT_FILE_ACCESS_OVERWRITE)
	{
		/*
		// if the file is being opened with the OVERWRITE flag we must free all the clusters
		// currently allocated to the file and update it's directory entry to point to cluster 1
		*/
		/* uint32 size = entry->raw.size; */
		/*
		// if the file is not already empty then
		// we'll empty it
		*/
		if (entry->raw.ENTRY.STD.first_cluster_lo != 0x0 || entry->raw.ENTRY.STD.first_cluster_hi != 0x0)
		{
#if defined(FAT_ALLOCATE_VOLUME_BUFFER)
			uint8* buffer = file->volume->sector_buffer;
#elif defined(FAT_ALLOCATE_SHARED_BUFFER)
			uint8* buffer = fat_shared_buffer;
#else
			ALIGN16 uint8 buffer[MAX_SECTOR_LENGTH];
#endif
			/*
			// update the entry to point to cluster 0
			*/
			entry->raw.ENTRY.STD.first_cluster_lo = 0x0;
			entry->raw.ENTRY.STD.first_cluster_hi = 0x0;
			entry->raw.ENTRY.STD.size = 0x0;
			handle->directory_entry = *entry;
			handle->current_size = entry->size;
			/*
			// mark the cached sector as unknown
			*/
			FAT_SET_LOADED_SECTOR(volume, FAT_UNKNOWN_SECTOR);
			/*
			// read the sector that contains the entry
			*/
			bSuccess = volume->device->Read(entry->sector_addr, buffer);
			ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (ret != STORAGE_SUCCESS)
			{
				handle->magic = 0;
				return FAT_CANNOT_READ_MEDIA;
			}
			/*
			// copy the modified file entry to the sector buffer
			*/
#if defined(NO_STRUCT_PACKING) || defined(BIG_ENDIAN)
			fat_write_raw_directory_entry(&entry->raw, buffer + entry->sector_offset);
#else
			memcpy(buffer + entry->sector_offset, &entry->raw, sizeof(FAT_RAW_DIRECTORY_ENTRY));
#endif
			/*
			// write the modified entry to the media
			*/
			bSuccess = volume->device->Write(entry->sector_addr, buffer);
			ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (ret != STORAGE_SUCCESS)
			{
				handle->magic = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}
			/*
			// free the clusters allocated to the file
			*/
			/* if (size > 0) */
			fat_free_cluster_chain(volume, handle->current_clus_addr);
		}
		/*
		// reset the file handle to reflect an
		// empty file
		*/
		handle->current_size = 0x0;
		handle->current_sector_idx = 0x0;
		handle->current_clus_addr = 0x0;
		handle->current_clus_idx = 0;
		handle->buffer_head = handle->buffer;
		handle->no_of_clusters_after_pos = 0;
	}
	else
#endif
	{
		/*
		// if no append or overwrite flags are set then
		// simply set the handle to point to the beggining
		// of the 1st cluster
		*/
		handle->current_sector_idx = 0x0;
		handle->buffer_head = handle->buffer;
	}
	return FAT_SUCCESS;
}


/*
// sets the buffer of the file
*/
uint16 fat_file_set_buffer(FAT_FILE* file, uint8* buffer)
{
	if (file->buffer_head != file->buffer)
	{
		uint16	ret;
		uint32	sector_address;
		bool	bSuccess;

		sector_address = file->current_sector_idx + FIRST_SECTOR_OF_CLUSTER(file->volume, file->current_clus_addr);
		file->buffer = buffer;
		file->buffer_head = buffer + (uintptr_t)file->buffer_head;
		bSuccess = file->volume->device->Read(sector_address, file->buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
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
uint32 fat_file_get_unique_id(FAT_FILE* file)
{
	/*return file->directory_entry.raw.ENTRY.STD.first_cluster_lo
		& (file->directory_entry.raw.ENTRY.STD.first_cluster_hi << 16); */

	uint32 id;
	((uint16*)&id)[INT32_WORD0] = file->directory_entry.raw.ENTRY.STD.first_cluster_lo;

	if (file->volume->fs_type == FAT_FS_TYPE_FAT32)
	{
		((uint16*)&id)[INT32_WORD1] = file->directory_entry.raw.ENTRY.STD.first_cluster_hi;
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
uint16 fat_file_delete(FAT_VOLUME* volume, char* filename)
{
#if defined(FAT_READ_ONLY)
	return FAT_FEATURE_NOT_SUPPORTED;
#else
	uint16					ret;
	uint32					first_cluster;
	FAT_DIRECTORY_ENTRY		entry;
	uint8					buffer[MAX_SECTOR_LENGTH];
	bool					bSuccess;

#if !defined(FAT_DISABLE_LONG_FILENAMES)
	char path_part[256];
	char* name_part;
	uint8 checksum;
	FAT_FILESYSTEM_QUERY query;
#endif

	/*
	// get the entry for the file
	*/
	ret = fat_get_file_entry(volume, filename, &entry);
	if (ret != FAT_SUCCESS)
		return ret;
	/*
	// if the entry is located go ahead and delete it.
	*/
	if (entry.name != 0)
	{
		/*
		// compute the checksum for the file
		*/
#if !defined(FAT_DISABLE_LONG_FILENAMES)
		checksum = fat_long_entry_checksum((uint8*)entry.raw.ENTRY.STD.name);
#endif
		/*
		// make sure we're not trying to delete a directory
		*/
		if (entry.attributes & FAT_ATTR_DIRECTORY)
			return FAT_NOT_A_FILE;
		/*
		// find the entry's first cluster address
		*/
		((uint16*)&first_cluster)[INT32_WORD0] = entry.raw.ENTRY.STD.first_cluster_lo;
		((uint16*)&first_cluster)[INT32_WORD1] = (volume->fs_type == FAT_FS_TYPE_FAT32) ? entry.raw.ENTRY.STD.first_cluster_hi : 0;
		/*
		// free the file's cluster change
		*/
		if (first_cluster)
		{
			ret = fat_free_cluster_chain(volume, first_cluster);
			if (ret != FAT_SUCCESS)
				return ret;
		}
		/*
		// mark the entry as deleted
		*/
		entry.raw.ENTRY.STD.name[0] = FAT_DELETED_ENTRY;

		bSuccess = volume->device->Read(entry.sector_addr, buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
			return ret;

		memcpy(buffer + entry.sector_offset, &entry.raw, sizeof(entry.raw));

		bSuccess = volume->device->Write(entry.sector_addr, buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
		{
			return ret;
		}
	}

#if !defined(FAT_DISABLE_LONG_FILENAMES)
	/*
	// parse the filename
	*/
	fat_parse_path(filename, path_part, &name_part);
	/*
	// get the 1st LFN entry of the parent directory
	*/
	ret = fat_find_first_entry(volume, path_part, FAT_ATTR_LONG_NAME, 0, &query);
	if (ret != FAT_SUCCESS)
		return ret;
	/*
	// loop through each entry.
	*/
	while (*query.current_entry.raw.ENTRY.STD.name != 0)
	{
		if (query.current_entry.raw.ENTRY.LFN.lfn_checksum == checksum)
		{
			/*
			// mark the entry as deleted
			*/
			query.current_entry.raw.ENTRY.STD.name[0] = FAT_DELETED_ENTRY;

			bSuccess = volume->device->Read(query.current_entry.sector_addr, buffer);
			ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (ret != STORAGE_SUCCESS)
			{
				return ret;
			}

			memcpy(buffer + query.current_entry.sector_offset, &query.current_entry.raw, sizeof(entry.raw));

			bSuccess = volume->device->Write(query.current_entry.sector_addr, buffer);
			ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (ret != STORAGE_SUCCESS)
			{
				return ret;
			}
		}
		/*
		// get the next LFN entry
		*/
		fat_find_next_entry(volume, 0, &query);
	}
#endif
	/*
	// return success code
	*/
	return FAT_SUCCESS;
#endif
}

/*
// renames a file
*/
uint16 fat_file_rename(FAT_VOLUME* volume, char* original_filename, char* new_filename)
{
#if defined(FAT_READ_ONLY)
	return FAT_FEATURE_NOT_SUPPORTED;
#else
	uint16					ret;
	uint32					entry_cluster;
	char					new_parent[256];
	char*					original_filename_part;
	FAT_DIRECTORY_ENTRY		original_entry;
	FAT_DIRECTORY_ENTRY		new_entry;
	bool					bSuccess;

#if !defined(FAT_DISABLE_LONG_FILENAMES)
	uint8					checksum = 0;
	char					original_parent[256];
#else
	char					original_parent[13];
#endif

	char*					new_filename_part;

#if defined(FAT_ALLOCATE_VOLUME_BUFFER)
	uint8* buffer = file->volume->sector_buffer;
#elif defined(FAT_ALLOCATE_SHARED_BUFFER)
	uint8* buffer = fat_shared_buffer;
#else
	ALIGN16 uint8 buffer[MAX_SECTOR_LENGTH];
#endif

	/*
	// parse paths
	*/
	fat_parse_path(original_filename, original_parent, &original_filename_part);
	fat_parse_path(new_filename, new_parent, &new_filename_part);
	/*
	// try to get the new entry to see if it exists.
	*/
	fat_get_file_entry(volume, new_filename, &new_entry);
	if (*new_entry.name != 0)
		return FAT_FILENAME_ALREADY_EXISTS;

	/*
	// get the directory entry
	*/
	fat_get_file_entry(volume, original_filename, &original_entry);

	if (*original_entry.name != 0)
	{
		FAT_DIRECTORY_ENTRY parent;
		/*
		// compute the checksum for the file
		*/
#if !defined(FAT_DISABLE_LONG_FILENAMES)
		checksum = fat_long_entry_checksum((uint8*)original_entry.raw.ENTRY.STD.name);
#endif
		/*
		// get the cluster # for the entry
		*/
		((uint16*)&entry_cluster)[INT32_WORD0] = original_entry.raw.ENTRY.STD.first_cluster_lo;
		((uint16*)&entry_cluster)[INT32_WORD1] = original_entry.raw.ENTRY.STD.first_cluster_hi;
		/*
		// get the new parent entry
		*/
		ret = fat_get_file_entry(volume, new_parent, &parent);
		if (ret != FAT_SUCCESS)
			return ret;
		/*
		// create the new entry in the parent folder
		*/
		ret = fat_create_directory_entry(volume, &parent.raw, new_filename_part, original_entry.attributes, entry_cluster, &new_entry);
		if (ret != FAT_SUCCESS)
			return ret;
		/*
		// copy all info except name from the old entry to the new one
		*/
		new_entry.raw.ENTRY.STD.access_date = original_entry.raw.ENTRY.STD.access_date;
		new_entry.raw.ENTRY.STD.attributes = original_entry.raw.ENTRY.STD.attributes;
		new_entry.raw.ENTRY.STD.create_date = original_entry.raw.ENTRY.STD.create_date;
		new_entry.raw.ENTRY.STD.create_time = original_entry.raw.ENTRY.STD.create_time;
		new_entry.raw.ENTRY.STD.create_time_tenth = original_entry.raw.ENTRY.STD.create_time_tenth;
		new_entry.raw.ENTRY.STD.first_cluster_hi = original_entry.raw.ENTRY.STD.first_cluster_hi;
		new_entry.raw.ENTRY.STD.first_cluster_lo = original_entry.raw.ENTRY.STD.first_cluster_lo;
		new_entry.raw.ENTRY.STD.modify_date = original_entry.raw.ENTRY.STD.modify_date;
		new_entry.raw.ENTRY.STD.modify_time = original_entry.raw.ENTRY.STD.modify_time;
		new_entry.raw.ENTRY.STD.reserved = original_entry.raw.ENTRY.STD.reserved;
		new_entry.raw.ENTRY.STD.size = original_entry.raw.ENTRY.STD.size;
		/*
		// write modified entry to drive
		*/
		FAT_SET_LOADED_SECTOR(volume, FAT_UNKNOWN_SECTOR);
		bSuccess = volume->device->Read(new_entry.sector_addr, buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
		{
			return ret;
		}
		memcpy(buffer + new_entry.sector_offset, &new_entry.raw, sizeof(new_entry.raw));
		bSuccess = volume->device->Write(new_entry.sector_addr, buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
		{
			return ret;
		}
		/*
		// mark the original entry as deleted.
		*/
		*original_entry.raw.ENTRY.STD.name = FAT_DELETED_ENTRY;
		bSuccess = volume->device->Read(original_entry.sector_addr, buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
		{
			return ret;
		}
		memcpy(buffer + original_entry.sector_offset, &original_entry.raw, sizeof(original_entry.raw));
		bSuccess = volume->device->Write(original_entry.sector_addr, buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
		{
			return ret;
		}
		/*
		// release lock on the buffer
		*/
	}
#if !defined(FAT_DISABLE_LONG_FILENAMES)
	{
		FAT_FILESYSTEM_QUERY query;
		/*
		// get the 1st LFN entry of the parent directory
		*/
		ret = fat_find_first_entry(volume, original_parent, FAT_ATTR_LONG_NAME, 0, &query);
		if (ret != FAT_SUCCESS)
			return ret;
		/*
		// loop through each entry.
		*/
		while (*query.current_entry.raw.ENTRY.STD.name != 0)
		{
			if (query.current_entry.raw.ENTRY.LFN.lfn_checksum == checksum)
			{
				/*
				// mark the entry as deleted
				*/
				FAT_SET_LOADED_SECTOR(volume, FAT_UNKNOWN_SECTOR);
				query.current_entry.raw.ENTRY.STD.name[0] = FAT_DELETED_ENTRY;
				bSuccess = volume->device->Read(query.current_entry.sector_addr, buffer);
				ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
				if (ret != STORAGE_SUCCESS)
				{
					return ret;
				}
				memcpy(buffer + query.current_entry.sector_offset, &query.current_entry.raw, sizeof(query.current_entry.raw));
				bSuccess = volume->device->Write(query.current_entry.sector_addr, buffer);
				ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
				if (ret != STORAGE_SUCCESS)
				{
					return ret;
				}
			}
			/*
			// get the next LFN entry
			*/
			fat_find_next_entry(volume, 0, &query);
		}
	}
#endif

	return FAT_SUCCESS;
#endif
}

/*
// pre-allocates disk space for a file
*/
uint16 fat_file_alloc(FAT_FILE* file, uint32 bytes)
{
#if defined(FAT_READ_ONLY)
	return FAT_FEATURE_NOT_SUPPORTED;
#else
	uint16	ret;
	uint32	new_cluster;
	uint32	no_of_clusters_needed;
	bool	bSuccess;
#if defined(FAT_ALLOCATE_VOLUME_BUFFER)
	uint8*	buffer = file->volume->sector_buffer;
#elif defined(FAT_ALLOCATE_SHARED_BUFFER)
	uint8*	buffer = fat_shared_buffer;
#else
	ALIGN16 uint8 buffer[MAX_SECTOR_LENGTH];
#endif
	/*
	// check that this is a valid handle
	*/
	if (file->magic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}


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
	/*
	// calculate how many clusters we need
	*/
	no_of_clusters_needed = (bytes + file->volume->no_of_bytes_per_serctor - 1) / file->volume->no_of_bytes_per_serctor;
	no_of_clusters_needed = (no_of_clusters_needed + file->volume->no_of_sectors_per_cluster - 1) / file->volume->no_of_sectors_per_cluster;
	no_of_clusters_needed = (file->no_of_clusters_after_pos > no_of_clusters_needed) ? 0 : (no_of_clusters_needed - file->no_of_clusters_after_pos);
	/*
	// if we already got all the clusters requested then thre's nothing to do
	*/
	if (no_of_clusters_needed == 0)
	{
		file->busy = 0;
		return FAT_SUCCESS;
	}
	/*
	// allocate a new cluster
	*/
#if defined(FAT_OPTIMIZE_FOR_FLASH)
	{
		if (file->access_flags & FAT_FILE_FLAG_OPTIMIZE_FOR_FLASH)
		{
			uint32 current_cluster;
			uint32 next_cluster;
			uint32 page_size;
			uint32 start_address;
			uint32 end_address;

			page_size = file->volume->device->GetPageSize();

			if (page_size > file->volume->no_of_sectors_per_cluster)
			{
				uint32 clusters_per_page = page_size / file->volume->no_of_sectors_per_cluster;

				if (no_of_clusters_needed % clusters_per_page)
				{
					no_of_clusters_needed += clusters_per_page - (no_of_clusters_needed % clusters_per_page);
				}

				if ((no_of_clusters_needed % clusters_per_page) != 0)
				{
					return FAT_UNKNOWN_ERROR;
				}

				new_cluster = fat_allocate_data_cluster_ex(file->volume, no_of_clusters_needed, 0, page_size, &ret);
				if (ret != FAT_SUCCESS)
				{
					file->busy = 0;
					return ret;
				}
			}
			else
			{
				new_cluster = fat_allocate_data_cluster(file->volume, no_of_clusters_needed, 1, &ret);
				if (ret != FAT_SUCCESS)
				{
					file->busy = 0;
					return ret;
				}
			}
			/*
			// find out how many clusters are allocated sequentially
			// to this file following the current cursor location
			*/
			current_cluster = new_cluster;

			while (!fat_is_eof_entry(file->volume, current_cluster))
			{
				/*
				// calculate the start and end address the cluster
				*/
				start_address = FIRST_SECTOR_OF_CLUSTER(file->volume, current_cluster);
				end_address = start_address + file->volume->no_of_sectors_per_cluster;
				/*
				// find the last sequential sector after this address
				*/
				while (!fat_is_eof_entry(file->volume, current_cluster))
				{
					fat_get_cluster_entry(file->volume, current_cluster, &next_cluster);

					if (next_cluster == (current_cluster + 1))
					{
						end_address += file->volume->no_of_sectors_per_cluster;
						current_cluster = next_cluster;
					}
					else
					{
						current_cluster = next_cluster;
						break;
					}
				}
				/*
				// pre-erase the clusters
				*/
				file->volume->device->Erase(start_address, end_address - 1);
			}
		}
		else
		{
			new_cluster = fat_allocate_data_cluster(file->volume, no_of_clusters_needed, 0, &ret);
			if (ret != FAT_SUCCESS)
			{
				file->busy = 0;
				return ret;
			}
		}
	}
#else
	new_cluster = fat_allocate_data_cluster(file->volume, no_of_clusters_needed, 0, &ret);
	if (ret != FAT_SUCCESS)
	{
		file->busy = 0;
		return ret;
	}
#endif
	/*
	// if this is the 1st cluster cluster allocated
	// to the file then we must modify the file's entry
	*/
	if (!file->directory_entry.raw.ENTRY.STD.first_cluster_lo && !file->directory_entry.raw.ENTRY.STD.first_cluster_hi)
	{
		/*
		// modify the file entry to point to the
		// new cluster
		*/
		file->directory_entry.raw.ENTRY.STD.first_cluster_lo = LO16(new_cluster);
		file->directory_entry.raw.ENTRY.STD.first_cluster_hi = HI16(new_cluster);
		/*
		// mark the cached sector as unknown
		*/
		FAT_SET_LOADED_SECTOR(file->volume, FAT_UNKNOWN_SECTOR);
		/*
		// try load the sector that contains the entry
		*/
		bSuccess = file->volume->device->Read(file->directory_entry.sector_addr, buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
		{
			file->busy = 0;
			return ret;
		}
		/*
		// copy the modified file entry to the
		// sector buffer
		*/
#if defined(NO_STRUCT_PACKING) || defined(BIG_ENDIAN)
		fat_write_raw_directory_entry(&file->directory_entry.raw, buffer + file->directory_entry.sector_offset);
#else
		memcpy(buffer + file->directory_entry.sector_offset, &file->directory_entry.raw, sizeof(FAT_RAW_DIRECTORY_ENTRY));
#endif
		/*
		// write the modified entry to the media
		*/
		bSuccess = file->volume->device->Write(file->directory_entry.sector_addr, buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
		{
			file->busy = 0;
			return ret;
		}
	}
	/*
	// if there are clusters allocated to the file update the last FAT entry
	// of the file to point to the newly allocated cluster(s)
	*/
	else
	{

		/*
		uint32 last_cluster;
		uint32 cluster_number;
		//
		// calculate the last cluster number (it's zero based index)
		//
		cluster_number = (file->current_size + file->volume->no_of_bytes_per_serctor - 1) / file->volume->no_of_bytes_per_serctor;
		cluster_number = cluster_number / file->volume->no_of_sectors_per_cluster; // round down
		//
		// set last_cluster to the 1st cluster of the file
		//
		((uint16*) &last_cluster)[INT32_WORD0] = file->directory_entry.raw.first_cluster_lo;
		((uint16*) &last_cluster)[INT32_WORD1] = (file->volume->fs_type == FAT_FS_TYPE_FAT32) ? file->directory_entry.raw.first_cluster_hi : 0;
		//
		// if the file has more than 1 cluster follow the change to
		// the last cluster and store it's value on last_cluster
		//
		if (cluster_number)
		{
			if (!fat_increase_cluster_address(file->volume, last_cluster, cluster_number - 1, &last_cluster))
				return FAT_CORRUPTED_FILE;
		}
		*/

		uint32 last_cluster;

		if (file->no_of_clusters_after_pos)
		{
			if (!fat_increase_cluster_address(file->volume, file->current_clus_addr, file->no_of_clusters_after_pos, &last_cluster))
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
		ret = fat_set_cluster_entry(file->volume, last_cluster, (FAT_ENTRY)new_cluster);
		if (ret != FAT_SUCCESS)
		{
			file->busy = 0;
			return ret;
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
#endif
}

/*
// moves the file cursor to the specified offset
*/
uint16 fat_file_seek(FAT_FILE* file, uint32 offset, char mode)
{
	uint32	new_pos;
	uint32	sector_count;
	uint32	cluster_count;
	uint32	old_cluster;
	uint32	sector_address;
	uint16	ret;
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
		new_pos = file->current_clus_idx * file->volume->no_of_sectors_per_cluster * file->volume->no_of_bytes_per_serctor;
		new_pos += file->current_sector_idx * file->volume->no_of_bytes_per_serctor;
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
		if (new_pos % file->volume->no_of_bytes_per_serctor)
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
	sector_count = (new_pos + file->volume->no_of_bytes_per_serctor - 1) / file->volume->no_of_bytes_per_serctor;
	/*
	// set the 1st cluster as the current cluster, we'll seek from there
	*/
	((uint16*)&file->current_clus_addr)[INT32_WORD0] = file->directory_entry.raw.ENTRY.STD.first_cluster_lo;
	((uint16*)&file->current_clus_addr)[INT32_WORD1] =
		(file->volume->fs_type == FAT_FS_TYPE_FAT32) ? file->directory_entry.raw.ENTRY.STD.first_cluster_hi : 0;
	/*
	// if the file occupies more than one cluster
	*/
	if (sector_count > file->volume->no_of_sectors_per_cluster)
	{
		/*
		// calculate the count of clusters occupied by the file and
		// update the ClustersAllocated value of the file
		*/
		cluster_count = (sector_count + file->volume->no_of_sectors_per_cluster - 1) / file->volume->no_of_sectors_per_cluster;
		/*
		// set the file handle to point to the last cluster. if the file doesn't have
		// that many clusters allocated this function will return 0. if that ever happens it means
		// that the file is corrupted
		*/
		if (!fat_increase_cluster_address(
			file->volume, file->current_clus_addr, (cluster_count - 1), &file->current_clus_addr))
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
		file->current_sector_idx = (((new_pos + file->volume->no_of_bytes_per_serctor - 1) / file->volume->no_of_bytes_per_serctor) - 1) % file->volume->no_of_sectors_per_cluster;
		file->buffer_head = (uint8*)((uintptr_t)file->buffer) + (new_pos % file->volume->no_of_bytes_per_serctor);

		if (new_pos % file->volume->no_of_bytes_per_serctor == 0)
		{
			file->buffer_head = (uint8*)((uintptr_t)file->buffer) + file->volume->no_of_bytes_per_serctor;
		}
	}
	else
	{
		file->current_sector_idx = 0;
		file->buffer_head = file->buffer;
	}

	sector_address = file->current_sector_idx + FIRST_SECTOR_OF_CLUSTER(file->volume, file->current_clus_addr);
	file->current_clus_idx = cluster_count - 1;
	/*
	// load the last sector
	*/
	if (file->buffer)
	{
		bSuccess = file->volume->device->Read(sector_address, file->buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
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

/*
// writes a file synchronously
*/
uint16 fat_file_write(FAT_FILE* handle, uint8* buffer, uint32 length)
{
#if defined(FAT_READ_ONLY)
	return FAT_FEATURE_NOT_SUPPORTED;
#else
	return fat_file_write_internal(handle, buffer, length, 0, 0, 0);
#endif
}

/*
// writes a file asynchronously
*/
uint16 fat_file_write_async(FAT_FILE* handle, uint8* buffer, uint32 length, uint16* result, FAT_ASYNC_CALLBACK* callback, void* callback_context)
{
#if defined(FAT_READ_ONLY)
	return FAT_FEATURE_NOT_SUPPORTED;
#else
	/*
	// make sure the user supplied a state pointer
	*/
	if (!result)
		return FAT_INVALID_PARAMETERS;
	/*
	// call internal write function
	*/
	return fat_file_write_internal(handle, buffer, length, result, callback, callback_context);
#endif
}

/*
// writes to a file
*/
#if !defined(FAT_READ_ONLY)
uint16 fat_file_write_internal(FAT_FILE* handle, uint8* buff, uint32 length, uint16* async_state, FAT_ASYNC_CALLBACK* callback, void* callback_context)
{
	uint32 ret;
	/*
	// check that this is a valid handle
	*/
	if (handle->magic != FAT_OPEN_HANDLE_MAGIC)
		return FAT_INVALID_HANDLE;
	/*
	// check that the file is open for write access
	*/
	if (!(handle->access_flags & FAT_FILE_ACCESS_WRITE))
		return FAT_FILE_NOT_OPENED_FOR_WRITE_ACCESS;
	/*
	// make sure length is not larger than 16-bit
	*/
	if (length > 0xFFFF)
		return FAT_BUFFER_TOO_BIG;
	/*
	// if there's no clusters allocated to this file allocate
	// enough clusters for this request
	*/
	ret = fat_file_alloc(handle, length);
	if (ret != FAT_SUCCESS)
		return ret;
	/*
	// make sure that either a buffer is set or the file has been opened in
	// unbuffered mode.
	*/
	if (!handle->buffer && !(handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING))
		return FAT_FILE_BUFFER_NOT_SET;
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
	handle->op_state.pos = handle->current_clus_idx * handle->volume->no_of_sectors_per_cluster * handle->volume->no_of_bytes_per_serctor;
	handle->op_state.pos += (handle->current_sector_idx) * handle->volume->no_of_bytes_per_serctor;
	handle->op_state.pos += (uintptr_t)(handle->buffer_head - handle->buffer);
	/*
	// if the file is opened in unbuffered mode make sure that
	// we're reading a multiple of the sector size
	*/
	if (handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (length % handle->volume->no_of_bytes_per_serctor)
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
		handle->op_state.end_of_buffer = handle->buffer + handle->volume->no_of_bytes_per_serctor;
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
		FIRST_SECTOR_OF_CLUSTER(handle->volume, handle->current_clus_addr);


	handle->op_state.callback = callback;
	handle->op_state.callback_context = callback_context;
	handle->op_state.storage_callback_info.Callback = (STORAGE_CALLBACK)&fat_file_write_callback;
	handle->op_state.storage_callback_info.Context = handle;
	handle->op_state.async_state = async_state;
	handle->op_state.internal_state = 0x0;
	handle->op_state.length = (uint16)length;
	handle->op_state.buffer = buff;
	/*
	// call the callback routine which actually does
	// the initial work and all succeeding callbacks
	*/
	if (async_state)
	{
		/*
		// set result code to op in progress
		*/
		*async_state = FAT_OP_IN_PROGRESS;
		/*
		// invoke internal write function
		*/
		fat_file_write_callback(handle, async_state);
		/*
		// return op in progress code
		*/
		return FAT_OP_IN_PROGRESS;
	}
	else
	{
		uint16 internal_async_state;
		fat_file_write_callback(handle, &internal_async_state);
		return internal_async_state;
	}
}
#endif


/*
// asynchronous write callback
*/
#if !defined(FAT_READ_ONLY)
void fat_file_write_callback(FAT_FILE* handle, uint16* async_state_in)
{
	uint16	ret;
	uint16* async_state;
	bool	bSuccess;

	if (handle->op_state.async_state)
	{
		async_state = handle->op_state.async_state;
	}
	else
	{
		async_state = async_state_in;
	}

	/*
	// loop while there are bytes to be
	// read
	*/
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
				handle->op_state.end_of_buffer = handle->buffer + handle->volume->no_of_bytes_per_serctor;
			}
			else
			{
				handle->op_state.end_of_buffer = handle->buffer + handle->volume->no_of_bytes_per_serctor;
				handle->buffer_head = handle->buffer;
			}
			/*
			// write the cached sector to media
			*/
			bSuccess = handle->volume->device->Write(handle->op_state.sector_addr, handle->buffer);
			ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;

			/*
			// if we were unable to read the sector
			// then throw an error
			*/
			if (ret != STORAGE_SUCCESS)
			{
				/*
				// set result code
				*/
				*async_state = FAT_CANNOT_WRITE_MEDIA;
				/*
				// mark the file handle as no longer in use
				*/
				handle->busy = 0;
				/*
				// invoke callback function
				*/
				if (handle->op_state.callback)
					handle->op_state.callback(handle->op_state.callback_context, async_state);
				/*
				// leave
				*/
				return;
			}
			/*
			// if this sector is the last of the current cluster then
			// locate the next cluster and continue writing
			*/
			if (handle->current_sector_idx == handle->volume->no_of_sectors_per_cluster - 1)
			{
				ret = fat_get_cluster_entry(handle->volume,
					handle->current_clus_addr, &handle->current_clus_addr);
				if (ret != STORAGE_SUCCESS)
				{
					/*
					// set result code
					*/
					*async_state = FAT_CANNOT_WRITE_MEDIA;
					/*
					// mark the file handle as no longer in use
					*/
					handle->busy = 0;
					/*
					// invoke callback function
					*/
					if (handle->op_state.callback)
						handle->op_state.callback(handle->op_state.callback_context, async_state);
					/*
					// leave
					*/
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
					FIRST_SECTOR_OF_CLUSTER(handle->volume, handle->current_clus_addr);
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
			/*if (!(handle->op_state.pos % (handle->volume->no_of_bytes_per_serctor * FAT_FLUSH_INTERVAL)))
			{
				fat_file_flush(handle);
			}*/
		}
		if (handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
		{
			handle->buffer_head = handle->op_state.end_of_buffer;
			handle->op_state.pos += handle->volume->no_of_bytes_per_serctor;

			if (handle->op_state.pos >= handle->current_size)
			{
				handle->current_size = handle->op_state.pos;
			}
			handle->op_state.bytes_remaining -= handle->volume->no_of_bytes_per_serctor;
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
	/*
	// return success
	*/
	*async_state = FAT_SUCCESS;
	/*
	// mark the file handle as no longer in use
	*/
	handle->busy = 0;
	/*
	// invoke callback function
	*/
	if (handle->op_state.callback)
		handle->op_state.callback(handle->op_state.callback_context, async_state);
	/*
	// leave
	*/
	return;
}
#endif


/*
// reads from a file synchronously
*/
uint16 fat_file_read(FAT_FILE* hdl, uint8* buffer, uint32 length, uint32* bytes_read)
{
	return fat_file_read_internal(hdl, buffer, length, bytes_read, 0, 0, 0);
}


/*
// reads data from a file asynchronously
*/
uint16 fat_file_read_async(FAT_FILE* handle, uint8* buff, uint32 length, uint32* bytes_read, uint16* state, FAT_ASYNC_CALLBACK* callback, void* callback_context)
{
	/*
	// make sure the user supplied a state pointer
	*/
	if (!state)
		return FAT_INVALID_PARAMETERS;
	/*
	// call internal read function
	*/
	return fat_file_read_internal(handle, buff, length, bytes_read, state, callback, callback_context);
}


/*
// reads from a file synchronously or asynchronously
*/
uint16 fat_file_read_internal(FAT_FILE* handle, uint8* buff, uint32 length, uint32* bytes_read, uint16* async_state, FAT_ASYNC_CALLBACK* callback, void* callback_context)
{
	/*uint32 ret;*/
	/*
	// check that this is a valid handle
	*/
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
	handle->op_state.pos = handle->current_clus_idx * handle->volume->no_of_sectors_per_cluster * handle->volume->no_of_bytes_per_serctor;
	handle->op_state.pos += (handle->current_sector_idx) * handle->volume->no_of_bytes_per_serctor;
	handle->op_state.pos += (uintptr_t)(handle->buffer_head - handle->buffer);
	/*
	// calculate the address of the current
	// sector and the address of the end of the buffer
	*/
	handle->op_state.sector_addr = handle->current_sector_idx + FIRST_SECTOR_OF_CLUSTER(handle->volume, handle->current_clus_addr);
	handle->op_state.end_of_buffer = handle->buffer + handle->volume->no_of_bytes_per_serctor;
	/*
	// if the file is opened in unbuffered mode make sure that
	// we're reading a multiple of the sector size and set the buffer
	// to the one supplied by the user in this call
	*/
	if (handle->access_flags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (length % handle->volume->no_of_bytes_per_serctor)
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
	handle->op_state.callback = callback;
	handle->op_state.callback_context = callback_context;
	handle->op_state.storage_callback_info.Callback = (STORAGE_CALLBACK)&fat_file_read_callback;
	handle->op_state.storage_callback_info.Context = handle;
	handle->op_state.async_state = async_state;
	handle->op_state.internal_state = 0x0;
	handle->op_state.length = (uint16)length;
	handle->op_state.buffer = buff;
	handle->op_state.bytes_read = bytes_read;
	/*
	// call the callback routine which actually does
	// the initial work and all succeeding callbacks
	*/
	if (async_state)
	{
		/*
		// set async state to op in progress.
		*/
		*async_state = FAT_OP_IN_PROGRESS;
		/*
		// begin read
		*/
		fat_file_read_callback(handle, async_state);
		/*
		// return the value returned by the callback
		// routine
		*/
		return FAT_OP_IN_PROGRESS;
	}
	else
	{
		uint16 internal_async_state;
		fat_file_read_callback(handle, &internal_async_state);
		return internal_async_state;
	}
}

/*
// asynchronous write callback
*/
void fat_file_read_callback(FAT_FILE* handle, uint16* async_state)
{
	uint16	ret;
	bool	bSuccess;
	/*
	// reset the # of bytes read
	*/
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
		bSuccess = handle->volume->device->Read(handle->op_state.sector_addr, handle->buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
		{
			*async_state = FAT_CANNOT_READ_MEDIA;
			/*
			// mark the file handle as no longer in use
			*/
			handle->busy = 0;
			/*
			// invoke callback function
			*/
			if (handle->op_state.callback)
				handle->op_state.callback(handle->op_state.callback_context, async_state);
			/*
			// leave
			*/
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
			handle->op_state.bytes_remaining -= handle->volume->no_of_bytes_per_serctor;
			handle->buffer_head += handle->volume->no_of_bytes_per_serctor;
			handle->op_state.end_of_buffer = handle->buffer_head;
			if (handle->op_state.bytes_read)
				*handle->op_state.bytes_read += handle->volume->no_of_bytes_per_serctor;

			handle->op_state.pos += handle->volume->no_of_bytes_per_serctor;

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
				handle->op_state.end_of_buffer = handle->buffer_head + handle->volume->no_of_bytes_per_serctor;
			}
			else
			{
				handle->buffer_head = handle->buffer;
			}
			/*
			// if this sector is the last of the current cluster
			// we must find the next cluster
			*/
			if (handle->current_sector_idx == handle->volume->no_of_sectors_per_cluster - 1)
			{
				/*
				// update the cluster address with the address of the
				// next cluster
				*/
				if (!fat_increase_cluster_address(handle->volume, handle->current_clus_addr, 1, &handle->current_clus_addr))
				{
					*async_state = FAT_CORRUPTED_FILE;
					/*
					// mark the file handle as no longer in use
					*/
					handle->busy = 0;
					/*
					// invoke callback function
					*/
					if (handle->op_state.callback)
						handle->op_state.callback(handle->op_state.callback_context, async_state);
					/*
					// leave
					*/
					return;
				}
				/*
				// reset the current sector increase the current cluster
				// number and calculate the address of the first sector of the
				// cluster
				*/
				handle->current_clus_idx++;
				handle->current_sector_idx = 0x0;
				handle->op_state.sector_addr = FIRST_SECTOR_OF_CLUSTER(handle->volume, handle->current_clus_addr);
			}
			else
			{
				/*
				// if there are more sectors in the current cluster then
				// simply increase the current sector counter and address
				*/
				handle->current_sector_idx++;
				handle->op_state.sector_addr++;
			}

			/*
			// read the next sector into the cache
			*/
			bSuccess = handle->volume->device->Read(handle->op_state.sector_addr, handle->buffer);
			ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (ret != STORAGE_SUCCESS)
			{
				/*
				// set the error code
				*/
				*async_state = FAT_CANNOT_READ_MEDIA;
				/*
				// mark the file handle as no longer in use
				*/
				handle->busy = 0;
				/*
				// invoke callback function
				*/
				if (handle->op_state.callback)
					handle->op_state.callback(handle->op_state.callback_context, async_state);
				/*
				// leave
				*/
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
			handle->op_state.bytes_remaining -= handle->volume->no_of_bytes_per_serctor;
			if (handle->op_state.bytes_read)
				(*handle->op_state.bytes_read) += handle->volume->no_of_bytes_per_serctor;

			handle->op_state.pos += handle->volume->no_of_bytes_per_serctor;

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
	*async_state = FAT_SUCCESS;
	/*
	// mark file handle as no longer in use
	*/
	handle->busy = 0;
	/*
	// invoke callback function
	*/
	if (handle->op_state.callback)
		handle->op_state.callback(handle->op_state.callback_context, async_state);
	/*
	// leave
	*/
	return;
}

/*
// flushes file buffers
*/
uint16 fat_file_flush(FAT_FILE* handle)
{
#if defined(FAT_READ_ONLY)
	return FAT_FEATURE_NOT_SUPPORTED;
#else
	uint16	ret;
	uint32	sector_address = 0;
	bool	bSuccess;
#if defined(FAT_ALLOCATE_VOLUME_BUFFER)
	uint8*	buffer = handle->volume->sector_buffer;
#elif defined(FAT_ALLOCATE_SHARED_BUFFER)
	uint8*	buffer = fat_shared_buffer;
#else
	ALIGN16 uint8 buffer[MAX_SECTOR_LENGTH];
#endif
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
				FIRST_SECTOR_OF_CLUSTER(handle->volume, handle->current_clus_addr);
			/*
			// if the buffer is only partially filled we need to merge it
			// with the one on the drive
			*/
			if (handle->buffer_head <= handle->buffer + handle->volume->no_of_bytes_per_serctor)
			{
				uint16 i;
				uint8 buff[MAX_SECTOR_LENGTH];
				bSuccess = handle->volume->device->Read(sector_address, buff);
				ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
				if (ret != STORAGE_SUCCESS)
				{
					handle->busy = 0;
					return ret;
				}

				for (i = (uint16)(handle->buffer_head -
					handle->buffer); i < handle->volume->no_of_bytes_per_serctor; i++)
				{
					handle->buffer[i] = buff[i];
				}
			}
			/*
			// write the cached sector to media
			*/
			bSuccess = handle->volume->device->Write(sector_address, handle->buffer);
			ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (ret != STORAGE_SUCCESS)
			{
				handle->busy = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}
		}
		/*
		// update the file size on the entry
		*/
		handle->directory_entry.raw.ENTRY.STD.size = handle->current_size;
		handle->directory_entry.raw.ENTRY.STD.modify_date = rtc_get_fat_date();
		handle->directory_entry.raw.ENTRY.STD.modify_time = rtc_get_fat_time();
		handle->directory_entry.raw.ENTRY.STD.access_date = handle->directory_entry.raw.ENTRY.STD.modify_date;
		/*
		// try load the sector that contains the entry
		*/
		FAT_SET_LOADED_SECTOR(volume, FAT_UNKNOWN_SECTOR);

		bSuccess = handle->volume->device->Read(handle->directory_entry.sector_addr, buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
		{
			handle->busy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}
		/*
		// copy the modified file entry to the
		// sector buffer
		*/
#if defined(NO_STRUCT_PACKING) || defined(BIG_ENDIAN)
		fat_write_raw_directory_entry(&handle->directory_entry.raw, buffer + handle->directory_entry.sector_offset);
#else
		memcpy(buffer + handle->directory_entry.sector_offset, &handle->directory_entry.raw, sizeof(FAT_RAW_DIRECTORY_ENTRY));
#endif
		/*
		// write the modified entry to the media
		*/
		bSuccess = handle->volume->device->Write(handle->directory_entry.sector_addr, buffer);
		ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (ret != STORAGE_SUCCESS)
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
			bSuccess = handle->volume->device->Read(handle->directory_entry.sector_addr + 1, buffer);
			ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (ret != STORAGE_SUCCESS)
			{
				handle->busy = 0;
				return FAT_CANNOT_READ_MEDIA;
			}
			bSuccess = handle->volume->device->Write(handle->directory_entry.sector_addr + 1, buffer);
			ret = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (ret != STORAGE_SUCCESS)
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
#endif
}

/*
// closes an open file
*/
uint16 fat_file_close(FAT_FILE* handle)
{
	uint16 ret;
	FAT_ENTRY fat_entry;
	/*
	// check that this is a valid handle
	*/
	if (handle->magic != FAT_OPEN_HANDLE_MAGIC)
		return FAT_INVALID_HANDLE;
	/*
	// flush the file buffers
	*/
#if !defined(FAT_READ_ONLY)
	ret = fat_file_flush(handle);
	if (ret != FAT_SUCCESS)
		return ret;


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
		ret = fat_file_seek(handle, handle->current_size - 1, FAT_SEEK_START);
		if (ret != FAT_SUCCESS)
			return ret;
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
		ret = fat_get_cluster_entry(handle->volume, handle->current_clus_addr, &fat_entry);
		if (ret != FAT_SUCCESS)
			return ret;

		if (!fat_is_eof_entry(handle->volume, fat_entry))
		{
			ret = fat_free_cluster_chain(handle->volume, fat_entry);
			if (ret != FAT_SUCCESS)
				return ret;

			switch (handle->volume->fs_type)
			{
			case FAT_FS_TYPE_FAT12: fat_entry = FAT12_EOC; break;
			case FAT_FS_TYPE_FAT16: fat_entry = FAT16_EOC; break;
			case FAT_FS_TYPE_FAT32: fat_entry = FAT32_EOC; break;
			}
			ret = fat_set_cluster_entry(handle->volume, handle->current_clus_addr, fat_entry);
			if (ret != FAT_SUCCESS)
				return ret;
		}
	}
#endif
	/*
	// invalidate the file handle
	*/
	handle->magic = 0;
	/*
	// return success
	*/
	return FAT_SUCCESS;
}
