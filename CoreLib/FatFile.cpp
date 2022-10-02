/*
 * fat32lib - Portable FAT12/16/32 Filesystem Library
 * Copyright (C) 2013 Fernando Rodriguez (frodriguez.developer@outlook.com)
 * 
 * https://github.com/fdev1/fat32lib
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
	eResult = mpcVolume->GetFileEntry(filename, &sFileEntry);
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
			eResult = mpcVolume->GetFileEntry(szFilePath, &sParentEntry);
			RETURN_ON_FAT_FAILURE(eResult);

			// if the parent directory does not exists
			if (*sParentEntry.name == 0)
			{
				return FAT_DIRECTORY_DOES_NOT_EXIST;
			}

			// try to create the directory entry
			eResult = mpcVolume->CreateFATEntry(&sParentEntry.raw, pcFilenameScanner, 0, 0, &sFileEntry);
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
		uint32 uiWhatEven;

		eResult = mpcVolume->CountSequentialClusters(msFile.uiCursorClusterInVolume, &uiWhatEven);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatOpenFileByEntry(SFatDirectoryEntry* psEntry, uint8 uiAccessFlags)
{
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
	msFile.uiCursorClusterInVolume = GetFirstClusterFromFatEntry(&psEntry->raw, IsFat32Volume());

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

		eResult = mpcVolume->Free(psEntry);
		if (eResult != FAT_SUCCESS)
		{
			msFile.uiMagic = 0;
			return eResult;
		}

		msFile.uiFileSize = 0;
		msFile.uiFilePosition = 0;
	}
	else
	{
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
	return GetFirstClusterFromFatEntry(&msFile.sDirectoryEntry.raw, IsFat32Volume());
}


// pre-allocates disk space for a file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::FatFileAllocate(uint32 uiBytes)
{
	EFatCode	eResult;
	fatEntry	uiNewClusterInVolume;
	uint32		uiClustersNeeded;
	uint32		uiClustersInFile;
	fatEntry	uiCursorClusterInVolume;

	// check that this is a valid file
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}
	msFile.bBusy = true;

	// calculate the number of clusters currently allocated to this file.
	uiClustersInFile = (msFile.uiFileSize + mpcVolume->GetClusterSize() - 1) / mpcVolume->GetClusterSize();

	// calculate how many clusters we need to grow the file by.
	uiClustersNeeded = (uiBytes + mpcVolume->GetClusterSize() - 1) / mpcVolume->GetClusterSize();

	if (uiClustersNeeded > uiClustersInFile)
	{
		uiClustersNeeded = uiClustersNeeded - uiClustersInFile;
	}
	else
	{
		msFile.bBusy = false;
		return FAT_SUCCESS;
	}

	uiCursorClusterInVolume = CalculateFirstCluster();
	eResult = mpcVolume->Allocate(uiClustersNeeded, uiCursorClusterInVolume, &uiNewClusterInVolume);
	if (eResult != FAT_SUCCESS)
	{
		msFile.bBusy = false;
		return eResult;
	}

	eResult = mpcVolume->UpdateDirectoryEntry(&msFile.sDirectoryEntry, uiCursorClusterInVolume, uiNewClusterInVolume, uiClustersInFile);
	if (eResult != FAT_SUCCESS)
	{
		msFile.bBusy = false;
		return eResult;
	}

	// update the file to point to the new cluster
	if (msFile.uiCursorClusterInVolume == 0)
	{
		msFile.uiCursorClusterInVolume = uiNewClusterInVolume;
	}

	// update the count of sequential clusters
	uint32	uiWhatEven;
	mpcVolume->CountSequentialClusters(msFile.uiCursorClusterInVolume, &uiWhatEven);

	msFile.bBusy = 0;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatFile::CalculateFirstCluster(void)
{
	return GetFirstClusterFromFatEntry(&msFile.sDirectoryEntry.raw, IsFat32Volume());
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
	if (uiSectorIndexInFile > mpcVolume->NumSectorsPerCluster())
	{
		// calculate the count of clusters occupied by the file and
		// update the ClustersAllocated value of the file
		uiClusterIndex = ((uiSectorIndexInFile + mpcVolume->NumSectorsPerCluster() - 1) / mpcVolume->NumSectorsPerCluster()) - 1;

		// set the file file to point to the last cluster. if the file doesn't have
		// that many clusters allocated this function will return 0. if that ever happens it means
		// that the file is corrupted
		eResult = mpcVolume->IncreaseClusterAddress(msFile.uiCursorClusterInVolume, uiClusterIndex, &msFile.uiCursorClusterInVolume);
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

	eResult = FatFileWrite(uiLength, pvSource);
	return eResult;
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

		uiResult = mpcVolume->GetNextClusterEntry(msFile.uiCursorClusterInVolume, &msFile.uiCursorClusterInVolume);
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

		uiResult = mpcVolume->GetNextClusterEntry(msFile.uiCursorClusterInVolume, &msFile.uiCursorClusterInVolume);
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
		uint8*	pvBuffer;
		pvBuffer = mpcVolume->ReadInfoSector(msFile.sDirectoryEntry.uiSectorAddress);
		if (pvBuffer == NULL)
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}

		// copy the modified file entry to the sector buffer
		memcpy(pvBuffer + msFile.sDirectoryEntry.uiSectorOffset, &msFile.sDirectoryEntry.raw, sizeof(SFatRawDirectoryEntry));
		mpcVolume->SetInfoSectorDirty(pvBuffer);

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
		eResult = mpcVolume->GetNextClusterEntry(msFile.uiCursorClusterInVolume, &uiFatEntry);
		if (eResult != FAT_SUCCESS)
		{
			mcCache.Kill();
			return eResult;
		}

		if (!mpcVolume->FatIsEOFEntry(uiFatEntry))
		{
			eResult = mpcVolume->FreeChain(uiFatEntry);
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
			eResult = mpcVolume->SetClusterEntry(msFile.uiCursorClusterInVolume, uiFatEntry);
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
uint32 CFatFile::GetCurrentSize(void) { return msFile.uiFileSize; }
uint32 CFatFile::GetCurrentClusterAddress(void) { return msFile.uiCursorClusterInVolume; }
bool CFatFile::IsBusy(void) { return msFile.bBusy; }
uint8 CFatFile::GetMagic(void) { return msFile.uiMagic; }
uint8 CFatFile::GetAccessFlags(void) { return msFile.uiAccessFlags; }

