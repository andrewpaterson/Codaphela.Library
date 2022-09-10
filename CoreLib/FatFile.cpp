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



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatFile::Init(CFatVolume* pcVolume)
{
	memset(&msFile, 0, sizeof(SFatFile));
	mpcVolume = pcVolume;
	msFile.uiBuffer = msFile.auiBufferInternal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileUpdateSequentialClusterCount(CFatFile* file)
{
	uint32 current_cluster;
	uint32 next_cluster;

	// find out how many clusters are allocated sequentially
	// to this file following the current cursor location
	file->msFile.uiNoOfSequentialClusters = 0;
	current_cluster = file->msFile.uiCurrentClusterAddress;

	while (!file->mpcVolume->FatIsEOFEntry(current_cluster))
	{
		file->mpcVolume->FatGetClusterEntry(current_cluster, &next_cluster);

		if (next_cluster == (current_cluster + 1))
		{
			file->msFile.uiNoOfSequentialClusters++;
			current_cluster = next_cluster;
			if (file->msFile.uiNoOfSequentialClusters == 0xFFFF)
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
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileOpen(CFatVolume* volume, char* filename, uint8 uiAccessFlags, CFatFile* file)
{
	uint16					uiResult;
	SFatDirectoryEntry		file_entry;

	// use the internal uiBuffer
	if (!(uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		file->msFile.uiBuffer = file->msFile.auiBufferInternal;
	}

	// check that the user supplied a filename
	if (filename == 0 || strlen(filename) > FAT_MAX_PATH)
	{
		return FAT_INVALID_FILENAME;
	}

	// get the file entry
	uiResult = volume->FatGetFileEntry(filename, &file_entry);

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
		if (uiAccessFlags & FAT_FILE_ACCESS_CREATE)
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
			// the file_path uiBuffer
			memcpy(file_path, filename, path_len);

			// set the null terminator of the file_path uiBuffer
			file_path[path_len] = 0x0;

			// increase pointer to the beggining of the filename
			// part of the path
			filename_scanner++;

			// try to get the entry for the parent directory
			uiResult = volume->FatGetFileEntry(file_path, &parent_entry);

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
			uiResult = volume->FatCreateDirectoryEntry(&parent_entry.raw, filename_scanner, 0, 0, &file_entry);

			// make sure the file is opened with no append flags
			// todo: figure out why we need this and fix it
			uiAccessFlags = uiAccessFlags & (0xFF ^ FAT_FILE_ACCESS_APPEND);

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

	uiResult = FatOpenFileByEntry(volume, &file_entry, file, uiAccessFlags);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if the file has no clusters allocated then allocate one
	if (file->msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		FatFileUpdateSequentialClusterCount(file);
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatFile::SetVolume(CFatVolume* pcVolume)
{
	mpcVolume = pcVolume;
}


// opens a file given a pointer to it's
// directory entry
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatOpenFileByEntry(CFatVolume* volume, SFatDirectoryEntry* entry, CFatFile* file, uint8 uiAccessFlags)
{
	bool	bSuccess;
	uint16	uiResult;

	// set implicit access flags
	uiAccessFlags |= FAT_FILE_ACCESS_READ;
	if (uiAccessFlags & (FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_APPEND | FAT_FILE_ACCESS_OVERWRITE))
	{
		uiAccessFlags |= FAT_FILE_ACCESS_WRITE;
	}

	// if the user is trying to open a directory then
	// return an error code, otherwise it would corrupt
	// the volume
	if (entry->attributes & FAT_ATTR_DIRECTORY)
	{
		return FAT_NOT_A_FILE;
	}

	// copy the volume file and the entry's
	// structure to the file file
	file->SetVolume(volume);
	file->msFile.sDirectoryEntry = *entry;
	file->msFile.uiCurrentSize = entry->size;
	file->msFile.uiCurrentClusterIdx = 0;
	file->msFile.uiAccessFlags = uiAccessFlags;
	file->msFile.uiMagic = FAT_OPEN_HANDLE_MAGIC;
	file->msFile.bBusy = 0;

	// calculate the # of clusters allocated
	file->msFile.uiNoOfClustersAfterPos = (entry->size + volume->GetNoOfBytesPerSector() - 1) / volume->GetNoOfBytesPerSector();
	file->msFile.uiNoOfClustersAfterPos = (file->msFile.uiNoOfClustersAfterPos + volume->GetNoOfSectorsPerCluster() - 1) / volume->GetNoOfSectorsPerCluster();
	if (file->msFile.uiNoOfClustersAfterPos)
	{
		file->msFile.uiNoOfClustersAfterPos--;
	}

	// set the sector cache as dirty, this will
	// indicate that the current sector has not yet
	// been read to other file functions
	file->msFile.bBufferDirty = 1;

	// read the the cluster number
	((uint16*)&file->msFile.uiCurrentClusterAddress)[INT32_WORD0] = entry->raw.uEntry.sFatRawCommon.first_cluster_lo;
	((uint16*)&file->msFile.uiCurrentClusterAddress)[INT32_WORD1] = (volume->GetFileSystemType() == FAT_FS_TYPE_FAT32) ? entry->raw.uEntry.sFatRawCommon.first_cluster_hi : 0;

	if (uiAccessFlags & FAT_FILE_ACCESS_APPEND)
	{
		// if the file is being opened for append access we
		// seek to the end of the file
		uiResult = FatFileSeek(file, 0, FAT_SEEK_END);
		if (uiResult != FAT_SUCCESS)
		{
			file->msFile.uiMagic = 0;
			return uiResult;
		}
	}
	else if (uiAccessFlags & FAT_FILE_ACCESS_OVERWRITE)
	{
		// if the file is being opened with the OVERWRITE flag we must free all the clusters
		// currently allocated to the file and update it's directory entry to point to cluster 1

		// if the file is not already empty then
		// we'll empty it
		if (entry->raw.uEntry.sFatRawCommon.first_cluster_lo != 0x0 || entry->raw.uEntry.sFatRawCommon.first_cluster_hi != 0x0)
		{
			uint8* uiBuffer = fat_shared_buffer;

			// update the entry to point to cluster 0
			entry->raw.uEntry.sFatRawCommon.first_cluster_lo = 0x0;
			entry->raw.uEntry.sFatRawCommon.first_cluster_hi = 0x0;
			entry->raw.uEntry.sFatRawCommon.size = 0x0;
			file->msFile.sDirectoryEntry = *entry;
			file->msFile.uiCurrentSize = entry->size;

			// mark the cached sector as unknown
			fat_shared_buffer_sector = FAT_UNKNOWN_SECTOR;

			// read the sector that contains the entry
			bSuccess = volume->Read(entry->sector_addr, uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				file->msFile.uiMagic = 0;
				return FAT_CANNOT_READ_MEDIA;
			}
			// copy the modified file entry to the sector uiBuffer
			memcpy(uiBuffer + entry->sector_offset, &entry->raw, sizeof(SFatRawDirectoryEntry));

			// write the modified entry to the media
			bSuccess = volume->Write(entry->sector_addr, uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				file->msFile.uiMagic = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}

			// free the clusters allocated to the file
			volume->FatFreeClusterChain(file->msFile.uiCurrentClusterAddress);
		}

		// reset the file file to reflect an
		// empty file
		file->msFile.uiCurrentSize = 0x0;
		file->msFile.uiCurrentSectorIdx = 0x0;
		file->msFile.uiCurrentClusterAddress = 0x0;
		file->msFile.uiCurrentClusterIdx = 0;
		file->msFile.pvBufferHead = file->msFile.uiBuffer;
		file->msFile.uiNoOfClustersAfterPos = 0;
	}
	else
	{
		// if no append or overwrite flags are set then
		// simply set the file to point to the beggining
		// of the 1st cluster
		file->msFile.uiCurrentSectorIdx = 0x0;
		file->msFile.pvBufferHead = file->msFile.uiBuffer;
	}
	return FAT_SUCCESS;
}


// sets the buffer of the file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileSetBuffer(CFatFile* file, uint8* uiBuffer)
{
	if (file->GetBufferHead() != file->GetBuffer())
	{
		uint16	uiResult;
		uint32	sector_address;
		bool	bSuccess;

		sector_address = file->msFile.uiCurrentSectorIdx + file->mpcVolume->CalculateFirstSectorOfCluster(file->msFile.uiCurrentClusterAddress);
		file->msFile.uiBuffer = uiBuffer;
		file->msFile.pvBufferHead = uiBuffer + (uintptr_t)file->msFile.pvBufferHead;
		bSuccess = file->mpcVolume->Read(sector_address, file->msFile.uiBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			file->msFile.bBusy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}
		file->msFile.bBufferDirty = 0;
	}
	else
	{
		file->msFile.uiBuffer = uiBuffer;
		file->msFile.pvBufferHead = uiBuffer;
	}

	return FAT_SUCCESS;
}

// gets a unique identifier of the file (ie. first cluster)
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatFile::FatFileGetUniqueId(CFatFile* file)
{
	uint32 id;

	((uint16*)&id)[INT32_WORD0] = file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo;

	if (file->mpcVolume->GetFileSystemType() == FAT_FS_TYPE_FAT32)
	{
		((uint16*)&id)[INT32_WORD1] = file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi;
	}
	else
	{
		((uint16*)&id)[INT32_WORD1] = 0;
	}

	return id;
}


// pre-allocates disk space for a file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileAllocate(CFatFile* file, uint32 bytes)
{
	uint16	uiResult;
	uint32	new_cluster;
	uint32	no_of_clusters_needed;
	bool	bSuccess;
	uint8*	uiBuffer = fat_shared_buffer;

	// check that this is a valid file
	if (file->msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// check that another operation is not using the
	// file at this time
	if (file->msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	file->msFile.bBusy = 1;

	// calculate how many clusters we need
	no_of_clusters_needed = (bytes + file->mpcVolume->GetNoOfBytesPerSector() - 1) / file->mpcVolume->GetNoOfBytesPerSector();
	no_of_clusters_needed = (no_of_clusters_needed + file->mpcVolume->GetNoOfSectorsPerCluster() - 1) / file->mpcVolume->GetNoOfSectorsPerCluster();
	no_of_clusters_needed = (file->msFile.uiNoOfClustersAfterPos > no_of_clusters_needed) ? 0 : (no_of_clusters_needed - file->msFile.uiNoOfClustersAfterPos);

	// if we already got all the clusters requested then thre's nothing to do
	if (no_of_clusters_needed == 0)
	{
		file->msFile.bBusy = 0;
		return FAT_SUCCESS;
	}

	// allocate a new cluster
	if (file->msFile.uiAccessFlags & FAT_FILE_FLAG_OPTIMIZE_FOR_FLASH)
	{
		uint32 current_cluster;
		uint32 next_cluster;
		uint32 page_size;
		uint32 start_address;
		uint32 end_address;

		page_size = file->mpcVolume->GetPageSize();

		if (page_size > file->mpcVolume->GetNoOfSectorsPerCluster())
		{
			uint32 clusters_per_page = page_size / file->mpcVolume->GetNoOfSectorsPerCluster();

			if (no_of_clusters_needed % clusters_per_page)
			{
				no_of_clusters_needed += clusters_per_page - (no_of_clusters_needed % clusters_per_page);
			}

			if ((no_of_clusters_needed % clusters_per_page) != 0)
			{
				return FAT_UNKNOWN_ERROR;
			}

			new_cluster = file->mpcVolume->FatAllocateDataClusterEx(no_of_clusters_needed, 0, page_size, &uiResult);
			if (uiResult != FAT_SUCCESS)
			{
				file->msFile.bBusy = 0;
				return uiResult;
			}
		}
		else
		{
			new_cluster = file->mpcVolume->FatAllocateDataCluster(no_of_clusters_needed, 1, &uiResult);
			if (uiResult != FAT_SUCCESS)
			{
				file->msFile.bBusy = 0;
				return uiResult;
			}
		}

		// find out how many clusters are allocated sequentially
		// to this file following the current cursor location
		current_cluster = new_cluster;

		while (!file->mpcVolume->FatIsEOFEntry(current_cluster))
		{
			// calculate the start and end address the cluster
			start_address = file->mpcVolume->CalculateFirstSectorOfCluster(current_cluster);
			end_address = start_address + file->mpcVolume->GetNoOfSectorsPerCluster();

			// find the last sequential sector after this address
			while (!file->mpcVolume->FatIsEOFEntry(current_cluster))
			{
				file->mpcVolume->FatGetClusterEntry(current_cluster, &next_cluster);

				if (next_cluster == (current_cluster + 1))
				{
					end_address += file->mpcVolume->GetNoOfSectorsPerCluster();
					current_cluster = next_cluster;
				}
				else
				{
					current_cluster = next_cluster;
					break;
				}
			}

			// pre-erase the clusters
			file->mpcVolume->Erase(start_address, end_address - 1);
		}
	}
	else
	{
		new_cluster = file->mpcVolume->FatAllocateDataCluster(no_of_clusters_needed, 0, &uiResult);
		if (uiResult != FAT_SUCCESS)
		{
			file->msFile.bBusy = 0;
			return uiResult;
		}
	}

	// if this is the 1st cluster cluster allocated
	// to the file then we must modify the file's entry
	if (!file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo && !file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi)
	{
		// modify the file entry to point to the
		// new cluster
		file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo = LO16(new_cluster);
		file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi = HI16(new_cluster);

		// mark the cached sector as unknown
		fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);

		// try load the sector that contains the entry
		bSuccess = file->mpcVolume->Read(file->msFile.sDirectoryEntry.sector_addr, uiBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			file->msFile.bBusy = 0;
			return uiResult;
		}

		// copy the modified file entry to the
		// sector uiBuffer
		memcpy(uiBuffer + file->msFile.sDirectoryEntry.sector_offset, &file->msFile.sDirectoryEntry.raw, sizeof(SFatRawDirectoryEntry));

		// write the modified entry to the media
		bSuccess = file->mpcVolume->Write(file->msFile.sDirectoryEntry.sector_addr, uiBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			file->msFile.bBusy = 0;
			return uiResult;
		}
	}
	// if there are clusters allocated to the file update the last FAT entry
	// of the file to point to the newly allocated cluster(s)
	else
	{
		uint32 last_cluster;

		if (file->msFile.uiNoOfClustersAfterPos)
		{
			if (!file->mpcVolume->FatIncreaseClusterAddress(file->msFile.uiCurrentClusterAddress, file->msFile.uiNoOfClustersAfterPos, &last_cluster))
			{
				file->msFile.bBusy = 0;
				return FAT_CORRUPTED_FILE;
			}
		}
		else
		{
			last_cluster = file->msFile.uiCurrentClusterAddress;
		}

		// set the FAT entry for the last cluster to the beggining of the newly
		// allocated cluster chain (ie. link them)
		uiResult = file->mpcVolume->FatSetClusterEntry(last_cluster, (FatEntry)new_cluster);
		if (uiResult != FAT_SUCCESS)
		{
			file->msFile.bBusy = 0;
			return uiResult;
		}
	}

	// update the file to point to the
	// new cluster
	if (!file->msFile.uiCurrentClusterAddress)
	{
		file->msFile.uiCurrentClusterAddress = new_cluster;
		file->msFile.uiNoOfClustersAfterPos = no_of_clusters_needed - 1;
	}
	else
	{
		file->msFile.uiNoOfClustersAfterPos += no_of_clusters_needed;
	}

	// update the count of sequential clusters
	FatFileUpdateSequentialClusterCount(file);

	file->msFile.bBusy = 0;
	return FAT_SUCCESS;
}


// moves the file cursor to the specified offset
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileSeek(CFatFile* file, uint32 offset, char mode)
{
	uint32	new_pos;
	uint32	sector_count;
	uint32	cluster_count;
	uint32	old_cluster;
	uint32	sector_address;
	uint16	uiResult;
	bool	bSuccess;

	// check that this is a valid file
	if (file->msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// check that another operation is not using the
	// file at this time
	if (file->msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	file->msFile.bBusy = 1;

	switch (mode)
	{
		case FAT_SEEK_START:
			new_pos = offset;
			break;

		case FAT_SEEK_CURRENT:
			new_pos = file->msFile.uiCurrentClusterIdx * file->mpcVolume->GetNoOfSectorsPerCluster() * file->mpcVolume->GetNoOfBytesPerSector();
			new_pos += file->msFile.uiCurrentSectorIdx * file->mpcVolume->GetNoOfBytesPerSector();
			new_pos += (uint32)(file->msFile.pvBufferHead - file->msFile.uiBuffer);
			new_pos += offset;
			break;

		case FAT_SEEK_END:
			new_pos = file->msFile.uiCurrentSize;
			if (offset)
			{
				file->msFile.bBusy = 0;
				return FAT_SEEK_FAILED;
			}
			break;

		default:
			file->msFile.bBusy = 0;
			return FAT_SEEK_FAILED;
	}

	// if the seek goes out of bounds return error
	if (new_pos > file->msFile.uiCurrentSize)
	{
		file->msFile.bBusy = 0;
		return FAT_SEEK_FAILED;
	}

	// if the file is open in unbuffered mode make sure that
	// we're seeking to a sector boundry
	if (file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (new_pos % file->mpcVolume->GetNoOfBytesPerSector())
		{
			file->msFile.bBusy = 0;
			return FAT_MISALIGNED_IO;
		}
	}

	old_cluster = file->msFile.uiCurrentClusterIdx;
	file->msFile.uiCurrentClusterIdx = 0;
	cluster_count = 1;

	// calculate the count of sectors being used by the file up to the desired position
	sector_count = (new_pos + file->mpcVolume->GetNoOfBytesPerSector() - 1) / file->mpcVolume->GetNoOfBytesPerSector();

	// set the 1st cluster as the current cluster, we'll seek from there
	((uint16*)&file->msFile.uiCurrentClusterAddress)[INT32_WORD0] = file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo;
	((uint16*)&file->msFile.uiCurrentClusterAddress)[INT32_WORD1] = (file->mpcVolume->GetFileSystemType() == FAT_FS_TYPE_FAT32) ? file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi : 0;

	// if the file occupies more than one cluster
	if (sector_count > file->mpcVolume->GetNoOfSectorsPerCluster())
	{
		// calculate the count of clusters occupied by the file and
		// update the ClustersAllocated value of the file
		cluster_count = (sector_count + file->mpcVolume->GetNoOfSectorsPerCluster() - 1) / file->mpcVolume->GetNoOfSectorsPerCluster();

		// set the file file to point to the last cluster. if the file doesn't have
		// that many clusters allocated this function will return 0. if that ever happens it means
		// that the file is corrupted
		if (!file->mpcVolume->FatIncreaseClusterAddress(file->msFile.uiCurrentClusterAddress, (cluster_count - 1), &file->msFile.uiCurrentClusterAddress))
		{
			file->msFile.bBusy = 0;
			return FAT_CORRUPTED_FILE;
		}
	}

	// calculate the last sector of the file and set the uiBuffer
	// head to point to the byte after the last one
	if (new_pos)
	{
		file->msFile.uiCurrentSectorIdx = (((new_pos + file->mpcVolume->GetNoOfBytesPerSector() - 1) / file->mpcVolume->GetNoOfBytesPerSector()) - 1) % file->mpcVolume->GetNoOfSectorsPerCluster();
		file->msFile.pvBufferHead = (uint8*)((uintptr_t)file->msFile.uiBuffer) + (new_pos % file->mpcVolume->GetNoOfBytesPerSector());

		if (new_pos % file->mpcVolume->GetNoOfBytesPerSector() == 0)
		{
			file->msFile.pvBufferHead = (uint8*)((uintptr_t)file->msFile.uiBuffer) + file->mpcVolume->GetNoOfBytesPerSector();
		}
	}
	else
	{
		file->msFile.uiCurrentSectorIdx = 0;
		file->msFile.pvBufferHead = file->msFile.uiBuffer;
	}

	sector_address = file->msFile.uiCurrentSectorIdx + file->mpcVolume->CalculateFirstSectorOfCluster(file->msFile.uiCurrentClusterAddress);
	file->msFile.uiCurrentClusterIdx = cluster_count - 1;

	// load the last sector
	if (file->msFile.uiBuffer)
	{
		bSuccess = file->mpcVolume->Read(sector_address, file->msFile.uiBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			file->msFile.bBusy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}
		file->msFile.bBufferDirty = 0;
	}

	// maintain the count of allocated clusters
	if (old_cluster < file->msFile.uiCurrentClusterIdx)
	{
		file->msFile.uiNoOfClustersAfterPos -= (file->msFile.uiCurrentClusterIdx - old_cluster);
	}
	else if (old_cluster > file->msFile.uiCurrentClusterIdx)
	{
		file->msFile.uiNoOfClustersAfterPos += (old_cluster - file->msFile.uiCurrentClusterIdx);
	}
	file->msFile.bBusy = 0;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatFile::FatFileWriteCallback(CFatFile* file, uint16* async_state_in)
{
	uint16	uiResult;
	uint16* state;
	bool	bSuccess;

	state = async_state_in;

	// loop while there are bytes to be read
	while (file->msFile.sOperationState.bytes_remaining)
	{
		// if we've reached the end of the current
		// sector then we must flush it
		if (file->msFile.pvBufferHead == file->msFile.sOperationState.end_of_buffer)
		{
			// update the sector head
			if (file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
			{
				file->msFile.uiBuffer = file->msFile.pvBufferHead;
				file->msFile.sOperationState.end_of_buffer = file->msFile.uiBuffer + file->mpcVolume->GetNoOfBytesPerSector();
			}
			else
			{
				file->msFile.sOperationState.end_of_buffer = file->msFile.uiBuffer + file->mpcVolume->GetNoOfBytesPerSector();
				file->msFile.pvBufferHead = file->msFile.uiBuffer;
			}
			// write the cached sector to media
			bSuccess = file->mpcVolume->Write(file->msFile.sOperationState.sector_addr, file->msFile.uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;

			// if we were unable to read the sector
			// then throw an error
			if (uiResult != STORAGE_SUCCESS)
			{
				*state = FAT_CANNOT_WRITE_MEDIA;
				file->msFile.bBusy = 0;
				return;
			}
			// if this sector is the last of the current cluster then
			// locate the next cluster and continue writing
			if (file->msFile.uiCurrentSectorIdx == file->mpcVolume->GetNoOfSectorsPerCluster() - 1)
			{
				uiResult = file->mpcVolume->FatGetClusterEntry(file->msFile.uiCurrentClusterAddress, &file->msFile.uiCurrentClusterAddress);
				if (uiResult != STORAGE_SUCCESS)
				{
					*state = FAT_CANNOT_WRITE_MEDIA;
					file->msFile.bBusy = 0;
					return;
				}
				// reset the current sector and increase the cluster index
				file->msFile.uiCurrentSectorIdx = 0x0;
				file->msFile.uiCurrentClusterIdx++;
				file->msFile.uiNoOfClustersAfterPos--;
				// calculate the sector address
				file->msFile.sOperationState.sector_addr = file->mpcVolume->CalculateFirstSectorOfCluster(file->msFile.uiCurrentClusterAddress);
			}
			// if there are more sectors in the
			// current cluster then simply increase
			// the current sector counter and address
			else
			{
				file->msFile.uiCurrentSectorIdx++;
				file->msFile.sOperationState.sector_addr++;
			}
		}
		if (file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
		{
			file->msFile.pvBufferHead = file->msFile.sOperationState.end_of_buffer;
			file->msFile.sOperationState.pos += file->mpcVolume->GetNoOfBytesPerSector();

			if (file->msFile.sOperationState.pos >= file->msFile.uiCurrentSize)
			{
				file->msFile.uiCurrentSize = file->msFile.sOperationState.pos;
			}
			file->msFile.sOperationState.bytes_remaining -= file->mpcVolume->GetNoOfBytesPerSector();
		}
		else
		{
			// write the next byte to the file cache
			*file->msFile.pvBufferHead++ = *file->msFile.sOperationState.uiBuffer++;

			// update the file size only if we're writting past
			// the end of the file
			if (file->msFile.sOperationState.pos++ >= file->msFile.uiCurrentSize)
			{
				file->msFile.uiCurrentSize++;
			}
			file->msFile.sOperationState.bytes_remaining--;
		}
	}

	*state = FAT_SUCCESS;
	file->msFile.bBusy = 0;
	return;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileWrite(CFatFile* file, uint8* buff, uint32 length)
{
	uint32 uiResult;

	// check that this is a valid file
	if (file->msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}
	// check that the file is open for write access
	if (!(file->msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE))
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
	uiResult = FatFileAllocate(file, length);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// make sure that either a uiBuffer is set or the file has been opened in
	// unbuffered mode.
	if (!file->msFile.uiBuffer && !(file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		return FAT_FILE_BUFFER_NOT_SET;
	}

	// check that another operation is not using the
	// file at this time
	if (file->msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	file->msFile.bBusy = 1;

	// calculate current pos
	file->msFile.sOperationState.pos = file->msFile.uiCurrentClusterIdx * file->mpcVolume->GetNoOfSectorsPerCluster() * file->mpcVolume->GetNoOfBytesPerSector();
	file->msFile.sOperationState.pos += (file->msFile.uiCurrentSectorIdx) * file->mpcVolume->GetNoOfBytesPerSector();
	file->msFile.sOperationState.pos += (uintptr_t)(file->msFile.pvBufferHead - file->msFile.uiBuffer);

	// if the file is opened in unbuffered mode make sure that
	// we're reading a multiple of the sector size
	if (file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (length % file->mpcVolume->GetNoOfBytesPerSector())
		{
			file->msFile.bBusy = 0;
			return FAT_MISALIGNED_IO;
		}
		file->msFile.uiBuffer = buff;
		file->msFile.sOperationState.end_of_buffer = file->msFile.pvBufferHead = buff;
	}
	else
	{
		// calculate the address of the end of
		// the current sector
		file->msFile.sOperationState.end_of_buffer = file->msFile.uiBuffer + file->mpcVolume->GetNoOfBytesPerSector();
	}

	// copy the length of the uiBuffer to be writen
	// into the counter
	file->msFile.sOperationState.bytes_remaining = (uint16)length;

	// calculate the address of the current sector
	file->msFile.sOperationState.sector_addr = file->msFile.uiCurrentSectorIdx + file->mpcVolume->CalculateFirstSectorOfCluster(file->msFile.uiCurrentClusterAddress);


	file->msFile.sOperationState.internal_state = 0x0;
	file->msFile.sOperationState.length = (uint16)length;
	file->msFile.sOperationState.uiBuffer = buff;

	uint16 internal_state;
	FatFileWriteCallback(file, &internal_state);
	return internal_state;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatFile::FatFileReadCallback(CFatFile* file, uint16* state)
{
	uint16	uiResult;
	bool	bSuccess;

	// reset the # of bytes read
	if (file->msFile.sOperationState.bytes_read)
	{
		*file->msFile.sOperationState.bytes_read = 0;
	}

	// if the sector cache is invalid
	if (file->msFile.bBufferDirty)
	{
		// read the current sector synchronously
		bSuccess = file->mpcVolume->Read(file->msFile.sOperationState.sector_addr, file->msFile.uiBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			*state = FAT_CANNOT_READ_MEDIA;
			file->msFile.bBusy = 0;
			return;
		}
		// mark the cache as clean
		file->msFile.bBufferDirty = 0;

		// if the file is unbuffered decrease the count of bytes remaining
		// by one sector and set the uiBuffer head to the end of the uiBuffer
		if (file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
		{
			file->msFile.sOperationState.bytes_remaining -= file->mpcVolume->GetNoOfBytesPerSector();
			file->msFile.pvBufferHead += file->mpcVolume->GetNoOfBytesPerSector();
			file->msFile.sOperationState.end_of_buffer = file->msFile.pvBufferHead;
			if (file->msFile.sOperationState.bytes_read)
			{
				*file->msFile.sOperationState.bytes_read += file->mpcVolume->GetNoOfBytesPerSector();
			}

			file->msFile.sOperationState.pos += file->mpcVolume->GetNoOfBytesPerSector();

			if (file->msFile.sOperationState.pos >= file->msFile.uiCurrentSize)
			{
				if (file->msFile.sOperationState.bytes_read)
				{
					*file->msFile.sOperationState.bytes_read -= file->msFile.sOperationState.pos - file->msFile.uiCurrentSize;
				}
				file->msFile.sOperationState.bytes_remaining = 0;
			}
		}
	}

	// make sure that we haven't reaced the end of the file
	if (file->msFile.sOperationState.pos >= file->msFile.uiCurrentSize)
	{
		file->msFile.sOperationState.bytes_remaining = 0;
	}

	// loop while there are bytes to be
	// read
	while (file->msFile.sOperationState.bytes_remaining)
	{
		// if we've reached the end of the current
		// sector then we must load the next...
		if (file->msFile.pvBufferHead == file->msFile.sOperationState.end_of_buffer)
		{
			// if the file is unbuffered change the uiBuffer to the
			// new position of the uiBuffer head. otherwise reset the position
			// of the position of the uiBuffer head
			if (file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
			{
				file->msFile.uiBuffer = file->msFile.pvBufferHead;
				file->msFile.sOperationState.end_of_buffer = file->msFile.pvBufferHead + file->mpcVolume->GetNoOfBytesPerSector();
			}
			else
			{
				file->msFile.pvBufferHead = file->msFile.uiBuffer;
			}

			// if this sector is the last of the current cluster
			// we must find the next cluster
			if (file->msFile.uiCurrentSectorIdx == file->mpcVolume->GetNoOfSectorsPerCluster() - 1)
			{
				// update the cluster address with the address of the
				// next cluster
				if (!file->mpcVolume->FatIncreaseClusterAddress(file->msFile.uiCurrentClusterAddress, 1, &file->msFile.uiCurrentClusterAddress))
				{
					*state = FAT_CORRUPTED_FILE;
					file->msFile.bBusy = 0;
					return;
				}

				// reset the current sector increase the current cluster
				// number and calculate the address of the first sector of the
				// cluster
				file->msFile.uiCurrentClusterIdx++;
				file->msFile.uiCurrentSectorIdx = 0x0;
				file->msFile.sOperationState.sector_addr = file->mpcVolume->CalculateFirstSectorOfCluster(file->msFile.uiCurrentClusterAddress);
			}
			else
			{

				// if there are more sectors in the current cluster then
				// simply increase the current sector counter and address
				file->msFile.uiCurrentSectorIdx++;
				file->msFile.sOperationState.sector_addr++;
			}

			// read the next sector into the cache
			bSuccess = file->mpcVolume->Read(file->msFile.sOperationState.sector_addr, file->msFile.uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				*state = FAT_CANNOT_READ_MEDIA;
				file->msFile.bBusy = 0;
				return;
			}
		}

		// update the count of bytes read/remaining and if the file
		// is buffered copy data to file uiBuffer
		if (file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
		{
			file->msFile.pvBufferHead = file->msFile.sOperationState.end_of_buffer;
			file->msFile.sOperationState.bytes_remaining -= file->mpcVolume->GetNoOfBytesPerSector();
			if (file->msFile.sOperationState.bytes_read)
			{
				(*file->msFile.sOperationState.bytes_read) += file->mpcVolume->GetNoOfBytesPerSector();
			}

			file->msFile.sOperationState.pos += file->mpcVolume->GetNoOfBytesPerSector();

			if (file->msFile.sOperationState.pos >= file->msFile.uiCurrentSize)
			{
				if (file->msFile.sOperationState.bytes_read)
				{
					*file->msFile.sOperationState.bytes_read -= (file->msFile.sOperationState.pos - file->msFile.uiCurrentSize);
				}
				file->msFile.sOperationState.bytes_remaining = 0;
			}
		}
		else
		{
			// copy the next byte to the uiBuffer
			*file->msFile.sOperationState.uiBuffer++ = *file->msFile.pvBufferHead++;

			// update the  count of bytes read
			if (file->msFile.sOperationState.bytes_read)
			{
				(*file->msFile.sOperationState.bytes_read)++;
			}

			// decrease the count of remaining bytes
			file->msFile.sOperationState.bytes_remaining--;

			// increase the file pointer
			file->msFile.sOperationState.pos++;

			// check if we've reached the end of the file
			if (file->msFile.sOperationState.pos >= file->msFile.uiCurrentSize)
			{
				file->msFile.sOperationState.bytes_remaining = 0;
			}
		}
	}

	*state = FAT_SUCCESS;
	file->msFile.bBusy = 0;
	return;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileRead(CFatFile* file, uint8* buff, uint32 length, uint32* bytes_read)
{
	// check that this is a valid file
	if (file->msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// make sure that either a uiBuffer is set or the file has been
	// opened in unbuffered mode
	if (!file->msFile.uiBuffer && !(file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		return FAT_FILE_BUFFER_NOT_SET;
	}

	// make sure length is not larger than 16-bit
	if (length > 0xFFFF)
	{
		return FAT_BUFFER_TOO_BIG;
	}

	// check that another operation is not using the
	// file at this time
	if (file->msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	file->msFile.bBusy = 1;

	// calculate current pos
	file->msFile.sOperationState.pos = file->msFile.uiCurrentClusterIdx * file->mpcVolume->GetNoOfSectorsPerCluster() * file->mpcVolume->GetNoOfBytesPerSector();
	file->msFile.sOperationState.pos += (file->msFile.uiCurrentSectorIdx) * file->mpcVolume->GetNoOfBytesPerSector();
	file->msFile.sOperationState.pos += (uintptr_t)(file->msFile.pvBufferHead - file->msFile.uiBuffer);

	// calculate the address of the current
	// sector and the address of the end of the uiBuffer
	file->msFile.sOperationState.sector_addr = file->msFile.uiCurrentSectorIdx + file->mpcVolume->CalculateFirstSectorOfCluster(file->msFile.uiCurrentClusterAddress);
	file->msFile.sOperationState.end_of_buffer = file->msFile.uiBuffer + file->mpcVolume->GetNoOfBytesPerSector();

	// if the file is opened in unbuffered mode make sure that
	// we're reading a multiple of the sector size and set the uiBuffer
	// to the one supplied by the user in this call
	if (file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (length % file->mpcVolume->GetNoOfBytesPerSector())
		{
			return FAT_MISALIGNED_IO;
		}
		file->msFile.uiBuffer = buff;
		file->msFile.sOperationState.end_of_buffer = file->msFile.pvBufferHead = buff;
	}

	// set the async op context
	file->msFile.sOperationState.bytes_remaining = (uint16)length;
	file->msFile.sOperationState.internal_state = 0x0;
	file->msFile.sOperationState.length = (uint16)length;
	file->msFile.sOperationState.uiBuffer = buff;
	file->msFile.sOperationState.bytes_read = bytes_read;

	uint16 internal_state;
	FatFileReadCallback(file, &internal_state);
	return internal_state;
}


// flushes file buffers
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileFlush(CFatFile* file)
{
	uint16	uiResult;
	uint32	sector_address = 0;
	bool	bSuccess;
	uint8*	uiBuffer = fat_shared_buffer;

	// check that this is a valid file
	if (file->msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// make sure that either a uiBuffer has been set or the file was
	// opened in unbuffered mode
	if (!file->msFile.uiBuffer && !(file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		return FAT_FILE_BUFFER_NOT_SET;
	}

	// check that we got write access
	if (file->msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		// check that another operation is not using the
		// file at this time
		if (file->msFile.bBusy)
		{
			return FAT_FILE_HANDLE_IN_USE;
		}

		// mark the file as in use
		file->msFile.bBusy = 1;

		// if the file is buffered then flush the uiBuffer
		if (!(file->msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING))
		{
			// calculate the address of the current
			// sector
			sector_address = file->msFile.uiCurrentSectorIdx + file->mpcVolume->CalculateFirstSectorOfCluster(file->msFile.uiCurrentClusterAddress);

			// if the uiBuffer is only partially filled we need to merge it
			// with the one on the drive
			if (file->msFile.pvBufferHead <= file->msFile.uiBuffer + file->mpcVolume->GetNoOfBytesPerSector())
			{
				uint16 i;
				uint8 buff[MAX_SECTOR_LENGTH];
				bSuccess = file->mpcVolume->Read(sector_address, buff);
				uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
				if (uiResult != STORAGE_SUCCESS)
				{
					file->msFile.bBusy = 0;
					return uiResult;
				}

				for (i = (uint16)(file->msFile.pvBufferHead -
					file->msFile.uiBuffer); i < file->mpcVolume->GetNoOfBytesPerSector(); i++)
				{
					file->msFile.uiBuffer[i] = buff[i];
				}
			}

			// write the cached sector to media
			bSuccess = file->mpcVolume->Write(sector_address, file->msFile.uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				file->msFile.bBusy = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}
		}

		// update the file size on the entry
		file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.size = file->msFile.uiCurrentSize;
		file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.modify_date = rtc_get_fat_date();
		file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.modify_time = rtc_get_fat_time();
		file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.access_date = file->msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.modify_date;

		// try load the sector that contains the entry
		fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);

		bSuccess = file->mpcVolume->Read(file->msFile.sDirectoryEntry.sector_addr, uiBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			file->msFile.bBusy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}

		// copy the modified file entry to the
		// sector uiBuffer
		memcpy(uiBuffer + file->msFile.sDirectoryEntry.sector_offset, &file->msFile.sDirectoryEntry.raw, sizeof(SFatRawDirectoryEntry));

		// write the modified entry to the media
		bSuccess = file->mpcVolume->Write(file->msFile.sDirectoryEntry.sector_addr, uiBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			file->msFile.bBusy = 0;
			return FAT_CANNOT_WRITE_MEDIA;
		}

		// some cards seem not to update the sector correctly if it
		// is the first sector on the page and is the only sector written
		// to so for now until we figure this out we'll write the next
		// sector too TODO: implement this on driver!!
		{
			bSuccess = file->mpcVolume->Read(file->msFile.sDirectoryEntry.sector_addr + 1, uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				file->msFile.bBusy = 0;
				return FAT_CANNOT_READ_MEDIA;
			}
			bSuccess = file->mpcVolume->Write(file->msFile.sDirectoryEntry.sector_addr + 1, uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				file->msFile.bBusy = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}
		}

		// mark the file file as not bBusy
		file->msFile.bBusy = 0;
	}
	return FAT_SUCCESS;
}


// closes an open file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileClose(CFatFile* file)
{
	uint16		uiResult;
	FatEntry	fat_entry;

	// check that this is a valid file
	if (file->msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// flush the file buffers
	uiResult = FatFileFlush(file);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	if (file->msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		// clear the no buffering attribute so we can seek to a misaligned
		// position (since we want the very end of the last written, not the
		// beginning of the next one.)
		file->msFile.uiAccessFlags &= (FAT_FILE_FLAG_NO_BUFFERING ^ 0xFF);

		// seek to the end of the file
		uiResult = FatFileSeek(file, file->msFile.uiCurrentSize - 1, FAT_SEEK_START);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// check that another operation is not using the
		// file at this time
		if (file->msFile.bBusy)
		{
			return FAT_FILE_HANDLE_IN_USE;
		}

		// mark the file as in use
		file->msFile.bBusy = 1;

		// free unused clusters
		uiResult = file->mpcVolume->FatGetClusterEntry(file->msFile.uiCurrentClusterAddress, &fat_entry);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		if (!file->mpcVolume->FatIsEOFEntry(fat_entry))
		{
			uiResult = file->mpcVolume->FatFreeClusterChain(fat_entry);
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}

			switch (file->mpcVolume->GetFileSystemType())
			{
				case FAT_FS_TYPE_FAT12: 
					fat_entry = FAT12_EOC; 
					break;

				case FAT_FS_TYPE_FAT16: 
					fat_entry = FAT16_EOC; 
					break;

				case FAT_FS_TYPE_FAT32: 
					fat_entry = FAT32_EOC; 
					break;
			}
			uiResult = file->mpcVolume->FatSetClusterEntry(file->msFile.uiCurrentClusterAddress, fat_entry);
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}
		}
	}

	// invalidate the file file
	file->msFile.uiMagic = 0;
	return FAT_SUCCESS;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatFile::GetCurrentSize(void) { return msFile.uiCurrentSize;  }
uint32 CFatFile::GetCurrentClusterAddress(void) { return msFile.uiCurrentClusterAddress; }
uint32 CFatFile::GetCurrentClusterIdx(void) { return msFile.uiCurrentClusterIdx; }
uint32 CFatFile::GetCurrentSectorIdx(void) { return msFile.uiCurrentSectorIdx; }
uint32 CFatFile::GetNoOfClustersAfterPos(void) { return msFile.uiNoOfClustersAfterPos; }
uint16 CFatFile::GetNoOfSequentialClusters(void) { return msFile.uiNoOfSequentialClusters; }
uint8* CFatFile::GetBufferHead(void) { return msFile.pvBufferHead; }
bool CFatFile::IsBufferDirty(void) { return msFile.bBufferDirty; }
bool CFatFile::IsBusy(void) { return msFile.bBusy; }
uint8 CFatFile::GetMagic(void) { return msFile.uiMagic; }
uint8 CFatFile::GetAccessFlags(void) { return msFile.uiAccessFlags; }
uint8* CFatFile::GetBuffer(void) { return msFile.uiBuffer; }

