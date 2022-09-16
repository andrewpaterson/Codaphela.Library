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
#include "BaseLib/PointerFunctions.h"
#include "FatVolume.h"
#include "FatTime.h"
#include "FatFile.h"


 //////////////////////////////////////////////////////////////////////////
 //
 //
 //////////////////////////////////////////////////////////////////////////
void CFatFile::Init(CFatVolume* pcVolume)
{
	memset(&msFile, 0, sizeof(SFatFile));
	mpcVolume = pcVolume;
	mpvBuffer = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatFileUpdateSequentialClusterCount(void)
{
	uint32		uiCachedClusterInVolume;
	uint32		uiNextCluster;
	EFatCode	eResult;

	// find out how many clusters are allocated sequentially
	// to this file following the current cursor location
	msFile.uiNoOfSequentialClusters = 0;
	uiCachedClusterInVolume = msFile.uiCachedClusterInVolume;

	while (!mpcVolume->FatIsEOFEntry(uiCachedClusterInVolume))
	{
		eResult = mpcVolume->GetNextClusterEntry(uiCachedClusterInVolume, &uiNextCluster);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		if (uiNextCluster == (uiCachedClusterInVolume + 1))
		{
			msFile.uiNoOfSequentialClusters++;
			uiCachedClusterInVolume = uiNextCluster;
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

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatFile::AllocateBuffer(void)
{
	mpvBuffer = (uint8*)malloc(mpcVolume->GetClusterSize());
}


// opens a file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Open(char* filename, uint8 uiAccessFlags)
{
	EFatCode				uiResult;
	SFatDirectoryEntry		file_entry;

	AllocateBuffer();

	// check that the user supplied a filename
	if (filename == 0 || strlen(filename) > FAT_MAX_PATH)
	{
		return FAT_INVALID_FILENAME;
	}

	// get the file entry
	uiResult = mpcVolume->FatGetFileEntry(filename, &file_entry);

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
			char* filename_scanner;
			char				file_path[FAT_MAX_PATH + 1];
			SFatDirectoryEntry	sParentEntry;

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
			uiResult = mpcVolume->FatGetFileEntry(file_path, &sParentEntry);

			// if FatGetFileEntry returned an error
			// then we return the error code to the calling
			// function
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}

			// if the parent directory does not exists
			if (*sParentEntry.name == 0)
			{
				return FAT_DIRECTORY_DOES_NOT_EXIST;
			}

			// try to create the directory entry
			uiResult = mpcVolume->FatCreateDirectoryEntry(&sParentEntry.raw, filename_scanner, 0, 0, &file_entry);

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

	uiResult = FatOpenFileByEntry(&file_entry, uiAccessFlags);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if the file has no clusters allocated then allocate one
	if (msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		uiResult = FatFileUpdateSequentialClusterCount();
		return uiResult;
	}
	else
	{
		return FAT_SUCCESS;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Open(SFatDirectoryEntry* psEntry, uint8 uiAccessFlags)
{
	AllocateBuffer();

	return FatOpenFileByEntry(psEntry, uiAccessFlags);
}


// opens a file given a pointer to it's directory entry
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatOpenFileByEntry(SFatDirectoryEntry* psEntry, uint8 uiAccessFlags)
{
	bool		bSuccess;
	EFatCode	uiResult;

	// set implicit access flags
	uiAccessFlags |= FAT_FILE_ACCESS_READ;
	if (uiAccessFlags & (FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_APPEND | FAT_FILE_ACCESS_OVERWRITE))
	{
		uiAccessFlags |= FAT_FILE_ACCESS_WRITE;
	}

	// if the user is trying to open a directory then
	// return an error code, otherwise it would corrupt
	// the volume
	if (psEntry->attributes & FAT_ATTR_DIRECTORY)
	{
		return FAT_NOT_A_FILE;
	}

	// copy the volume file and the entry's
	// structure to the file file
	msFile.sDirectoryEntry = *psEntry;
	msFile.uiFileSize = psEntry->size;
	msFile.uiCachedClusterIndex = 0;
	msFile.uiAccessFlags = uiAccessFlags;
	msFile.uiMagic = FAT_OPEN_HANDLE_MAGIC;
	msFile.bBusy = 0;

	// calculate the # of clusters allocated
	msFile.uiNoOfClustersAfterPos = (psEntry->size + mpcVolume->GetNoOfBytesPerSector() - 1) / mpcVolume->GetNoOfBytesPerSector();
	msFile.uiNoOfClustersAfterPos = (msFile.uiNoOfClustersAfterPos + mpcVolume->GetNoOfSectorsPerCluster() - 1) / mpcVolume->GetNoOfSectorsPerCluster();
	if (msFile.uiNoOfClustersAfterPos)
	{
		msFile.uiNoOfClustersAfterPos--;
	}

	// read the the cluster number
	((uint16*)&msFile.uiCachedClusterInVolume)[INT32_WORD0] = psEntry->raw.uEntry.sFatRawCommon.first_cluster_lo;
	((uint16*)&msFile.uiCachedClusterInVolume)[INT32_WORD1] = (mpcVolume->GetFileSystemType() == FAT_FS_TYPE_FAT32) ? psEntry->raw.uEntry.sFatRawCommon.first_cluster_hi : 0;

	if (uiAccessFlags & FAT_FILE_ACCESS_APPEND)
	{
		// if the file is being opened for append access we
		// seek to the end of the file
		uiResult = Seek(0, FAT_SEEK_END);
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
		if (psEntry->raw.uEntry.sFatRawCommon.first_cluster_lo != 0x0 || psEntry->raw.uEntry.sFatRawCommon.first_cluster_hi != 0x0)
		{
			uint8* uiBuffer = mpcVolume->GetFatSharedBuffer();

			// update the entry to point to cluster 0
			psEntry->raw.uEntry.sFatRawCommon.first_cluster_lo = 0x0;
			psEntry->raw.uEntry.sFatRawCommon.first_cluster_hi = 0x0;
			psEntry->raw.uEntry.sFatRawCommon.size = 0x0;
			msFile.sDirectoryEntry = *psEntry;
			msFile.uiFileSize = psEntry->size;

			// mark the cached sector as unknown
			mpcVolume->SetFatSharedBufferSector(FAT_UNKNOWN_SECTOR);

			// read the sector that contains the entry
			bSuccess = mpcVolume->Read(psEntry->uiSectorAddress, uiBuffer);
			if (!bSuccess)
			{
				msFile.uiMagic = 0;
				return FAT_CANNOT_READ_MEDIA;
			}
			// copy the modified file entry to the sector uiBuffer
			memcpy(uiBuffer + psEntry->uiSectorOffset, &psEntry->raw, sizeof(SFatRawDirectoryEntry));

			// write the modified entry to the media
			bSuccess = mpcVolume->Write(psEntry->uiSectorAddress, uiBuffer);
			if (!bSuccess)
			{
				msFile.uiMagic = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}

			// free the clusters allocated to the file
			mpcVolume->FatFreeClusterChain(msFile.uiCachedClusterInVolume);
		}

		// reset the file file to reflect an
		// empty file
		msFile.uiFileSize = 0x0;
		msFile.uiFirstCachedSectorIndexInCluster = 0x0;
		msFile.uiCachedClusterInVolume = 0x0;
		msFile.uiCachedClusterIndex = 0;
		msFile.uiBufferSeekByteIndexInSector = 0;
		msFile.uiNoOfClustersAfterPos = 0;
	}
	else
	{
		// if no append or overwrite flags are set then
		// simply set the file to point to the beggining
		// of the 1st cluster
		msFile.uiFirstCachedSectorIndexInCluster = 0x0;
		msFile.uiBufferSeekByteIndexInSector = 0;
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
EFatCode CFatFile::FatFileAllocate(uint32 uiBytes)
{
	EFatCode	uiResult;
	uint32		uiNewCluster;
	uint32		uiClustersNeeded;
	bool		bSuccess;
	uint8* pvBuffer = mpcVolume->GetFatSharedBuffer();

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
	uiClustersNeeded = (uiBytes + mpcVolume->GetNoOfBytesPerSector() - 1) / mpcVolume->GetNoOfBytesPerSector();
	uiClustersNeeded = (uiClustersNeeded + mpcVolume->GetNoOfSectorsPerCluster() - 1) / mpcVolume->GetNoOfSectorsPerCluster();
	uiClustersNeeded = (msFile.uiNoOfClustersAfterPos > uiClustersNeeded) ? 0 : (uiClustersNeeded - msFile.uiNoOfClustersAfterPos);

	// if we already got all the clusters requested then there is nothing to do
	if (uiClustersNeeded == 0)
	{
		msFile.bBusy = 0;
		return FAT_SUCCESS;
	}

	// allocate a new cluster
	uint32 uiCachedClusterInVolume;
	uint32 uiNextCluster;
	uint32 page_size;
	uint32 start_address;
	uint32 end_address;

	page_size = mpcVolume->GetPageSize();

	if (page_size > mpcVolume->GetNoOfSectorsPerCluster())
	{
		uint32 clusters_per_page = page_size / mpcVolume->GetNoOfSectorsPerCluster();

		if (uiClustersNeeded % clusters_per_page)
		{
			uiClustersNeeded += clusters_per_page - (uiClustersNeeded % clusters_per_page);
		}

		if ((uiClustersNeeded % clusters_per_page) != 0)
		{
			return FAT_UNKNOWN_ERROR;
		}

		uiResult = FAT_UNKNOWN_ERROR;
		uiNewCluster = mpcVolume->FatAllocateDataClusterEx(uiClustersNeeded, 0, page_size, &uiResult);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return uiResult;
		}
	}
	else
	{
		uiResult = FAT_UNKNOWN_ERROR;
		uiNewCluster = mpcVolume->FatAllocateDataCluster(uiClustersNeeded, 1, &uiResult);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return uiResult;
		}
	}

	// find out how many clusters are allocated sequentially
	// to this file following the current cursor location
	uiCachedClusterInVolume = uiNewCluster;

	while (!mpcVolume->FatIsEOFEntry(uiCachedClusterInVolume))
	{
		// calculate the start and end address the cluster
		start_address = mpcVolume->CalculateFirstSectorOfCluster(uiCachedClusterInVolume);
		end_address = start_address + mpcVolume->GetNoOfSectorsPerCluster();

		// find the last sequential sector after this address
		while (!mpcVolume->FatIsEOFEntry(uiCachedClusterInVolume))
		{
			mpcVolume->GetNextClusterEntry(uiCachedClusterInVolume, &uiNextCluster);

			if (uiNextCluster == (uiCachedClusterInVolume + 1))
			{
				end_address += mpcVolume->GetNoOfSectorsPerCluster();
				uiCachedClusterInVolume = uiNextCluster;
			}
			else
			{
				uiCachedClusterInVolume = uiNextCluster;
				break;
			}
		}

		// pre-erase the clusters
		mpcVolume->Erase(start_address, end_address - 1);
	}

	// if this is the 1st cluster cluster allocated
	// to the file then we must modify the file's entry
	if (!msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo && !msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi)
	{
		// modify the file entry to point to the
		// new cluster
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo = LO16(uiNewCluster);
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi = HI16(uiNewCluster);

		// mark the cached sector as unknown
		mpcVolume->SetFatSharedBufferSector(FAT_UNKNOWN_SECTOR);

		// try load the sector that contains the entry
		bSuccess = mpcVolume->Read(msFile.sDirectoryEntry.uiSectorAddress, pvBuffer);
		if (!bSuccess)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}

		// copy the modified file entry to the
		// sector uiBuffer
		memcpy(pvBuffer + msFile.sDirectoryEntry.uiSectorOffset, &msFile.sDirectoryEntry.raw, sizeof(SFatRawDirectoryEntry));

		// write the modified entry to the media
		bSuccess = mpcVolume->Write(msFile.sDirectoryEntry.uiSectorAddress, pvBuffer);
		if (!bSuccess)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_WRITE_MEDIA;
		}
	}
	// if there are clusters allocated to the file update the last FAT entry
	// of the file to point to the newly allocated cluster(s)
	else
	{
		uint32 last_cluster;

		if (msFile.uiNoOfClustersAfterPos)
		{
			uiResult = mpcVolume->FatIncreaseClusterAddress(msFile.uiCachedClusterInVolume, msFile.uiNoOfClustersAfterPos, &last_cluster);
			if (uiResult != FAT_SUCCESS)
			{
				msFile.bBusy = 0;
				return FAT_CORRUPTED_FILE;
			}
		}
		else
		{
			last_cluster = msFile.uiCachedClusterInVolume;
		}

		// set the FAT entry for the last cluster to the beggining of the newly
		// allocated cluster chain (ie. link them)
		uiResult = mpcVolume->FatSetClusterEntry(last_cluster, (FatEntry)uiNewCluster);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return uiResult;
		}
	}

	// update the file to point to the new cluster
	if (!msFile.uiCachedClusterInVolume)
	{
		msFile.uiCachedClusterInVolume = uiNewCluster;
		msFile.uiNoOfClustersAfterPos = uiClustersNeeded - 1;
	}
	else
	{
		msFile.uiNoOfClustersAfterPos += uiClustersNeeded;
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
EFatCode CFatFile::Seek(uint32 offset, char mode)
{
	uint32	uiBytePositionInFile;
	uint32	uiSectorIndexInFile;
	uint32	uiClusterIndex;
	uint32	uiOldClusterIndex;
	uint32	uiSectorAddress;
	uint16	uiResult;

	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	msFile.bBusy = 1;

	switch (mode)
	{
	case FAT_SEEK_START:
		uiBytePositionInFile = offset;
		break;

	case FAT_SEEK_CURRENT:
		uiBytePositionInFile = msFile.uiCachedClusterIndex * mpcVolume->GetNoOfSectorsPerCluster() * mpcVolume->GetNoOfBytesPerSector();
		uiBytePositionInFile += msFile.uiFirstCachedSectorIndexInCluster * mpcVolume->GetNoOfBytesPerSector();
		uiBytePositionInFile += msFile.uiBufferSeekByteIndexInSector;
		uiBytePositionInFile += offset;
		break;

	case FAT_SEEK_END:
		uiBytePositionInFile = msFile.uiFileSize - offset;
		break;

	default:
		msFile.bBusy = 0;
		return FAT_SEEK_FAILED;
	}

	// if the seek goes out of bounds return error
	if (uiBytePositionInFile > msFile.uiFileSize)
	{
		msFile.bBusy = 0;
		return FAT_SEEK_FAILED;
	}

	uiOldClusterIndex = msFile.uiCachedClusterIndex;
	msFile.uiCachedClusterIndex = 0;
	uiClusterIndex = 0;

	// calculate the count of sectors being used by the file up to the desired position
	uiSectorIndexInFile = (uiBytePositionInFile + mpcVolume->GetNoOfBytesPerSector() - 1) / mpcVolume->GetNoOfBytesPerSector();

	// set the 1st cluster as the current cluster, we'll seek from there
	((uint16*)&msFile.uiCachedClusterInVolume)[INT32_WORD0] = msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_lo;
	((uint16*)&msFile.uiCachedClusterInVolume)[INT32_WORD1] = (mpcVolume->GetFileSystemType() == FAT_FS_TYPE_FAT32) ? msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.first_cluster_hi : 0;

	// if the file occupies more than one cluster
	if (uiSectorIndexInFile > mpcVolume->GetNoOfSectorsPerCluster())
	{
		// calculate the count of clusters occupied by the file and
		// update the ClustersAllocated value of the file
		uiClusterIndex = ((uiSectorIndexInFile + mpcVolume->GetNoOfSectorsPerCluster() - 1) / mpcVolume->GetNoOfSectorsPerCluster()) - 1;

		// set the file file to point to the last cluster. if the file doesn't have
		// that many clusters allocated this function will return 0. if that ever happens it means
		// that the file is corrupted
		uiResult = mpcVolume->FatIncreaseClusterAddress(msFile.uiCachedClusterInVolume, uiClusterIndex, &msFile.uiCachedClusterInVolume);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return FAT_CORRUPTED_FILE;
		}
	}

	// calculate the last sector of the file and set the uiBuffer
	// head to point to the byte after the last one
	if (uiBytePositionInFile)
	{
		msFile.uiFirstCachedSectorIndexInCluster = (((uiBytePositionInFile + mpcVolume->GetNoOfBytesPerSector() - 1) / mpcVolume->GetNoOfBytesPerSector()) - 1) % mpcVolume->GetNoOfSectorsPerCluster();

		if (uiBytePositionInFile % mpcVolume->GetNoOfBytesPerSector() == 0)
		{
			msFile.uiBufferSeekByteIndexInSector = mpcVolume->GetNoOfBytesPerSector();
		}
		else
		{
			msFile.uiBufferSeekByteIndexInSector = uiBytePositionInFile % mpcVolume->GetNoOfBytesPerSector();
		}
	}
	else
	{
		msFile.uiFirstCachedSectorIndexInCluster = 0;
		msFile.uiBufferSeekByteIndexInSector = 0;
	}

	uiSectorAddress = msFile.uiFirstCachedSectorIndexInCluster + mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCachedClusterInVolume);
	msFile.uiCachedClusterIndex = uiClusterIndex;

	// maintain the count of allocated clusters
	if (uiOldClusterIndex < msFile.uiCachedClusterIndex)
	{
		msFile.uiNoOfClustersAfterPos -= (msFile.uiCachedClusterIndex - uiOldClusterIndex);
	}
	else if (uiOldClusterIndex > msFile.uiCachedClusterIndex)
	{
		msFile.uiNoOfClustersAfterPos += (uiOldClusterIndex - msFile.uiCachedClusterIndex);
	}
	msFile.bBusy = 0;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Write(uint8* pvSource, uint32 uiLength)
{
	EFatCode					uiResult;
	SFatWriteOperationState		sOperation;
	uint32						uiBytePosition;

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

	// if there's no clusters allocated to this file allocate
	// enough clusters for this request
	uiResult = FatFileAllocate(uiLength);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// make sure that either a uiBuffer is set or the file has been opened in
	// unbuffered mode.
	if (!mpvBuffer)
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
	uiBytePosition = msFile.uiCachedClusterIndex * mpcVolume->GetNoOfSectorsPerCluster() * mpcVolume->GetNoOfBytesPerSector();
	uiBytePosition += (msFile.uiFirstCachedSectorIndexInCluster) * mpcVolume->GetNoOfBytesPerSector();
	uiBytePosition += msFile.uiBufferSeekByteIndexInSector;

	// copy the length of the buffer to be writen
	// into the counter
	sOperation.uiBytesRemaining = uiLength;

	// calculate the address of the current sector
	sOperation.uiSectorAddress = msFile.uiFirstCachedSectorIndexInCluster + mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCachedClusterInVolume);

	sOperation.pvUserMemory = pvSource;

	return FatFileWriteCallback(&sOperation, uiBytePosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatFileWriteCallback(SFatWriteOperationState* psOperation, uint32 uiBytePosition)
{
	EFatCode	uiResult;
	bool		bSuccess;

	// loop while there are bytes to be read
	while (psOperation->uiBytesRemaining)
	{
		// if we've reached the end of the current
		// sector then we must flush it
		if (msFile.uiBufferSeekByteIndexInSector == mpcVolume->GetNoOfBytesPerSector())
		{
			msFile.uiBufferSeekByteIndexInSector = 0;

			// write the cached sector to media
			bSuccess = mpcVolume->Write(psOperation->uiSectorAddress, mpvBuffer);
			if (!bSuccess)
			{
				msFile.bBusy = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}

			// if this sector is the last of the current cluster then
			// locate the next cluster and continue writing
			if (msFile.uiFirstCachedSectorIndexInCluster == mpcVolume->GetNoOfSectorsPerCluster() - 1)
			{
				uiResult = mpcVolume->GetNextClusterEntry(msFile.uiCachedClusterInVolume, &msFile.uiCachedClusterInVolume);
				if (uiResult != FAT_SUCCESS)
				{
					msFile.bBusy = 0;
					return uiResult;
				}
				// reset the current sector and increase the cluster index
				msFile.uiFirstCachedSectorIndexInCluster = 0x0;
				msFile.uiCachedClusterIndex++;
				msFile.uiNoOfClustersAfterPos--;
				// calculate the sector address
				psOperation->uiSectorAddress = mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCachedClusterInVolume);
			}
			// if there are more sectors in the
			// current cluster then simply increase
			// the current sector counter and address
			else
			{
				msFile.uiFirstCachedSectorIndexInCluster++;
				psOperation->uiSectorAddress++;
			}
		}

		// write the next byte to the file cache
		mpvBuffer[msFile.uiBufferSeekByteIndexInSector] = *psOperation->pvUserMemory++;
		msFile.uiBufferSeekByteIndexInSector++;

		// update the file size only if we're writting past the end of the file.
		uiBytePosition++;
		if (uiBytePosition >= msFile.uiFileSize)
		{
			msFile.uiFileSize++;
		}
		psOperation->uiBytesRemaining--;
	}

	msFile.bBusy = 0;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Read(uint8* pvDestination, uint32 uiLength, uint32* puiBytesRead)
{
	SFatReadOperationState	sOperation;
	uint32					uiBytePosition;

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// make sure that either a buffer is set or the file has been opened in unbuffered mode.
	if (!mpvBuffer)
	{
		return FAT_FILE_BUFFER_NOT_SET;
	}

	// check that another operation is not using the file at this time.
	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	msFile.bBusy = 1;

	// calculate current pos
	uiBytePosition = msFile.uiCachedClusterIndex * mpcVolume->GetNoOfSectorsPerCluster() * mpcVolume->GetNoOfBytesPerSector();
	uiBytePosition += msFile.uiFirstCachedSectorIndexInCluster * mpcVolume->GetNoOfBytesPerSector();
	uiBytePosition += msFile.uiBufferSeekByteIndexInSector;

	// calculate the address of the current sector and the address of the end of the buffer
	sOperation.uiSectorAddress = msFile.uiFirstCachedSectorIndexInCluster + mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCachedClusterInVolume);

	// set the async op context
	sOperation.uiBytesRemaining = uiLength;
	sOperation.pvUserMemory = pvDestination;
	sOperation.puiBytesRead = puiBytesRead;

	return FatFileRead(&sOperation);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatFileRead(SFatReadOperationState* psOperation)
{
	EFatCode	uiResult;
	bool		bSuccess;
	uint32		uiStartReadSector;
	uint32		uiRemaingingSectors;
	uint32		uiEndReadSector;
	uint32		uiLastClusterSector;
	uint32		uiSectorsToRead;
	uint32		uiBytesToRead;
	uint32		uiTotalBytesToRead;



	if (msFile.uiBufferSeekByteIndexInSector == mpcVolume->GetNoOfBytesPerSector())
	{
		msFile.uiBufferSeekByteIndexInSector = 0;

		// if this sector is the last of the current cluster
		// we must find the next cluster
		if (msFile.uiFirstCachedSectorIndexInCluster == mpcVolume->GetNoOfSectorsPerCluster() - 1)
		{
			// update the cluster address with the address of the next cluster
			uiResult = mpcVolume->FatIncreaseClusterAddress(msFile.uiCachedClusterInVolume, 1, &msFile.uiCachedClusterInVolume);
			if (uiResult != FAT_SUCCESS)
			{
				msFile.bBusy = 0;
				return FAT_CORRUPTED_FILE;
			}

			// reset the current sector increase the current cluster
			// number and calculate the address of the first sector of the
			// cluster
			msFile.uiCachedClusterIndex++;
			msFile.uiFirstCachedSectorIndexInCluster = 0x0;
			psOperation->uiSectorAddress = mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCachedClusterInVolume);
		}
		else
		{

			// if there are more sectors in the current cluster then
			// simply increase the current sector counter and address
			msFile.uiFirstCachedSectorIndexInCluster++;
			psOperation->uiSectorAddress++;
		}
	}





	uiTotalBytesToRead = 0;

	for (;;)
	{
		uiStartReadSector = psOperation->uiSectorAddress;

		uiLastClusterSector = mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCachedClusterInVolume) + (mpcVolume->GetNoOfSectorsPerCluster() - 1);

		uiRemaingingSectors = psOperation->uiBytesRemaining / mpcVolume->GetNoOfBytesPerSector();
		if (psOperation->uiBytesRemaining % mpcVolume->GetNoOfBytesPerSector() != 0)
		{
			uiRemaingingSectors++;
		}

		uiEndReadSector = uiStartReadSector + uiRemaingingSectors;
		if (uiEndReadSector > uiLastClusterSector)
		{
			uiEndReadSector = uiLastClusterSector;
		}

		uiSectorsToRead = uiEndReadSector - uiStartReadSector + 1;
		bSuccess = mpcVolume->Read(uiStartReadSector, uiSectorsToRead, mpvBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_READ_MEDIA;
		}

		uiBytesToRead = uiSectorsToRead * mpcVolume->GetNoOfBytesPerSector();
		if (psOperation->uiBytesRemaining < uiBytesToRead)
		{
			uiBytesToRead = psOperation->uiBytesRemaining;
		}

		uiBytesToRead -= msFile.uiBufferSeekByteIndexInSector;

		memcpy(psOperation->pvUserMemory, &mpvBuffer[msFile.uiBufferSeekByteIndexInSector], uiBytesToRead);
		psOperation->pvUserMemory += uiBytesToRead;

		psOperation->uiBytesRemaining -= uiBytesToRead;
		uiTotalBytesToRead += uiBytesToRead;
		if (psOperation->uiBytesRemaining == 0)
		{
			psOperation->uiSectorAddress += uiSectorsToRead;
			msFile.uiBufferSeekByteIndexInSector += uiBytesToRead;
			break;
		}

		// update the cluster address with the address of the next cluster
		uiResult = mpcVolume->FatIncreaseClusterAddress(msFile.uiCachedClusterInVolume, 1, &msFile.uiCachedClusterInVolume);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return FAT_CORRUPTED_FILE;
		}

		// reset the current sector increase the current cluster
		// number and calculate the address of the first sector of the
		// cluster
		msFile.uiCachedClusterIndex++;
		msFile.uiFirstCachedSectorIndexInCluster = 0x0;
		psOperation->uiSectorAddress = mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCachedClusterInVolume);

		msFile.uiBufferSeekByteIndexInSector = 0;
	}

	SafeAssign(psOperation->puiBytesRead, uiTotalBytesToRead);

	msFile.bBusy = 0;
	return FAT_SUCCESS;
}


// flushes file buffers
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatFileFlush(void)
{
	uint32	uiSectorAddress = 0;
	bool	bSuccess;
	uint8* pvBuffer = mpcVolume->GetFatSharedBuffer();

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// make sure that either a buffer has been set or the file was
	// opened in unbuffered mode
	if (!mpvBuffer)
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

		// calculate the address of the current sector
		uiSectorAddress = msFile.uiFirstCachedSectorIndexInCluster + mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCachedClusterInVolume);

		// if the buffer is only partially filled we need to merge it
		// with the one on the drive
		if (msFile.uiBufferSeekByteIndexInSector <= mpcVolume->GetNoOfBytesPerSector())
		{
			uint16 i;
			uint8 buff[MAX_SECTOR_LENGTH];
			bSuccess = mpcVolume->Read(uiSectorAddress, buff);
			if (!bSuccess)
			{
				msFile.bBusy = 0;
				return FAT_CANNOT_READ_MEDIA;
			}

			for (i = msFile.uiBufferSeekByteIndexInSector; i < mpcVolume->GetNoOfBytesPerSector(); i++)
			{
				mpvBuffer[i] = buff[i];
			}
		}

		// write the cached sector to media
		bSuccess = mpcVolume->Write(uiSectorAddress, mpvBuffer);
		if (!bSuccess)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_WRITE_MEDIA;
		}

		// update the file size on the entry
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.size = msFile.uiFileSize;
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.modify_date = GetSystemClockDate();
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.modify_time = GetSystemClockTime();
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.access_date = msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.modify_date;

		// try load the sector that contains the entry
		mpcVolume->SetFatSharedBufferSector(FAT_UNKNOWN_SECTOR);

		bSuccess = mpcVolume->Read(msFile.sDirectoryEntry.uiSectorAddress, pvBuffer);
		if (!bSuccess)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}

		// copy the modified file entry to the
		// sector buffer
		memcpy(pvBuffer + msFile.sDirectoryEntry.uiSectorOffset, &msFile.sDirectoryEntry.raw, sizeof(SFatRawDirectoryEntry));

		// write the modified entry to the media
		bSuccess = mpcVolume->Write(msFile.sDirectoryEntry.uiSectorAddress, pvBuffer);
		if (!bSuccess)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_WRITE_MEDIA;
		}

		// some cards seem not to update the sector correctly if it
		// is the first sector on the page and is the only sector written
		// to so for now until we figure this out we'll write the next
		// sector too TODO: implement this on driver!!
		{
			bSuccess = mpcVolume->Read(msFile.sDirectoryEntry.uiSectorAddress + 1, pvBuffer);
			if (!bSuccess)
			{
				msFile.bBusy = 0;
				return FAT_CANNOT_READ_MEDIA;
			}
			bSuccess = mpcVolume->Write(msFile.sDirectoryEntry.uiSectorAddress + 1, pvBuffer);
			if (!bSuccess)
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
EFatCode CFatFile::Close(void)
{
	EFatCode	uiResult;
	FatEntry	fat_entry;

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		SafeFree(mpvBuffer);
		return FAT_INVALID_HANDLE;
	}

	// flush the file buffers
	uiResult = FatFileFlush();
	if (uiResult != FAT_SUCCESS)
	{
		SafeFree(mpvBuffer);
		return uiResult;
	}

	if (msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		// seek to the end of the file
		uiResult = Seek(msFile.uiFileSize - 1, FAT_SEEK_START);
		if (uiResult != FAT_SUCCESS)
		{
			SafeFree(mpvBuffer);
			return uiResult;
		}

		// check that another operation is not using the
		// file at this time
		if (msFile.bBusy)
		{
			SafeFree(mpvBuffer);
			return FAT_FILE_HANDLE_IN_USE;
		}

		// mark the file as in use
		msFile.bBusy = 1;

		// free unused clusters
		uiResult = mpcVolume->GetNextClusterEntry(msFile.uiCachedClusterInVolume, &fat_entry);
		if (uiResult != FAT_SUCCESS)
		{
			SafeFree(mpvBuffer);
			return uiResult;
		}

		if (!mpcVolume->FatIsEOFEntry(fat_entry))
		{
			uiResult = mpcVolume->FatFreeClusterChain(fat_entry);
			if (uiResult != FAT_SUCCESS)
			{
				SafeFree(mpvBuffer);
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
			uiResult = mpcVolume->FatSetClusterEntry(msFile.uiCachedClusterInVolume, fat_entry);
			if (uiResult != FAT_SUCCESS)
			{
				SafeFree(mpvBuffer);
				return uiResult;
			}
		}
	}

	// invalidate the file file
	msFile.uiMagic = 0;
	SafeFree(mpvBuffer);
	return FAT_SUCCESS;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatFile::GetCurrentSize(void) { return msFile.uiFileSize; }
uint32 CFatFile::GetCurrentClusterAddress(void) { return msFile.uiCachedClusterInVolume; }
uint32 CFatFile::GetCurrentClusterIdx(void) { return msFile.uiCachedClusterIndex; }
uint32 CFatFile::GetCurrentSectorIdx(void) { return msFile.uiFirstCachedSectorIndexInCluster; }
uint32 CFatFile::GetNoOfClustersAfterPos(void) { return msFile.uiNoOfClustersAfterPos; }
uint16 CFatFile::GetNoOfSequentialClusters(void) { return msFile.uiNoOfSequentialClusters; }
bool CFatFile::IsBusy(void) { return msFile.bBusy; }
uint8 CFatFile::GetMagic(void) { return msFile.uiMagic; }
uint8 CFatFile::GetAccessFlags(void) { return msFile.uiAccessFlags; }
uint8* CFatFile::GetBuffer(void) { return mpvBuffer; }

