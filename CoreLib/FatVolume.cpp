#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "BaseLib/MemCompare.h"
#include "FatTime.h"
#include "FatFilenameHelper.h"
#include "FatVolume.h"


#define READ_SECTOR(c, s) c = mcSectorCache.ReadSector(s); if (!c.IsValid()) { return FAT_CANNOT_READ_MEDIA; }



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::CheckFileAllocationTableLargeEnough(EFatFileSystemType eFileSystem, uint32 uiFatSize, uint32 uiNoOfClusters, uint16 uiBytesPerSector)
{
	switch (eFileSystem)
	{
		case FAT_FS_TYPE_FAT12:
		{
			if (uiFatSize < (((uiNoOfClusters + (uiNoOfClusters >> 1)) + uiBytesPerSector - 1) / uiBytesPerSector))
			{
				return false;
			}
			break;
		}
		case FAT_FS_TYPE_FAT16:
		{
			if (uiFatSize < (((uiNoOfClusters * 2) + uiBytesPerSector - 1) / uiBytesPerSector))
			{
				return false;
			}
			break;
		}
		case FAT_FS_TYPE_FAT32:
		{
			if (uiFatSize < (((uiNoOfClusters * 4) + uiBytesPerSector - 1) / uiBytesPerSector))
			{
				return false;
			}
			break;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::CheckSectorsPerClusterIsPowerOfTwo(uint8 uiSectorsPerCluster)
{
	while (uiSectorsPerCluster != 1)
	{
		if (uiSectorsPerCluster & 1)
		{
			return false;
		}
		uiSectorsPerCluster >>= 1;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::InitialiseVolumeParametersFromBPB(uint8* puiPartitionsTried, SFatBIOSParameterBlock* psBPB, uint32 uiHiddenSectors)
{
	// get all the info we need from BPB.
	muiRootDirectorySectors = ((psBPB->uiRootEntryCount * 32) + (psBPB->uiBytsPerSector - 1)) / psBPB->uiBytsPerSector;
	muiFatSize = (psBPB->uiFATSzFat16) ? psBPB->uiFATSzFat16 : psBPB->uFatEx.sFat32.uiFATSzFat32;
	muiNoOfSectors = (psBPB->uiTotalSectorsFat16) ? psBPB->uiTotalSectorsFat16 : psBPB->uiTotalSectorsFat32;
	muiNoOfDataSectors = muiNoOfSectors - (psBPB->uiReservedSectorCount + (psBPB->uiNumFileAllocationTables * muiFatSize) + muiRootDirectorySectors);
	muiNoOfClusters = muiNoOfDataSectors / psBPB->uiSectorsPerCluster;
	muiBytesPerSector = psBPB->uiBytsPerSector;
	meFileSystem = (muiNoOfClusters < 4085) ? FAT_FS_TYPE_FAT12 : (muiNoOfClusters < 65525) ? FAT_FS_TYPE_FAT16 : FAT_FS_TYPE_FAT32;

	if (!CheckFileAllocationTableLargeEnough(meFileSystem, muiFatSize, muiNoOfClusters, muiBytesPerSector))
	{
		(*puiPartitionsTried)++;
		return false;
	}

	muiHiddenSectors = uiHiddenSectors;
	muiFirstDataSector = psBPB->uiReservedSectorCount + muiHiddenSectors + (psBPB->uiNumFileAllocationTables * muiFatSize) + muiRootDirectorySectors;
	muiNoOfReservedSectors = psBPB->uiReservedSectorCount + muiHiddenSectors;
	muiNoOfSectorsPerCluster = psBPB->uiSectorsPerCluster;
	muiNoOfFatTables = psBPB->uiNumFileAllocationTables;
	muiFileSystemInfoSector = psBPB->uFatEx.sFat32.uiFileSystemInformation;
	muiBytesPerCluster = muiBytesPerSector * muiNoOfSectorsPerCluster;

	// read the volume label from the boot sector.
	if (meFileSystem == FAT_FS_TYPE_FAT16)
	{
		muiID = psBPB->uFatEx.sFat16.uiVolumeID;
		mszLabel.Kill();
		mszLabel.Init(psBPB->uFatEx.sFat16.szVolumeLabel, 0, 10);
		mszLabel.StripWhiteSpace();
	}
	else if (meFileSystem == FAT_FS_TYPE_FAT32)
	{
		muiID = psBPB->uFatEx.sFat32.uiVolumeID;
		mszLabel.Kill();
		mszLabel.Init(psBPB->uFatEx.sFat32.szVolumeLabel, 0, 10);
		mszLabel.StripWhiteSpace();
	}

	// if the volume is FAT32 then copy the root entry's cluster from the uiRootCluster field on the BPB .
	if (meFileSystem == FAT_FS_TYPE_FAT32)
	{
		muiRootCluster = psBPB->uFatEx.sFat32.uiRootCluster;
	}
	else
	{
		muiRootCluster = 0;
	}

	muiRootSector = CalculateRootSector();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FindBiosParameterBlock(SFatCache sMBRSector)
{
	uint32					uiHiddenSectors;
	SFatPartitionEntry*		psPartitionEntry;
	uint8					uiPartitionsTried;
	EFatCode				eResult;
	bool					bContinue;
	SFatCache				sLBASector;
	uint32					uiFATSector;
	SFatCache				sFATSector;

	eResult = ValidateFatCache(sMBRSector);
	RETURN_ON_FAT_FAILURE(eResult);

	uiHiddenSectors = 0;
	memcpy(&msBPB, sMBRSector.Get(), sizeof(SFatBIOSParameterBlock));

	uiPartitionsTried = 0;
	psPartitionEntry = (SFatPartitionEntry*)(sMBRSector.Get() + 0x1BE);

	for (;;)
	{
		eResult = ValidateFatCache(sMBRSector);
		RETURN_ON_FAT_FAILURE(eResult);

		// if we've already tried to mount the volume as partitionless try to mount the next partition
		if (uiPartitionsTried > 0)
		{
			// if we've already tried all 4 partitions then we're out of luck
			if (uiPartitionsTried > 4)
			{
				return FAT_INVALID_FAT_VOLUME;
			}

			// if we've tried to mount a partition volume (not the unpartioned one) then we must reload sector 0 (MBR)
			if (uiPartitionsTried > 1)
			{
				psPartitionEntry++;
			}

			// remember how many sectors before this partition
			uiHiddenSectors = psPartitionEntry->uiLBAFirstSector;

			// make sure the partition doesn't exceeds the physical boundries of the device
			if (psPartitionEntry->uiLBAFirstSector + psPartitionEntry->uiTotalSectors > mpcDevice->GetTotalSectors())
			{
				uiPartitionsTried++;
				continue;
			}

			// retrieve the 1st sector of partition
			READ_SECTOR(sLBASector, psPartitionEntry->uiLBAFirstSector);

			// set our pointer to the BPB
			memcpy(&msBPB, sLBASector.pvCachedSector, sizeof(SFatBIOSParameterBlock));
		}

		// if the sector size is larger than what this build allows do not mount the volume.
		if (msBPB.uiBytsPerSector > MAX_SECTOR_LENGTH)
		{
			return FAT_SECTOR_SIZE_NOT_SUPPORTED;
		}

		// make sure uiBytsPerSector and uiSectorsPerCluster are valid before we divide by them.
		if (!msBPB.uiBytsPerSector || !msBPB.uiSectorsPerCluster)
		{
			uiPartitionsTried++;
			continue;
		}

		if (!CheckSectorsPerClusterIsPowerOfTwo(msBPB.uiSectorsPerCluster))
		{
			uiPartitionsTried++;
			continue;
		}

		bContinue = !InitialiseVolumeParametersFromBPB(&uiPartitionsTried, &msBPB, uiHiddenSectors);
		if (bContinue)
		{
			continue;
		}

		uiFATSector = muiNoOfReservedSectors;
		READ_SECTOR(sFATSector, uiFATSector);

		// if the lower byte of the 1st FAT entry is not the same as uiMediaType then this is not a valid volume
		if (sFATSector.Get()[0] != msBPB.uiMediaType)
		{
			uiPartitionsTried++;
			continue;
		}

		return FAT_SUCCESS;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::InitialiseVolumeLabel(void)
{
	EFatCode			eResult;
	SFatQueryState		sQuery;

	sQuery.Init();
	eResult = QueryFirstEntry(0, FAT_ATTR_VOLUME_ID, &sQuery, 1);
	if (eResult == FAT_SUCCESS)
	{
		if (sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[0] != '\0')
		{
			if (sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes == FAT_ATTR_VOLUME_ID)
			{
				mszLabel.Kill();
				mszLabel.Init((char*)sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName, 0, 10);
				mszLabel.StripWhiteSpace();
			}
		}
	}

	sQuery.Kill(GetSectorCache());
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::Mount(CFileDrive* pcDevice)
{
	uint32					uiHiddenSectors = 0;
	uint8					uiPartitionsTried = 0;
	SFatCache				sMBRSector;
	EFatCode				eResult;

	mcSectorCache.Init(pcDevice, 3);
	mszLabel.Init();

	mpcDevice = pcDevice;

	READ_SECTOR(sMBRSector, 0);

	eResult = FindBiosParameterBlock(sMBRSector);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	eResult = InitialiseVolumeLabel();
	RETURN_ON_FAT_FAILURE(eResult);

	// if we find a valid FatFileSystemInfo structure we'll use it
	if ((meFileSystem == FAT_FS_TYPE_FAT32) && (muiFileSystemInfoSector > 0) && (muiFileSystemInfoSector != 0xFFFFFFFF))
	{
		eResult = InitialiseFat32FileSystemInfo();
		RETURN_ON_FAT_FAILURE(eResult);
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::Unmount(void)
{
	SFatFileSystemInfo*		psFileSystemInfo;
	EFatCode				eResult;
	SFatCache				sBuffer;
	bool					bResult;

	if (meFileSystem == FAT_FS_TYPE_FAT32 && muiFileSystemInfoSector != 0xFFFFFFFF)
	{
		READ_SECTOR(sBuffer, muiFileSystemInfoSector);

		psFileSystemInfo = (SFatFileSystemInfo*)sBuffer.Get();

		psFileSystemInfo->uiNextFreeCluster = GetNextFreeCluster();
		psFileSystemInfo->uiNumFreeClusters = GetTotalFreeClusters();
		psFileSystemInfo->uiLeadSignature = 0x41615252;
		psFileSystemInfo->uiStructSignature = 0x61417272;
		psFileSystemInfo->uiTrailSignature = 0xAA550000;

		DirtySector(sBuffer);

		eResult = Flush();
		RETURN_ON_FAT_FAILURE(eResult);
	}

	mszLabel.Kill();
	bResult = mcSectorCache.Kill();
	if (bResult)
	{
		return FAT_SUCCESS;
	}
	else
	{
		return FAT_NO_CACHE_LOCK;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::InitialiseFat32FileSystemInfo(void)
{
	SFatFileSystemInfo* psFileSystemInfo;
	SFatCache			sInfoSector;

	muiFileSystemInfoSector = muiHiddenSectors + muiFileSystemInfoSector;

	READ_SECTOR(sInfoSector, muiFileSystemInfoSector);

	psFileSystemInfo = (SFatFileSystemInfo*)sInfoSector.Get();

	if (psFileSystemInfo->uiLeadSignature == 0x41615252 && psFileSystemInfo->uiStructSignature == 0x61417272 && psFileSystemInfo->uiTrailSignature == 0xAA550000)
	{
		muiNextFreeCluster = psFileSystemInfo->uiNextFreeCluster;

		if (psFileSystemInfo->uiNumFreeClusters < muiNoOfClusters)
		{
			muiTotalFreeClusters = psFileSystemInfo->uiNumFreeClusters;
		}
		else
		{
			muiTotalFreeClusters = muiNoOfClusters - 1;
		}
	}
	else
	{
		muiNextFreeCluster = 0xFFFFFFFF;
		muiTotalFreeClusters = muiNoOfClusters - 1;
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::Flush(void)
{
	bool	bSuccess;

	bSuccess = mcSectorCache.Flush();
	if (!bSuccess)
	{
		return FAT_CANNOT_WRITE_MEDIA;
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
bool CFatVolume::Erase(uint32 uiStartSector, uint32 uiStopSectorInclusive)
{
	return mpcDevice->Erase(uiStartSector, uiStopSectorInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::QueryFirstEntry(SFatRawDirectoryEntry* psDirectoryEntry, uint8 attributes, SFatQueryState* psQuery, bool bBufferLocked)
{
	uint32		uiFirstSector;
	EFatCode	eResult;

	psQuery->auiLongFilename[0] = 0;

	if ((psDirectoryEntry == NULL) || 
		(psDirectoryEntry->uEntry.sFatRawCommon.uiFirstClusterHighWord == 0 && psDirectoryEntry->uEntry.sFatRawCommon.uiFirstClusterLowWord == 0))
	{
		psQuery->uiCurrentCluster = GetRootCluster();
		uiFirstSector = GetRootSector();
	}
	else
	{
		if (!(psDirectoryEntry->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_DIRECTORY))
		{
			return FAT_NOT_A_DIRECTORY;
		}

		psQuery->uiCurrentCluster = GetFirstClusterFromFatEntry(psDirectoryEntry, meFileSystem == FAT_FS_TYPE_FAT32);

		uiFirstSector = CalculateFirstSectorOfCluster(psQuery->uiCurrentCluster);
	}

	psQuery->sBuffer = ReadSector(uiFirstSector, true);
	if (!psQuery->sBuffer.IsValid())
	{
		return FAT_CANNOT_READ_MEDIA;
	}
	if (!psQuery->Lock(uiFirstSector))
	{
		return FAT_NO_CACHE_LOCK;
	}

	psQuery->psFirstEntryRaw = (SFatRawDirectoryEntry*)psQuery->sBuffer.Get();
	psQuery->psCurrentEntryRaw = (SFatRawDirectoryEntry*)psQuery->sBuffer.Get();

	psQuery->uiAttributes = attributes;
	psQuery->uiCurrentSector = 0;

	eResult = QueryNextEntry(psQuery, bBufferLocked, true);
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetQueryLongFilenamePart(SFatQueryState* psQuery, int8 iLongOffset, uint8* puiChars, int8 iRawOffset)
{
	((uint8*)&psQuery->auiLongFilename[((psQuery->uiSequence - 1) * 13) + iLongOffset])[0] = puiChars[iRawOffset + 0];
	((uint8*)&psQuery->auiLongFilename[((psQuery->uiSequence - 1) * 13) + iLongOffset])[1] = puiChars[iRawOffset + 1];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetQueryLongFilenamePart(SFatQueryState* psQuery)
{
	SetQueryLongFilenamePart(psQuery, 0, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1, 0);
	SetQueryLongFilenamePart(psQuery, 1, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1, 2);
	SetQueryLongFilenamePart(psQuery, 2, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1, 4);
	SetQueryLongFilenamePart(psQuery, 3, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1, 6);
	SetQueryLongFilenamePart(psQuery, 4, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1, 8);
	SetQueryLongFilenamePart(psQuery, 5, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2, 0);
	SetQueryLongFilenamePart(psQuery, 6, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2, 2);
	SetQueryLongFilenamePart(psQuery, 7, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2, 4);
	SetQueryLongFilenamePart(psQuery, 8, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2, 6);
	if (psQuery->uiSequence < 20)
	{
		SetQueryLongFilenamePart(psQuery, 9, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2, 8);
		SetQueryLongFilenamePart(psQuery, 10, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2, 10);
		SetQueryLongFilenamePart(psQuery, 11, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars3, 0);
		SetQueryLongFilenamePart(psQuery, 12, psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars3, 2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FindNextRawDirectoryEntrySectorInCluster(uint32* puiSector, SFatQueryState* psQuery)
{
	fatEntry	uiFat;
	EFatCode	eResult;

	eResult = GetNextClusterEntry(psQuery->uiCurrentCluster, &uiFat);
	RETURN_ON_FAT_FAILURE(eResult);

	if (FatIsEOFEntry(uiFat))
	{
		psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[0] = '\0';
		return FAT_SHORT_NAME_FOUND;
	}

	psQuery->uiCurrentCluster = uiFat;

	psQuery->uiCurrentSector = 0;

	*puiSector = CalculateFirstSectorOfCluster(psQuery->uiCurrentCluster) + psQuery->uiCurrentSector;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::IsRootDirectoryCluster(uint32 uiDirectoryCluster)
{
	return uiDirectoryCluster == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FindNextRawDirectoryEntryCluster(uint32* puiSector, SFatQueryState* psQuery)
{
	psQuery->uiCurrentSector++;

	if (IsRootDirectoryCluster(psQuery->uiCurrentCluster))
	{
		if (psQuery->uiCurrentSector == GetRootDirectorySectors())
		{
			psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[0] = '\0';
			return FAT_SHORT_NAME_FOUND;
		}
		*puiSector = GetRootDirectorySector() + psQuery->uiCurrentSector;
	}
	else
	{
		*puiSector = CalculateFirstSectorOfCluster(psQuery->uiCurrentCluster) + psQuery->uiCurrentSector;
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FindNextRawDirectoryEntry(SFatQueryState* psQuery)
{
	uint32		uiSector;
	EFatCode	eResult;

	eResult = ValidateFatCache(psQuery->sBuffer);
	RETURN_ON_FAT_FAILURE(eResult);

	if (((uintptr_t)psQuery->psCurrentEntryRaw - (uintptr_t)psQuery->psFirstEntryRaw) == GetSectorSize() - 0x20)
	{
		if ((psQuery->uiCurrentCluster > 0) && (psQuery->uiCurrentSector == NumSectorsPerCluster() - 1))
		{
			eResult = FindNextRawDirectoryEntrySectorInCluster(&uiSector, psQuery);
			RETURN_ON_FAT_FAILURE(eResult);
		}
		else
		{
			eResult = FindNextRawDirectoryEntryCluster(&uiSector, psQuery);
			RETURN_ON_FAT_FAILURE(eResult);
		}

		eResult = ValidateFatCache(psQuery->sBuffer);
		RETURN_ON_FAT_FAILURE(eResult);

		// read the next sector into the query buffer
		psQuery->sBuffer = ReadSector(uiSector, true);
		if (!psQuery->sBuffer.IsValid())
		{
			return FAT_CANNOT_READ_MEDIA;
		}
		if (!psQuery->Lock(uiSector))
		{
			return FAT_NO_CACHE_LOCK;
		}

		psQuery->psFirstEntryRaw = (SFatRawDirectoryEntry*)psQuery->sBuffer.Get();
		psQuery->psCurrentEntryRaw = (SFatRawDirectoryEntry*)psQuery->sBuffer.Get();
	}
	else
	{
		psQuery->psCurrentEntryRaw++;
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::IsFreeDirectoryEntry(SFatRawDirectoryEntry* psEntry)
{
	return *psEntry->uEntry.sFatRawCommon.szShortName == FAT_DELETED_ENTRY || IsLastDirectoryEntry(psEntry);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::IsLastDirectoryEntry(SFatRawDirectoryEntry* psEntry)
{
	return *psEntry->uEntry.sFatRawCommon.szShortName == '\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::ProcessQueryLongFileNameEntry(SFatQueryState* psQuery)
{
	if (psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence & FAT_FIRST_LFN_ENTRY)
	{
		psQuery->uiSequence = (psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence ^ FAT_FIRST_LFN_ENTRY) + 1;
		psQuery->uiChecksum = psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.uiChecksum;

		memset(psQuery->auiLongFilename, 0, sizeof(uint16) * (FAT_MAX_FILENAME + 1));
	}

	if (psQuery->uiChecksum == psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.uiChecksum &&
		(psQuery->uiSequence == (psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence & (0xFF ^ FAT_FIRST_LFN_ENTRY)) + 1))
	{
		psQuery->uiSequence = psQuery->psCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence & (0xFF ^ FAT_FIRST_LFN_ENTRY);
		SetQueryLongFilenamePart(psQuery);
	}
	else
	{
		psQuery->uiChecksum = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::ConstructQueryLongFileNameFromShortName(SFatQueryState* psQuery)
{
	::ConstructFatLongFileNameFromShortName(psQuery->auiLongFilename, (char*)psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName, psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.uiReserved & FAT_LOWERCASE_BASENAME, psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.uiReserved & FAT_LOWERCASE_EXTENSION);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::QueryNextEntry(SFatQueryState* psQuery, bool bBufferLocked, bool bFirstEntry)
{
	bool		bPass;
	EFatCode	eResult;

	do
	{
		if (!bFirstEntry)
		{
			eResult = FindNextRawDirectoryEntry(psQuery);
			if (eResult == FAT_SHORT_NAME_FOUND)
			{
				return FAT_SUCCESS;
			}
			else if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}
		}
		else
		{
			bFirstEntry = false;
		}

		if (psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes == FAT_ATTR_LONG_NAME && !IsFreeDirectoryEntry(psQuery->psCurrentEntryRaw))
		{
			ProcessQueryLongFileNameEntry(psQuery);
			bPass = (psQuery->uiAttributes == FAT_ATTR_LONG_NAME);
		}
		else
		{
			bPass =
				(!(psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_HIDDEN) || (psQuery->uiAttributes & FAT_ATTR_HIDDEN)) &&
				(!(psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_SYSTEM) || (psQuery->uiAttributes & FAT_ATTR_SYSTEM)) &&
				(!(psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_VOLUME_ID) || (psQuery->uiAttributes & FAT_ATTR_VOLUME_ID)) &&
				(!(psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_LONG_NAME) || (psQuery->uiAttributes & FAT_ATTR_LONG_NAME));
		}
	}
	while (!bPass || psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[0] == FAT_DELETED_ENTRY);

	if (psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[0] != '\0')
	{
		if (psQuery->uiChecksum != FatLongEntryChecksum((char*)psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName))
		{
			psQuery->auiLongFilename[0] = 0;
		}
	}

	if (psQuery->auiLongFilename[0] == 0 && (psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.uiReserved & (FAT_LOWERCASE_EXTENSION | FAT_LOWERCASE_BASENAME)))
	{
		ConstructQueryLongFileNameFromShortName(psQuery);
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::GetNextClusterEntry(uint32 uiCurrentCluster, fatEntry* puiNextCluster)
{
	uint32		uiFirstClusterSector;
	uint32		uiOffsetInSector;
	EFatCode	eResult;

	CalculateFATIndexAndOffset(&uiOffsetInSector, uiCurrentCluster, &uiFirstClusterSector);

	eResult = ReadFatEntry(uiOffsetInSector, uiCurrentCluster, uiFirstClusterSector, puiNextCluster);
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::EraseClusterChainContents(fatEntry uiCluster)
{
	uint32		uiStartSector;
	uint32		uiEndSector;
	EFatCode	eResult;
	bool		bSuccess;

	while (!FatIsEOFEntry(uiCluster))
	{
		uiStartSector = CalculateFirstSectorOfCluster(uiCluster);
		uiEndSector = uiStartSector + NumSectorsPerCluster();

		bSuccess = Erase(uiStartSector, uiEndSector - 1);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}

		eResult = GetNextClusterEntry(uiCluster, &uiCluster);
		RETURN_ON_FAT_FAILURE(eResult);
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::AllocateClusters(uint32 uiClustersNeeded, uint32 uiCluster, uint32* puiNewClusterInVolume, uint32 uiPreviousCluster)
{
	uint32		uiPageSize;
	EFatCode	eResult;

	uiPageSize = GetPageSize();

	if (uiPageSize > NumSectorsPerCluster())
	{
		uint32 uiClustersPerPage = uiPageSize / NumSectorsPerCluster();

		if (uiClustersNeeded % uiClustersPerPage > 0)
		{
			uiClustersNeeded += uiClustersPerPage - (uiClustersNeeded % uiClustersPerPage);
		}

		if ((uiClustersNeeded % uiClustersPerPage) != 0)
		{
			return FAT_UNKNOWN_ERROR;
		}

		*puiNewClusterInVolume = AllocateDataClusterEx(uiClustersNeeded, false, uiPageSize, &eResult, uiPreviousCluster);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}
	else
	{
		*puiNewClusterInVolume = AllocateDataCluster(uiClustersNeeded, false, &eResult, uiPreviousCluster);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}

	eResult = EraseClusterChainContents(*puiNewClusterInVolume);
	return eResult;
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::AllocateDirectoryCluster(SFatRawDirectoryEntry* parent, EFatCode* peResult)
{
	return AllocateClusters(parent, 1, true, 1, peResult, GetEndOfClusterMarker());
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::AllocateDataCluster(uint32 uiClusterCount, bool bEraseCluster, EFatCode* peResult, uint32 uiPreviousCluster)
{
	return AllocateClusters(NULL, uiClusterCount, bEraseCluster, 1, peResult, uiPreviousCluster);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::AllocateDataClusterEx(uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult, uint32 uiPreviousCluster)
{
	return AllocateClusters(NULL, uiClusterCount, bEraseCluster, uiPageSize, peResult, uiPreviousCluster);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::FindNextPageCluster(uint32 uiPageSize, uint32 uiCluster, uint16 uiStep)
{
	uint32 uiSector;
	uint16 uiStepCount;

	if (NumSectorsPerCluster() < uiPageSize)
	{
		uiStepCount = 0;
		uiSector = CalculateFirstSectorOfCluster(uiCluster);
		while (uiSector % uiPageSize && uiStepCount < uiStep)
		{
			uiCluster++;
			uiStepCount++;
			uiSector = CalculateFirstSectorOfCluster(uiCluster);
		}
	}
	return uiCluster;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
fatEntry CFatVolume::GetEndOfClusterMarker(void)
{
	switch (meFileSystem)
	{
		case FAT_FS_TYPE_FAT12:
			return FAT12_EOC;

		case FAT_FS_TYPE_FAT16:
			return FAT16_EOC;

		case FAT_FS_TYPE_FAT32:
			return FAT32_EOC;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
fatEntry CFatVolume::GetBadClusterMarker(void)
{
	switch (meFileSystem)
	{
		case FAT_FS_TYPE_FAT12:
			return FAT12_BAD_CLUSTER;

		case FAT_FS_TYPE_FAT16:
			return FAT16_BAD_CLUSTER;

		case FAT_FS_TYPE_FAT32:
			return FAT32_BAD_CLUSTER;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
fatEntry CFatVolume::GetClusterEntryMask(void)
{
	switch (meFileSystem)
	{
	case FAT_FS_TYPE_FAT12:
		return FAT12_CLUSTER_MASK;

	case FAT_FS_TYPE_FAT16:
		return FAT16_CLUSTER_MASK;

	case FAT_FS_TYPE_FAT32:
		return FAT32_CLUSTER_MASK;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::GetEntryCluster(SFatRawDirectoryEntry* psEntry)
{
	fatEntry	uiClusterEntryMask;
	fatEntry	uiEntry;

	uiClusterEntryMask = GetClusterEntryMask();

	uiEntry = psEntry->uEntry.sFatRawCommon.uiFirstClusterLowWord | psEntry->uEntry.sFatRawCommon.uiFirstClusterHighWord << 16;
	uiEntry &= uiClusterEntryMask;

	return uiEntry;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::HasNextFreeCluser(void)
{
	if (muiNextFreeCluster != 0xFFFFFFFF)
	{
		if (muiNextFreeCluster <= muiNoOfClusters + 1)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::CalculateClusterStepSize(uint32 uiPageSize)
{
	// find the step between clusters allocated on page boundaries
	if (NumSectorsPerCluster() < uiPageSize)
	{
		return (uint16)(uiPageSize / NumSectorsPerCluster());
	}
	else
	{
		return 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::EraseClusterContents(uint32 uiCluster)
{
	bool	bSuccess;
	uint16	uiSectorsPerCluster;
	uint32	uiCurrentSector;

	uiCurrentSector = CalculateFirstSectorOfCluster(uiCluster);
	uiSectorsPerCluster = NumSectorsPerCluster();

	bSuccess = mpcDevice->Erase(uiCurrentSector, uiCurrentSector + uiSectorsPerCluster - 1);
	if (bSuccess)
	{
		return FAT_SUCCESS;
	}
	else
	{
		return FAT_CANNOT_WRITE_MEDIA;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::InitialiseDottyDirectoryEntry(SFatRawDirectoryEntry* psEntries, char* sz, uint32 uiCluster)
{
	psEntries->uEntry.sFatRawCommon.szShortName[0x0] = sz[0];
	psEntries->uEntry.sFatRawCommon.szShortName[0x1] = sz[1];
	psEntries->uEntry.sFatRawCommon.szShortName[0x2] = ' ';
	psEntries->uEntry.sFatRawCommon.szShortName[0x3] = ' ';
	psEntries->uEntry.sFatRawCommon.szShortName[0x4] = ' ';
	psEntries->uEntry.sFatRawCommon.szShortName[0x5] = ' ';
	psEntries->uEntry.sFatRawCommon.szShortName[0x6] = ' ';
	psEntries->uEntry.sFatRawCommon.szShortName[0x7] = ' ';
	psEntries->uEntry.sFatRawCommon.szShortName[0x8] = ' ';
	psEntries->uEntry.sFatRawCommon.szShortName[0x9] = ' ';
	psEntries->uEntry.sFatRawCommon.szShortName[0xA] = ' ';
	psEntries->uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_DIRECTORY;
	psEntries->uEntry.sFatRawCommon.uiSize = 0x0;
	psEntries->uEntry.sFatRawCommon.uiReserved = 0;
	psEntries->uEntry.sFatRawCommon.uiFirstClusterLowWord = (uint16)uiCluster;
	psEntries->uEntry.sFatRawCommon.uiFirstClusterHighWord = (uint16)(uiCluster >> 16);
	psEntries->uEntry.sFatRawCommon.uiCreateDate = GetSystemClockDate();
	psEntries->uEntry.sFatRawCommon.uiCreateTime = GetSystemClockTime();
	psEntries->uEntry.sFatRawCommon.uiModifyDate = psEntries->uEntry.sFatRawCommon.uiCreateDate;
	psEntries->uEntry.sFatRawCommon.uiModifyTime = psEntries->uEntry.sFatRawCommon.uiCreateTime;
	psEntries->uEntry.sFatRawCommon.uiAccessDate = psEntries->uEntry.sFatRawCommon.uiCreateDate;
	psEntries->uEntry.sFatRawCommon.uiCreateTimeTenths = 0xc6;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::InitializeDirectoryCluster(SFatRawDirectoryEntry* psDirectoryParent, uint32 uiCluster)
{
	uint32					uiFirstClusterSector;
	SFatRawDirectoryEntry*	psEntries;
	SFatCache				sBuffer;
	bool					bSuccess;
	uint32					uiLastClusterSector;
	uint32					uiSector;

	uiSector = CalculateFirstSectorOfCluster(uiCluster);

	READ_SECTOR(sBuffer, uiSector);

	psEntries = (SFatRawDirectoryEntry*)sBuffer.Get();

	memset(psEntries, 0, GetSectorSize());

	InitialiseDottyDirectoryEntry(psEntries, ". ", uiCluster);
	
	psEntries++;

	InitialiseDottyDirectoryEntry(psEntries, "..", GetEntryCluster(psDirectoryParent));
	
	DirtySector(sBuffer);

	// when the parent is the root directory the dotdot entry always points to cluster 0, even in FAT32 when the root directory is not actually on
	// cluster 0 so we need to check if the parent is the root directory and in that case set the 1st cluster to 0.
	if (meFileSystem == FAT_FS_TYPE_FAT32)
	{
		uint32 uiParentCluster;

		uiParentCluster = GetFirstClusterFromFatEntry(psDirectoryParent, true);
		if (GetRootCluster() == uiParentCluster)
		{
			psEntries->uEntry.sFatRawCommon.uiFirstClusterLowWord = 0;
			psEntries->uEntry.sFatRawCommon.uiFirstClusterHighWord = 0;
		}
	}

	uiFirstClusterSector = CalculateFirstSectorOfCluster(uiCluster) + 1;
	uiLastClusterSector = uiFirstClusterSector + NumSectorsPerCluster() - 2;
	if (uiLastClusterSector >= uiFirstClusterSector)
	{
		bSuccess = mpcDevice->Erase(uiFirstClusterSector, uiLastClusterSector);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::InitialiseAllocatedFatCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 uiCluster, bool bZero)
{
	EFatCode	eResult;

	// if we're allocating a cluster for a directory then initialize it properly
	if (psParentDirectory)
	{
		eResult = InitializeDirectoryCluster(psParentDirectory, uiCluster);
		RETURN_ON_FAT_FAILURE(eResult);
	}
	else
	{
		if (bZero)
		{
			eResult = EraseClusterContents(uiCluster);
			RETURN_ON_FAT_FAILURE(eResult);
		}
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WrapAllocate(uint32 uiStartCluster, uint32 uiLastFatEntry, bool* pbWrappedAround, uint32* puiClusterIndex, uint32* puiOffsetInSector, uint32* puiFirstClusterSector)
{
	if (uiStartCluster > 2 && !*pbWrappedAround)
	{
		*puiClusterIndex = 2;
		*pbWrappedAround = true;

		CalculateFATIndexAndOffset(puiOffsetInSector, *puiClusterIndex, puiFirstClusterSector);

		// break from this loop so that sector gets loaded
		return FAT_SUCCESS;
	}
	else if (*pbWrappedAround && (*puiClusterIndex >= uiStartCluster))
	{
		// free any clusters that we've allocated so far
		if (!FatIsEOFEntry(uiLastFatEntry))
		{
			FreeChain(uiLastFatEntry);
		}

		return FAT_INSUFFICIENT_DISK_SPACE;
	}
	else
	{
		return FAT_UNKNOWN_ERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteFat12Entry(uint32 uiOffsetInSector, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiLastFatEntry)
{
	bool		bNextSector;
	SFatCache	sBuffer;
	uint8*		puiBuffer;

	READ_SECTOR(sBuffer, uiSector);
	puiBuffer = sBuffer.Get();

	if (uiCluster & 1)
	{
		puiBuffer[uiOffsetInSector] &= 0x0F;
		puiBuffer[uiOffsetInSector] |= (uint8)(FAT12_EOC << 4);
	}
	else
	{
		puiBuffer[uiOffsetInSector] = (uint8)FAT12_EOC;
	}
	DirtySector(sBuffer);

	if (uiOffsetInSector == GetSectorSize() - 1)
	{
		READ_SECTOR(sBuffer, uiSector + 1);
		puiBuffer = sBuffer.Get();
		bNextSector = true;
	}
	else
	{
		bNextSector = false;
	}

	if (uiCluster & 1)
	{
		puiBuffer[bNextSector ? 0 : (uiOffsetInSector + 1)] = ((uint16)FAT12_EOC << 4) >> 8;  //Wut?
	}
	else
	{
		//Even cluster.
		puiBuffer[bNextSector ? 0 : (uiOffsetInSector + 1)] &= 0xF0;
		puiBuffer[bNextSector ? 0 : (uiOffsetInSector + 1)] |= FAT12_EOC >> 8;
	}
	DirtySector(sBuffer);

	// if this is not the 1st cluster allocated update the last one to link to this one
	if (uiLastFatEntry != FAT12_EOC)
	{
		READ_SECTOR(sBuffer, uiPreviousSector);
		puiBuffer = sBuffer.Get();

		if (uiLastFatEntry & 1)
		{
			puiBuffer[uiPreviousOffset] &= 0x0F;
			puiBuffer[uiPreviousOffset] |= (uint8)(uiCluster << 4);
		}
		else
		{
			puiBuffer[uiPreviousOffset] = (uint8)uiCluster;
		}
		DirtySector(sBuffer);

		// if the FAT entry spans a sector boundary flush the currently loaded sector to the drive and load the next one.
		if (uiPreviousOffset == GetSectorSize() - 1)
		{
			READ_SECTOR(sBuffer, uiPreviousSector + 1);
			puiBuffer = sBuffer.Get();
			bNextSector = true;
		}

		// write the 2nd byte
		if (uiLastFatEntry & 1)
		{
			puiBuffer[bNextSector ? 0 : (uiPreviousOffset + 1)] = ((uint16)uiCluster << 4) >> 8;
		}
		else
		{
			puiBuffer[bNextSector ? 0 : (uiPreviousOffset + 1)] &= 0xF0;
			puiBuffer[bNextSector ? 0 : (uiPreviousOffset + 1)] |= (uint8)(uiCluster >> 8);
		}
		DirtySector(sBuffer);
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteFat16Entry(uint32 uiOffsetInSector, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiLastFatEntry)
{
	SFatCache	sBuffer;
	uint8*		puiBuffer;

	READ_SECTOR(sBuffer, uiSector);
	puiBuffer = sBuffer.Get();

	*((uint16*)&puiBuffer[uiOffsetInSector]) = FAT16_EOC;
	DirtySector(sBuffer);

	if (uiLastFatEntry != FAT16_EOC)
	{
		READ_SECTOR(sBuffer, uiPreviousSector);
		puiBuffer = sBuffer.Get();

		*((uint16*)&puiBuffer[uiPreviousOffset]) = (uint16)uiCluster;
		DirtySector(sBuffer);
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteFat32Entry(uint32 uiOffsetInSector, uint32 uiLastEntryOffset, uint32 uiClusterIndexInTable, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiLastFatEntry)
{
	SFatCache	sBuffer;
	uint8*		puiBuffer;

	READ_SECTOR(sBuffer, uiSector);
	puiBuffer = sBuffer.Get();

	*((uint32*)&puiBuffer[uiOffsetInSector]) = FAT32_EOC;
	DirtySector(sBuffer);

	// if this is not the 1st cluster allocated update the last one to link to this one
	if (uiLastFatEntry != FAT32_EOC)
	{
		READ_SECTOR(sBuffer, uiPreviousSector);
		puiBuffer = sBuffer.Get();

		// update the last entry to point to this one
		*((uint32*)&puiBuffer[uiLastEntryOffset]) = (*((uint32*)&puiBuffer[uiLastEntryOffset]) & (~FAT32_CLUSTER_MASK)) | uiClusterIndexInTable & FAT32_CLUSTER_MASK;
		DirtySector(sBuffer);
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteAllocatedFatEntry(uint32 uiClusterIndexInTable, uint32 uiOffsetInSector, uint32 uiSector, fatEntry uiPreviousFatEntry, uint32 uiPreviousSector, uint32 uiPreviousEntryOffset)
{
	// maintain the count of free cluster and the next cluster that may be free.
	SetNextFreeCluster(uiClusterIndexInTable + 1);
	SetTotalFreeClusters(GetTotalFreeClusters() - 1);

	// mark the FAT as the the new 1st link of the cluster chain (or the end of the chain if we're only allocating 1 cluster)
	switch (meFileSystem)
	{
		case FAT_FS_TYPE_FAT12:
		{
			return WriteFat12Entry(uiOffsetInSector, uiPreviousEntryOffset, uiClusterIndexInTable, uiSector, uiPreviousSector, uiPreviousFatEntry);
		}
		case FAT_FS_TYPE_FAT16:
		{
			return WriteFat16Entry(uiOffsetInSector, uiPreviousEntryOffset, uiClusterIndexInTable, uiSector, uiPreviousSector, uiPreviousFatEntry);
		}
		case FAT_FS_TYPE_FAT32:
		{
			return WriteFat32Entry(uiOffsetInSector, uiPreviousEntryOffset, uiClusterIndexInTable, uiSector, uiPreviousSector, uiPreviousFatEntry);
		}
	}

	return FAT_UNKNOWN_ERROR;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::AllocateClusters(SFatRawDirectoryEntry* psParentDirectory, uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult, uint32 uiPreviousCluster)
{
	uint32		uiFirstClusterSector;
	uint32		uiCurrentSector;
	fatEntry	uiCluster;
	uint32		uiOffsetInSector = 0;
	bool		bNextSectorLoaded = false;
	fatEntry	uiLastClusterIndex;
	fatEntry	uiFatEntry;
	uint32		uiFirstEmptyCluster;
	uint32		uiLastClusterSector;
	uint32		uiLastEntryOffset;
	uint32		uiStartCluster;
	bool		bWrappedAround;
	uint16		uiClusterStep;
	int			iCount;

	if ((uiClusterCount > 1) && (psParentDirectory != NULL))
	{
		*peResult = FAT_UNKNOWN_ERROR;
		return 0;
	}

	if (uiClusterCount == 0)
	{
		*peResult = FAT_UNKNOWN_ERROR;
		return 0;
	}

	uiCluster = 2;
	uiFirstEmptyCluster = 0;

	if (HasNextFreeCluser())
	{
		uiCluster = GetNextFreeCluster();
	}

	uiClusterStep = CalculateClusterStepSize(uiPageSize);
	uiCluster = FindNextPageCluster(uiPageSize, uiCluster, uiClusterStep);

	if (!FatIsEOFEntry(uiPreviousCluster))
	{
		CalculateFATIndexAndOffset(&uiLastEntryOffset, uiPreviousCluster, &uiLastClusterSector);
	}
	else
	{
		uiLastClusterSector = 0;
		uiLastEntryOffset = 0;
	}

	CalculateFATIndexAndOffset(&uiOffsetInSector, uiCluster, &uiFirstClusterSector);

	uiStartCluster = uiCluster;
	uiLastClusterIndex = uiPreviousCluster;

	for (iCount = 0;; iCount++)
	{
		uiCurrentSector = uiFirstClusterSector;
		bWrappedAround = false;

		do
		{
			if ((uiCluster > GetNumClusters() + 1))
			{
				*peResult = WrapAllocate(uiStartCluster, uiLastClusterIndex, &bWrappedAround, &uiCluster, &uiOffsetInSector, &uiFirstClusterSector);
				if (*peResult != FAT_SUCCESS)
				{
					return 0;
				}
				break;
			}

			*peResult = ReadFatEntry(uiOffsetInSector, uiCluster, uiFirstClusterSector, &uiFatEntry);
			if (*peResult != FAT_SUCCESS)
			{
				return 0;
			}

			bool bIsFatFree = IsFreeFat(uiFatEntry);
			if (bIsFatFree)
			{
				if (uiFirstEmptyCluster == 0)
				{
					uiFirstEmptyCluster = uiCluster;
				}

				*peResult = WriteAllocatedFatEntry(uiCluster, uiOffsetInSector, uiFirstClusterSector, uiLastClusterIndex, uiLastClusterSector, uiLastEntryOffset);
				if (*peResult != FAT_SUCCESS)
				{
					return 0;
				}

				uiClusterCount--;

				if (uiClusterCount == 0)
				{
					*peResult = InitialiseAllocatedFatCluster(psParentDirectory, uiCluster, bEraseCluster);
					if (*peResult != FAT_SUCCESS)
					{
						return 0;
					}

					return uiFirstEmptyCluster;
				}

				uiLastClusterIndex = uiCluster;
				uiLastClusterSector = uiFirstClusterSector;
				uiLastEntryOffset = uiOffsetInSector;
			}

			if (uiFirstEmptyCluster == 0)
			{
				uiCluster += uiClusterStep;
			}
			else
			{
				uiCluster++;
			}

			CalculateFATIndexAndOffset(&uiOffsetInSector, uiCluster, &uiFirstClusterSector);

		} while (uiCurrentSector == uiFirstClusterSector);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::UpdateDirectoryEntry(SFatDirectoryEntry* psEntry, uint32 uiFirstCluster, uint32 uiNewClusterInVolume, uint32 uiClustersInFile)
{
	SFatCache	sBuffer;

	if (IsRootDirectoryCluster(uiFirstCluster))  //Why do we only do anything with a zero cluster.  What about other directories?
	{
		psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterLowWord = (uint16)uiNewClusterInVolume;
		psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterHighWord = (uint16)(uiNewClusterInVolume >> 16);
		psEntry->sRaw.uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_ARCHIVE;

		READ_SECTOR(sBuffer, psEntry->uiSectorAddress);

		memcpy(sBuffer.Get() + psEntry->uiSectorOffset, &psEntry->sRaw, sizeof(SFatRawDirectoryEntry));
		DirtySector(sBuffer);
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::SetFat12ClusterEntry(uint32 uiClusterIndex, fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector)
{
	uint8*		puiBuffer;
	SFatCache	sBuffer;

	READ_SECTOR(sBuffer, uiFirstClusterSector);
	puiBuffer = sBuffer.Get();

	// write the 1st byte
	if (uiClusterIndex & 1)
	{
		uiClusterInVolume <<= 4;										// odd entries occupy the upper 12 bits so we must shift
		puiBuffer[uiOffsetInSector] &= 0x0F;							// clear entry bits on 1st byte
		puiBuffer[uiOffsetInSector] |= (uint8)uiClusterInVolume;	// set entry bits on 1st byte
	}
	else
	{
		puiBuffer[uiOffsetInSector] = (uint8)uiClusterInVolume;		// just copy the 1st byte
	}
	DirtySector(sBuffer);

	// if the FAT entry spans a sector boundary flush the currently
	// loaded sector to the drive and load the next one.
	if (uiOffsetInSector == GetSectorSize() - 1)
	{
		READ_SECTOR(sBuffer, uiFirstClusterSector + 1);
		puiBuffer = sBuffer.Get();

		// the next byte is now loacted at offset 0 on the uiBuffer
		uiOffsetInSector = 0;
	}
	else
	{
		// the next byte is located next to the 1st one on the uiBuffer
		uiOffsetInSector++;
	}

	// write the 2nd byte
	if (uiClusterIndex & 1)
	{
		//puiBuffer[uiOffsetInSector] = ((uint16)uiClusterInVolume << 4) >> 8;  //Isn't this correct?
		puiBuffer[uiOffsetInSector] = ((uint16)uiClusterInVolume) >> 8;
	}
	else
	{
		puiBuffer[uiOffsetInSector] &= 0xF0;							
		puiBuffer[uiOffsetInSector] |= ((uint16)uiClusterInVolume) >> 8;
	}
	DirtySector(sBuffer);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::SetFat16ClusterEntry(fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector)
{
	uint8*		puiBuffer;
	SFatCache	sBuffer;

	READ_SECTOR(sBuffer, uiFirstClusterSector);
	puiBuffer = sBuffer.Get();

	*((uint16*)&puiBuffer[uiOffsetInSector]) = (uint16)uiClusterInVolume;
	DirtySector(sBuffer);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::SetFat32ClusterEntry(fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector)
{
	uint8*		puiBuffer;
	SFatCache	sBuffer;

	READ_SECTOR(sBuffer, uiFirstClusterSector);
	puiBuffer = sBuffer.Get();

	*((uint32*)&puiBuffer[uiOffsetInSector]) = uiClusterInVolume & FAT32_CLUSTER_MASK;
	DirtySector(sBuffer);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::SetClusterEntry(uint32 uiClusterIndex, fatEntry uiClusterInVolume)
{
	uint32		uiFirstClusterSector;
	uint32		uiOffsetInSector;
	EFatCode	eResult;

	CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiFirstClusterSector);

	// set the FAT entry
	switch (meFileSystem)
	{
		case FAT_FS_TYPE_FAT12:
		{
			eResult = SetFat12ClusterEntry(uiClusterIndex, uiClusterInVolume, uiFirstClusterSector, uiOffsetInSector);
			return eResult;
		}
		case FAT_FS_TYPE_FAT16:
		{
			eResult = SetFat16ClusterEntry(uiClusterInVolume, uiFirstClusterSector, uiOffsetInSector);
			return eResult;
		}
		case FAT_FS_TYPE_FAT32:
		{
			eResult = SetFat32ClusterEntry(uiClusterInVolume, uiFirstClusterSector, uiOffsetInSector);
			return eResult;
		}
	}

	return FAT_UNKNOWN_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::IncreaseFat12ClusterAddress(uint32* puiClusterIndex, uint32* puiSector, uint32* puiOffsetInSector, bool* pbFat12OddClusterBeingProcessed, bool* pbFat12MultiStepProgress)
{
	SFatCache	sBuffer;
	uint8*		puiBuffer;

	READ_SECTOR(sBuffer, *puiSector);
	puiBuffer = sBuffer.Get();

	if (!*pbFat12MultiStepProgress)
	{
		*pbFat12OddClusterBeingProcessed = *puiClusterIndex & 1;

		*puiClusterIndex = 0;
		((uint8*)&puiClusterIndex)[0] = puiBuffer[*puiOffsetInSector];
	}

	if (*puiOffsetInSector == GetSectorSize() - 1)
	{
		(*puiSector)++;
		*puiOffsetInSector = 0;
		*pbFat12MultiStepProgress = true;

		return FAT_SUCCESS;
	}
	else if (!*pbFat12MultiStepProgress)
	{
		(*puiOffsetInSector)++;
	}

	((uint8*)&puiClusterIndex)[1] = puiBuffer[*puiOffsetInSector];

	if (*pbFat12OddClusterBeingProcessed)
	{
		*puiClusterIndex >>= 4;
	}
	else
	{
		*puiClusterIndex &= FAT12_CLUSTER_MASK;
	}

	*pbFat12MultiStepProgress = false;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::IncreaseFat16ClusterAddress(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector)
{
	SFatCache	sBuffer;
	uint8*		puiBuffer;

	READ_SECTOR(sBuffer, uiSector);
	puiBuffer = sBuffer.Get();

	*puiClusterIndex = *((uint16*)&puiBuffer[uiOffsetInSector]);
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::IncreaseFat32ClusterAddress(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector)
{
	SFatCache	sBuffer;
	uint8*		puiBuffer;

	READ_SECTOR(sBuffer, uiSector);
	puiBuffer = sBuffer.Get();

	*puiClusterIndex = *((uint32*)&puiBuffer[uiOffsetInSector]) & FAT32_CLUSTER_MASK;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::SeekByClusterCount(uint32 uiClusterIndex, uint16 uiClusterCount, uint32* puiNewClusterIndex)
{
	uint32		uiOffsetInSector;
	uint32		uiFirstClusterSector;
	uint32		uiCurrentSector;
	bool		bFat12OddClusterBeingProcessed;
	bool		bFat12MultiStepProgress;
	EFatCode	eResult;

	if (uiClusterCount == 0)
	{
		*puiNewClusterIndex = uiClusterIndex;
		return FAT_SUCCESS;
	}

	CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiFirstClusterSector);

	bFat12OddClusterBeingProcessed = false;
	bFat12MultiStepProgress = false;

	for (;;)
	{
		uiCurrentSector = uiFirstClusterSector;

		while (uiCurrentSector == uiFirstClusterSector)
		{
			if (uiClusterIndex < 2)
			{
				return FAT_INVALID_CLUSTER;
			}

			switch (meFileSystem)
			{
				case FAT_FS_TYPE_FAT12:
				{
					eResult = IncreaseFat12ClusterAddress(&uiClusterIndex, &uiFirstClusterSector, &uiOffsetInSector, &bFat12OddClusterBeingProcessed, &bFat12MultiStepProgress);
					RETURN_ON_FAT_FAILURE(eResult);
					if (uiOffsetInSector != 0)
					{
						break;
					}
					else
					{
						continue;
					}
				}
				case FAT_FS_TYPE_FAT16:
				{
					eResult = IncreaseFat16ClusterAddress(&uiClusterIndex, uiFirstClusterSector, uiOffsetInSector);
					RETURN_ON_FAT_FAILURE(eResult);
					break;
				}
				case FAT_FS_TYPE_FAT32:
				{
					eResult = IncreaseFat32ClusterAddress(&uiClusterIndex, uiFirstClusterSector, uiOffsetInSector);
					RETURN_ON_FAT_FAILURE(eResult);
					break;
				}
			}

			if (FatIsEOFEntry(uiClusterIndex))
			{
				return FAT_UNKNOWN_ERROR;
			}

			uiClusterCount--;
			if (uiClusterCount == 0)
			{
				*puiNewClusterIndex = (uint32)uiClusterIndex;
				return FAT_SUCCESS;
			}

			CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiFirstClusterSector);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FreeDirectoryEntry(SFatDirectoryEntry* psEntry)
{
	uint32		uiCluster;
	EFatCode	eResult;
	SFatCache	sBuffer;

	uiCluster = GetFirstClusterFromFatEntry(&psEntry->sRaw, meFileSystem == FAT_FS_TYPE_FAT32);

	if (uiCluster != 0)
	{
		psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterLowWord = 0;
		psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterHighWord = 0;
		psEntry->sRaw.uEntry.sFatRawCommon.uiSize = 0x0;
		FillDirectoryEntryFromRawEntry(psEntry, &psEntry->sRaw);


		READ_SECTOR(sBuffer, psEntry->uiSectorAddress);

		memcpy(sBuffer.Get() + psEntry->uiSectorOffset, &psEntry->sRaw, sizeof(SFatRawDirectoryEntry));
		DirtySector(sBuffer);

		eResult = FreeChain(uiCluster);
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
void CFatVolume::CalculateFATIndexAndOffset(uint32* puiOffsetInSector, uint32 uiClusterIndex, uint32* puiSector)
{
	*puiOffsetInSector = CalculateFATEntryIndex(uiClusterIndex);
	*puiSector = GetNoOfReservedSectors() + (*puiOffsetInSector / GetSectorSize());
	*puiOffsetInSector = *puiOffsetInSector % GetSectorSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::CalculateFATEntryIndex(uint32 uiClusterIndexInFAT)
{
	switch (meFileSystem)
	{
		case FAT_FS_TYPE_FAT12:
			return uiClusterIndexInFAT + (uiClusterIndexInFAT >> 1);

		case FAT_FS_TYPE_FAT16:
			return uiClusterIndexInFAT * 2;

		case FAT_FS_TYPE_FAT32:
			return uiClusterIndexInFAT * 4;
	}

	return 0xFFFFFFFF;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFatCache CFatVolume::ReadSector(uint32 uiFatInfoSector, bool bLock)
{
	SFatCache sSectorCache;

	sSectorCache = mcSectorCache.ReadSector(uiFatInfoSector);
	if ((sSectorCache.pvCachedSector != NULL) && bLock)
	{
		mcSectorCache.Lock(sSectorCache);
	}
	return sSectorCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::DirtySector(SFatCache sCachedSector)
{
	mcSectorCache.Dirty(sCachedSector);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CFatVolume::FatLongEntryChecksum(char* szFilename)
{
	uint16	len;
	uint8	sum = 0;

	for (len = 11; len != 0; len--)
	{
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *szFilename++;
	}
	return sum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::GetPageSize(void)
{
	return mpcDevice->GetPageSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::FatIsEOFEntry(fatEntry uiFat)
{
	switch (meFileSystem)
	{
		case FAT_FS_TYPE_FAT12:
			return uiFat >= 0x0FF8;

		case FAT_FS_TYPE_FAT16:
			return uiFat >= 0xFFF8;

		case FAT_FS_TYPE_FAT32:
			return uiFat >= 0x0FFFFFF8;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::CalculateRootSector(void)
{
	if (meFileSystem == FAT_FS_TYPE_FAT32)
	{
		return CalculateFirstSectorOfCluster(muiRootCluster);
	}
	else
	{
		return muiNoOfReservedSectors + (muiNoOfFatTables * muiFatSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::CalculateFirstSectorOfCluster(uint32 uiCluster)
{
	return (((uiCluster - 2) * muiNoOfSectorsPerCluster) + muiFirstDataSector);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::ReadFat12Entry(uint32 uiOffsetInSector, uint32 uiCluster, uint32 uiSector, fatEntry* puiFatEntry)
{
	fatEntry	uiFatEntry;
	bool		bNextSector;
	SFatCache	sBuffer;

	uiFatEntry = 0;

	READ_SECTOR(sBuffer, uiSector);

	// read the 1st byte
	((uint8*)&uiFatEntry)[INT32_BYTE0] = sBuffer.Get()[uiOffsetInSector];

	// load the next sector (if necessary) and set the offset for the next byte in the uiBuffer
	if (uiOffsetInSector == GetSectorSize() - 1)
	{
		READ_SECTOR(sBuffer, uiSector + 1);
		bNextSector = true;
	}
	else
	{
		bNextSector = false;
	}

	// read the 2nd byte
	((uint8*)&uiFatEntry)[INT32_BYTE1] = sBuffer.Get()[bNextSector ? 0 : (uiOffsetInSector + 1)];

	// Since a FAT12 entry is only 12 bits (1.5 bytes) we need to adjust the peResult.  For odd cluster numbers the FAT entry is stored in the upper 12 bits of the
	// 16 bits where it is stored, so we need to shift the value 4 bits to the right. For even cluster numbers the FAT entry is stored in the lower 12 bits of the
	// 16 bits where it is stored, so we need to clear the upper 4 bits.
	if (uiCluster & 0x1)
	{
		uiFatEntry >>= 4;
	}
	else
	{
		uiFatEntry &= FAT12_CLUSTER_MASK;
	}

	*puiFatEntry = uiFatEntry;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::ReadFat16Entry(uint32 uiOffsetInSector, uint32 uiSector, fatEntry* puiFatEntry)
{
	SFatCache	sBuffer;

	READ_SECTOR(sBuffer, uiSector);

	*puiFatEntry = *((uint16*)&(sBuffer.Get()[uiOffsetInSector]));
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::ReadFat32Entry(uint32 uiOffsetInSector, uint32 uiSector, fatEntry* puiFatEntry)
{
	SFatCache	sBuffer;

	READ_SECTOR(sBuffer, uiSector);
	*puiFatEntry = *((uint32*)&(sBuffer.Get()[uiOffsetInSector])) & FAT32_CLUSTER_MASK;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::ReadFatEntry(uint32 uiOffsetInSector, uint32 uiClusterIndex, uint32 uiSector, fatEntry* puiFatEntry)
{
	EFatCode	eResult;

	switch (meFileSystem)
	{
		case FAT_FS_TYPE_FAT12:
		{
			eResult = ReadFat12Entry(uiOffsetInSector, uiClusterIndex, uiSector, puiFatEntry);
			break;
		}
		case FAT_FS_TYPE_FAT16:
		{
			eResult = ReadFat16Entry(uiOffsetInSector, uiSector, puiFatEntry);
			break;
		}
		case FAT_FS_TYPE_FAT32:
		{
			eResult = ReadFat32Entry(uiOffsetInSector, uiSector, puiFatEntry);
			break;
		}
	}
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FreeFat12Chain(bool* pbFat12MultiStepProgress, uint32* puiFreedClusterIndex, uint32* puiSector, uint32* puiOffsetInSector)
{
	SFatCache	sBuffer;
	uint8*		puiBuffer;

	READ_SECTOR(sBuffer, *puiSector);
	puiBuffer = sBuffer.Get();

	if (!*pbFat12MultiStepProgress)
	{
		// remember whether this is an odd cluster or not
		*pbFat12MultiStepProgress = *puiFreedClusterIndex & 1;

		// set the cluster to zero to make sure that the upper bytes are cleared
		// since we're only updating the lower 16 bits.
		*puiFreedClusterIndex = 0;

		// read the 1st byte
		((uint8*)puiFreedClusterIndex)[0] = puiBuffer[*puiOffsetInSector];

		// write the 1st byte
		//
		// note: since the value that we're writting is FREE_FAT which expands
		// to zero we can skip a step or two in the update process. I left the code
		// for the steps that we're skipping commented out for clarity.
		if (*pbFat12MultiStepProgress)
		{
			puiBuffer[*puiOffsetInSector] &= 0x0F;
		}
		else
		{
			puiBuffer[*puiOffsetInSector] = FREE_FAT;
		}
		DirtySector(sBuffer);
	}

	if (*puiOffsetInSector == GetSectorSize() - 1)
	{
		// if the entry spans a sector boundary set bFat12MultiStepProgress to 1 so that we don't read the 1st byte again when we come back.
		// also increase the sector number and set the uiOffsetInSector to 0 since the next byte will be on offset zero when the next sector is loaded.
		(*puiSector)++;
		*puiOffsetInSector = 0;
		*pbFat12MultiStepProgress = true;

		// continue with the next iteration of the loop. We'll come right back here with the next sector loaded
		return FAT_SUCCESS;
	}
	else if (!*pbFat12MultiStepProgress)
	{
		// increase the offset to point to the next byte
		(*puiOffsetInSector)++;
	}

	// read the 2nd byte
	((uint8*)puiFreedClusterIndex)[1] = puiBuffer[*puiOffsetInSector];

	// Since a FAT12 entry is only 12 bits (1.5 bytes) we need to adjust the result.  For odd cluster numbers the FAT entry is stored in the upper 12 bits of the
	// 16 bits where it is stored, so we need to shift the value 4 bits to the right.  For even cluster numbers the FAT entry is stored in the lower 12 bits of the
	// 16 bits where it is stored, so we need to clear the upper 4 bits.
	if (*pbFat12MultiStepProgress)
	{
		*puiFreedClusterIndex >>= 4;
	}
	else
	{
		*puiFreedClusterIndex &= 0xFFF;
	}

	// write the 2nd byte
	//
	// note: since the value that we're writting is FREE_FAT which expands
	// to zero we can skip a step or two in the update process. I left the code
	// for the steps that we're skipping commented out for clarity.
	//
	// note: we could bundle this with the above if...else statement but
	// since that is part of the read operation and this is the write operation
	// i've decided to keep it separate for code clarity. maybe the compiler
	// can optimize this for us??
	if (*pbFat12MultiStepProgress)
	{
		puiBuffer[*puiOffsetInSector] = FREE_FAT;
	}
	else
	{
		puiBuffer[*puiOffsetInSector] &= 0xF0;	/* clear bits that 1st byte will be written to */
	}
	DirtySector(sBuffer);

	// clear bFat12MultiStepProgress flag.
	*pbFat12MultiStepProgress = false;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FreeFat16Chain(uint32* puiFreedClusterIndex, uint32 uiSector, uint32 uiOffsetInSector)
{
	SFatCache	sBuffer;

	READ_SECTOR(sBuffer, uiSector);

	*puiFreedClusterIndex = *((uint16*)&(sBuffer.Get()[uiOffsetInSector]));

	*((uint16*)&(sBuffer.Get()[uiOffsetInSector])) = FREE_FAT;
	DirtySector(sBuffer);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FreeFat32Chain(uint32* puiFreedClusterIndex, uint32 uiSector, uint32 uiOffsetInSector)
{
	SFatCache	sBuffer;

	READ_SECTOR(sBuffer, uiSector);

	*puiFreedClusterIndex = *((uint32*)&(sBuffer.Get()[uiOffsetInSector])) & FAT32_CLUSTER_MASK;

	*((uint32*)&(sBuffer.Get()[uiOffsetInSector])) &= (~FAT32_CLUSTER_MASK);
	DirtySector(sBuffer);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FreeChain(uint32 uiClusterIndex)
{
	EFatCode	eResult;
	uint32		uiOffsetInSector;						
	uint32		uiFirstClusterSector;					
	uint32		uiCurrentSector;						
	bool		bFat12OddClusterBeingProcessed = false;	
	bool		bFat12MultiStepProgress = false;		

	CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiFirstClusterSector);

	for (;;)
	{
		uiCurrentSector = uiFirstClusterSector;

		while (uiCurrentSector == uiFirstClusterSector)
		{
			if (uiClusterIndex < 2)
			{
				return FAT_INVALID_CLUSTER;
			}

			switch (meFileSystem)
			{
				case FAT_FS_TYPE_FAT12:
				{
					eResult = FreeFat12Chain(&bFat12OddClusterBeingProcessed, &uiClusterIndex, &uiFirstClusterSector, &uiOffsetInSector);
					RETURN_ON_FAT_FAILURE(eResult);
					if (uiOffsetInSector != 0)
					{
						break;
					}
					else
					{
						continue;
					}
				}
				case FAT_FS_TYPE_FAT16:
				{
					eResult = FreeFat16Chain(&uiClusterIndex, uiFirstClusterSector, uiOffsetInSector);
					RETURN_ON_FAT_FAILURE(eResult);
					break;
				}
				case FAT_FS_TYPE_FAT32:
				{
					eResult = FreeFat32Chain(&uiClusterIndex, uiFirstClusterSector, uiOffsetInSector);
					RETURN_ON_FAT_FAILURE(eResult);
					break;
				}
			}

			SetTotalFreeClusters(GetTotalFreeClusters() + 1);

			if (FatIsEOFEntry(uiClusterIndex))
			{
				return FAT_SUCCESS;
			}

			CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiFirstClusterSector);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::IsFreeFat(fatEntry uifat)
{
	switch (meFileSystem)
	{
		case FAT_FS_TYPE_FAT32:
			return (uifat & FAT32_CLUSTER_MASK) == 0;

		case FAT_FS_TYPE_FAT16:
			return (uifat & FAT16_CLUSTER_MASK) == 0;

		case FAT_FS_TYPE_FAT12:
			return (uifat & FAT12_CLUSTER_MASK) == 0;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetNextFreeCluster(uint32 uiCluster)
{
	muiNextFreeCluster = uiCluster;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetTotalFreeClusters(uint32 muiTotalFreeClusters)
{
	muiTotalFreeClusters = muiTotalFreeClusters;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::CreateFakeRootEntry(SFatDirectoryEntry* psEntry)
{
	uint32	uiRootCluster;

	strcpy((char*)psEntry->szName, "ROOT");
	GetFatShortNameForEntry((char*)psEntry->sRaw.uEntry.sFatRawCommon.szShortName, (char*)psEntry->szName, 1);

	psEntry->uiAttributes = psEntry->sRaw.uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_DIRECTORY;
	psEntry->uiSize = psEntry->sRaw.uEntry.sFatRawCommon.uiSize = 0x0;

	psEntry->uiSectorAddress = 0;
	psEntry->uiSectorOffset = 0;

	uiRootCluster = GetRootCluster();
	psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterLowWord = (uint16)uiRootCluster;
	psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterHighWord = (uint16)(uiRootCluster >> 16);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FindNextPathItem(char* szPath, char* szCurrentLevelPath)
{
	char* szPathLevel;

	szPathLevel = szCurrentLevelPath;

	szPath = StepPathOverSlash(szPath);

	uint16 uiCount = 0;
	while (*szPath != '\0' && *szPath != '\\')
	{
		if (uiCount++ > FAT_MAX_PATH)
		{
			return NULL;
		}
		*szPathLevel++ = *szPath++;
	}
	*szPathLevel = NULL;

	return szPath;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::GetRootDirectorySector(void)
{
	return GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::HasNoMoreEntries(SFatFileSystemQuery* psQuery)
{
	return psQuery->sQuery.psCurrentEntryRaw == NULL;
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::TerminalQueryEntry(SFatFileSystemQuery* psQuery)
{
	psQuery->sCurrentEntry.szName[0] = '\0';
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::CalculateQuerySector(SFatFileSystemQuery* psQuery)
{
	if (IsRootDirectoryCluster(psQuery->sQuery.uiCurrentCluster))
	{
		return GetRootDirectorySector() + psQuery->sQuery.uiCurrentSector;
	}
	else
	{
		return CalculateFirstSectorOfCluster(psQuery->sQuery.uiCurrentCluster) + psQuery->sQuery.uiCurrentSector;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::CalculateQuerySectorOffset(SFatFileSystemQuery* psQuery)
{
	return (uint16)((uintptr_t)psQuery->sQuery.psCurrentEntryRaw) - ((uintptr_t)psQuery->sQuery.psFirstEntryRaw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FindFirstFATEntry(char* szParentPath, uint8 uiAttributes, SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* psQuery)
{
	EFatCode			eResult;
	SFatDirectoryEntry	sParentEntry;

	szParentPath = StepPathOverSlash(szParentPath);
	
	if (szParentPath != NULL)
	{
		eResult = GetFileEntry(szParentPath, &sParentEntry);
		RETURN_ON_FAT_FAILURE(eResult);

		eResult = QueryFirstEntry(&sParentEntry.sRaw, uiAttributes, &psQuery->sQuery, 0);
		RETURN_ON_FAT_FAILURE(eResult);
	}
	else
	{
		eResult = QueryFirstEntry(0, uiAttributes, &psQuery->sQuery, 0);
		RETURN_ON_FAT_FAILURE(eResult);
	}

	if (HasNoMoreEntries(psQuery))
	{
		return TerminalQueryEntry(psQuery);
	}

	FillDirectoryEntryFromRawEntry(&psQuery->sCurrentEntry, psQuery->sQuery.psCurrentEntryRaw);

	psQuery->sCurrentEntry.uiSectorAddress = CalculateQuerySector(psQuery);
	psQuery->sCurrentEntry.uiSectorOffset = CalculateQuerySectorOffset(psQuery);
	psQuery->sCurrentEntry.sRaw = *psQuery->sQuery.psCurrentEntryRaw;

	CopyLongFilenameIntoString((char*)psQuery->sCurrentEntry.szName, psQuery->sQuery.auiLongFilename);

	SafeAssign(ppsDirectoryEntry, &psQuery->sCurrentEntry);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FindNextFATEntry(SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* psQuery)
{
	EFatCode				eResult;

	eResult = QueryNextEntry(&psQuery->sQuery, false, false);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	if (HasNoMoreEntries(psQuery))
	{
		return TerminalQueryEntry(psQuery);
	}


	FillDirectoryEntryFromRawEntry(&psQuery->sCurrentEntry, psQuery->sQuery.psCurrentEntryRaw);

	psQuery->sCurrentEntry.uiSectorAddress = CalculateQuerySector(psQuery);
	psQuery->sCurrentEntry.uiSectorOffset = CalculateQuerySectorOffset(psQuery);
	psQuery->sCurrentEntry.sRaw = *psQuery->sQuery.psCurrentEntryRaw;

	CopyLongFilenameIntoString((char*)psQuery->sCurrentEntry.szName, psQuery->sQuery.auiLongFilename);

	SafeAssign(ppsDirectoryEntry, &psQuery->sCurrentEntry);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::FillDirectoryEntryFromRawEntry(SFatDirectoryEntry* psEntry, SFatRawDirectoryEntry* psRawEntry)
{
	ConvertFATShortInternalNameInto8Dot3Format(psEntry->szName, (uint8*)psRawEntry->uEntry.sFatRawCommon.szShortName);

	psEntry->uiAttributes = psRawEntry->uEntry.sFatRawCommon.uiAttributes;
	psEntry->uiSize = psRawEntry->uEntry.sFatRawCommon.uiSize;
	psEntry->tCreateTime = FatDecodeDateTime(psRawEntry->uEntry.sFatRawCommon.uiCreateDate, psRawEntry->uEntry.sFatRawCommon.uiCreateTime);
	psEntry->tModifyTime = FatDecodeDateTime(psRawEntry->uEntry.sFatRawCommon.uiModifyDate, psRawEntry->uEntry.sFatRawCommon.uiModifyTime);
	psEntry->tAccessTime = FatDecodeDateTime(psRawEntry->uEntry.sFatRawCommon.uiAccessDate, 0);
	psEntry->sRaw = *psRawEntry;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::GenerateUniqueShortNameWithSuffix(SFatRawDirectoryEntry* psParentDirectory, uint8* szShortName)
{
	SFatQueryState	sQuery;
	uint16			uiNameSuffix;
	bool			bIsValidEntry;
	EFatCode		eResult;

	uiNameSuffix = 0;
	sQuery.Init();
	do
	{
		bIsValidEntry = true;

		eResult = QueryFirstEntry(psParentDirectory, 0, &sQuery, 0);
		if (eResult != FAT_SUCCESS)
		{
			break;
		}

		GenerateShortNameWithSuffix(uiNameSuffix, szShortName);
		uiNameSuffix++;

		while (sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[0] != '\0')
		{
			if (memcmp(sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName, szShortName, 11) == 0)
			{
				bIsValidEntry = false;
				break;
			}
			eResult = QueryNextEntry(&sQuery, false, false);
			if (eResult != FAT_SUCCESS)
			{
				break;
			}
		}
		if (eResult != FAT_SUCCESS)
		{
			break;
		}

		if (!bIsValidEntry)
		{
			GenerateShortNameWithSuffix(uiNameSuffix, szShortName);
			uiNameSuffix++;
		}
	} 
	while (!bIsValidEntry);

	sQuery.Kill(GetSectorCache());
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::InitialiseNewEntry(SFatDirectoryEntry* psNewEntry, char* szName, uint8 uiAttributes, uint32 uiEntryCluster)
{
	strcpy((char*)psNewEntry->szName, szName);
	psNewEntry->uiAttributes = uiAttributes;
	psNewEntry->uiSize = 0;
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiAttributes = uiAttributes;
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiReserved = 0;
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiSize = 0;
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterLowWord = (uint16)uiEntryCluster;
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterHighWord = (uint16)(uiEntryCluster >> 16);
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiCreateTimeTenths = 0;
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiCreateDate = GetSystemClockDate();
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiCreateTime = GetSystemClockTime();
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiModifyDate = psNewEntry->sRaw.uEntry.sFatRawCommon.uiCreateDate;
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiModifyTime = psNewEntry->sRaw.uEntry.sFatRawCommon.uiCreateTime;
	psNewEntry->sRaw.uEntry.sFatRawCommon.uiAccessDate = psNewEntry->sRaw.uEntry.sFatRawCommon.uiCreateDate;
	psNewEntry->tCreateTime = FatDecodeDateTime(psNewEntry->sRaw.uEntry.sFatRawCommon.uiCreateDate, psNewEntry->sRaw.uEntry.sFatRawCommon.uiCreateTime);
	psNewEntry->tModifyTime = FatDecodeDateTime(psNewEntry->sRaw.uEntry.sFatRawCommon.uiModifyDate, psNewEntry->sRaw.uEntry.sFatRawCommon.uiModifyTime);
	psNewEntry->tAccessTime = FatDecodeDateTime(psNewEntry->sRaw.uEntry.sFatRawCommon.uiAccessDate, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::LongNameCharToWideChar(uint16 uiLFNIndex, uint16 uiNameLength, char* szName, uint16 uiNameIndex)
{
	if (uiLFNIndex + uiNameIndex > uiNameLength)
	{
		return FAT_LONG_NAME_END_WCHAR;
	}
	else
	{
		return (uint16)szName[uiLFNIndex + uiNameIndex];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::GenerateDirectoryEntryNameParts(uint8* aui, uint16 uiLFNIndex, uint16 uiNameLength, char* szName, uint16 uiNameIndex)
{
	uint16	uiWideChar;

	uiWideChar = LongNameCharToWideChar(uiLFNIndex, uiNameLength, szName, uiNameIndex);
	aui[0x0] = (uint8)uiWideChar;
	aui[0x1] = (uint8)(uiWideChar >> 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::InitialiseParentEntry(SFatRawDirectoryEntry* psParentEntry, char* szName, char uiChecksum, int iLFNEntriesNeeded, int iLFNEntriesFound)
{
	uint16 uiLFNIndex;
	uint16 uiNameLength;

	// set the required fields for this entry
	psParentEntry->uEntry.sFatRawLongFileName.uiChecksum = uiChecksum;
	psParentEntry->uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_LONG_NAME;
	psParentEntry->uEntry.sFatRawLongFileName.uiFirstCluster = 0;
	psParentEntry->uEntry.sFatRawLongFileName.uiType = 0;

	// mark entry as the 1st entry if it is so.
	if (iLFNEntriesFound == iLFNEntriesNeeded - 1)
	{
		psParentEntry->uEntry.sFatRawLongFileName.uiSequence = (uint8)iLFNEntriesFound | FAT_FIRST_LFN_ENTRY;
	}
	else
	{
		psParentEntry->uEntry.sFatRawLongFileName.uiSequence = (uint8)iLFNEntriesFound;
	}

	// copy the lfn chars
	uiNameLength = (uint16)strlen(szName);
	uiLFNIndex = ((iLFNEntriesFound - 1) * 13);
	
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x0], uiLFNIndex, uiNameLength, szName, 0x0);
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x2], uiLFNIndex, uiNameLength, szName, 0x1);
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x4], uiLFNIndex, uiNameLength, szName, 0x2);
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x6], uiLFNIndex, uiNameLength, szName, 0x3);
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x8], uiLFNIndex, uiNameLength, szName, 0x4);
	
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x0], uiLFNIndex, uiNameLength, szName, 0x5);
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x2], uiLFNIndex, uiNameLength, szName, 0x6);
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x4], uiLFNIndex, uiNameLength, szName, 0x7);
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x6], uiLFNIndex, uiNameLength, szName, 0x8);
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x8], uiLFNIndex, uiNameLength, szName, 0x9);
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0xA], uiLFNIndex, uiNameLength, szName, 0xA);
	
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars3[0x0], uiLFNIndex, uiNameLength, szName, 0xB);
	GenerateDirectoryEntryNameParts(&psParentEntry->uEntry.sFatRawLongFileName.auiChars3[0x2], uiLFNIndex, uiNameLength, szName, 0xC);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::ValidateFatCache(SFatCache sCache)
{
	if (mcSectorCache.ValidateTimeStamp(sCache))
	{
		return FAT_SUCCESS;
	}
	else
	{
		return FAT_INVALID_FAT_CACHE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
fatEntry CFatVolume::GetDirectoryCluster(SFatRawDirectoryEntry* psParentDirectory)
{
	fatEntry	uiDirectoryCluster;

	if ((psParentDirectory != NULL) && (psParentDirectory->uEntry.sFatRawCommon.uiFirstClusterLowWord != 0 || psParentDirectory->uEntry.sFatRawCommon.uiFirstClusterHighWord != 0))
	{
		uiDirectoryCluster = GetFirstClusterFromFatEntry(psParentDirectory, meFileSystem == FAT_FS_TYPE_FAT32);
	}
	else
	{
		uiDirectoryCluster = GetRootCluster();
	}

	return uiDirectoryCluster;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::GetDirectorySector(SFatRawDirectoryEntry* psParentDirectory)
{
	uint32	uiFirstSectorOfCluster;

	if ((psParentDirectory != NULL) && (psParentDirectory->uEntry.sFatRawCommon.uiFirstClusterLowWord != 0 || psParentDirectory->uEntry.sFatRawCommon.uiFirstClusterHighWord != 0))
	{
		uiFirstSectorOfCluster = 0;
	}
	else
	{
		uiFirstSectorOfCluster = GetRootSector();
	}

	return uiFirstSectorOfCluster;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FindEnoughEntries(fatEntry* puiLastDirectoryCluster, fatEntry* puiDirectoryCluster, uint32* puiFirstSectorOfCluster, uint32* puiSector, int* piLFNEntriesFound, SFatRawDirectoryEntry** ppsParentEntry, SFatRawDirectoryEntry* psParentDirectory, int iLFNEntriesNeeded)
{
	uint32					uiFirstSectorOfCluster;
	fatEntry				uiDirectoryCluster;
	EFatCode				eResult;
	uint32					uiSector;
	int						iLFNEntriesFound;
	bool					bAllLFNEntriesFound;
	SFatCache				sBuffer;
	SFatRawDirectoryEntry*	psParentEntry;
	uintptr_t				puiLastEntryAddress;
	uint16					uiDirectoryEntries;

	uiDirectoryCluster = GetDirectoryCluster(psParentDirectory);
	uiFirstSectorOfCluster = GetDirectorySector(psParentDirectory);

	uiDirectoryEntries = 0;
	iLFNEntriesFound = 0;
	bAllLFNEntriesFound = false;
	sBuffer.Clear();
	psParentEntry = NULL;

	for (;;)
	{
		if (!IsRootDirectoryCluster(uiDirectoryCluster))
		{
			uiFirstSectorOfCluster = CalculateFirstSectorOfCluster(uiDirectoryCluster);
		}

		uiSector = uiFirstSectorOfCluster;

		while (IsRootDirectoryCluster(uiDirectoryCluster) || uiSector < (uiFirstSectorOfCluster + NumSectorsPerCluster()))
		{
			READ_SECTOR(sBuffer, uiSector);
			puiLastEntryAddress = ((uintptr_t)sBuffer.Get() + GetSectorSize()) - 0x20;
			psParentEntry = (SFatRawDirectoryEntry*)sBuffer.Get();

			while ((uintptr_t)psParentEntry <= puiLastEntryAddress)
			{
				if ((uiDirectoryEntries + iLFNEntriesNeeded) == FAT_MAX_DIRECTORY_ENTRIES)
				{
					return FAT_DIRECTORY_LIMIT_EXCEEDED;
				}

				uiDirectoryEntries++;

				if (IsFreeDirectoryEntry(psParentEntry))
				{
					iLFNEntriesFound++;

					if (IsLastDirectoryEntry(psParentEntry) || iLFNEntriesFound == iLFNEntriesNeeded)
					{
						bAllLFNEntriesFound = true;
						break;
					}
				}
				else
				{
					iLFNEntriesFound = 0;
				}

				psParentEntry++;
			}

			if (bAllLFNEntriesFound)
			{
				break;
			}

			uiSector++;

			if (IsRootDirectoryCluster(uiDirectoryCluster))
			{
				if (uiSector > uiFirstSectorOfCluster + GetRootDirectorySectors())
				{
					return FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED;
				}
			}
		}

		if (bAllLFNEntriesFound)
		{
			break;
		}

		*puiLastDirectoryCluster = uiDirectoryCluster;
		eResult = GetNextClusterEntry(uiDirectoryCluster, &uiDirectoryCluster);
		RETURN_ON_FAT_FAILURE(eResult);

		if (FatIsEOFEntry(uiDirectoryCluster))
		{
			fatEntry uiNewFat;

			uiNewFat = AllocateDataCluster(1, true, &eResult, GetEndOfClusterMarker());  // GetEndOfClusterMarker() ?  Should this be the actual previous cluster?
			RETURN_ON_FAT_FAILURE(eResult);

			eResult = SetClusterEntry(*puiLastDirectoryCluster, uiNewFat);
			RETURN_ON_FAT_FAILURE(eResult);

			uiDirectoryCluster = uiNewFat;
		}
	}

	*puiDirectoryCluster = uiDirectoryCluster;
	*ppsParentEntry = psParentEntry;
	*puiSector = uiSector;
	*piLFNEntriesFound = iLFNEntriesFound;
	*puiFirstSectorOfCluster = uiFirstSectorOfCluster;

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::RewindFoundEntries(SFatRawDirectoryEntry** ppsParentEntry, uint32* puiFirstSectorOfCluster, fatEntry* puiDirectoryCluster, int iLFNEntriesFound, fatEntry uiLastDirectoryCluster, uint32 uiSector)
{
	uintptr_t				puiLastEntryAddress;
	SFatCache				sBuffer;
	SFatRawDirectoryEntry*	psParentEntry;
	uint32					uiFirstSectorOfCluster;
	fatEntry				uiDirectoryCluster;

	uiFirstSectorOfCluster = *puiFirstSectorOfCluster;
	uiDirectoryCluster = *puiDirectoryCluster;
	psParentEntry = *ppsParentEntry;

	while (iLFNEntriesFound > 1)
	{
		READ_SECTOR(sBuffer, uiSector);
		puiLastEntryAddress = ((uintptr_t)sBuffer.Get() + GetSectorSize()) - 0x20;

		if ((uintptr_t)psParentEntry > (uintptr_t)sBuffer.Get())
		{
			psParentEntry--;
		}
		else
		{
			if (uiSector > uiFirstSectorOfCluster)
			{
				uiSector--;
			}
			else
			{
				if (IsRootDirectoryCluster(uiLastDirectoryCluster))
				{
					uiFirstSectorOfCluster = GetRootDirectorySector();
				}
				else
				{
					uiDirectoryCluster = uiLastDirectoryCluster;
					uiFirstSectorOfCluster = CalculateFirstSectorOfCluster(uiDirectoryCluster);
				}
				uiSector = uiFirstSectorOfCluster + NumSectorsPerCluster();
			}

			READ_SECTOR(sBuffer, uiSector);

			puiLastEntryAddress = ((uintptr_t)sBuffer.Get() + GetSectorSize()) - 0x20;
		  //psParentEntry = (SFatRawDirectoryEntry*)sBuffer.Get();  // ?? Isn't this more correct?
			psParentEntry = (SFatRawDirectoryEntry*)puiLastEntryAddress;
		}
		iLFNEntriesFound--;
	}

	*ppsParentEntry = psParentEntry;
	*puiFirstSectorOfCluster = uiFirstSectorOfCluster;
	*puiDirectoryCluster = uiDirectoryCluster;

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::CheckEntryName(char* szName)
{
	uint16					uiLength;

	uiLength = (uint16)strlen(szName);

	if (uiLength > FAT_MAX_FILENAME)
	{
		return FAT_FILENAME_TOO_LONG;
	}

	if (IsIllegalFilename(szName, uiLength))
	{
		return FAT_INVALID_FILENAME;
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::IncrementDirectorySector(uint32* puiSector, uint32 uiFirstSectorOfCluster, fatEntry uiDirectoryCluster)
{
	(*puiSector)++;

	if (IsRootDirectoryCluster(uiDirectoryCluster))
	{
		if ((*puiSector) > uiFirstSectorOfCluster + GetRootDirectorySectors())
		{
			return FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED;
		}
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::IncrementOrAllocateDirectoryCluster(uint32* puiSector, fatEntry* puiDirectoryCluster, fatEntry uiLastDirectoryCluster)
{
	EFatCode	eResult;

	eResult = GetNextClusterEntry(*puiDirectoryCluster, puiDirectoryCluster);
	RETURN_ON_FAT_FAILURE(eResult);

	if (FatIsEOFEntry(*puiDirectoryCluster))
	{
		fatEntry uiNewFat = AllocateDataCluster(1, true, &eResult, uiLastDirectoryCluster);
		RETURN_ON_FAT_FAILURE(eResult);

		eResult = SetClusterEntry(uiLastDirectoryCluster, uiNewFat);
		RETURN_ON_FAT_FAILURE(eResult);

		*puiDirectoryCluster = uiNewFat;
	}

	*puiSector = CalculateFirstSectorOfCluster(*puiDirectoryCluster);
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteFATEntry(SFatDirectoryEntry* psNewEntry, char* szName, SFatRawDirectoryEntry* psParentDirectory, int iLFNEntriesNeeded)
{
	EFatCode				eResult;
	uint32					uiSector;
	int						iLFNEntriesFound;
	uint32					uiFirstSectorOfCluster;
	fatEntry				uiDirectoryCluster;
	fatEntry				uiLastDirectoryCluster;
	char					uiChecksum;
	SFatCache				sBuffer;
	uintptr_t				puiLastEntryAddress;
	SFatRawDirectoryEntry*	psParentEntry;

	iLFNEntriesFound = 0;
	uiSector = 0;
	uiDirectoryCluster = 0;
	uiFirstSectorOfCluster = 0;
	uiLastDirectoryCluster = 0;

	eResult = FindEnoughEntries(&uiLastDirectoryCluster, &uiDirectoryCluster, &uiFirstSectorOfCluster, &uiSector, &iLFNEntriesFound, &psParentEntry, psParentDirectory, iLFNEntriesNeeded);
	RETURN_ON_FAT_FAILURE(eResult);

	eResult = RewindFoundEntries(&psParentEntry, &uiFirstSectorOfCluster, &uiDirectoryCluster, iLFNEntriesFound, uiLastDirectoryCluster, uiSector);
	RETURN_ON_FAT_FAILURE(eResult);

	uiChecksum = FatLongEntryChecksum((char*)psNewEntry->sRaw.uEntry.sFatRawCommon.szShortName);

	iLFNEntriesFound = iLFNEntriesNeeded;
	while (iLFNEntriesFound--)
	{
		if (iLFNEntriesFound > 0)
		{
			READ_SECTOR(sBuffer, uiSector);
			puiLastEntryAddress = ((uintptr_t)sBuffer.Get() + GetSectorSize()) - 0x20;

			InitialiseParentEntry(psParentEntry, szName, uiChecksum, iLFNEntriesNeeded, iLFNEntriesFound);
			DirtySector(sBuffer);

			if ((uintptr_t)psParentEntry < (uintptr_t)puiLastEntryAddress)
			{
				psParentEntry++;
			}
			else
			{
				if (IsRootDirectoryCluster(uiDirectoryCluster) || uiSector < uiFirstSectorOfCluster + NumSectorsPerCluster() - 1)
				{
					eResult = IncrementDirectorySector(&uiSector, uiFirstSectorOfCluster, uiDirectoryCluster);
					RETURN_ON_FAT_FAILURE(eResult);
				}
				else
				{
					uiLastDirectoryCluster = uiDirectoryCluster;
					eResult = IncrementOrAllocateDirectoryCluster(&uiSector, &uiDirectoryCluster, uiLastDirectoryCluster);
					RETURN_ON_FAT_FAILURE(eResult);
				}

				READ_SECTOR(sBuffer, uiSector);

				puiLastEntryAddress = ((uintptr_t)sBuffer.Get() + GetSectorSize()) - 0x20;
				psParentEntry = (SFatRawDirectoryEntry*)sBuffer.Get();
			}
		}
		else
		{
			eResult = ValidateFatCache(sBuffer);
			RETURN_ON_FAT_FAILURE(eResult);

			*psParentEntry = psNewEntry->sRaw;
			DirtySector(sBuffer);
		}
	}

	eResult = ValidateFatCache(sBuffer);
	RETURN_ON_FAT_FAILURE(eResult);

	psNewEntry->uiSectorAddress = uiSector;
	psNewEntry->uiSectorOffset = (uintptr_t)psParentEntry - (uintptr_t)sBuffer.Get();

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::CreateFATEntry(SFatRawDirectoryEntry* psParentDirectory, char* szName, uint8 uiAttributes, uint32 uiEntryCluster, SFatDirectoryEntry* psNewEntry)
{
	EFatCode				eResult;
	int						iLFNEntriesNeeded;

	eResult = CheckEntryName(szName);
	RETURN_ON_FAT_FAILURE(eResult);

	memset(&psNewEntry->sRaw, 0, sizeof(SFatRawDirectoryEntry));

	eResult = GetFatShortNameForEntry((char*)psNewEntry->sRaw.uEntry.sFatRawCommon.szShortName, szName, false);
	if (eResult != FAT_SUCCESS && eResult != FAT_LFN_GENERATED)
	{
		return FAT_INVALID_FILENAME;
	}

	if (eResult == FAT_LFN_GENERATED)
	{
		eResult = GenerateUniqueShortNameWithSuffix(psParentDirectory, psNewEntry->sRaw.uEntry.sFatRawCommon.szShortName);
		RETURN_ON_FAT_FAILURE(eResult);

		iLFNEntriesNeeded = ((strlen(szName) + 12) / 13) + 1;
	}
	else
	{
		iLFNEntriesNeeded = 0;
	}

	if (IsRootDirectoryCluster(uiEntryCluster) && (uiAttributes & FAT_ATTR_DIRECTORY))
	{
		uiEntryCluster = AllocateDirectoryCluster(psParentDirectory, &eResult);
		RETURN_ON_FAT_FAILURE(eResult);
	}

	InitialiseNewEntry(psNewEntry, szName, uiAttributes, uiEntryCluster);

	eResult = WriteFATEntry(psNewEntry, szName, psParentDirectory, iLFNEntriesNeeded);
	return eResult;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::CreateDirectory(char* szDirectory, SFatDirectoryEntry* psEntry)
{
	size_t				uiPathLength;
	char*				szPathScanner;
	char				szFilePath[FAT_MAX_PATH + 1];
	SFatDirectoryEntry	sParentEntry;
	EFatCode			eResult;

	uiPathLength = strlen(szDirectory);

	szPathScanner = szDirectory + (uiPathLength - 0x1);

	if (szPathScanner[0] == '\\')
	{
		return FAT_INVALID_FILENAME;
	}

	szPathScanner = StepPathToSlash(szPathScanner);

	uiPathLength = (size_t)(szPathScanner - szDirectory);

	memcpy(szFilePath, szDirectory, uiPathLength);

	szFilePath[uiPathLength] = '\0';
	szPathScanner++;

	eResult = GetFileEntry(szFilePath, &sParentEntry);
	RETURN_ON_FAT_FAILURE(eResult);

	if (sParentEntry.szName[0] == '\0')
	{
		return FAT_DIRECTORY_DOES_NOT_EXIST;
	}

	eResult = CreateFATEntry(&sParentEntry.sRaw, szPathScanner, FAT_ATTR_DIRECTORY, 0, psEntry);
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::CreateDirectory(char* szDirectory)
{
	EFatCode			eResult;
	SFatDirectoryEntry	sEntry;

	if (StrEmpty(szDirectory) || strlen(szDirectory) > FAT_MAX_PATH)
	{
		return FAT_INVALID_FILENAME;
	}

	eResult = GetFileEntry(szDirectory, &sEntry);
	RETURN_ON_FAT_FAILURE(eResult);

	if (sEntry.szName[0] == '\0')
	{
		eResult = CreateDirectory(szDirectory, &sEntry);
		return eResult;
	}

	return FAT_FILENAME_ALREADY_EXISTS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::GetFileEntry(char* szPath, SFatDirectoryEntry* psEntry)
{
	EFatCode				eResult;
	bool					bMatch;
	char					szConstructedShortFileName[13];
	SFatRawDirectoryEntry*	psCurrentEntry;
	SFatQueryState			sQuery;
	bool					bLongFilename;

	uint16					auiConstructedLongFilename[FAT_MAX_PATH + 1];
	char					szCurrentLevelPath[FAT_MAX_PATH + 1];

	szPath = StepPathOverSlash(szPath);

	if (szPath[0] != '\0')
	{
		psCurrentEntry = NULL;
	}
	else
	{
		return CreateFakeRootEntry(psEntry);
	}

	memset(szCurrentLevelPath, 0, FAT_MAX_PATH + 1);

	// for each level on the path....
	sQuery.Init();
	do
	{
		szPath = FindNextPathItem(szPath, szCurrentLevelPath);
		if (szPath == NULL)
		{
			eResult = FAT_INVALID_FILENAME;
			break;
		}

		// try to find the first psEntry
		eResult = QueryFirstEntry(psCurrentEntry, 0, &sQuery, true);
		if (eResult != FAT_SUCCESS)
		{
			break;
		}

		// if the output of FatQueryFirstEntry indicates that there are no entries available...
		if (*sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName == 0)
		{
			psEntry->szName[0] = '\0';
			sQuery.Kill(GetSectorCache());
			return FAT_SUCCESS;
		}

		eResult = MatchesFatFileName(&bMatch, &bLongFilename, szConstructedShortFileName, auiConstructedLongFilename, szCurrentLevelPath, sQuery.auiLongFilename, sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
		if (eResult != FAT_SUCCESS)
		{
			break;
		}

		// if the file doesn't match then get the next file.
		while (!bMatch)
		{
			//  try to get the next file.
			eResult = QueryNextEntry(&sQuery, true, false);
			if (eResult != FAT_SUCCESS)
			{
				break;
			}

			eResult = ValidateFatCache(sQuery.sBuffer);
			RETURN_ON_FAT_FAILURE(eResult);

			// if the output of FatQueryFirstEntry indicates that  there are no entries available then set the psEntry name to 0 and return success.
			if (IsLastDirectoryEntry(sQuery.psCurrentEntryRaw))
			{
				psEntry->szName[0] = '\0';
				sQuery.Kill(GetSectorCache());
				return FAT_SUCCESS;
			}

			// match the filename against the next psEntry.
			if (bLongFilename)
			{
				bMatch = CompareFatLongName(auiConstructedLongFilename, sQuery.auiLongFilename) || CompareFatShortName(szConstructedShortFileName, (char*)sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
			}
			else
			{
				bMatch = CompareFatShortName(szConstructedShortFileName, (char*)sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
			}
		}
		if (eResult != FAT_SUCCESS)
		{
			break;
		}

		// set the current entry to the entry that we've just found.
		psCurrentEntry = sQuery.psCurrentEntryRaw;
	} 
	while (szPath[0] != '\0');

	if (eResult == FAT_SUCCESS)
	{
		eResult = ValidateFatCache(sQuery.sBuffer);
		RETURN_ON_FAT_FAILURE(eResult);

		// copy the filename and transform the filename from the internal structure to the public one.
		ConvertFATShortInternalNameInto8Dot3Format(psEntry->szName, (uint8*)sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);

		// copy other data from the internal psEntry structure
		// to the public one
		psEntry->uiAttributes = sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes;
		psEntry->uiSize = sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.uiSize;
		psEntry->tCreateTime = FatDecodeDateTime(sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.uiCreateDate, sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.uiCreateTime);
		psEntry->tModifyTime = FatDecodeDateTime(sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.uiModifyDate, sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.uiModifyTime);
		psEntry->tAccessTime = FatDecodeDateTime(sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.uiAccessDate, 0);

		// calculate the sector address of the psEntry - if query->CurrentCluster equals zero then this is the root directory of a FAT12/FAT16 volume and the calculation is different.
		if (IsRootDirectoryCluster(sQuery.uiCurrentCluster))
		{
			psEntry->uiSectorAddress = GetRootDirectorySector() + sQuery.uiCurrentSector;
		}
		else
		{
			psEntry->uiSectorAddress = CalculateFirstSectorOfCluster(sQuery.uiCurrentCluster) + sQuery.uiCurrentSector;
		}

		// calculate the offset of the psEntry within it's sector
		psEntry->uiSectorOffset = (uint16)((uintptr_t)sQuery.psCurrentEntryRaw) - ((uintptr_t)sQuery.psFirstEntryRaw);

		// store a copy of the original FAT directory entry within the SFatDirectoryEntry structure that is returned to users.
		psEntry->sRaw = *sQuery.psCurrentEntryRaw;
	}

	sQuery.Kill(GetSectorCache());
	return eResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::FatParsePath(char* szPath, char* szPathPart, char** pszFilenamePart)
{
	*pszFilenamePart = szPath + strlen(szPath);

	while (*--(*pszFilenamePart) != '\\' && (*pszFilenamePart) != szPath);

	while (szPath != *pszFilenamePart)
	{
		*szPathPart++ = *szPath++;
	}
	*szPathPart = 0;
	(*pszFilenamePart)++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::Delete(char* szFilename)
{
	EFatCode				eResult;
	uint32					uiFirstCluster;
	SFatDirectoryEntry		sEntry;
	char					szPathPart[FAT_MAX_PATH];
	char*					szNamePart;
	uint8					uiChecksum;
	SFatCache				sBuffer;


	// get the entry for the file
	eResult = GetFileEntry(szFilename, &sEntry);
	RETURN_ON_FAT_FAILURE(eResult);

	// if the entry is located go ahead and delete it.
	if (sEntry.szName != 0)
	{
		// compute the checksum for the file
		uiChecksum = FatLongEntryChecksum((char*)sEntry.sRaw.uEntry.sFatRawCommon.szShortName);

		// make sure we're not trying to delete a directory.
		if (sEntry.uiAttributes & FAT_ATTR_DIRECTORY)
		{
			return FAT_NOT_A_FILE;
		}

		// find the entry's first cluster address.
		uiFirstCluster = GetFirstClusterFromFatEntry(&sEntry.sRaw, meFileSystem == FAT_FS_TYPE_FAT32);

		// free the file's cluster change
		if (uiFirstCluster)
		{
			eResult = FreeChain(uiFirstCluster);
			RETURN_ON_FAT_FAILURE(eResult);
		}

		// mark the entry as deleted.
		sEntry.sRaw.uEntry.sFatRawCommon.szShortName[0] = (char)FAT_DELETED_ENTRY;

		READ_SECTOR(sBuffer, sEntry.uiSectorAddress);

		memcpy(sBuffer.Get() + sEntry.uiSectorOffset, &sEntry.sRaw, sizeof(sEntry.sRaw));
		DirtySector(sBuffer);
	}

	// parse the filename
	FatParsePath(szFilename, szPathPart, &szNamePart);

	SFatFileSystemQuery		sQuery;

	sQuery.Init();

	// get the 1st LFN entry of the parent directory
	eResult = FindFirstFATEntry(szPathPart, FAT_ATTR_LONG_NAME, 0, &sQuery);
	if (eResult == FAT_SUCCESS)
	{
		// loop through each entry.
		while (sQuery.sCurrentEntry.sRaw.uEntry.sFatRawCommon.szShortName[0] != '\0')
		{
			if (sQuery.sCurrentEntry.sRaw.uEntry.sFatRawLongFileName.uiChecksum == uiChecksum)
			{
				// mark the entry as deleted
				sQuery.sCurrentEntry.sRaw.uEntry.sFatRawCommon.szShortName[0] = (char)FAT_DELETED_ENTRY;

				sBuffer = mcSectorCache.ReadSector(sQuery.sCurrentEntry.uiSectorAddress);
				if (!sBuffer.IsValid())
				{
					eResult = FAT_CANNOT_READ_MEDIA;
					break;
				}

				memcpy(sBuffer.Get() + sQuery.sCurrentEntry.uiSectorOffset, &sQuery.sCurrentEntry.sRaw, sizeof(sEntry.sRaw));
				DirtySector(sBuffer);
			}

			// get the next LFN entry
			eResult = FindNextFATEntry(0, &sQuery);
			if (eResult != FAT_SUCCESS)
			{
				break;
			}
		}
	}

	sQuery.Kill(GetSectorCache());
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::Rename(char* szOriginalFilename, char* szNewFilename)
{
	EFatCode				eResult;
	uint32					uiEntryCluster;
	char					szNewParent[256];
	char*					szOriginalFilenamePart;
	SFatDirectoryEntry		sOriginalEntry;
	SFatDirectoryEntry		sNewEntry;
	uint8					uiChecksum = 0;
	char					szOriginalParent[256];
	char*					szNewFilenamePart;
	SFatCache				sBuffer;

	FatParsePath(szOriginalFilename, szOriginalParent, &szOriginalFilenamePart);
	FatParsePath(szNewFilename, szNewParent, &szNewFilenamePart);

	// try to get the new entry to see if it exists.
	GetFileEntry(szNewFilename, &sNewEntry);
	if (*sNewEntry.szName != 0)
	{
		return FAT_FILENAME_ALREADY_EXISTS;
	}

	// get the directory entry
	GetFileEntry(szOriginalFilename, &sOriginalEntry);

	if (*sOriginalEntry.szName != 0)
	{
		SFatDirectoryEntry parent;

		// compute the checksum for the file
		uiChecksum = FatLongEntryChecksum((char*)sOriginalEntry.sRaw.uEntry.sFatRawCommon.szShortName);

		// get the cluster # for the entry
		uiEntryCluster = GetFirstClusterFromFatEntry(&sOriginalEntry.sRaw, GetFileSystemType() == FAT_FS_TYPE_FAT32);

		// get the new parent entry
		eResult = GetFileEntry(szNewParent, &parent);
		RETURN_ON_FAT_FAILURE(eResult);

		// create the new entry in the parent folder
		eResult = CreateFATEntry(&parent.sRaw, szNewFilenamePart, sOriginalEntry.uiAttributes, uiEntryCluster, &sNewEntry);
		RETURN_ON_FAT_FAILURE(eResult);

		// copy all info except name from the old entry to the new one
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiAccessDate = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiAccessDate;
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiAttributes = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiAttributes;
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiCreateDate = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiCreateDate;
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiCreateTime = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiCreateTime;
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiCreateTimeTenths = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiCreateTimeTenths;
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiFirstClusterHighWord = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiFirstClusterHighWord;
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiFirstClusterLowWord = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiFirstClusterLowWord;
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiModifyDate = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiModifyDate;
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiModifyTime = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiModifyTime;
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiReserved = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiReserved;
		sNewEntry.sRaw.uEntry.sFatRawCommon.uiSize = sOriginalEntry.sRaw.uEntry.sFatRawCommon.uiSize;

		READ_SECTOR(sBuffer, sNewEntry.uiSectorAddress);

		memcpy(sBuffer.Get() + sNewEntry.uiSectorOffset, &sNewEntry.sRaw, sizeof(sNewEntry.sRaw));
		DirtySector(sBuffer);

		// mark the original entry as deleted.
		sOriginalEntry.sRaw.uEntry.sFatRawCommon.szShortName[0] = (char)FAT_DELETED_ENTRY;

		READ_SECTOR(sBuffer, sOriginalEntry.uiSectorAddress);

		memcpy(sBuffer.Get() + sOriginalEntry.uiSectorOffset, &sOriginalEntry.sRaw, sizeof(sOriginalEntry.sRaw));
		DirtySector(sBuffer);
	}

	SFatFileSystemQuery		sQuery;

	sQuery.Init();

	// get the 1st LFN entry of the parent directory
	eResult = FindFirstFATEntry(szOriginalParent, FAT_ATTR_LONG_NAME, 0, &sQuery);
	if (eResult == FAT_SUCCESS)
	{
		// loop through each entry.
		while (sQuery.sCurrentEntry.sRaw.uEntry.sFatRawCommon.szShortName[0] != '\0')
		{
			if (sQuery.sCurrentEntry.sRaw.uEntry.sFatRawLongFileName.uiChecksum == uiChecksum)
			{
				// mark the entry as deleted
				sQuery.sCurrentEntry.sRaw.uEntry.sFatRawCommon.szShortName[0] = (char)FAT_DELETED_ENTRY;

				sBuffer = mcSectorCache.ReadSector(sQuery.sCurrentEntry.uiSectorAddress); 
				if (!sBuffer.IsValid())
				{ 
					eResult = FAT_CANNOT_READ_MEDIA; 
					break;
				}

				memcpy(sBuffer.Get() + sQuery.sCurrentEntry.uiSectorOffset, &sQuery.sCurrentEntry.sRaw, sizeof(sQuery.sCurrentEntry.sRaw));
				DirtySector(sBuffer);
			}

			// get the next LFN entry
			eResult = FindNextFATEntry(0, &sQuery);
			if (eResult != FAT_SUCCESS)
			{
				break;
			}
		}
	}

	sQuery.Kill(GetSectorCache());
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::GetClusterSize(void) { return muiBytesPerCluster; }
uint32 CFatVolume::GetID(void) { return muiID; }
uint32 CFatVolume::GetFatSize(void) { return muiFatSize; }
uint32 CFatVolume::GetRootCluster(void) { return muiRootCluster; }
uint32 CFatVolume::GetRootSector(void) { return muiRootSector; }
uint32 CFatVolume::GetFirstDataSector(void) { return muiFirstDataSector; }
uint32 CFatVolume::GetNoOfSectors(void) { return muiNoOfSectors; }
uint32 CFatVolume::GetNoOfDataSectors(void) { return muiNoOfDataSectors; }
uint32 CFatVolume::GetNumClusters(void) { return muiNoOfClusters; }
uint32 CFatVolume::GetNoOfReservedSectors(void) { return muiNoOfReservedSectors; }
uint32 CFatVolume::GetNextFreeCluster(void) { return muiNextFreeCluster; }
uint32 CFatVolume::GetTotalFreeClusters(void) { return muiTotalFreeClusters; }
uint32 CFatVolume::GetFileSystemInfoSector(void) { return muiFileSystemInfoSector; }
uint16 CFatVolume::GetRootDirectorySectors(void) { return muiRootDirectorySectors; }
uint16 CFatVolume::GetSectorSize(void) { return muiBytesPerSector; }
uint16 CFatVolume::NumSectorsPerCluster(void) { return muiNoOfSectorsPerCluster; }
bool CFatVolume::IsUseLongFilenames(void) { return mbUseLongFilenames; }
EFatFileSystemType CFatVolume::GetFileSystemType(void) { return meFileSystem; }
uint8 CFatVolume::GetNoOfFatTables(void) { return muiNoOfFatTables; }
char* CFatVolume::GetLabel(void) { return mszLabel.Text(); }
CFileDrive* CFatVolume::GetFileDrive(void) { return mpcDevice; }
CFatInfoSectorCache* CFatVolume::GetSectorCache(void) { return &mcSectorCache; }

