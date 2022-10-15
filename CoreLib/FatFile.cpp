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
	RETURN_ON_FAT_FAILURE(eResult);

	if (sFileEntry.szName[0] == '\0')
	{
		if (uiAccessFlags & FAT_FILE_ACCESS_CREATE)
		{
			ptrdiff_t			iPathLength;
			char*				pcFilenameScanner;
			char				szFilePath[FAT_MAX_PATH + 1];
			SFatDirectoryEntry	sParentEntry;

			eResult = FindBackslash(filename, &pcFilenameScanner);
			RETURN_ON_FAT_FAILURE(eResult);

			iPathLength = pcFilenameScanner - filename;

			memcpy(szFilePath, filename, iPathLength);
			szFilePath[iPathLength] = '\0';


			eResult = mpcVolume->GetFileEntry(szFilePath, &sParentEntry);
			RETURN_ON_FAT_FAILURE(eResult);

			if (*sParentEntry.szName == 0)
			{
				return FAT_DIRECTORY_DOES_NOT_EXIST;
			}

			pcFilenameScanner++;
			eResult = mpcVolume->CreateFATEntry(&sParentEntry.sRaw, pcFilenameScanner, FAT_ATTR_ARCHIVE, 0, &sFileEntry);
			RETURN_ON_FAT_FAILURE(eResult);

			uiAccessFlags = uiAccessFlags & (0xFF ^ FAT_FILE_ACCESS_APPEND);
			
			msFile.uiFileSize = 0;
			msFile.uiAccessFlags = uiAccessFlags;
			msFile.uiMagic = FAT_OPEN_HANDLE_MAGIC;
			msFile.bBusy = false;

			RETURN_ON_FAT_FAILURE(eResult);
		}
		else
		{
			return FAT_FILE_NOT_FOUND;
		}
	}

	strcpy(mszName, (char*)sFileEntry.szName);

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

	uiAccessFlags |= FAT_FILE_ACCESS_READ;
	if (uiAccessFlags & (FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_APPEND | FAT_FILE_ACCESS_OVERWRITE))
	{
		uiAccessFlags |= FAT_FILE_ACCESS_WRITE;
	}

	if (psEntry->uiAttributes & FAT_ATTR_DIRECTORY)
	{
		return FAT_NOT_A_FILE;
	}

	msFile.sDirectoryEntry = *psEntry;
	msFile.uiFileSize = psEntry->uiSize;
	msFile.uiAccessFlags = uiAccessFlags;
	msFile.uiMagic = FAT_OPEN_HANDLE_MAGIC;
	msFile.bBusy = false;

	msFile.uiCursorClusterInVolume = GetFirstClusterFromFatEntry(&psEntry->sRaw, IsFat32Volume());

	if (uiAccessFlags & FAT_FILE_ACCESS_APPEND)
	{
		eResult = Seek(0, FAT_SEEK_END);
		if (eResult != FAT_SUCCESS)
		{
			msFile.uiMagic = 0;
			return eResult;
		}
	}
	else if (uiAccessFlags & FAT_FILE_ACCESS_OVERWRITE)
	{
		eResult = mpcVolume->FreeDirectoryEntry(psEntry);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatFile::FatFileGetUniqueId(void)
{
	return GetFirstClusterFromFatEntry(&msFile.sDirectoryEntry.sRaw, IsFat32Volume());
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

	eResult = mpcVolume->AllocateClusters(uiClustersNeeded, uiFirstCluster, &uiNewCluster, uiPreviousCluster);
	if (eResult != FAT_SUCCESS)
	{
		msFile.bBusy = false;
		return eResult;
	}

	eResult = mpcVolume->UpdateDirectoryEntry(&msFile.sDirectoryEntry, uiFirstCluster, uiNewCluster, uiClustersInFile);
	if (eResult != FAT_SUCCESS)
	{
		msFile.bBusy = false;
		return eResult;
	}

	msFile.bBusy = false;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatFile::CalculateFirstCluster(void)
{
	return GetFirstClusterFromFatEntry(&msFile.sDirectoryEntry.sRaw, IsFat32Volume());
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
	uint16	eResult;

	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}

	msFile.bBusy = true;

	switch (mode)
	{
		case FAT_SEEK_START:
		{
			uiBytePositionInFile = offset;
			break;
		}

		case FAT_SEEK_CURRENT:
		{
			uiBytePositionInFile = msFile.uiFilePosition;
			uiBytePositionInFile += offset;
			break;
		}

		case FAT_SEEK_END:
		{
			uiBytePositionInFile = msFile.uiFileSize - offset;
			break;
		}

		default:
		{
			msFile.bBusy = false;
			return FAT_SEEK_FAILED;
		}
	}

	if (uiBytePositionInFile > msFile.uiFileSize)
	{
		msFile.bBusy = false;
		return FAT_SEEK_FAILED;
	}
	
	uiSectorIndexInFile = uiBytePositionInFile / mpcVolume->GetSectorSize();
	uiClusterIndex = uiSectorIndexInFile / mpcVolume->NumSectorsPerCluster();

	msFile.uiCursorClusterInVolume = CalculateFirstCluster();

	if (uiClusterIndex > 0)
	{
		eResult = mpcVolume->SeekByClusterCount(msFile.uiCursorClusterInVolume, uiClusterIndex, &msFile.uiCursorClusterInVolume);
		if (eResult != FAT_SUCCESS)
		{
			msFile.bBusy = false;
			return FAT_CORRUPTED_FILE;
		}
	}

	msFile.uiFilePosition = uiBytePositionInFile;

	msFile.bBusy = false;
	return FAT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::ValidateCanWrite(void)
{
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}
	if (!(msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE))
	{
		return FAT_FILE_NOT_OPENED_FOR_WRITE_ACCESS;
	}
	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}
	return FAT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::PotentiallyAllocateClusters(uint32 uiLength)
{
	EFatCode	eResult;
	uint32		uiOverFileLength;

	if ((msFile.uiFilePosition + uiLength) > msFile.uiFileSize)
	{
		uiOverFileLength = (msFile.uiFilePosition + uiLength);
		eResult = AllocateClusters(uiOverFileLength);
		RETURN_ON_FAT_FAILURE(eResult);

		eResult = Seek(msFile.uiFilePosition, FAT_SEEK_START);
		RETURN_ON_FAT_FAILURE(eResult);
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Write(uint8* pvSource, uint32 uiLength)
{
	EFatCode	eResult;

	eResult = ValidateCanWrite();
	RETURN_ON_FAT_FAILURE(eResult);

	eResult = PotentiallyAllocateClusters(uiLength);
	RETURN_ON_FAT_FAILURE(eResult);

	eResult = WriteIntoExistingClusters(uiLength, pvSource);
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::WriteIntoExistingClusters(uint32 uiBytesRemaining, uint8* puiSource)
{
	EFatCode	uiResult;
	bool		bSuccess;
	uint32		uiFirstSectorOfCluster;
	uint32		uiClustersInFile;
	uint32		uiWriteClusterIndex;
	uint16		uiWriteOffsetInCluster;
	uint16		uiPreviousMaximumOffset;
	uint32		uiBytesWritten;

	msFile.bBusy = true;

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
		if (!bSuccess)
		{
			msFile.bBusy = false;
			return FAT_CANNOT_WRITE_MEDIA;
		}

		uiBytesWritten -= uiBytesRemaining;

		puiSource += uiBytesWritten;
		msFile.uiFilePosition += uiBytesWritten;
		if (msFile.uiFilePosition > msFile.uiFileSize)
		{
			msFile.uiFileSize = msFile.uiFilePosition;
		}

		if (uiBytesRemaining == 0)
		{
			msFile.bBusy = false;
			return FAT_SUCCESS;
		}

		uiResult = mpcVolume->GetNextClusterEntry(msFile.uiCursorClusterInVolume, &msFile.uiCursorClusterInVolume);
		if (uiResult != FAT_SUCCESS)
		{
			msFile.bBusy = false;
			return uiResult;
		}
	}
}





//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::ValidateCanRead(void)
{
	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}

	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Read(uint8* puiDestination, uint32 uiLength, uint32* puiBytesRead)
{
	EFatCode	eResult;

	eResult = ValidateCanRead();
	RETURN_ON_FAT_FAILURE(eResult);

	return ReadFromClusters(uiLength, puiBytesRead, puiDestination);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::ReadFromClusters(uint32 uiBytesRemaining, uint32* puiBytesRead, uint8* puiDestination)
{
	EFatCode	eResult;
	bool		bSuccess;
	uint32		uiFirstSector;
	uint32		uiClustersInFile;
	uint32		uiReadClusterIndex;
	uint16		uiReadOffsetInCluster;
	uint16		uiPreviousMaximumOffset;
	uint32		uiBytesRead;
	uint32		uiTotalBytesRead;

	msFile.bBusy = true;

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
			msFile.bBusy = false;
			SafeAssign(puiBytesRead, uiTotalBytesRead);
			return FAT_CANNOT_READ_MEDIA;
		}

		if (uiBytesRemaining == 0)
		{
			msFile.bBusy = false;
			SafeAssign(puiBytesRead, uiTotalBytesRead);
			return FAT_SUCCESS;
		}

		eResult = mpcVolume->GetNextClusterEntry(msFile.uiCursorClusterInVolume, &msFile.uiCursorClusterInVolume);
		if (eResult != FAT_SUCCESS)
		{
			msFile.bBusy = false;
			SafeAssign(puiBytesRead, uiTotalBytesRead);
			return eResult;
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
EFatCode CFatFile::FlushCacheUpdateAccess(void)
{
	SFatCache	sBuffer;

	msFile.bBusy = true;

	mcCache.Flush();

	msFile.sDirectoryEntry.sRaw.uEntry.sFatRawCommon.uiSize = msFile.uiFileSize;
	msFile.sDirectoryEntry.sRaw.uEntry.sFatRawCommon.uiModifyDate = GetSystemClockDate();
	msFile.sDirectoryEntry.sRaw.uEntry.sFatRawCommon.uiModifyTime = GetSystemClockTime();
	msFile.sDirectoryEntry.sRaw.uEntry.sFatRawCommon.uiAccessDate = msFile.sDirectoryEntry.sRaw.uEntry.sFatRawCommon.uiModifyDate;

	sBuffer = mpcVolume->ReadSector(msFile.sDirectoryEntry.uiSectorAddress);
	if (!sBuffer.IsValid())
	{
		msFile.bBusy = false;
		return FAT_CANNOT_READ_MEDIA;
	}

	memcpy(sBuffer.Get() + msFile.sDirectoryEntry.uiSectorOffset, &msFile.sDirectoryEntry.sRaw, sizeof(SFatRawDirectoryEntry));
	mpcVolume->DirtySector(sBuffer);

	msFile.bBusy = false;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatFile::Flush(void)
{
	EFatCode	eResult;

	if (msFile.uiMagic != FAT_OPEN_HANDLE_MAGIC)
	{
		return FAT_INVALID_HANDLE;
	}
	if (msFile.bBusy)
	{
		return FAT_FILE_HANDLE_IN_USE;
	}
	if (msFile.uiAccessFlags & FAT_FILE_ACCESS_WRITE)
	{
		eResult = FlushCacheUpdateAccess();
		RETURN_ON_FAT_FAILURE(eResult);
	}
	return FAT_SUCCESS;
}


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
		msFile.bBusy = true;

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
uint32 CFatFile::Size(void) { return msFile.uiFileSize; }
bool CFatFile::IsBusy(void) { return msFile.bBusy; }
uint8 CFatFile::GetAccessFlags(void) { return msFile.uiAccessFlags; }

