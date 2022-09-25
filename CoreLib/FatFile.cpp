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
#include "BaseLib/Logger.h"
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatFileUpdateSequentialClusterCount(uint32 uiCursorClusterInVolume)
{
	uint32		uiNextCluster;
	EFatCode	eResult;
	uint16		uiSequentialClustersAfterCursor;

	// find out how many clusters are allocated sequentially to this file following the current cursor location
	uiSequentialClustersAfterCursor = 0;

	while (!mpcVolume->FatIsEOFEntry(uiCursorClusterInVolume))
	{
		eResult = GetNextClusterEntry(__METHOD__, uiCursorClusterInVolume, &uiNextCluster);
		RETURN_ON_FAT_FAILURE(eResult);

		if (uiNextCluster == (uiCursorClusterInVolume + 1))
		{
			uiSequentialClustersAfterCursor++;
			uiCursorClusterInVolume = uiNextCluster;
			if (uiSequentialClustersAfterCursor == 0xFFFF)
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
EFatCode CFatFile::Open(char* filename, uint8 uiAccessFlags)
{
	EFatCode			eResult;
	SFatDirectoryEntry	sFileEntry;

	memset(mszName, 0, FAT_MAX_FILENAME);
	mcCache.Init(mpcVolume->GetFileDrive(), mpcVolume->GetClusterSize(), mpcVolume->GetSectorSize());

	if (filename == 0 || strlen(filename) > FAT_MAX_PATH)
	{
		return FAT_INVALID_FILENAME;
	}

	// get the file entry
	eResult = FatGetFileEntry(__METHOD__, filename, &sFileEntry);
	RETURN_ON_FAT_FAILURE(eResult);

	// if the entry was not found...
	if (*sFileEntry.name == 0)
	{
		// if the create access flag was set then
		// we create the file
		if (uiAccessFlags & FAT_FILE_ACCESS_CREATE)
		{
			// allocate memory for the file path
			ptrdiff_t			iPathLength;
			char*				pcFilenameScanner;
			char				szFilePath[FAT_MAX_PATH + 1];
			SFatDirectoryEntry	sParentEntry;

			// get the name of the file path including
			// the filename
			iPathLength = strlen(filename);

			// set the pointer that will be used to scan
			// filename to the end of the filename
			pcFilenameScanner = filename + (iPathLength - 0x1);

			// if the filename ends with a backslash then it
			// is an invalid filename ( it's actually a directory
			// path )
			if (*pcFilenameScanner == BACKSLASH)
			{
				return FAT_INVALID_FILENAME;
			}

			// scan the filename starting at the end until
			// a backslash is found - when we exit this loop
			// path_scanner will point to the last character
			// of the filepath
			for (;;)
			{
				if (*pcFilenameScanner == BACKSLASH)
				{
					break;
				}

				if (pcFilenameScanner == filename)
				{
					return FAT_INVALID_PATH;
				}
				pcFilenameScanner--;
			}

			// calculate the length of the path part of the
			// filename
			iPathLength = pcFilenameScanner - filename;

			// copy the path part of the filename to
			// the szFilePath uiBuffer
			memcpy(szFilePath, filename, iPathLength);

			// set the null terminator of the szFilePath uiBuffer
			szFilePath[iPathLength] = 0x0;

			// increase pointer to the beggining of the filename
			// part of the path
			pcFilenameScanner++;

			// try to get the entry for the parent directory
			eResult = FatGetFileEntry(__METHOD__, szFilePath, &sParentEntry);
			RETURN_ON_FAT_FAILURE(eResult);

			// if the parent directory does not exists
			if (*sParentEntry.name == 0)
			{
				return FAT_DIRECTORY_DOES_NOT_EXIST;
			}

			// try to create the directory entry
			eResult = FatCreateDirectoryEntry(__METHOD__,  &sParentEntry.raw, pcFilenameScanner, &sFileEntry);
			RETURN_ON_FAT_FAILURE(eResult);


			// make sure the file is opened with no append flags
			// todo: figure out why we need this and fix it
			uiAccessFlags = uiAccessFlags & (0xFF ^ FAT_FILE_ACCESS_APPEND);


		}
		// if the create flag is not set then return the
		// file not found error code
		else
		{
			return FAT_FILE_NOT_FOUND;
		}
	}

	strcpy(mszName, (char*)sFileEntry.name);

	eResult = FatOpenFileByEntry(&sFileEntry, uiAccessFlags);
	RETURN_ON_FAT_FAILURE(eResult);

	// if the file has no clusters allocated then allocate one
	if (msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		eResult = FatFileUpdateSequentialClusterCount(msFile.uiCursorClusterInVolume);
		return eResult;
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
	mcCache.Init(mpcVolume->GetFileDrive(), mpcVolume->GetClusterSize(), mpcVolume->GetSectorSize());

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
	EFatCode	eResult;

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
	msFile.uiAccessFlags = uiAccessFlags;
	msFile.uiMagic = FAT_OPEN_HANDLE_MAGIC;
	msFile.bBusy = 0;

	// read the the cluster number
	msFile.uiCursorClusterInVolume = GetFatClusterFromFatEntry(&psEntry->raw, IsFat32Volume());

#ifdef __LOG_FAT_VOLUME_CALLS__
	gcLogger.Info2(__METHOD__, " File [", mszName, "] first cluster [", IntToString(msFile.uiCursorClusterInVolume), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__

	if (uiAccessFlags & FAT_FILE_ACCESS_APPEND)
	{
		// if the file is being opened for append access we
		// seek to the end of the file
		eResult = Seek(0, FAT_SEEK_END);
		if (eResult != FAT_SUCCESS)
		{
			msFile.uiMagic = 0;
			return eResult;
		}
	}
	else if (uiAccessFlags & FAT_FILE_ACCESS_OVERWRITE)
	{
		// if the file is being opened with the OVERWRITE flag we must free all the clusters
		// currently allocated to the file and update it's directory entry to point to cluster 1

		// if the file is not already empty then
		// we'll empty it
		if (psEntry->raw.uEntry.sFatRawCommon.uiFirstClusterLowWord != 0x0 || psEntry->raw.uEntry.sFatRawCommon.uiFirstClusterHighWord != 0x0)
		{
			// update the entry to point to cluster 0
			psEntry->raw.uEntry.sFatRawCommon.uiFirstClusterLowWord = 0x0;
			psEntry->raw.uEntry.sFatRawCommon.uiFirstClusterHighWord = 0x0;
			psEntry->raw.uEntry.sFatRawCommon.uiSize = 0x0;
			msFile.sDirectoryEntry = *psEntry;
			msFile.uiFileSize = psEntry->size;

			// read the sector that contains the entry
			eResult = ReadFatSector(__METHOD__, psEntry->uiSectorAddress);
			if (eResult != FAT_SUCCESS)
			{
				msFile.uiMagic = 0;
				return eResult;
			}

			uint8* uiBuffer = mpcVolume->GetFatSharedBuffer();
			memcpy(uiBuffer + psEntry->uiSectorOffset, &psEntry->raw, sizeof(SFatRawDirectoryEntry));

			// write the modified entry to the media
			bSuccess = mpcVolume->Write(psEntry->uiSectorAddress, uiBuffer);

#ifdef __LOG_FAT_VOLUME_CALLS__
			gcLogger.Info2(__METHOD__, " File [", mszName, "] write sector [", IntToString(psEntry->uiSectorAddress), "] returned [", BoolToString(bSuccess), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__

			if (!bSuccess)
			{
				msFile.uiMagic = 0;
				return FAT_CANNOT_WRITE_MEDIA;
			}

			// free the clusters allocated to the file
			mpcVolume->FatFreeClusterChain(msFile.uiCursorClusterInVolume);
		}

		// reset the file file to reflect an
		// empty file
		msFile.uiFileSize = 0;
		msFile.uiFilePosition = 0;
	}
	else
	{
		// if no append or overwrite flags are set then
		// simply set the file to point to the beggining
		// of the 1st cluster
		msFile.uiFilePosition = 0;
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
	return GetFatClusterFromFatEntry(&msFile.sDirectoryEntry.raw, IsFat32Volume());
}


// pre-allocates disk space for a file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatFileAllocate(uint32 uiBytes)
{
	EFatCode	eResult;
	uint32		uiNewCluster;
	uint32		uiClustersNeeded;
	uint32		uiClustersInFile;
	bool		bSuccess;

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}
	msFile.bBusy = 1;

	// calculate the number of clusters currently allocated
	uiClustersInFile = (msFile.uiFileSize + mpcVolume->GetSectorSize() - 1) / mpcVolume->GetSectorSize();
	uiClustersInFile = (uiClustersInFile + mpcVolume->GetNoOfSectorsPerCluster() - 1) / mpcVolume->GetNoOfSectorsPerCluster();

	// calculate how many clusters we need
	uiClustersNeeded = (uiBytes + mpcVolume->GetSectorSize() - 1) / mpcVolume->GetSectorSize();
	uiClustersNeeded = (uiClustersNeeded + mpcVolume->GetNoOfSectorsPerCluster() - 1) / mpcVolume->GetNoOfSectorsPerCluster();

	if (uiClustersNeeded > uiClustersInFile)
	{
		uiClustersNeeded = uiClustersNeeded - uiClustersInFile;
	}
	else
	{
		msFile.bBusy = 0;
		return FAT_SUCCESS;
	}

	// allocate a new cluster
	uint32 uiCursorClusterInVolume;
	uint32 uiNextCluster;
	uint32 uiPageSize;
	uint32 uiStartSector;
	uint32 uiEndSector;

	uiPageSize = mpcVolume->GetPageSize();

	if (uiPageSize > mpcVolume->GetNoOfSectorsPerCluster())
	{
		uint32 clusters_per_page = uiPageSize / mpcVolume->GetNoOfSectorsPerCluster();

		if (uiClustersNeeded % clusters_per_page)
		{
			uiClustersNeeded += clusters_per_page - (uiClustersNeeded % clusters_per_page);
		}

		if ((uiClustersNeeded % clusters_per_page) != 0)
		{
			return FAT_UNKNOWN_ERROR;
		}

		eResult = FAT_UNKNOWN_ERROR;
		uiNewCluster = mpcVolume->FatAllocateDataClusterEx(uiClustersNeeded, 0, uiPageSize, &eResult);
		if (eResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return eResult;
		}
	}
	else
	{
		eResult = FAT_UNKNOWN_ERROR;
		uiNewCluster = mpcVolume->FatAllocateDataCluster(uiClustersNeeded, 1, &eResult);
		if (eResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return eResult;
		}
	}

	// find out how many clusters are allocated sequentially
	// to this file following the current cursor location
	uiCursorClusterInVolume = CalculateFirstCluster();

	while (!mpcVolume->FatIsEOFEntry(uiCursorClusterInVolume))
	{
		// calculate the start and end address the cluster
		uiStartSector = mpcVolume->CalculateFirstSectorOfCluster(uiCursorClusterInVolume);
		uiEndSector = uiStartSector + mpcVolume->GetNoOfSectorsPerCluster();

		// find the last sequential sector after this address
		while (!mpcVolume->FatIsEOFEntry(uiCursorClusterInVolume))
		{
			eResult = GetNextClusterEntry(__METHOD__, uiCursorClusterInVolume, &uiNextCluster);
			if (eResult != FAT_SUCCESS)
			{
				msFile.bBusy = 0;
				return eResult;
			}

			if (uiNextCluster == (uiCursorClusterInVolume + 1))
			{
				uiEndSector += mpcVolume->GetNoOfSectorsPerCluster();
				uiCursorClusterInVolume = uiNextCluster;
			}
			else
			{
				uiCursorClusterInVolume = uiNextCluster;
				break;
			}
		}

		// pre-erase the clusters
		mpcVolume->Erase(uiStartSector, uiEndSector - 1);
	}

	// if this is the 1st cluster cluster allocated to the file then we must modify the file's entry
	if (!msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.uiFirstClusterLowWord && !msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.uiFirstClusterHighWord)
	{
		// modify the file entry to point to the  new cluster
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.uiFirstClusterLowWord = LO16(uiNewCluster);
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.uiFirstClusterHighWord = HI16(uiNewCluster);

		// mark the cached sector as unknown
		mpcVolume->SetFatSharedBufferSector(FAT_UNKNOWN_SECTOR);

		// try load the sector that contains the entry
		eResult = ReadFatSector(__METHOD__, msFile.sDirectoryEntry.uiSectorAddress);
		if (eResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return eResult;
		}

		// copy the modified file entry to the sector buffer
		uint8* pvBuffer = mpcVolume->GetFatSharedBuffer();
		memcpy(pvBuffer + msFile.sDirectoryEntry.uiSectorOffset, &msFile.sDirectoryEntry.raw, sizeof(SFatRawDirectoryEntry));

		// write the modified entry to the media
		bSuccess = mpcVolume->Write(msFile.sDirectoryEntry.uiSectorAddress, pvBuffer);

#ifdef __LOG_FAT_VOLUME_CALLS__
		gcLogger.Info2(__METHOD__, " File [", mszName, "] write sector [", IntToString(msFile.sDirectoryEntry.uiSectorAddress), "] returned [", BoolToString(bSuccess), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__

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

		if (uiClustersInFile != 0)
		{
			eResult = FatIncreaseClusterAddress(__METHOD__, msFile.uiCursorClusterInVolume, uiClustersInFile, &last_cluster);
			if (eResult != FAT_SUCCESS)
			{
				msFile.bBusy = 0;
				return FAT_CORRUPTED_FILE;
			}
		}
		else
		{
			last_cluster = msFile.uiCursorClusterInVolume;
		}

		// set the FAT entry for the last cluster to the beggining of the newly allocated cluster chain (ie. link them).
		eResult = FatSetClusterEntry(__METHOD__, last_cluster, (fatEntry)uiNewCluster);
		if (eResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return eResult;
		}
	}

	// update the file to point to the new cluster
	if (msFile.uiCursorClusterInVolume == 0)
	{
		msFile.uiCursorClusterInVolume = uiNewCluster;
	}

	// update the count of sequential clusters
	FatFileUpdateSequentialClusterCount(msFile.uiCursorClusterInVolume);

	msFile.bBusy = 0;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatFile::CalculateFirstCluster(void)
{
	return GetFatClusterFromFatEntry(&msFile.sDirectoryEntry.raw, IsFat32Volume());
}


// moves the file cursor to the specified offset
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Seek(uint32 offset, EFatSeek mode)
{
	uint32	uiBytePositionInFile;
	uint32	uiSectorIndexInFile;
	uint32	uiClusterIndex;
	uint32	uiOldClusterIndex;
	uint16	eResult;

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
		uiBytePositionInFile = msFile.uiFilePosition;
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

	uiOldClusterIndex = msFile.uiFilePosition / mpcVolume->GetClusterSize();
	uiClusterIndex = 0;

	// calculate the count of sectors being used by the file up to the desired position
	uiSectorIndexInFile = (uiBytePositionInFile + mpcVolume->GetSectorSize() - 1) / mpcVolume->GetSectorSize();

	// set the 1st cluster as the current cluster, we'll seek from there
	msFile.uiCursorClusterInVolume = CalculateFirstCluster();

	// if the file occupies more than one cluster
	if (uiSectorIndexInFile > mpcVolume->GetNoOfSectorsPerCluster())
	{
		// calculate the count of clusters occupied by the file and
		// update the ClustersAllocated value of the file
		uiClusterIndex = ((uiSectorIndexInFile + mpcVolume->GetNoOfSectorsPerCluster() - 1) / mpcVolume->GetNoOfSectorsPerCluster()) - 1;

		// set the file file to point to the last cluster. if the file doesn't have
		// that many clusters allocated this function will return 0. if that ever happens it means
		// that the file is corrupted
		eResult = FatIncreaseClusterAddress(__METHOD__, msFile.uiCursorClusterInVolume, uiClusterIndex, &msFile.uiCursorClusterInVolume);
		if (eResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return FAT_CORRUPTED_FILE;
		}
	}

	msFile.uiFilePosition = uiBytePositionInFile;

	msFile.bBusy = 0;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Write(uint8* pvSource, uint32 uiLength)
{
	EFatCode	eResult;
	uint32		uiOverFileLength;

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
	// You need to allocate the difference between the end of the file and the lengh.  It could be zero.

	if ((msFile.uiFilePosition + uiLength) > msFile.uiFileSize)
	{
		uiOverFileLength = (msFile.uiFilePosition + uiLength);
		eResult = FatFileAllocate(uiOverFileLength);
		RETURN_ON_FAT_FAILURE(eResult);
	}


	// check that another operation is not using the
	// file at this time
	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	msFile.bBusy = 1;

	return FatFileWrite(uiLength, pvSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatFileWrite(uint32 uiBytesRemaining, uint8* puiSource)
{
	EFatCode	uiResult;
	bool		bSuccess;
	uint32		uiFirstSectorOfCluster;
	uint32		uiClustersInFile;
	uint32		uiWriteClusterIndex;
	uint16		uiWriteOffsetInCluster;
	uint16		uiPreviousMaximumOffset;
	uint32		uiBytesWritten;

	for (;;)
	{
		uiFirstSectorOfCluster = mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCursorClusterInVolume);
		uiClustersInFile = msFile.uiFileSize / mpcVolume->GetClusterSize();
		uiWriteClusterIndex = msFile.uiFilePosition / mpcVolume->GetClusterSize();
		uiWriteOffsetInCluster = msFile.uiFilePosition % mpcVolume->GetClusterSize();
		if (uiWriteClusterIndex < uiClustersInFile)
		{
			uiPreviousMaximumOffset = mpcVolume->GetClusterSize();
		}
		else
		{
			uiPreviousMaximumOffset = msFile.uiFileSize % mpcVolume->GetClusterSize();
		}

		uiBytesWritten = uiBytesRemaining;
		bSuccess = mcCache.Write(puiSource, msFile.uiCursorClusterInVolume, uiFirstSectorOfCluster, uiWriteOffsetInCluster, &uiBytesRemaining, uiPreviousMaximumOffset);
		uiBytesWritten -= uiBytesRemaining;

		puiSource += uiBytesWritten;
		msFile.uiFilePosition += uiBytesWritten;
		if (msFile.uiFilePosition > msFile.uiFileSize)
		{
			msFile.uiFileSize = msFile.uiFilePosition;
		}

		if (!bSuccess)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_WRITE_MEDIA;
		}

		if (uiBytesRemaining == 0)
		{
			msFile.bBusy = 0;
			return FAT_SUCCESS;
		}

		uiResult = GetNextClusterEntry(__METHOD__, msFile.uiCursorClusterInVolume, &msFile.uiCursorClusterInVolume);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return uiResult;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Read(uint8* puiDestination, uint32 uiLength, uint32* puiBytesRead)
{
	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	// check that another operation is not using the file at this time.
	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	msFile.bBusy = 1;

	return FatFileRead(uiLength, puiBytesRead, puiDestination);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatFileRead(uint32 uiBytesRemaining, uint32* puiBytesRead, uint8* puiDestination)
{
	EFatCode	uiResult;
	bool		bSuccess;
	uint32		uiFirstSector;
	uint32		uiClustersInFile;
	uint32		uiReadClusterIndex;
	uint16		uiReadOffsetInCluster;
	uint16		uiPreviousMaximumOffset;
	uint32		uiBytesRead;
	uint32		uiTotalBytesRead;

	uiTotalBytesRead = 0;
	for (;;)
	{
		uiFirstSector = mpcVolume->CalculateFirstSectorOfCluster(msFile.uiCursorClusterInVolume);
		uiClustersInFile = msFile.uiFileSize / mpcVolume->GetClusterSize();
		uiReadClusterIndex = msFile.uiFilePosition / mpcVolume->GetClusterSize();
		uiReadOffsetInCluster = msFile.uiFilePosition % mpcVolume->GetClusterSize();
		if (uiReadClusterIndex < uiClustersInFile)
		{
			uiPreviousMaximumOffset = mpcVolume->GetClusterSize();
		}
		else
		{
			uiPreviousMaximumOffset = msFile.uiFileSize % mpcVolume->GetClusterSize();
		}

		uiBytesRead = uiBytesRemaining;
		bSuccess = mcCache.Read(puiDestination, msFile.uiCursorClusterInVolume, uiFirstSector, uiReadOffsetInCluster, &uiBytesRemaining, uiPreviousMaximumOffset);
		uiBytesRead -= uiBytesRemaining;

		uiTotalBytesRead += uiBytesRead;
		puiDestination += uiBytesRead;
		msFile.uiFilePosition += uiBytesRead;
		if (msFile.uiFilePosition >= msFile.uiFileSize)
		{
			 msFile.uiFilePosition = msFile.uiFileSize;
			 uiBytesRemaining = 0;
		}

		if (!bSuccess)
		{
			msFile.bBusy = 0;
			SafeAssign(puiBytesRead, uiTotalBytesRead);
			return FAT_CANNOT_READ_MEDIA;
		}

		if (uiBytesRemaining == 0)
		{
			msFile.bBusy = 0;
			SafeAssign(puiBytesRead, uiTotalBytesRead);
			return FAT_SUCCESS;
		}

		uiResult = GetNextClusterEntry(__METHOD__, msFile.uiCursorClusterInVolume, &msFile.uiCursorClusterInVolume);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			SafeAssign(puiBytesRead, uiTotalBytesRead);
			return uiResult;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatFile::IsFat32Volume(void)
{
	return mpcVolume->GetFileSystemType() == FAT_FS_TYPE_FAT32;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatFileFlush(void)
{
	uint32		uiSectorAddress = 0;
	bool		bSuccess;
	EFatCode	eResult;

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
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

		mcCache.Flush();

		// update the file size on the entry
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.uiSize = msFile.uiFileSize;
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.uiModifyDate = GetSystemClockDate();
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.uiModifyTime = GetSystemClockTime();
		msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.uiAccessDate = msFile.sDirectoryEntry.raw.uEntry.sFatRawCommon.uiModifyDate;

		// try load the sector that contains the entry
		mpcVolume->SetFatSharedBufferSector(FAT_UNKNOWN_SECTOR);

		eResult = ReadFatSector(__METHOD__, msFile.sDirectoryEntry.uiSectorAddress);
		if (eResult != FAT_SUCCESS)
		{
			msFile.bBusy = 0;
			return eResult;
		}

		// copy the modified file entry to the sector buffer
		uint8* pvBuffer = mpcVolume->GetFatSharedBuffer();
		memcpy(pvBuffer + msFile.sDirectoryEntry.uiSectorOffset, &msFile.sDirectoryEntry.raw, sizeof(SFatRawDirectoryEntry));

		// write the modified entry to the media
		bSuccess = mpcVolume->Write(msFile.sDirectoryEntry.uiSectorAddress, pvBuffer);

#ifdef __LOG_FAT_VOLUME_CALLS__
		gcLogger.Info2(__METHOD__, " File [", mszName, "] write sector [", IntToString(msFile.sDirectoryEntry.uiSectorAddress), "] returned [", BoolToString(bSuccess), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__

		if (!bSuccess)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_WRITE_MEDIA;
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
	EFatCode	eResult;
	fatEntry	uiFatEntry;

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		mcCache.Kill();
		return FAT_INVALID_HANDLE;
	}

	// flush the file buffers
	eResult = FatFileFlush();
	if (eResult != FAT_SUCCESS)
	{
		mcCache.Kill();
		return eResult;
	}

	if (msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		// seek to the end of the file
		if (msFile.uiFileSize > 0)
		{
			eResult = Seek(msFile.uiFileSize - 1, FAT_SEEK_START);
		}
		else
		{
			eResult = Seek(0, FAT_SEEK_START);
		}

		if (eResult != FAT_SUCCESS)
		{
			mcCache.Kill();
			return eResult;
		}

		// check that another operation is not using the
		// file at this time
		if (msFile.bBusy)
		{
			mcCache.Kill();
			return FAT_FILE_HANDLE_IN_USE;
		}

		// mark the file as in use
		msFile.bBusy = 1;

		// free unused clusters
		eResult = GetNextClusterEntry(__METHOD__, msFile.uiCursorClusterInVolume, &uiFatEntry);
		if (eResult != FAT_SUCCESS)
		{
			mcCache.Kill();
			return eResult;
		}

		if (!mpcVolume->FatIsEOFEntry(uiFatEntry))
		{
			eResult = mpcVolume->FatFreeClusterChain(uiFatEntry);
			if (eResult != FAT_SUCCESS)
			{
				mcCache.Kill();
				return eResult;
			}

			switch (mpcVolume->GetFileSystemType())
			{
			case FAT_FS_TYPE_FAT12:
				uiFatEntry = FAT12_EOC;
				break;

			case FAT_FS_TYPE_FAT16:
				uiFatEntry = FAT16_EOC;
				break;

			case FAT_FS_TYPE_FAT32:
				uiFatEntry = FAT32_EOC;
				break;
			}
			eResult = FatSetClusterEntry(__METHOD__, msFile.uiCursorClusterInVolume, uiFatEntry);
			if (eResult != FAT_SUCCESS)
			{
				mcCache.Kill();
				return eResult;
			}
		}
	}

	mcCache.Flush();

	// invalidate the file file
	msFile.uiMagic = 0;
	mcCache.Kill();
	return FAT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFatFile::GetShortFileName(void)
{
	if (mszName[0] == '\0')
	{
		return "<NOT YET SET>";
	}
	else
	{
		return mszName;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatIncreaseClusterAddress(char* szMethod, uint32 uiCluster, uint16 uiCount, uint32* puiNewCluster)
{
	EFatCode eResult;

	eResult = mpcVolume->FatIncreaseClusterAddress(uiCluster, uiCount, puiNewCluster);

#ifdef __LOG_FAT_VOLUME_CALLS__
	gcLogger.Info2(__METHOD__, " Filename [", GetShortFileName(), "] increase cluster address for [", IntToString(uiCluster), "] and count [", IntToString(uiCount), "] increased to [", IntToString(*puiNewCluster), "] and returned [", FatCodeToString(eResult), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__

	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::GetNextClusterEntry(char* szMethod, uint32 uiCurrentCluster, uint32* puiNextCluster)
{
	EFatCode eResult;

	eResult = mpcVolume->GetNextClusterEntry(uiCurrentCluster, puiNextCluster);

#ifdef __LOG_FAT_VOLUME_CALLS__
	gcLogger.Info2(__METHOD__, " File [", GetShortFileName(), "] get next cluster entry for cluster [", IntToString(uiCurrentCluster), "] found cluster [", IntToString(*puiNextCluster), "] and returned code [", FatCodeToString(eResult), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__

	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatGetFileEntry(char* szMethod, char* szPath, SFatDirectoryEntry* psEntry)
{
	EFatCode eResult;

	eResult = mpcVolume->FatGetFileEntry(szPath, psEntry);

#ifdef __LOG_FAT_VOLUME_CALLS__
	gcLogger.Info2(__METHOD__, " File [", GetShortFileName(), "] get file entry for path [", StringToString(szPath), "] found [", FatEntryToString(psEntry, IsFat32Volume()), "] returned code[", FatCodeToString(eResult), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__

	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatSetClusterEntry(char* szMethod, uint32 uiCluster, fatEntry uiFatEntry)
{
	EFatCode eResult;

	eResult = mpcVolume->FatSetClusterEntry(msFile.uiCursorClusterInVolume, uiFatEntry);

#ifdef __LOG_FAT_VOLUME_CALLS__
	gcLogger.Info2(__METHOD__, " File [", GetShortFileName(), "] set cluster [", IntToString(uiCluster), "] FAT entry [", IntToString(uiFatEntry), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__

	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::ReadFatSector(char* szMethod, uint32 uiFatSector)
{
	EFatCode	eResult;

	eResult = mpcVolume->FatReadFatSector(uiFatSector);

#ifdef __LOG_FAT_VOLUME_CALLS__
	gcLogger.Info2(__METHOD__, " File [", mszName, "] read FAT sector [", IntToString(uiFatSector), "] returned [", FatCodeToString(eResult), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__
		
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FlushFatSector(char* szMethod)
{
	EFatCode	eResult;

	eResult = mpcVolume->FatFlushFatSector();

#ifdef __LOG_FAT_VOLUME_CALLS__
	gcLogger.Info2(__METHOD__, " File [", mszName, "] flsuh FAT sector returned [", FatCodeToString(eResult), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__

	return eResult;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatCreateDirectoryEntry(char* szMethod, SFatRawDirectoryEntry* psParentEntry, char* szName, SFatDirectoryEntry* psDirectoryEntry)
{
	EFatCode	eResult;

	eResult = mpcVolume->FatCreateDirectoryEntry(psParentEntry, szName, 0, 0, psDirectoryEntry);

#ifdef __LOG_FAT_VOLUME_CALLS__
	gcLogger.Info2(__METHOD__, " File [", GetShortFileName(), "] create directory [", pcFilenameScanner, "] for parenty entry [", FatEntryToString(&sParentEntry, IsFat32Volume()), "] created [", FatEntryToString(&sFileEntry, IsFat32Volume()), "] and returned code [", FatCodeToString(eResult), "].", NULL);
#endif // __LOG_FAT_VOLUME_CALLS__

	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatFile::GetCurrentSize(void) { return msFile.uiFileSize; }
uint32 CFatFile::GetCurrentClusterAddress(void) { return msFile.uiCursorClusterInVolume; }
bool CFatFile::IsBusy(void) { return msFile.bBusy; }
uint8 CFatFile::GetMagic(void) { return msFile.uiMagic; }
uint8 CFatFile::GetAccessFlags(void) { return msFile.uiAccessFlags; }

