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
EFatCode FindBackslash(char* filename, char** ppcFilenameScanner)
{
	ptrdiff_t			iPathLength;
	char*				pcFilenameScanner;

	iPathLength = strlen(filename);

	pcFilenameScanner = filename + (iPathLength - 1);

	if (*pcFilenameScanner == BACKSLASH)
	{
		return FAT_INVALID_FILENAME;
	}

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

	*ppcFilenameScanner = pcFilenameScanner;
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
	eResult = mpcVolume->GetFileEntry(filename, &sFileEntry);
	RETURN_ON_FAT_FAILURE(eResult)

	if (sFileEntry.name[0] == '\0')
	{
		if (uiAccessFlags & FAT_FILE_ACCESS_CREATE)
		{
			ptrdiff_t			iPathLength;
			char*				pcFilenameScanner;
			char				szFilePath[FAT_MAX_PATH + 1];
			SFatDirectoryEntry	sParentEntry;

			eResult = FindBackslash(filename, &pcFilenameScanner);
			RETURN_ON_FAT_FAILURE(eResult)

			iPathLength = pcFilenameScanner - filename;

			// copy the path part of the filename to
			// the szFilePath uiBuffer
			memcpy(szFilePath, filename, iPathLength);

			// set the null terminator of the szFilePath uiBuffer
			szFilePath[iPathLength] = '\0';

			// increase pointer to the beggining of the filename
			// part of the path
			pcFilenameScanner++;

			// try to get the entry for the parent directory
			eResult = mpcVolume->GetFileEntry(szFilePath, &sParentEntry);
			RETURN_ON_FAT_FAILURE(eResult)

			// if the parent directory does not exists
			if (*sParentEntry.name == 0)
			{
				return FAT_DIRECTORY_DOES_NOT_EXIST;
			}

			// try to create the directory entry
			eResult = mpcVolume->CreateFATEntry(&sParentEntry.raw, pcFilenameScanner, FAT_ATTR_ARCHIVE, 0, &sFileEntry);
			RETURN_ON_FAT_FAILURE(eResult)

			// make sure the file is opened with no append flags
			// todo: figure out why we need this and fix it
			uiAccessFlags = uiAccessFlags & (0xFF ^ FAT_FILE_ACCESS_APPEND);
			
			msFile.uiFileSize = 0;
			msFile.uiAccessFlags = uiAccessFlags;
			msFile.uiMagic = FAT_OPEN_HANDLE_MAGIC;
			msFile.bBusy = 0;

			RETURN_ON_FAT_FAILURE(eResult)
		}
		else
		{
			return FAT_FILE_NOT_FOUND;
		}
	}

	strcpy(mszName, (char*)sFileEntry.name);

	eResult = OpenFileByEntry(&sFileEntry, uiAccessFlags);
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Open(SFatDirectoryEntry* psEntry, uint8 uiAccessFlags)
{
	mcCache.Init(mpcVolume->GetFileDrive(), mpcVolume->GetClusterSize(), mpcVolume->GetSectorSize());

	return OpenFileByEntry(psEntry, uiAccessFlags);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::OpenFileByEntry(SFatDirectoryEntry* psEntry, uint8 uiAccessFlags)
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::AllocateClusters(uint32 uiBytes)
{
	EFatCode	eResult;
	fatEntry	uiNewCluster;
	uint32		uiClustersNeeded;
	uint32		uiClustersInFile;
	fatEntry	uiFirstCluster;
	fatEntry	uiCluster;
	fatEntry	uiNextCluster;
	fatEntry	uiPreviousCluster;

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

	uiFirstCluster = CalculateFirstCluster();

	uiClustersInFile = 0;
	uiPreviousCluster = mpcVolume->GetEndOfClusterMarker();
	if (uiFirstCluster != 0)
	{
		uiCluster = uiFirstCluster;
		while (!mpcVolume->FatIsEOFEntry(uiCluster))
		{
			eResult = mpcVolume->GetNextClusterEntry(uiCluster, &uiNextCluster);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}

			uiClustersInFile++;
			uiPreviousCluster = uiCluster;
			uiCluster = uiNextCluster;
		}
	}

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

	eResult = mpcVolume->Allocate(uiClustersNeeded, uiFirstCluster, &uiNewCluster, uiPreviousCluster);
	if (eResult != FAT_SUCCESS)
	{
		msFile.bBusy = false;
		return eResult;
	}

	// UpdateDirectoryEntry overwrites the FAT with uiNewCluster == 6 but it should a) not overwrite at all and b) uiNewCluster should == 7
	eResult = mpcVolume->UpdateDirectoryEntry(&msFile.sDirectoryEntry, uiFirstCluster, uiNewCluster, uiClustersInFile);
	if (eResult != FAT_SUCCESS)
	{
		msFile.bBusy = false;
		return eResult;
	}

	// update the file to point to the new cluster
	if (msFile.uiCursorClusterInVolume == 0)
	{
		msFile.uiCursorClusterInVolume = uiNewCluster;
	}

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
		eResult = AllocateClusters(uiOverFileLength);
		RETURN_ON_FAT_FAILURE(eResult)
	}

	// check that another operation is not using the file at this time
	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	// mark the file as in use
	msFile.bBusy = true;

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
EFatCode CFatFile::Flush(void)
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
		SFatCache	sBuffer;

		sBuffer = mpcVolume->ReadSector(msFile.sDirectoryEntry.uiSectorAddress);
		if (!sBuffer.IsValid())
		{
			msFile.bBusy = 0;
			return FAT_CANNOT_READ_MEDIA;
		}

		// copy the modified file entry to the sector buffer
		memcpy(sBuffer.Get() + msFile.sDirectoryEntry.uiSectorOffset, &msFile.sDirectoryEntry.raw, sizeof(SFatRawDirectoryEntry));
		mpcVolume->DirtySector(sBuffer);

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
	eResult = Flush();
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

