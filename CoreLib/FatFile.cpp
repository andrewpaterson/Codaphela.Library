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
uint16 CFatFile::FatFileUpdateSequentialClusterCount(void)
{
	uint32 current_cluster;
	uint32 next_cluster;

	// find out how many clusters are allocated sequentially
	// to this file following the current cursor location
	msFile.uiNoOfSequentialClusters = 0;
	current_cluster = msFile.uiCurrentClusterAddress;

	while (!mpcVolume->FatIsEOFEntry(current_cluster))
	{
		mpcVolume->FatGetClusterEntry(current_cluster, &next_cluster);

		if (next_cluster == (current_cluster + 1))
		{
			msFile.uiNoOfSequentialClusters++;
			current_cluster = next_cluster;
			if (msFile.uiNoOfSequentialClusters == 0xFFFF)
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
uint16 CFatFile::FatFileOpen(CFatVolume* volume, char* filename, uint8 uiAccessFlags)
{
	uint16					uiResult;
	SFatDirectoryEntry		file_entry;

	// use the internal uiBuffer
	if (!(uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		msFile.uiBuffer = msFile.auiBufferInternal;
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

	uiResult = FatOpenFileByEntry(volume, &file_entry, uiAccessFlags);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if the file has no clusters allocated then allocate one
	if (msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		FatFileUpdateSequentialClusterCount();
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
uint16 CFatFile::FatOpenFileByEntry(CFatVolume* volume, SFatDirectoryEntry* entry, uint8 uiAccessFlags)
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
	SetVolume(volume);
	msFile.sDirectoryEntry = *entry;
	msFile.uiCurrentSize = entry->size;
	msFile.uiCurrentClusterIdx = 0;
	msFile.uiAccessFlags = uiAccessFlags;
	msFile.uiMagic = FAT_OPEN_HANDLE_MAGIC;
	msFile.bBusy = 0;

	// calculate the # of clusters allocated
	msFile.uiNoOfClustersAfterPos = (entry->size + volume->GetNoOfBytesPerSector() - 1) / volume->GetNoOfBytesPerSector();
	msFile.uiNoOfClustersAfterPos = (msFile.uiNoOfClustersAfterPos + volume->GetNoOfSectorsPerCluster() - 1) / volume->GetNoOfSectorsPerCluster();
	if (msFile.uiNoOfClustersAfterPos)
	{
		msFile.uiNoOfClustersAfterPos--;
	}

	// set the sector cache as dirty, this will
	// indicate that the current sector has not yet
	// been read to other file functions
	msFile.bBufferDirty = 1;

	// read the the cluster number
	((uint16*)&msFile.uiCurrentClusterAddress)[INT32_WORD0] = entry->raw.uEntry.sFatRawCommon.first_cluster_lo;
	((uint16*)&msFile.uiCurrentClusterAddress)[INT32_WORD1] = (volume->GetFileSystemType() == FAT_FS_TYPE_FAT32) ? entry->raw.uEntry.sFatRawCommon.first_cluster_hi : 0;

	if (uiAccessFlags & FAT_FILE_ACCESS_APPEND)
	{
		// if the file is being opened for append access we
		// seek to the end of the file
		uiResult = FatFileSeek(0, FAT_SEEK_END);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.uiMagic = 0;
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
			uint8* uiBuffer = mpcVolume->GetFatSharedBuffer();

			// update the entry to point to cluster 0
			entry->raw.uEntry.sFatRawCommon.first_cluster_lo = 0x0;
			entry->raw.uEntry.sFatRawCommon.first_cluster_hi = 0x0;
			entry->raw.uEntry.sFatRawCommon.size = 0x0;
			msFile.sDirectoryEntry = *entry;
			msFile.uiCurrentSize = entry->size;

			// mark the cached sector as unknown
			mpcVolume->SetFatSharedBufferSector(FAT_UNKNOWN_SECTOR);

			// read the sector that contains the entry
			bSuccess = volume->Read(entry->sector_addr, uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				msFile.uiMagic = 0;
				return FAT_CANNOT_READ_MEDIA;
			}
			// copy the modified file entry to the sector uiBuffer
			memcpy(uiBuffer + entry->sector_offset, &entry->raw, sizeof(SFatRawDirectoryEntry));

			// write the modified entry to the media
			bSuccess = volume->Write(entry->sector_addr, uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				msFile.uiMagic = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}

			// free the clusters allocated to the file
			volume->FatFreeClusterChain(msFile.uiCurrentClusterAddress);
		}

		// reset the file file to reflect an
		// empty file
		msFile.uiCurrentSize = 0x0;
		msFile.uiCurrentSectorIdx = 0x0;
		msFile.uiCurrentClusterAddress = 0x0;
		msFile.uiCurrentClusterIdx = 0;
		msFile.pvBufferHead = msFile.uiBuffer;
		msFile.uiNoOfClustersAfterPos = 0;
	}
	else
	{
		// if no append or overwrite flags are set then
		// simply set the file to point to the beggining
		// of the 1st cluster
		msFile.uiCurrentSectorIdx = 0x0;
		msFile.pvBufferHead = msFile.uiBuffer;
	}
	return FAT_SUCCESS;
}


// sets the buffer of the file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileSetBuffer(uint8* uiBuffer)
{
	if (GetBufferHead() != GetBuffer())
	{
		uint16	uiResult;
		uint32	sector_address;
		bool	bSuccess;

		sector_address = msFile.uiCurrentSectorIdx + mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCurrentClusterAddress);
		msFile.uiBuffer = uiBuffer;
		msFile.pvBufferHead = uiBuffer + (uintptr_t)msFile.pvBufferHead;
		bSuccess = mpcVolume->Read(sector_address, msFile.uiBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}
		msFile.bBufferDirty = 0;
	}
	else
	{
		msFile.uiBuffer = uiBuffer;
		msFile.pvBufferHead = uiBuffer;
	}

	return FAT_SUCCESS;
}

// gets a unique identifier of the file (ie. first cluster)
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatFile::FatFileGetUniqueId(void)
{
	uint32 id;

	((uint16*)&id)[INT32_WORD0] = msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo;

	if (mpcVolume->GetFileSystemType() == FAT_FS_TYPE_FAT32)
	{
		((uint16*)&id)[INT32_WORD1] = msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi;
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
uint16 CFatFile::FatFileAllocate(uint32 bytes)
{
	uint16	uiResult;
	uint32	new_cluster;
	uint32	no_of_clusters_needed;
	bool	bSuccess;
	uint8*	pvBuffer = mpcVolume->GetFatSharedBuffer();

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// check that another operation is not using the
	// file at this time
	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	msFile.bBusy = 1;

	// calculate how many clusters we need
	no_of_clusters_needed = (bytes + mpcVolume->GetNoOfBytesPerSector() - 1) / mpcVolume->GetNoOfBytesPerSector();
	no_of_clusters_needed = (no_of_clusters_needed + mpcVolume->GetNoOfSectorsPerCluster() - 1) / mpcVolume->GetNoOfSectorsPerCluster();
	no_of_clusters_needed = (msFile.uiNoOfClustersAfterPos > no_of_clusters_needed) ? 0 : (no_of_clusters_needed - msFile.uiNoOfClustersAfterPos);

	// if we already got all the clusters requested then thre's nothing to do
	if (no_of_clusters_needed == 0)
	{
		msFile.bBusy = 0;
		return FAT_SUCCESS;
	}

	// allocate a new cluster
	if (msFile.uiAccessFlags & FAT_FILE_FLAG_OPTIMIZE_FOR_FLASH)
	{
		uint32 current_cluster;
		uint32 next_cluster;
		uint32 page_size;
		uint32 start_address;
		uint32 end_address;

		page_size = mpcVolume->GetPageSize();

		if (page_size > mpcVolume->GetNoOfSectorsPerCluster())
		{
			uint32 clusters_per_page = page_size / mpcVolume->GetNoOfSectorsPerCluster();

			if (no_of_clusters_needed % clusters_per_page)
			{
				no_of_clusters_needed += clusters_per_page - (no_of_clusters_needed % clusters_per_page);
			}

			if ((no_of_clusters_needed % clusters_per_page) != 0)
			{
				return FAT_UNKNOWN_ERROR;
			}

			new_cluster = mpcVolume->FatAllocateDataClusterEx(no_of_clusters_needed, 0, page_size, &uiResult);
			if (uiResult != FAT_SUCCESS)
			{
				msFile.bBusy = 0;
				return uiResult;
			}
		}
		else
		{
			new_cluster = mpcVolume->FatAllocateDataCluster(no_of_clusters_needed, 1, &uiResult);
			if (uiResult != FAT_SUCCESS)
			{
				msFile.bBusy = 0;
				return uiResult;
			}
		}

		// find out how many clusters are allocated sequentially
		// to this file following the current cursor location
		current_cluster = new_cluster;

		while (!mpcVolume->FatIsEOFEntry(current_cluster))
		{
			// calculate the start and end address the cluster
			start_address = mpcVolume->CalculateFirstSectorOfCluster(current_cluster);
			end_address = start_address + mpcVolume->GetNoOfSectorsPerCluster();

			// find the last sequential sector after this address
			while (!mpcVolume->FatIsEOFEntry(current_cluster))
			{
				mpcVolume->FatGetClusterEntry(current_cluster, &next_cluster);

				if (next_cluster == (current_cluster + 1))
				{
					end_address += mpcVolume->GetNoOfSectorsPerCluster();
					current_cluster = next_cluster;
				}
				else
				{
					current_cluster = next_cluster;
					break;
				}
			}

			// pre-erase the clusters
			mpcVolume->Erase(start_address, end_address - 1);
		}
	}
	else
	{
		new_cluster = mpcVolume->FatAllocateDataCluster(no_of_clusters_needed, 0, &uiResult);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return uiResult;
		}
	}

	// if this is the 1st cluster cluster allocated
	// to the file then we must modify the file's entry
	if (!msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo && !msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi)
	{
		// modify the file entry to point to the
		// new cluster
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo = LO16(new_cluster);
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi = HI16(new_cluster);

		// mark the cached sector as unknown
		mpcVolume->SetFatSharedBufferSector(FAT_UNKNOWN_SECTOR);

		// try load the sector that contains the entry
		bSuccess = mpcVolume->Read(msFile.sDirectoryEntry.sector_addr, pvBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			msFile.bBusy = 0;
			return uiResult;
		}

		// copy the modified file entry to the
		// sector uiBuffer
		memcpy(pvBuffer + msFile.sDirectoryEntry.sector_offset, &msFile.sDirectoryEntry.raw, sizeof(SFatRawDirectoryEntry));

		// write the modified entry to the media
		bSuccess = mpcVolume->Write(msFile.sDirectoryEntry.sector_addr, pvBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			msFile.bBusy = 0;
			return uiResult;
		}
	}
	// if there are clusters allocated to the file update the last FAT entry
	// of the file to point to the newly allocated cluster(s)
	else
	{
		uint32 last_cluster;

		if (msFile.uiNoOfClustersAfterPos)
		{
			if (!mpcVolume->FatIncreaseClusterAddress(msFile.uiCurrentClusterAddress, msFile.uiNoOfClustersAfterPos, &last_cluster))
			{
				msFile.bBusy = 0;
				return FAT_CORRUPTED_FILE;
			}
		}
		else
		{
			last_cluster = msFile.uiCurrentClusterAddress;
		}

		// set the FAT entry for the last cluster to the beggining of the newly
		// allocated cluster chain (ie. link them)
		uiResult = mpcVolume->FatSetClusterEntry(last_cluster, (FatEntry)new_cluster);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return uiResult;
		}
	}

	// update the file to point to the
	// new cluster
	if (!msFile.uiCurrentClusterAddress)
	{
		msFile.uiCurrentClusterAddress = new_cluster;
		msFile.uiNoOfClustersAfterPos = no_of_clusters_needed - 1;
	}
	else
	{
		msFile.uiNoOfClustersAfterPos += no_of_clusters_needed;
	}

	// update the count of sequential clusters
	FatFileUpdateSequentialClusterCount();

	msFile.bBusy = 0;
	return FAT_SUCCESS;
}


// moves the file cursor to the specified offset
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileSeek(uint32 offset, char mode)
{
	uint32	new_pos;
	uint32	sector_count;
	uint32	cluster_count;
	uint32	old_cluster;
	uint32	sector_address;
	uint16	uiResult;
	bool	bSuccess;

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// check that another operation is not using the
	// file at this time
	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	msFile.bBusy = 1;

	switch (mode)
	{
		case FAT_SEEK_START:
			new_pos = offset;
			break;

		case FAT_SEEK_CURRENT:
			new_pos = msFile.uiCurrentClusterIdx * mpcVolume->GetNoOfSectorsPerCluster() * mpcVolume->GetNoOfBytesPerSector();
			new_pos += msFile.uiCurrentSectorIdx * mpcVolume->GetNoOfBytesPerSector();
			new_pos += (uint32)(msFile.pvBufferHead - msFile.uiBuffer);
			new_pos += offset;
			break;

		case FAT_SEEK_END:
			new_pos = msFile.uiCurrentSize;
			if (offset)
			{
				msFile.bBusy = 0;
				return FAT_SEEK_FAILED;
			}
			break;

		default:
			msFile.bBusy = 0;
			return FAT_SEEK_FAILED;
	}

	// if the seek goes out of bounds return error
	if (new_pos > msFile.uiCurrentSize)
	{
		msFile.bBusy = 0;
		return FAT_SEEK_FAILED;
	}

	// if the file is open in unbuffered mode make sure that
	// we're seeking to a sector boundry
	if (msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (new_pos % mpcVolume->GetNoOfBytesPerSector())
		{
			msFile.bBusy = 0;
			return FAT_MISALIGNED_IO;
		}
	}

	old_cluster = msFile.uiCurrentClusterIdx;
	msFile.uiCurrentClusterIdx = 0;
	cluster_count = 1;

	// calculate the count of sectors being used by the file up to the desired position
	sector_count = (new_pos + mpcVolume->GetNoOfBytesPerSector() - 1) / mpcVolume->GetNoOfBytesPerSector();

	// set the 1st cluster as the current cluster, we'll seek from there
	((uint16*)&msFile.uiCurrentClusterAddress)[INT32_WORD0] = msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo;
	((uint16*)&msFile.uiCurrentClusterAddress)[INT32_WORD1] = (mpcVolume->GetFileSystemType() == FAT_FS_TYPE_FAT32) ? msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi : 0;

	// if the file occupies more than one cluster
	if (sector_count > mpcVolume->GetNoOfSectorsPerCluster())
	{
		// calculate the count of clusters occupied by the file and
		// update the ClustersAllocated value of the file
		cluster_count = (sector_count + mpcVolume->GetNoOfSectorsPerCluster() - 1) / mpcVolume->GetNoOfSectorsPerCluster();

		// set the file file to point to the last cluster. if the file doesn't have
		// that many clusters allocated this function will return 0. if that ever happens it means
		// that the file is corrupted
		if (!mpcVolume->FatIncreaseClusterAddress(msFile.uiCurrentClusterAddress, (cluster_count - 1), &msFile.uiCurrentClusterAddress))
		{
			msFile.bBusy = 0;
			return FAT_CORRUPTED_FILE;
		}
	}

	// calculate the last sector of the file and set the uiBuffer
	// head to point to the byte after the last one
	if (new_pos)
	{
		msFile.uiCurrentSectorIdx = (((new_pos + mpcVolume->GetNoOfBytesPerSector() - 1) / mpcVolume->GetNoOfBytesPerSector()) - 1) % mpcVolume->GetNoOfSectorsPerCluster();
		msFile.pvBufferHead = (uint8*)((uintptr_t)msFile.uiBuffer) + (new_pos % mpcVolume->GetNoOfBytesPerSector());

		if (new_pos % mpcVolume->GetNoOfBytesPerSector() == 0)
		{
			msFile.pvBufferHead = (uint8*)((uintptr_t)msFile.uiBuffer) + mpcVolume->GetNoOfBytesPerSector();
		}
	}
	else
	{
		msFile.uiCurrentSectorIdx = 0;
		msFile.pvBufferHead = msFile.uiBuffer;
	}

	sector_address = msFile.uiCurrentSectorIdx + mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCurrentClusterAddress);
	msFile.uiCurrentClusterIdx = cluster_count - 1;

	// load the last sector
	if (msFile.uiBuffer)
	{
		bSuccess = mpcVolume->Read(sector_address, msFile.uiBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}
		msFile.bBufferDirty = 0;
	}

	// maintain the count of allocated clusters
	if (old_cluster < msFile.uiCurrentClusterIdx)
	{
		msFile.uiNoOfClustersAfterPos -= (msFile.uiCurrentClusterIdx - old_cluster);
	}
	else if (old_cluster > msFile.uiCurrentClusterIdx)
	{
		msFile.uiNoOfClustersAfterPos += (old_cluster - msFile.uiCurrentClusterIdx);
	}
	msFile.bBusy = 0;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatFile::FatFileWriteCallback(uint16* async_state_in)
{
	uint16	uiResult;
	uint16* state;
	bool	bSuccess;

	state = async_state_in;

	// loop while there are bytes to be read
	while (msFile.sOperationState.bytes_remaining)
	{
		// if we've reached the end of the current
		// sector then we must flush it
		if (msFile.pvBufferHead == msFile.sOperationState.end_of_buffer)
		{
			// update the sector head
			if (msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
			{
				msFile.uiBuffer = msFile.pvBufferHead;
				msFile.sOperationState.end_of_buffer = msFile.uiBuffer + mpcVolume->GetNoOfBytesPerSector();
			}
			else
			{
				msFile.sOperationState.end_of_buffer = msFile.uiBuffer + mpcVolume->GetNoOfBytesPerSector();
				msFile.pvBufferHead = msFile.uiBuffer;
			}
			// write the cached sector to media
			bSuccess = mpcVolume->Write(msFile.sOperationState.sector_addr, msFile.uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;

			// if we were unable to read the sector
			// then throw an error
			if (uiResult != STORAGE_SUCCESS)
			{
				*state = FAT_CANNOT_WRITE_MEDIA;
				msFile.bBusy = 0;
				return;
			}
			// if this sector is the last of the current cluster then
			// locate the next cluster and continue writing
			if (msFile.uiCurrentSectorIdx == mpcVolume->GetNoOfSectorsPerCluster() - 1)
			{
				uiResult = mpcVolume->FatGetClusterEntry(msFile.uiCurrentClusterAddress, &msFile.uiCurrentClusterAddress);
				if (uiResult != STORAGE_SUCCESS)
				{
					*state = FAT_CANNOT_WRITE_MEDIA;
					msFile.bBusy = 0;
					return;
				}
				// reset the current sector and increase the cluster index
				msFile.uiCurrentSectorIdx = 0x0;
				msFile.uiCurrentClusterIdx++;
				msFile.uiNoOfClustersAfterPos--;
				// calculate the sector address
				msFile.sOperationState.sector_addr = mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCurrentClusterAddress);
			}
			// if there are more sectors in the
			// current cluster then simply increase
			// the current sector counter and address
			else
			{
				msFile.uiCurrentSectorIdx++;
				msFile.sOperationState.sector_addr++;
			}
		}
		if (msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
		{
			msFile.pvBufferHead = msFile.sOperationState.end_of_buffer;
			msFile.sOperationState.pos += mpcVolume->GetNoOfBytesPerSector();

			if (msFile.sOperationState.pos >= msFile.uiCurrentSize)
			{
				msFile.uiCurrentSize = msFile.sOperationState.pos;
			}
			msFile.sOperationState.bytes_remaining -= mpcVolume->GetNoOfBytesPerSector();
		}
		else
		{
			// write the next byte to the file cache
			*msFile.pvBufferHead++ = *msFile.sOperationState.uiBuffer++;

			// update the file size only if we're writting past
			// the end of the file
			if (msFile.sOperationState.pos++ >= msFile.uiCurrentSize)
			{
				msFile.uiCurrentSize++;
			}
			msFile.sOperationState.bytes_remaining--;
		}
	}

	*state = FAT_SUCCESS;
	msFile.bBusy = 0;
	return;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileWrite(uint8* buff, uint32 length)
{
	uint32 uiResult;

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}
	// check that the file is open for write access
	if (!(msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE))
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
	uiResult = FatFileAllocate(length);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// make sure that either a uiBuffer is set or the file has been opened in
	// unbuffered mode.
	if (!msFile.uiBuffer && !(msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		return FAT_FILE_BUFFER_NOT_SET;
	}

	// check that another operation is not using the
	// file at this time
	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	msFile.bBusy = 1;

	// calculate current pos
	msFile.sOperationState.pos = msFile.uiCurrentClusterIdx * mpcVolume->GetNoOfSectorsPerCluster() * mpcVolume->GetNoOfBytesPerSector();
	msFile.sOperationState.pos += (msFile.uiCurrentSectorIdx) * mpcVolume->GetNoOfBytesPerSector();
	msFile.sOperationState.pos += (uintptr_t)(msFile.pvBufferHead - msFile.uiBuffer);

	// if the file is opened in unbuffered mode make sure that
	// we're reading a multiple of the sector size
	if (msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (length % mpcVolume->GetNoOfBytesPerSector())
		{
			msFile.bBusy = 0;
			return FAT_MISALIGNED_IO;
		}
		msFile.uiBuffer = buff;
		msFile.sOperationState.end_of_buffer = msFile.pvBufferHead = buff;
	}
	else
	{
		// calculate the address of the end of
		// the current sector
		msFile.sOperationState.end_of_buffer = msFile.uiBuffer + mpcVolume->GetNoOfBytesPerSector();
	}

	// copy the length of the uiBuffer to be writen
	// into the counter
	msFile.sOperationState.bytes_remaining = (uint16)length;

	// calculate the address of the current sector
	msFile.sOperationState.sector_addr = msFile.uiCurrentSectorIdx + mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCurrentClusterAddress);


	msFile.sOperationState.internal_state = 0x0;
	msFile.sOperationState.length = (uint16)length;
	msFile.sOperationState.uiBuffer = buff;

	uint16 internal_state;
	FatFileWriteCallback(&internal_state);
	return internal_state;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatFile::FatFileReadCallback(uint16* state)
{
	uint16	uiResult;
	bool	bSuccess;

	// reset the # of bytes read
	if (msFile.sOperationState.bytes_read)
	{
		*msFile.sOperationState.bytes_read = 0;
	}

	// if the sector cache is invalid
	if (msFile.bBufferDirty)
	{
		// read the current sector synchronously
		bSuccess = mpcVolume->Read(msFile.sOperationState.sector_addr, msFile.uiBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			*state = FAT_CANNOT_READ_MEDIA;
			msFile.bBusy = 0;
			return;
		}
		// mark the cache as clean
		msFile.bBufferDirty = 0;

		// if the file is unbuffered decrease the count of bytes remaining
		// by one sector and set the uiBuffer head to the end of the uiBuffer
		if (msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
		{
			msFile.sOperationState.bytes_remaining -= mpcVolume->GetNoOfBytesPerSector();
			msFile.pvBufferHead += mpcVolume->GetNoOfBytesPerSector();
			msFile.sOperationState.end_of_buffer = msFile.pvBufferHead;
			if (msFile.sOperationState.bytes_read)
			{
				*msFile.sOperationState.bytes_read += mpcVolume->GetNoOfBytesPerSector();
			}

			msFile.sOperationState.pos += mpcVolume->GetNoOfBytesPerSector();

			if (msFile.sOperationState.pos >= msFile.uiCurrentSize)
			{
				if (msFile.sOperationState.bytes_read)
				{
					*msFile.sOperationState.bytes_read -= msFile.sOperationState.pos - msFile.uiCurrentSize;
				}
				msFile.sOperationState.bytes_remaining = 0;
			}
		}
	}

	// make sure that we haven't reaced the end of the file
	if (msFile.sOperationState.pos >= msFile.uiCurrentSize)
	{
		msFile.sOperationState.bytes_remaining = 0;
	}

	// loop while there are bytes to be
	// read
	while (msFile.sOperationState.bytes_remaining)
	{
		// if we've reached the end of the current
		// sector then we must load the next...
		if (msFile.pvBufferHead == msFile.sOperationState.end_of_buffer)
		{
			// if the file is unbuffered change the uiBuffer to the
			// new position of the uiBuffer head. otherwise reset the position
			// of the position of the uiBuffer head
			if (msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
			{
				msFile.uiBuffer = msFile.pvBufferHead;
				msFile.sOperationState.end_of_buffer = msFile.pvBufferHead + mpcVolume->GetNoOfBytesPerSector();
			}
			else
			{
				msFile.pvBufferHead = msFile.uiBuffer;
			}

			// if this sector is the last of the current cluster
			// we must find the next cluster
			if (msFile.uiCurrentSectorIdx == mpcVolume->GetNoOfSectorsPerCluster() - 1)
			{
				// update the cluster address with the address of the
				// next cluster
				if (!mpcVolume->FatIncreaseClusterAddress(msFile.uiCurrentClusterAddress, 1, &msFile.uiCurrentClusterAddress))
				{
					*state = FAT_CORRUPTED_FILE;
					msFile.bBusy = 0;
					return;
				}

				// reset the current sector increase the current cluster
				// number and calculate the address of the first sector of the
				// cluster
				msFile.uiCurrentClusterIdx++;
				msFile.uiCurrentSectorIdx = 0x0;
				msFile.sOperationState.sector_addr = mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCurrentClusterAddress);
			}
			else
			{

				// if there are more sectors in the current cluster then
				// simply increase the current sector counter and address
				msFile.uiCurrentSectorIdx++;
				msFile.sOperationState.sector_addr++;
			}

			// read the next sector into the cache
			bSuccess = mpcVolume->Read(msFile.sOperationState.sector_addr, msFile.uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				*state = FAT_CANNOT_READ_MEDIA;
				msFile.bBusy = 0;
				return;
			}
		}

		// update the count of bytes read/remaining and if the file
		// is buffered copy data to file uiBuffer
		if (msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
		{
			msFile.pvBufferHead = msFile.sOperationState.end_of_buffer;
			msFile.sOperationState.bytes_remaining -= mpcVolume->GetNoOfBytesPerSector();
			if (msFile.sOperationState.bytes_read)
			{
				(*msFile.sOperationState.bytes_read) += mpcVolume->GetNoOfBytesPerSector();
			}

			msFile.sOperationState.pos += mpcVolume->GetNoOfBytesPerSector();

			if (msFile.sOperationState.pos >= msFile.uiCurrentSize)
			{
				if (msFile.sOperationState.bytes_read)
				{
					*msFile.sOperationState.bytes_read -= (msFile.sOperationState.pos - msFile.uiCurrentSize);
				}
				msFile.sOperationState.bytes_remaining = 0;
			}
		}
		else
		{
			// copy the next byte to the uiBuffer
			*msFile.sOperationState.uiBuffer++ = *msFile.pvBufferHead++;

			// update the  count of bytes read
			if (msFile.sOperationState.bytes_read)
			{
				(*msFile.sOperationState.bytes_read)++;
			}

			// decrease the count of remaining bytes
			msFile.sOperationState.bytes_remaining--;

			// increase the file pointer
			msFile.sOperationState.pos++;

			// check if we've reached the end of the file
			if (msFile.sOperationState.pos >= msFile.uiCurrentSize)
			{
				msFile.sOperationState.bytes_remaining = 0;
			}
		}
	}

	*state = FAT_SUCCESS;
	msFile.bBusy = 0;
	return;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileRead(uint8* buff, uint32 length, uint32* bytes_read)
{
	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// make sure that either a uiBuffer is set or the file has been
	// opened in unbuffered mode
	if (!msFile.uiBuffer && !(msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING))
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
	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	msFile.bBusy = 1;

	// calculate current pos
	msFile.sOperationState.pos = msFile.uiCurrentClusterIdx * mpcVolume->GetNoOfSectorsPerCluster() * mpcVolume->GetNoOfBytesPerSector();
	msFile.sOperationState.pos += (msFile.uiCurrentSectorIdx) * mpcVolume->GetNoOfBytesPerSector();
	msFile.sOperationState.pos += (uintptr_t)(msFile.pvBufferHead - msFile.uiBuffer);

	// calculate the address of the current
	// sector and the address of the end of the uiBuffer
	msFile.sOperationState.sector_addr = msFile.uiCurrentSectorIdx + mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCurrentClusterAddress);
	msFile.sOperationState.end_of_buffer = msFile.uiBuffer + mpcVolume->GetNoOfBytesPerSector();

	// if the file is opened in unbuffered mode make sure that
	// we're reading a multiple of the sector size and set the uiBuffer
	// to the one supplied by the user in this call
	if (msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING)
	{
		if (length % mpcVolume->GetNoOfBytesPerSector())
		{
			return FAT_MISALIGNED_IO;
		}
		msFile.uiBuffer = buff;
		msFile.sOperationState.end_of_buffer = msFile.pvBufferHead = buff;
	}

	// set the async op context
	msFile.sOperationState.bytes_remaining = (uint16)length;
	msFile.sOperationState.internal_state = 0x0;
	msFile.sOperationState.length = (uint16)length;
	msFile.sOperationState.uiBuffer = buff;
	msFile.sOperationState.bytes_read = bytes_read;

	uint16 internal_state;
	FatFileReadCallback(&internal_state);
	return internal_state;
}


// flushes file buffers
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileFlush(void)
{
	uint16	uiResult;
	uint32	sector_address = 0;
	bool	bSuccess;
	uint8*	pvBuffer = mpcVolume->GetFatSharedBuffer();

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// make sure that either a uiBuffer has been set or the file was
	// opened in unbuffered mode
	if (!msFile.uiBuffer && !(msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING))
	{
		return FAT_FILE_BUFFER_NOT_SET;
	}

	// check that we got write access
	if (msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		// check that another operation is not using the
		// file at this time
		if (msFile.bBusy)
		{
			return FAT_FILE_HANDLE_IN_USE;
		}

		// mark the file as in use
		msFile.bBusy = 1;

		// if the file is buffered then flush the uiBuffer
		if (!(msFile.uiAccessFlags & FAT_FILE_FLAG_NO_BUFFERING))
		{
			// calculate the address of the current
			// sector
			sector_address = msFile.uiCurrentSectorIdx + mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCurrentClusterAddress);

			// if the uiBuffer is only partially filled we need to merge it
			// with the one on the drive
			if (msFile.pvBufferHead <= msFile.uiBuffer + mpcVolume->GetNoOfBytesPerSector())
			{
				uint16 i;
				uint8 buff[MAX_SECTOR_LENGTH];
				bSuccess = mpcVolume->Read(sector_address, buff);
				uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
				if (uiResult != STORAGE_SUCCESS)
				{
					msFile.bBusy = 0;
					return uiResult;
				}

				for (i = (uint16)(msFile.pvBufferHead -
					msFile.uiBuffer); i < mpcVolume->GetNoOfBytesPerSector(); i++)
				{
					msFile.uiBuffer[i] = buff[i];
				}
			}

			// write the cached sector to media
			bSuccess = mpcVolume->Write(sector_address, msFile.uiBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				msFile.bBusy = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}
		}

		// update the file size on the entry
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.size = msFile.uiCurrentSize;
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.modify_date = rtc_get_fat_date();
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.modify_time = rtc_get_fat_time();
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.access_date = msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.modify_date;

		// try load the sector that contains the entry
		mpcVolume->SetFatSharedBufferSector(FAT_UNKNOWN_SECTOR);

		bSuccess = mpcVolume->Read(msFile.sDirectoryEntry.sector_addr, pvBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}

		// copy the modified file entry to the
		// sector uiBuffer
		memcpy(pvBuffer + msFile.sDirectoryEntry.sector_offset, &msFile.sDirectoryEntry.raw, sizeof(SFatRawDirectoryEntry));

		// write the modified entry to the media
		bSuccess = mpcVolume->Write(msFile.sDirectoryEntry.sector_addr, pvBuffer);
		uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
		if (uiResult != STORAGE_SUCCESS)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_WRITE_MEDIA;
		}

		// some cards seem not to update the sector correctly if it
		// is the first sector on the page and is the only sector written
		// to so for now until we figure this out we'll write the next
		// sector too TODO: implement this on driver!!
		{
			bSuccess = mpcVolume->Read(msFile.sDirectoryEntry.sector_addr + 1, pvBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				msFile.bBusy = 0;
				return FAT_CANNOT_READ_MEDIA;
			}
			bSuccess = mpcVolume->Write(msFile.sDirectoryEntry.sector_addr + 1, pvBuffer);
			uiResult = bSuccess ? STORAGE_SUCCESS : STORAGE_UNKNOWN_ERROR;
			if (uiResult != STORAGE_SUCCESS)
			{
				msFile.bBusy = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}
		}

		// mark the file file as not bBusy
		msFile.bBusy = 0;
	}
	return FAT_SUCCESS;
}


// closes an open file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatFile::FatFileClose(void)
{
	uint16		uiResult;
	FatEntry	fat_entry;

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// flush the file buffers
	uiResult = FatFileFlush();
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	if (msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		// clear the no buffering attribute so we can seek to a misaligned
		// position (since we want the very end of the last written, not the
		// beginning of the next one.)
		msFile.uiAccessFlags &= (FAT_FILE_FLAG_NO_BUFFERING ^ 0xFF);

		// seek to the end of the file
		uiResult = FatFileSeek(msFile.uiCurrentSize - 1, FAT_SEEK_START);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// check that another operation is not using the
		// file at this time
		if (msFile.bBusy)
		{
			return FAT_FILE_HANDLE_IN_USE;
		}

		// mark the file as in use
		msFile.bBusy = 1;

		// free unused clusters
		uiResult = mpcVolume->FatGetClusterEntry(msFile.uiCurrentClusterAddress, &fat_entry);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		if (!mpcVolume->FatIsEOFEntry(fat_entry))
		{
			uiResult = mpcVolume->FatFreeClusterChain(fat_entry);
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}

			switch (mpcVolume->GetFileSystemType())
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
			uiResult = mpcVolume->FatSetClusterEntry(msFile.uiCurrentClusterAddress, fat_entry);
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}
		}
	}

	// invalidate the file file
	msFile.uiMagic = 0;
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

