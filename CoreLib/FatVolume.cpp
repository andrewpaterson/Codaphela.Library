#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "BaseLib/MemCompare.h"
#include "FatTime.h"
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
EFatCode CFatVolume::FindBiosParameterBlock(SFatCache sMBRSector)
{
	uint32					uiHiddenSectors;
	SFatPartitionEntry*		psPartitionEntry;
	uint8					uiPartitionsTried;
	EFatCode				eResult;

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
				// move to the next partition entry
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

			SFatCache	sLBASector;

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

		// get all the info we need from BPB.
		muiRootDirectorySectors = ((msBPB.uiRootEntryCount * 32) + (msBPB.uiBytsPerSector - 1)) / msBPB.uiBytsPerSector;
		muiFatSize = (msBPB.uiFATSzFat16) ? msBPB.uiFATSzFat16 : msBPB.uFatEx.sFat32.uiFATSzFat32;
		muiNoOfSectors = (msBPB.uiTotalSectorsFat16) ? msBPB.uiTotalSectorsFat16 : msBPB.uiTotalSectorsFat32;
		muiNoOfDataSectors = muiNoOfSectors - (msBPB.uiReservedSectorCount + (msBPB.uiNumFileAllocationTables * muiFatSize) + muiRootDirectorySectors);
		muiNoOfClusters = muiNoOfDataSectors / msBPB.uiSectorsPerCluster;
		muiBytesPerSector = msBPB.uiBytsPerSector;
		meFileSystem = (muiNoOfClusters < 4085) ? FAT_FS_TYPE_FAT12 : (muiNoOfClusters < 65525) ? FAT_FS_TYPE_FAT16 : FAT_FS_TYPE_FAT32;

		if (!CheckFileAllocationTableLargeEnough(meFileSystem, muiFatSize, muiNoOfClusters, muiBytesPerSector))
		{
			uiPartitionsTried++;
			continue;
		}

		muiHiddenSectors = uiHiddenSectors;
		muiFirstDataSector = msBPB.uiReservedSectorCount + muiHiddenSectors + (msBPB.uiNumFileAllocationTables * muiFatSize) + muiRootDirectorySectors;
		muiNoOfReservedSectors = msBPB.uiReservedSectorCount + muiHiddenSectors;
		muiNoOfSectorsPerCluster = msBPB.uiSectorsPerCluster;
		muiNoOfFatTables = msBPB.uiNumFileAllocationTables;
		muiFileSystemInfoSector = msBPB.uFatEx.sFat32.uiFileSystemInformation;
		muiBytesPerCluster = muiBytesPerSector * muiNoOfSectorsPerCluster;

		// read the volume label from the boot sector.
		if (meFileSystem == FAT_FS_TYPE_FAT16)
		{
			muiID = msBPB.uFatEx.sFat16.uiVolumeID;
			mszLabel.Kill();
			mszLabel.Init(msBPB.uFatEx.sFat16.szVolumeLabel, 0, 10);
			mszLabel.StripWhiteSpace();
		}
		else if (meFileSystem == FAT_FS_TYPE_FAT32)
		{
			muiID = msBPB.uFatEx.sFat32.uiVolumeID;
			mszLabel.Kill();
			mszLabel.Init(msBPB.uFatEx.sFat32.szVolumeLabel, 0, 10);
			mszLabel.StripWhiteSpace();
		}

		// if the volume is FAT32 then copy the root entry's cluster from the uiRootCluster field on the BPB .
		if (meFileSystem == FAT_FS_TYPE_FAT32)
		{
			muiRootCluster = msBPB.uFatEx.sFat32.uiRootCluster;
		}
		else
		{
			muiRootCluster = 0;
		}

		muiRootSector = CalculateRootSector();

		uint32		uiFATSector;
		SFatCache	sFATSector;

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

	// if this is the last cluster of the directory...
	if (FatIsEOFEntry(uiFat))
	{
		psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[0] = '\0';
		return FAT_SHORT_NAME_FOUND;
	}

	// set the current cluster to the next cluster of the directory etry
	psQuery->uiCurrentCluster = uiFat;

	// reset the current sector
	psQuery->uiCurrentSector = 0;

	// calculate the address of the next sector.
	*puiSector = CalculateFirstSectorOfCluster(psQuery->uiCurrentCluster) + psQuery->uiCurrentSector;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FindNextRawDirectoryEntryCluster(uint32* puiSector, SFatQueryState* psQuery)
{
	// if there are more sectors on the current cluster then
	psQuery->uiCurrentSector++;

	// if this is the root directory of a FAT16/FAT12 volume and we have passed it's last sector then there are no more entries...
	if (psQuery->uiCurrentCluster == 0)
	{
		if (psQuery->uiCurrentSector == GetRootDirectorySectors())
		{
			psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[0] = '\0';
			return FAT_SHORT_NAME_FOUND;
		}
		*puiSector = (GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize())) + psQuery->uiCurrentSector;
	}
	else
	{
		// calculate the address of the next sector
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
	int		i;
	uint16	uiResult;

	i = 0;
	for (uiResult = 0; uiResult < 8; uiResult++)
	{
		if (psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult] != 0x20)
		{
			if (psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.uiReserved & FAT_LOWERCASE_BASENAME)
			{
				psQuery->auiLongFilename[i] = tolower(psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult]);
			}
			else
			{
				psQuery->auiLongFilename[i] = psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult];
			}
			i++;
		}
	}

	if (psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[8] != 0x20)
	{
		psQuery->auiLongFilename[i++] = '.';

		for (uiResult = 8; uiResult < 11; uiResult++)
		{
			if (psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult] != 0x20)
			{
				if (psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.uiReserved & FAT_LOWERCASE_EXTENSION)
				{
					psQuery->auiLongFilename[i] = tolower(psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult]);
				}
				else
				{
					psQuery->auiLongFilename[i] = psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult];
				}
				i++;
			}
		}
	}

	psQuery->auiLongFilename[i] = 0x0;
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
uint32 CFatVolume::GetEntryCluster(SFatDirectoryEntry* psEntry)
{
	fatEntry	uiClusterEntryMask;
	fatEntry	uiEntry;

	uiClusterEntryMask = GetClusterEntryMask();

	uiEntry = psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterLowWord | psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterHighWord << 16;
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

	// initialize the 1st sector of the directory cluster with the dot entry
	memset(psEntries, 0, GetSectorSize());
	psEntries->uEntry.sFatRawCommon.szShortName[0x0] = '.';
	psEntries->uEntry.sFatRawCommon.szShortName[0x1] = ' ';
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

	psEntries++;

	// initialize the dot dot entry.
	psEntries->uEntry.sFatRawCommon.szShortName[0x0] = '.';
	psEntries->uEntry.sFatRawCommon.szShortName[0x1] = '.';
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
	psEntries->uEntry.sFatRawCommon.uiFirstClusterLowWord = psDirectoryParent->uEntry.sFatRawCommon.uiFirstClusterLowWord;
	psEntries->uEntry.sFatRawCommon.uiFirstClusterHighWord = psDirectoryParent->uEntry.sFatRawCommon.uiFirstClusterHighWord;
	psEntries->uEntry.sFatRawCommon.uiCreateDate = GetSystemClockDate();
	psEntries->uEntry.sFatRawCommon.uiCreateTime = GetSystemClockTime();
	psEntries->uEntry.sFatRawCommon.uiModifyDate = psEntries->uEntry.sFatRawCommon.uiCreateDate;
	psEntries->uEntry.sFatRawCommon.uiModifyTime = psEntries->uEntry.sFatRawCommon.uiCreateTime;
	psEntries->uEntry.sFatRawCommon.uiAccessDate = psEntries->uEntry.sFatRawCommon.uiCreateDate;
	psEntries->uEntry.sFatRawCommon.uiCreateTimeTenths = 0xc6;
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
EFatCode CFatVolume::UpdateDirectoryEntry(SFatDirectoryEntry* psEntry, uint32 uiCluster, uint32 uiNewClusterInVolume, uint32 uiClustersInFile)
{
	SFatCache	sBuffer;

	// if this is the 1st cluster cluster allocated to the file then we must modify the file's entry
	if ((uiCluster == 0))
	{
		// modify the file entry to point to the new cluster
		psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterLowWord = (uint16)uiNewClusterInVolume;
		psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterHighWord = (uint16)(uiNewClusterInVolume >> 16);
		psEntry->sRaw.uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_ARCHIVE;

		READ_SECTOR(sBuffer, psEntry->uiSectorAddress);

		// copy the modified file entry to the sector buffer
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


// increase a cluster address by the amount of clusters indicated by count. This function will // follow the FAT entry chain to fat the count-th cluster allocated to a file relative from the input cluster
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


// increase a cluster address by the amount of clusters indicated by count. This function will follow the FAT entry chain to fat the count-th cluster allocated to a file relative from the input cluster.
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

	// if the count is zero we just return the same
	// uiCluster that we received
	if (uiClusterCount == 0)
	{
		*puiNewClusterIndex = uiClusterIndex;
		return FAT_SUCCESS;
	}

	// get the offset of the cluster entry within the FAT table, the sector of the FAT table that contains the entry and the offset of the fat entry within the sector
	CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiFirstClusterSector);

	bFat12OddClusterBeingProcessed = false;
	bFat12MultiStepProgress = false;

	for (;;)
	{
		uiCurrentSector = uiFirstClusterSector;

		// free all the fat entries on the current sector
		while (uiCurrentSector == uiFirstClusterSector)
		{
			// make sure we don't try to free an invalid uiCluster
			if (uiClusterIndex < 2)
			{
				return FAT_INVALID_CLUSTER;
			}

			// read the uiCluster entry and mark it as free
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

			// if the last uiCluster marks the end of the chian we return
			// false as we cannot increase the address by the # of clusters
			// requested by the caller
			if (FatIsEOFEntry(uiClusterIndex))
			{
				return FAT_UNKNOWN_ERROR;
			}

			// if we've followed the number of clusters requested by
			// the caller set the return puiNewClusterIndex to the current uiCluster
			// and return success
			uiClusterCount--;
			if (uiClusterCount == 0)
			{
				*puiNewClusterIndex = (uint32)uiClusterIndex;
				return FAT_SUCCESS;
			}

			// calculate the location of the next cluster in the chain
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

	// if the file is not already empty then  we'll empty it
	if (uiCluster != 0)
	{
		// update the entry to point to cluster 0
		psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterLowWord = 0;
		psEntry->sRaw.uEntry.sFatRawCommon.uiFirstClusterHighWord = 0;
		psEntry->sRaw.uEntry.sFatRawCommon.uiSize = 0x0;

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

	// get the offset of the cluster entry within the FAT table, the sector of the FAT table that contains the entry and the offset of the fat entry within the sector.
	CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiFirstClusterSector);

	// loop until we reach the EOC cluster or an error occurs.
	for (;;)
	{
		// store the address of the sector that's (will be) loaded in memory.
		uiCurrentSector = uiFirstClusterSector;

		// loop until a new sector needs to be loaded to continue with the operation.
		while (uiCurrentSector == uiFirstClusterSector)
		{
			// if cluster is less than 2 either we got a bug or the file system is corrupted.
			if (uiClusterIndex < 2)
			{
				return FAT_INVALID_CLUSTER;
			}

			// read the cluster entry and mark it as free.
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

			// increase the count of free clusters
			SetTotalFreeClusters(GetTotalFreeClusters() + 1);

			// if it's the EOF marker we're done, flush the uiBuffer and go
			if (FatIsEOFEntry(uiClusterIndex))
			{
				return FAT_SUCCESS;
			}

			// calculate the location of the next cluster in the chain
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

	// copy the file name to the psEntry and the raw entry in their respective formats
	strcpy((char*)psEntry->szName, "ROOT");
	GetShortNameForEntry((char*)psEntry->sRaw.uEntry.sFatRawCommon.szShortName, (char*)psEntry->szName, 1);

	// set the general fields of the entry
	psEntry->uiAttributes = psEntry->sRaw.uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_DIRECTORY;
	psEntry->uiSize = psEntry->sRaw.uEntry.sFatRawCommon.uiSize = 0x0;

	// since the psEntry does not physically exist the address fields are set to zero as well
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

	if (*szPath == '\\')
	{
		szPath++;
	}

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
char GetLongNameForEntry(uint16* puiDest, char* szSource)
{
	int i;

	for (i = 0; i < (int)strlen((char*)szSource); i++)
	{
		puiDest[i] = (uint16)szSource[i];
	}
	puiDest[i] = 0x0;

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FatCompareShortName(char* szName1, char* szName2)
{
	return memcmp(szName1, szName2, 11) == 0;
}


// performs an ASCII comparison on two UTF16 strings
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char FatCompareLongName(uint16* puiName1, uint16* puiName2)
{
	int		i;
	char	c1, c2;

	for (i = 0; i <= FAT_MAX_FILENAME; i++)
	{
		c1 = toupper((char)puiName1[i]);
		c2 = toupper((char)puiName2[i]);
		if (c1 != c2)
		{
			return 0;
		}

		if (c1 == '\0')
		{
			return 1;
		}
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::MatchesFileName(bool* pbMatch, bool* pbUsingLFN, char* szConstructedShortFileName, uint16* puiTargetFileLongName, char* szCurrentLevelPath, SFatQueryState* psQuery)
{
	bool bLongFilename = false;
	bool bMatch;

	if (GetShortNameForEntry(szConstructedShortFileName, szCurrentLevelPath, 1) == FAT_INVALID_FILENAME)
	{
		if (GetLongNameForEntry(puiTargetFileLongName, szCurrentLevelPath) == FAT_INVALID_FILENAME)
		{
			return FAT_INVALID_FILENAME;
		}
		bLongFilename = true;
		bMatch = FatCompareLongName(puiTargetFileLongName, psQuery->auiLongFilename) || FatCompareShortName(szConstructedShortFileName, (char*)psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
	}
	else
	{
		bMatch = FatCompareShortName(szConstructedShortFileName, (char*)psQuery->psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
	}

	*pbUsingLFN = bLongFilename;
	*pbMatch = bMatch;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int FatIndexOf(char chr, char* str, int index)
{
	//DELETE THIS FUNCTION
	int i = 0;

	str = str + index;
	do
	{
		if (str[i] == chr)
		{
			return i;
		}
		i++;
	} while (str[i]);

	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TrimPath(char* szDest, char* szSource, size_t uiMaxLength)
{
	//DELETE THIS FUNCTION
	uint32	uiSourceLength;
	uint32	uiLeadSpaces = 0;
	uint32	uiLastChar = 0;
	uint32	i;
	char*	szFirstCharOfDest = szDest;

	uiSourceLength = strlen(szSource);

	for (i = 0; i < uiSourceLength && szSource[i] == ' '; i++)
	{
		uiLeadSpaces++;
	}

	if (uiSourceLength == uiLeadSpaces)
	{
		*szDest = '\0';
		return;
	}

	for (uiLastChar = uiSourceLength - 1; uiLastChar > 0 && (szSource[uiLastChar] == ' '); uiLastChar--);

	for (i = uiLeadSpaces; i <= uiLastChar; i++)
	{
		*szDest++ = szSource[i];
		if (!uiMaxLength--)
		{
			*szFirstCharOfDest = '\0';
			return;
		}
	}

	*szDest = '\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::GetShortNameForEntry(char* szDest, char* szSource, bool bLFNDisabled)
{
	char		szName[13];
	bool		bUppercase = false;
	uint16		uiDotIndex;
	uint16		uiLength;
	uint16		i;

	// check that the name is actually a long filename before processing it as such.
	if (!bLFNDisabled)
	{
		uint8	c;
		bool	bLFN = false;

		uiLength = (uint16)strlen(szSource);
		uiDotIndex = FatIndexOf('.', szSource, 0);

		// if the file hs no extension and is longer than 8 chars or if the name part has more than 8 chars or the extension more than 8 or if it has more than one dot then we need to handle it as a LFN.
		if (uiDotIndex < 0 && uiLength > 8) bLFN = true;
		if (uiDotIndex >= 0)
		{
			if (uiDotIndex > 7 || (uiLength - uiDotIndex) > 4)
			{
				bLFN = true;
			}
			if (uiDotIndex >= 0)
			{
				if (FatIndexOf('.', (char*)szSource, 1) >= 0)
				{
					bLFN = true;
				}
			}
		}
		else
		{
			if (uiLength > 8) bLFN = true;
		}

		// if it has spaces or lowercase letters we must also handle it as a long filename
		if (!bLFN)
		{
			for (i = 0; i < uiLength; i++)
			{
				if (szSource[i] == ' ' || szSource[i] != toupper(szSource[i]))
				{
					bLFN = true;
					break;
				}
			}
		}
		if (bLFN)
		{
			// first we find the location of the LAST dot
			uiDotIndex = uiLength;
			for (i = uiLength - 1; i; i--)
			{
				if (szSource[i] == '.')
				{
					uiDotIndex = i;
					break;
				}
			}

			// now we copy the first 8 chars of the filename excluding dots and spaces and we pad it with spaces.
			c = 0;
			for (i = 0; i < 8; i++)
			{
				while (c < uiDotIndex)
				{
					if (szSource[c] == ' ' || szSource[c] == '.')
					{
						c++;
					}
					else
					{
						break;
					}
				}
				if (c < uiDotIndex)
				{
					szName[i] = toupper(szSource[c++]);
				}
				else
				{
					szName[i] = ' ';
				}
			}

			// do the same for the extension
			c = uiDotIndex + 1;
			for (i = 8; i < 11; i++)
			{
				while (c < uiLength)
				{
					if (szSource[c] == ' ' || szSource[c] == '.')
					{
						c++;
					}
					else
					{
						break;
					}
				}
				if (c < uiLength)
				{
					szName[i] = toupper(szSource[c++]);
				}
				else
				{
					szName[i] = ' ';
				}
			}

			// now we copy it to the callers buffer and we're done
			for (i = 0; i < 11; i++)
			{
				*szDest++ = szName[i];
			}

			// return special code so the caller knows to store the long name.
			return FAT_LFN_GENERATED;
		}
	}

	// trim-off spaces - if the result is greater than 12 it will return an empty string.
	TrimPath(szName, (char*)szSource, 12);

	// if the name uiLength was invalid return error code.
	if (*szName == 0 || strlen(szName) > 12)
	{
		return FAT_INVALID_FILENAME;
	}

	// find the location of the dot.
	uiDotIndex = (uintptr_t)strchr(szName, (int)'.');

	// strchr gave us the address of the dot, we now convert it to a 1-based index.
	if (uiDotIndex)
	{
		uiDotIndex -= (uintptr_t)szName - 1;
	}

	// get the uiLength of the input string
	uiLength = (uint16)strlen(szName);

	// check that this is a valid 8.3 filename
	if ((uiLength > 9 &&
		(uiDotIndex == 0 || (uiDotIndex) > 9)) ||
		(uiDotIndex > 0 && (uiLength - uiDotIndex) > 5))
	{
		return FAT_INVALID_FILENAME;
	}

	// copy the 1st part of the filename to the destination bBuffer.
	for (i = 0; i < 8; i++)
	{
		if (uiDotIndex == 0)
		{
			if (i < uiLength)
			{
				if (bLFNDisabled && (szName[i] != toupper(szName[i])))
					bUppercase = true;

				*szDest++ = toupper(szName[i]);
			}
			else
			{
				*szDest++ = ' ';
			}
		}
		else
		{
			if (i < uiDotIndex - 1)
			{
				if (bLFNDisabled && (szName[i] != toupper(szName[i])))
					bUppercase = true;

				*szDest++ = toupper(szName[i]);
			}
			else
			{
				*szDest++ = ' ';
			}
		}
	}

	// if there's not extension fill the extension characters with spaces.
	if (uiDotIndex == 0)
	{
		for (i = 0; i < 3; i++)
			*szDest++ = ' ';
	}
	// if there is an extension...
	else
	{
		// copy the extension characters to the destination bBuffer.
		for (i = uiDotIndex; i < uiDotIndex + 3; i++)
		{
			if (i < uiLength)
			{
				if (bLFNDisabled && (szName[i] != toupper(szName[i])))
					bUppercase = true;
				*szDest++ = toupper(szName[i]);
			}
			else
			{
				*szDest++ = ' ';
			}
		}
	}

	return bUppercase ? FAT_INVALID_FILENAME : FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ConvertFATShortInternalNameInto8Dot3Format(uint8* puiDest, const uint8* puiSource)
{
	if (*puiSource != FAT_KANJI_SKIP_ENTRY)
	{
		*puiDest++ = *puiSource++;
	}
	else
	{
		*puiDest++ = FAT_DELETED_ENTRY;
	}

	// if there's a second character...
	if (*puiSource != ' ')
	{
		*puiDest++ = *puiSource++;
		if (*puiSource != ' ')
		{
			*puiDest++ = *puiSource++;
			if (*puiSource != ' ')
			{
				*puiDest++ = *puiSource++;
				if (*puiSource != ' ')
				{
					*puiDest++ = *puiSource++;
					if (*puiSource != ' ')
					{
						*puiDest++ = *puiSource++;
						if (*puiSource != ' ')
						{
							*puiDest++ = *puiSource++;
							if (*puiSource != ' ')
							{
								*puiDest++ = *puiSource++;
							}
							else
							{
								puiSource++;
							}
						}
						else
						{
							puiSource += 0x2;
						}
					}
					else
					{
						puiSource += 0x3;
					}
				}
				else
				{
					puiSource += 0x4;
				}
			}
			else
			{
				puiSource += 0x5;
			}
		}
		else
		{
			puiSource += 0x6;
		}
	}
	else
	{
		puiSource += 0x7;
	}

	if (*puiSource != ' ')
	{
		*puiDest++ = '.';
		*puiDest++ = *puiSource++;
		if (*puiSource != ' ')
		{
			*puiDest++ = *puiSource++;
			if (*puiSource != ' ')
			{
				*puiDest++ = *puiSource;
			}
		}
	}
	*puiDest = '\0';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FindFirstFATEntry(char* szParentPath, uint8 uiAttributes, SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* psQuery)
{
	EFatCode			eResult;
	SFatDirectoryEntry	sParentEntry;

	// if the path starts with a backslash then advance to the next character
	if (szParentPath)
	{
		if (*szParentPath == '\\')
		{
			szParentPath++;
		}
	}

	// if a parent was specified...
	if (szParentPath != NULL)
	{
		// try to get the entry for the parent
		eResult = GetFileEntry(szParentPath, &sParentEntry);
		RETURN_ON_FAT_FAILURE(eResult);

		// try to get the 1st entry of the query results.
		eResult = QueryFirstEntry(&sParentEntry.sRaw, uiAttributes, &psQuery->sQuery, 0);
	}
	// if the parent was not supplied then we submit the query without it.
	else
	{
		eResult = QueryFirstEntry(0, uiAttributes, &psQuery->sQuery, 0);
	}

	// if we cant get the 1st entry then return the error that we received from fat_Query_First_entry.
	RETURN_ON_FAT_FAILURE(eResult);

	// if there are no more entries
	if (psQuery->sQuery.psCurrentEntryRaw == NULL)
	{
		psQuery->sCurrentEntry.szName[0] = '\0';
		return FAT_SUCCESS;
	}

	// fill the current entry structure with data from the current raw entry of the query.
	FillDirectoryEntryFromRawEntry(&psQuery->sCurrentEntry, psQuery->sQuery.psCurrentEntryRaw);

	// calculate the sector address of the entry - if query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is different.
	if (psQuery->sQuery.uiCurrentCluster == 0)
	{
		psQuery->sCurrentEntry.uiSectorAddress = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize()) + psQuery->sQuery.uiCurrentSector;
	}
	else
	{
		psQuery->sCurrentEntry.uiSectorAddress = CalculateFirstSectorOfCluster(psQuery->sQuery.uiCurrentCluster) + psQuery->sQuery.uiCurrentSector;
	}

	// calculate the offset of the entry within it's sector
	psQuery->sCurrentEntry.uiSectorOffset = (uint16)((uintptr_t)psQuery->sQuery.psCurrentEntryRaw) - ((uintptr_t)psQuery->sQuery.psFirstEntryRaw);

	// store a copy of the original FAT directory entry within the SFatDirectoryEntry structure that is returned to users
	psQuery->sCurrentEntry.sRaw = *psQuery->sQuery.psCurrentEntryRaw;

	// if long filenames are enabled copy the filename to the entry
	if (*psQuery->sCurrentEntry.szName != 0)
	{
		if (*psQuery->sQuery.auiLongFilename != 0)
		{
			uint16	uiIndex;
			for (uiIndex = 0; uiIndex < 256; uiIndex++)
			{
				psQuery->sCurrentEntry.szName[uiIndex] = (uint8)psQuery->sQuery.auiLongFilename[uiIndex];
				if (psQuery->sQuery.auiLongFilename[uiIndex] == 0)
				{
					break;
				}
			}
		}
	}

	if (ppsDirectoryEntry)
	{
		*ppsDirectoryEntry = &psQuery->sCurrentEntry;
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FindNextFATEntry(SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* psQuery)
{
	EFatCode				eResult;

	// try to get the next entry of the query.
	eResult = QueryNextEntry(&psQuery->sQuery, false, false);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	// if there are no more entries
	if (psQuery->sQuery.psCurrentEntryRaw == 0)
	{
		// set the filename of the current entry to 0.
		psQuery->sCurrentEntry.szName[0] = '\0';
		return FAT_SUCCESS;
	}

	// fill the current entry structure with data from the current raw entry of the query.
	FillDirectoryEntryFromRawEntry(&psQuery->sCurrentEntry, psQuery->sQuery.psCurrentEntryRaw);

	// calculate the sector address of the entry - if query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is different.
	if (psQuery->sQuery.uiCurrentCluster == 0x0)
	{
		psQuery->sCurrentEntry.uiSectorAddress = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize()) + psQuery->sQuery.uiCurrentSector;
	}
	else
	{
		psQuery->sCurrentEntry.uiSectorAddress = CalculateFirstSectorOfCluster(psQuery->sQuery.uiCurrentCluster) + psQuery->sQuery.uiCurrentSector;
	}

	// calculate the offset of the entry within it's sector
	psQuery->sCurrentEntry.uiSectorOffset = (uint16)((uintptr_t)psQuery->sQuery.psCurrentEntryRaw) - ((uintptr_t)psQuery->sQuery.psFirstEntryRaw);

	// store a copy of the original FAT directory entry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	psQuery->sCurrentEntry.sRaw = *psQuery->sQuery.psCurrentEntryRaw;

	if (*psQuery->sCurrentEntry.szName != 0)
	{
		if (*psQuery->sQuery.auiLongFilename != 0)
		{
			uint16 uiIndex;
			for (uiIndex = 0; uiIndex < 256; uiIndex++)
			{
				psQuery->sCurrentEntry.szName[uiIndex] = (uint8)psQuery->sQuery.auiLongFilename[uiIndex];
				if (psQuery->sQuery.auiLongFilename[uiIndex] == 0)
				{
					break;
				}
			}
		}
	}

	if (ppsDirectoryEntry)
	{
		*ppsDirectoryEntry = &psQuery->sCurrentEntry;
	}

	return FAT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::FillDirectoryEntryFromRawEntry(SFatDirectoryEntry* psEntry, SFatRawDirectoryEntry* psRawEntry)
{
	ConvertFATShortInternalNameInto8Dot3Format(psEntry->szName, (uint8*)psRawEntry->uEntry.sFatRawCommon.szShortName);

	// copy other data from the internal entry structure to the public one
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
bool CFatVolume::IsIllegalFilenameCharacter(char c)
{
	uint16	uiIndex;

	if (c <= 0x1F)
	{
		return true;
	}

	for (uiIndex = 0; uiIndex < ILLEGAL_CHARS_COUNT; uiIndex++)
	{
		if (c == ILLEGAL_CHARS[uiIndex] && c != '.')
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
bool CFatVolume::IsIllegalFilename(char* szName, uint16 uiLength)
{
	int16		uiCharIndex;
	
	uiCharIndex = FatIndexOf('.', szName, 0);

	if (uiCharIndex == 0 || uiCharIndex == (uiLength - 1))
	{
		return true;
	}

	for (uiCharIndex = 0x0; uiCharIndex < uiLength; uiCharIndex++)
	{
		if (IsIllegalFilenameCharacter(szName[uiCharIndex]))
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
void CFatVolume::GenerateShortNameWithSuffix(uint16 uiNameSuffix, uint8* szShortName)
{
	char			szNameSuffix[6];
	uint8			i;
	uint8			c;

	sprintf(szNameSuffix, "~%i", uiNameSuffix);

	for (i = 0; i < 8 - (char)strlen(szNameSuffix); i++)
	{
		if (szShortName[i] == ' ')
		{
			break;
		}
	}

	for (c = 0; c < (char)strlen(szNameSuffix); c++)
	{
		szShortName[i] = szNameSuffix[c];
		i++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::GenerateUniqueShortNameWithSuffix(SFatRawDirectoryEntry* psParentDirectory, uint8* szShortName)
{
	SFatQueryState	sQuery;
	uint16			uiNameSuffix = 0;
	bool			bIsValidEntry;
	EFatCode		eResult;

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

		// loop through all entries in the parent directory and if we find one with the same name as ours mark the name as invalid.
		while (*sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName != 0)
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

		// if the filename is taken we need to compute a new one
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
		if (uiDirectoryCluster != 0)
		{
			uiFirstSectorOfCluster = CalculateFirstSectorOfCluster(uiDirectoryCluster);
		}

		// set the current sector to the first  sector of the cluster
		uiSector = uiFirstSectorOfCluster;

		// for each sector in the cluster
		while (uiDirectoryCluster == 0 || uiSector < (uiFirstSectorOfCluster + NumSectorsPerCluster()))
		{
			READ_SECTOR(sBuffer, uiSector);
			puiLastEntryAddress = ((uintptr_t)sBuffer.Get() + GetSectorSize()) - 0x20;
			psParentEntry = (SFatRawDirectoryEntry*)sBuffer.Get();

			// for each directory entry in the sector...
			while ((uintptr_t)psParentEntry <= puiLastEntryAddress)
			{
				if ((uiDirectoryEntries + iLFNEntriesNeeded) == FAT_MAX_DIRECTORY_ENTRIES)
				{
					return FAT_DIRECTORY_LIMIT_EXCEEDED;
				}

				uiDirectoryEntries++;

				// if the directory entry is free.
				if (IsFreeDirectoryEntry(psParentEntry))
				{
					// we've found a free entry
					iLFNEntriesFound++;

					// if this is the last directory entry or if we've found all the entries that we need then let's get ready to write them.
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

				// move the parent entry pointer to the next entry in the sector.
				psParentEntry++;
			}

			if (bAllLFNEntriesFound)
			{
				break;
			}

			// move to the next sector in the cluster.
			uiSector++;

			// make sure that we don't overflow the root directory
			// on FAT12/16 volumes
			if (uiDirectoryCluster == 0)
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

		// get the next cluster, we'll remember the last one in case we need to rewind to write lfn entries
		*puiLastDirectoryCluster = uiDirectoryCluster;
		eResult = GetNextClusterEntry(uiDirectoryCluster, &uiDirectoryCluster);
		RETURN_ON_FAT_FAILURE(eResult);

		// if this is the end of the FAT chain allocate a new cluster to this folder and continue
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

	// if there where any free entries before this one then we need to rewind a bit.
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
			// if the last entry is on the same cluster we can just decrease the sector number, otherwise we need to get the sector address for the last cluster
			if (uiSector > uiFirstSectorOfCluster)
			{
				uiSector--;
			}
			else
			{
				if (uiLastDirectoryCluster == 0)
				{
					uiFirstSectorOfCluster = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize());
				}
				else
				{
					uiDirectoryCluster = uiLastDirectoryCluster;
					uiFirstSectorOfCluster = CalculateFirstSectorOfCluster(uiDirectoryCluster);
				}
				uiSector = uiFirstSectorOfCluster + NumSectorsPerCluster();
			}

			// read the last sector of the cache, calculate the last entry address and set our pointer to it
			READ_SECTOR(sBuffer, uiSector);

			puiLastEntryAddress = ((uintptr_t)sBuffer.Get() + GetSectorSize()) - 0x20;
			//								psParentEntry = (SFatRawDirectoryEntry*)sBuffer.Get();  ?? Isn't this more correct?
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
EFatCode CFatVolume::CreateFATEntry(SFatRawDirectoryEntry* psParentDirectory, char* szName, uint8 uiAttributes, uint32 uiEntryCluster, SFatDirectoryEntry* psNewEntry)
{
	EFatCode				eResult;
	uint16					uiLength;
	uint32					uiSector;
	uint32					uiFirstSectorOfCluster;
	fatEntry				uiDirectoryCluster;
	fatEntry				uiLastDirectoryCluster;
	int						iLFNEntriesFound;
	int						iLFNEntriesNeeded;
	char					uiChecksum;
	SFatCache				sBuffer;
	SFatRawDirectoryEntry*	psParentEntry;
	uintptr_t				puiLastEntryAddress;

	uiLength = (uint16)strlen(szName);

	if (uiLength > FAT_MAX_FILENAME)
	{
		return FAT_FILENAME_TOO_LONG;
	}

	if (IsIllegalFilename(szName, uiLength))
	{
		return FAT_INVALID_FILENAME;
	}

	memset(&psNewEntry->sRaw, 0, sizeof(SFatRawDirectoryEntry));

	eResult = GetShortNameForEntry((char*)psNewEntry->sRaw.uEntry.sFatRawCommon.szShortName, szName, false);
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

	// if the new entry is a directory and no cluster was supplied by the calling function then allocate a new cluster
	if (uiEntryCluster == 0 && (uiAttributes & FAT_ATTR_DIRECTORY))
	{
		uiEntryCluster = AllocateDirectoryCluster(psParentDirectory, &eResult);
		RETURN_ON_FAT_FAILURE(eResult);
	}

	InitialiseNewEntry(psNewEntry, szName, uiAttributes, uiEntryCluster);

	iLFNEntriesFound = 0;
	uiSector = 0;
	uiDirectoryCluster = 0;
	uiFirstSectorOfCluster = 0;
	uiLastDirectoryCluster = 0;
	eResult = FindEnoughEntries(&uiLastDirectoryCluster, &uiDirectoryCluster, &uiFirstSectorOfCluster, &uiSector, &iLFNEntriesFound, &psParentEntry, psParentDirectory, iLFNEntriesNeeded);
	RETURN_ON_FAT_FAILURE(eResult);

	eResult = RewindFoundEntries(&psParentEntry, &uiFirstSectorOfCluster, &uiDirectoryCluster, iLFNEntriesFound, uiLastDirectoryCluster, uiSector);
	RETURN_ON_FAT_FAILURE(eResult);

	// compute the checksum for this entry
	uiChecksum = FatLongEntryChecksum((char*)psNewEntry->sRaw.uEntry.sFatRawCommon.szShortName);

	// now we can start writting
	iLFNEntriesFound = iLFNEntriesNeeded;
	while (iLFNEntriesFound--)
	{
		if (iLFNEntriesFound > 0)
		{
			READ_SECTOR(sBuffer, uiSector);
			puiLastEntryAddress = ((uintptr_t)sBuffer.Get() + GetSectorSize()) - 0x20;

			InitialiseParentEntry(psParentEntry, szName, uiChecksum, iLFNEntriesNeeded, iLFNEntriesFound);
			DirtySector(sBuffer);

			// continue to next entry.
			if ((uintptr_t)psParentEntry < (uintptr_t)puiLastEntryAddress)
			{
				psParentEntry++;
			}
			else
			{
				if (uiDirectoryCluster == 0 || uiSector < uiFirstSectorOfCluster + NumSectorsPerCluster() - 1)
				{
					uiSector++;

					// make sure that we don't overflow the root directory on FAT12/16 volumes
					if (uiDirectoryCluster == 0)
					{
						if (uiSector > uiFirstSectorOfCluster + GetRootDirectorySectors())
						{
							return FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED;
						}
					}
				}
				else
				{
					// get the next cluster, we'll remember the last one so we can update it bellow if it's the eof cluster
					uiLastDirectoryCluster = uiDirectoryCluster;
					eResult = GetNextClusterEntry(uiDirectoryCluster, &uiDirectoryCluster);
					RETURN_ON_FAT_FAILURE(eResult);

					// if this is the end of the FAT chain allocate a new cluster to this folder and continue
					if (FatIsEOFEntry(uiDirectoryCluster))
					{
						fatEntry uiNewFat = AllocateDataCluster(1, true, &eResult, uiLastDirectoryCluster);  // uiLastDirectoryCluster is probably right but then is SetClusterEntry below needed?
						RETURN_ON_FAT_FAILURE(eResult);

						eResult = SetClusterEntry(uiLastDirectoryCluster, uiNewFat);
						RETURN_ON_FAT_FAILURE(eResult);

						uiDirectoryCluster = uiNewFat;
					}

					// continue working on the new cluster
					uiSector = CalculateFirstSectorOfCluster(uiDirectoryCluster);
				}

				// load the next sector
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
EFatCode CFatVolume::CreateDirectory(char* szDirectory)
{
	EFatCode			uiResult;
	SFatDirectoryEntry	sEntry;

	// check that we got a valid pathname
	if (!szDirectory || strlen(szDirectory) > FAT_MAX_PATH)
	{
		return FAT_INVALID_FILENAME;
	}

	// try get the file entry
	uiResult = GetFileEntry(szDirectory, &sEntry);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if we don'tfind a file or directory by that name
	// we can create it, otherwise return file already exists error
	if (*sEntry.szName == 0)
	{
		// allocate memory for the file path
		size_t				uiPathLength;
		char*				szPathScanner;
		char				szFilePath[FAT_MAX_PATH + 1];
		SFatDirectoryEntry	sParentEntry;

		// get the name of the file path including
		// the filename
		uiPathLength = strlen(szDirectory);

		// set the pointer that will be used to scan
		// filename to the end of the filename
		szPathScanner = szDirectory + (uiPathLength - 0x1);

		// if the filename ends with a backslash then it
		// is an invalid filename ( it's actually a directory
		// path )
		if (*szPathScanner == BACKSLASH)
		{
			return FAT_INVALID_FILENAME;
		}

		while (*(--szPathScanner) != BACKSLASH);	/*scan file backwords until we get to */

		uiPathLength = (size_t)(szPathScanner - szDirectory);

		memcpy(szFilePath, szDirectory, uiPathLength);

		szFilePath[uiPathLength] = 0x0;
		szPathScanner++;

		uiResult = GetFileEntry(szFilePath, &sParentEntry);

		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		if (*sParentEntry.szName == 0)
		{
			return FAT_DIRECTORY_DOES_NOT_EXIST;
		}

		// try to create the directory entry
		return CreateFATEntry(&sParentEntry.sRaw, szPathScanner, FAT_ATTR_DIRECTORY, 0, &sEntry);
	}

	// if we get here it means that a file or directory with that name already exists.
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

	if (szPath[0] == '\\')
	{
		szPath++;
	}

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

		eResult = MatchesFileName(&bMatch, &bLongFilename, szConstructedShortFileName, auiConstructedLongFilename, szCurrentLevelPath, &sQuery);
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
				bMatch = FatCompareLongName(auiConstructedLongFilename, sQuery.auiLongFilename) || FatCompareShortName(szConstructedShortFileName, (char*)sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
			}
			else
			{
				bMatch = FatCompareShortName(szConstructedShortFileName, (char*)sQuery.psCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
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
		if (sQuery.uiCurrentCluster == 0x0)
		{
			psEntry->uiSectorAddress = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize()) + sQuery.uiCurrentSector;
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
EFatCode CFatVolume::CountSequentialClusters(uint32 uiCluster, uint32* puiSequentialClustersAfter)
{
	uint32		uiNextCluster;
	EFatCode	eResult;
	uint16		uiSequentialClustersAfterCursor;

	//This is FAT16 specific.
	uiSequentialClustersAfterCursor = 0;
	*puiSequentialClustersAfter = 0;

	while (!FatIsEOFEntry(uiCluster))
	{
		eResult = GetNextClusterEntry(uiCluster, &uiNextCluster);
		RETURN_ON_FAT_FAILURE(eResult);

		if (uiNextCluster == (uiCluster + 1))
		{
			uiSequentialClustersAfterCursor++;
			uiCluster = uiNextCluster;
			if (uiSequentialClustersAfterCursor == 0xFFFF)  //GetEndOfClusterMarker
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	*puiSequentialClustersAfter = uiSequentialClustersAfterCursor;
	return FAT_SUCCESS;
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

