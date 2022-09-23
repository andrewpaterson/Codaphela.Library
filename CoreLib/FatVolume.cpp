#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "FatTime.h"
#include "FatVolume.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::Mount(CFileDrive* device)
{
	SFatBIOSParameterBlock*	psBPB;
	uint32					uiHiddenSectors = 0;
	SFatPartitionEntry*		sPartitionEntry;
	uint8					uiPartitionsTried = 0;
	char					szLabel[12];
	uint32					uiFileSystemInfoSector;
	EFatCode				eResult;

	muiFatSharedBufferSector = FAT_UNKNOWN_SECTOR;
	mbEntriesUpdated = false;

	// set the null terminator.
	szLabel[11] = 0;

	// save the storage device handle
	mpcDevice = device;

	// retrieve the boot sector (sector 0) from the storage device
	eResult = FatReadFatSector(0);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	// set the partition sEntry pointer
	sPartitionEntry = (SFatPartitionEntry*)(mauiFatSharedBuffer + 0x1BE);

	for (;;)
	{
		// if we've already tried to mount the mpsVolume as partitionless
		// try to mount the next partition
		if (uiPartitionsTried)
		{
			// if we've already tried all 4 partitions then we're
			// out of luck
			if (uiPartitionsTried > 4)
			{
				return FAT_INVALID_FAT_VOLUME;
			}

			// if we've tried to mount a partition mpsVolume (not the unpartioned one)
			// then we must reload sector 0 (MBR)
			if (uiPartitionsTried > 1)
			{
				// retrieve the boot sector (sector 0) from the storage device
				eResult = FatReadFatSector(0);
				if (eResult != FAT_SUCCESS)
				{
					return eResult;
				}

				// move to the next partition sEntry
				sPartitionEntry++;
			}

			// remember how many sectors before this partition
			uiHiddenSectors = sPartitionEntry->lba_first_sector;

			// make sure the partition doesn't exceeds the physical boundries
			// of the device
			if (sPartitionEntry->lba_first_sector + sPartitionEntry->total_sectors > mpcDevice->GetTotalSectors())
			{
				uiPartitionsTried++;
				continue;
			}

			// retrieve the 1st sector of partition
			eResult = FatReadFatSector(sPartitionEntry->lba_first_sector);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}
					}

		// set our pointer to the BPB
		psBPB = (SFatBIOSParameterBlock*)mauiFatSharedBuffer;

		// if the sector size is larger than what this build
		// allows do not mount the mpsVolume
		if (psBPB->BPB_BytsPerSec > MAX_SECTOR_LENGTH)
		{
			return FAT_SECTOR_SIZE_NOT_SUPPORTED;
		}

		// make sure BPB_BytsPerSec and BPB_SecPerClus are
		// valid before we divide by them
		if (!psBPB->BPB_BytsPerSec || !psBPB->BPB_SecPerClus)
		{
			uiPartitionsTried++;
			continue;
		}

		// make sure that SecPerClus is a power of two
		uint16 uiResult = psBPB->BPB_SecPerClus;
		while (uiResult != 0x1)
		{
			if (uiResult & 0x1)
			{
				uiPartitionsTried++;
				continue;
			}
			uiResult >>= 1;
		}

		// get all the info we need from BPB
		msVolume.uiRootDirectorySectors = ((psBPB->BPB_RootEntCnt * 32) + (psBPB->BPB_BytsPerSec - 1)) / psBPB->BPB_BytsPerSec;
		msVolume.uiFatSize = (psBPB->BPB_FATSz16) ? psBPB->BPB_FATSz16 : psBPB->uFatEx.sFat32.BPB_FATSz32;
		msVolume.uiNoOfSectors = (psBPB->BPB_TotSec16) ? psBPB->BPB_TotSec16 : psBPB->BPB_TotSec32;
		msVolume.uiNoOfDataSectors = msVolume.uiNoOfSectors - (psBPB->BPB_RsvdSecCnt + (psBPB->BPB_NumFATs * msVolume.uiFatSize) + msVolume.uiRootDirectorySectors);
		msVolume.uiNoOfClusters = msVolume.uiNoOfDataSectors / psBPB->BPB_SecPerClus;
		msVolume.uiFirstDataSector = psBPB->BPB_RsvdSecCnt + uiHiddenSectors + (psBPB->BPB_NumFATs * msVolume.uiFatSize) + msVolume.uiRootDirectorySectors;
		msVolume.uiNoOfReservedSectors = psBPB->BPB_RsvdSecCnt + uiHiddenSectors;
		msVolume.uiNoOfBytesPerSector = psBPB->BPB_BytsPerSec;
		msVolume.uiNoOfSectorsPerCluster = psBPB->BPB_SecPerClus;
		msVolume.uiNoOfFatTables = psBPB->BPB_NumFATs;
		uiFileSystemInfoSector = psBPB->uFatEx.sFat32.BPB_FSInfo;

		// determine the FAT file system type
		msVolume.eFileSystem = (msVolume.uiNoOfClusters < 4085) ? FAT_FS_TYPE_FAT12 : (msVolume.uiNoOfClusters < 65525) ? FAT_FS_TYPE_FAT16 : FAT_FS_TYPE_FAT32;

		// sanity check that the FAT table is big enough
		switch (msVolume.eFileSystem)
		{
		case FAT_FS_TYPE_FAT12:
			if (msVolume.uiFatSize < (((msVolume.uiNoOfClusters + (msVolume.uiNoOfClusters >> 1)) + psBPB->BPB_BytsPerSec - 1) / psBPB->BPB_BytsPerSec))
			{
				uiPartitionsTried++;
				continue;
			}
			break;
		case FAT_FS_TYPE_FAT16:
			if (msVolume.uiFatSize < (((msVolume.uiNoOfClusters * 2) + psBPB->BPB_BytsPerSec - 1) / psBPB->BPB_BytsPerSec))
			{
				uiPartitionsTried++;
				continue;
			}
			break;
		case FAT_FS_TYPE_FAT32:
			if (msVolume.uiFatSize < (((msVolume.uiNoOfClusters * 4) + psBPB->BPB_BytsPerSec - 1) / psBPB->BPB_BytsPerSec))
			{
				uiPartitionsTried++;
				continue;
			}
			break;
		}

		// read the mpsVolume label from the boot sector
		if (msVolume.eFileSystem == FAT_FS_TYPE_FAT16)
		{
			msVolume.uiID = psBPB->uFatEx.sFat16.BS_VolID;
			memcpy(szLabel, psBPB->uFatEx.sFat16.BS_VolLab, 11);
			TrimPath(msVolume.szLabel, szLabel, 11);
		}
		else
		{
			msVolume.uiID = psBPB->uFatEx.sFat32.BS_VolID;
			memcpy(szLabel, psBPB->uFatEx.sFat32.BS_VolLab, 11);
			TrimPath(msVolume.szLabel, szLabel, 11);
		}

		// if the mpsVolume is FAT32 then copy the root
		// sEntry's cluster from the BPB_RootClus field
		// on the BPB
		if (msVolume.eFileSystem == FAT_FS_TYPE_FAT32)
		{
			msVolume.uiRootCluster = psBPB->uFatEx.sFat32.BPB_RootClus;
		}
		else
		{
			msVolume.uiRootCluster = 0x0;
		}

		uint8	uiMedia = psBPB->BPB_Media;
		uint32	uiFATSector;

		uiFATSector = msVolume.uiNoOfReservedSectors;
		eResult = FatReadFatSector(uiFATSector);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// if the lower byte of the 1st FAT sEntry is not the same as
		// BPB_Media then this is not a valid mpsVolume
		if (mauiFatSharedBuffer[0] != uiMedia)
		{
			uiPartitionsTried++;
			continue;
		}
		break;
	}

	// read volume label sEntry from the root directory (if any)
	SFatQueryState query;

	if (FatQueryFirstEntry(0, FAT_ATTR_VOLUME_ID, &query, 1) == FAT_SUCCESS)
	{
		if (*query.sCurrentEntryRaw->uEntry.sFatRawCommon.name != 0)
		{
			TrimPath(msVolume.szLabel, (char*)query.sCurrentEntryRaw->uEntry.sFatRawCommon.name, 11);
		}
	}
	msVolume.uiFileSystemInfoSector = 0xFFFFFFFF;

	// if we find a valid psFileSystemInfo structure we'll use it
	if (msVolume.eFileSystem == FAT_FS_TYPE_FAT32)
	{
		SFatFileSystemInfo* psFileSystemInfo;

		// read the sector containing the FSInfo structure
		eResult = FatReadFatSector(uiHiddenSectors + uiFileSystemInfoSector);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// set psFileSystemInfo pointer
		psFileSystemInfo = (SFatFileSystemInfo*)mauiFatSharedBuffer;

		// check signatures before using
		if (psFileSystemInfo->LeadSig == 0x41615252 && psFileSystemInfo->StructSig == 0x61417272 && psFileSystemInfo->TrailSig == 0xAA550000)
		{
			msVolume.uiNextFreeCluster = psFileSystemInfo->Nxt_Free;

			// if this value is greater than or equal to the # of
			// clusters in the mpsVolume it cannot possible be valid
			if (psFileSystemInfo->Free_Count < msVolume.uiNoOfClusters)
			{
				msVolume.uiTotalFreeClusters = psFileSystemInfo->Free_Count;
			}
			else
			{
				msVolume.uiTotalFreeClusters = msVolume.uiNoOfClusters - 1;
			}
		}
		else
		{
			msVolume.uiNextFreeCluster = 0xFFFFFFFF;
			msVolume.uiTotalFreeClusters = msVolume.uiNoOfClusters - 1;
		}

		// remember psFileSystemInfo sector
		msVolume.uiFileSystemInfoSector = uiHiddenSectors + uiFileSystemInfoSector;
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

	// if this is a FAT32 volume we'll update the psFileSystemInfo structure
	if (msVolume.eFileSystem == FAT_FS_TYPE_FAT32 && msVolume.uiFileSystemInfoSector != 0xFFFFFFFF)
	{
		// read the sector containing the FSInfo structure
		eResult = FatReadFatSector(GetFsinfoSector());
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// set the pointer to the psFileSystemInfo structure
		psFileSystemInfo = (SFatFileSystemInfo*)mauiFatSharedBuffer;

		// check the signatures before writting
		// note: when you mount a removable device in windows it will channge
		// these signatures, i guess it feels it cannot be trusted. So we're going
		// to rebuild them no matter what as they significantly speed up this
		// implementation. After the mpsVolume has been mounted elsewhere Free_Count cannot
		// be trusted. This implementation doesn't actually use it but if you only
		// mount the mpsVolume with us it will keep it up to date.

		psFileSystemInfo->Nxt_Free = GetNextFreeCluster();
		psFileSystemInfo->Free_Count = GetTotalFreeClusters();
		psFileSystemInfo->LeadSig = 0x41615252;
		psFileSystemInfo->StructSig = 0x61417272;
		psFileSystemInfo->TrailSig = 0xAA550000;

		mbEntriesUpdated = true;

		// write the psFileSystemInfo sector
		eResult = FlushAndInvalidate();
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::Read(uint64 uiSector, void* pvData)
{
	return mpcDevice->Read(uiSector, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::Read(uint64 uiSector, uint32 uiNumSectors, void* pvData)
{
	return mpcDevice->Read(uiSector, uiNumSectors, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::Write(uint64 uiSector, void* pvData)
{
	return mpcDevice->Write(uiSector, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::Write(uint64 uiSector, uint32 uiNumSectors, void* pvData)
{
	return mpcDevice->Write(uiSector, uiNumSectors, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive)
{
	return mpcDevice->Erase(uiStartSector, uiStopSectorInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::GetClusterSize(void)
{
	return msVolume.uiNoOfBytesPerSector * msVolume.uiNoOfSectorsPerCluster;
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
bool CFatVolume::HasNextFreeCluser(void)
{
	if (msVolume.uiNextFreeCluster != 0xFFFFFFFF)
	{
		if (msVolume.uiNextFreeCluster <= msVolume.uiNoOfClusters + 1)
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
bool CFatVolume::IsFreeFat(uint32 uifat)
{
	switch (GetFileSystemType())
	{
		case FAT_FS_TYPE_FAT32:
			return !(uifat & 0x0FFFFFFF);

		case FAT_FS_TYPE_FAT16:
			return !(uifat & 0xFFFF);

		case FAT_FS_TYPE_FAT12:
			return !(uifat & 0x0FFF);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetNextFreeCluster(uint32 uiCluster)
{
	msVolume.uiNextFreeCluster = uiCluster;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetTotalFreeClusters(uint32 uiTotalFreeClusters)
{
	msVolume.uiTotalFreeClusters = uiTotalFreeClusters;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetFatSharedBufferSector(uint32 uiSector)
{
	muiFatSharedBufferSector = uiSector;
}


////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::IsFatSectorLoaded(uint32 uiSector)
{
	return (muiFatSharedBufferSector != FAT_UNKNOWN_SECTOR) && (muiFatSharedBufferSector == uiSector);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::CalculateFatEntryOffset(uint32 cluster)
{
	switch (GetFileSystemType())
	{
		case FAT_FS_TYPE_FAT12:
			return cluster + (cluster >> 1);

		case FAT_FS_TYPE_FAT16:
			return cluster * 2;

		case FAT_FS_TYPE_FAT32:
			return cluster * 4;
	}

	return 0xFFFFFFFF;
}


// allocates a cluster for a directory - finds a free cluster, initializes it as
// required by a directory, marks it's FAT sEntry as EOC and returns the cluster address
//
// NOTE: this function used the volume/shared uiBuffer (if enabled) so it must not be
// locked before calling this function
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::FatAllocateDirectoryCluster(SFatRawDirectoryEntry* parent, EFatCode* peResult)
{
	return FatAllocateCluster(parent, 1, 1, 1, peResult);
}


// allocates the requested number of clusters - finds the free clusters, initializes it to zeroes,
// links them as a cluster chain (marking the last one as EOC) and returns the cluster number for
// the 1st cluster in the chain
//
// NOTE: this function used the volume/shared uiBuffer (if enabled) so it must not be
// locked before calling this function
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::FatAllocateDataCluster(uint32 count, char zero, EFatCode* peResult)
{
	return FatAllocateCluster(0, count, zero, 1, peResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::FatAllocateDataClusterEx(uint32 count, char zero, uint32 uiPageSize, EFatCode* peResult)
{
	return FatAllocateCluster(0, count, zero, uiPageSize, peResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::FindNextPageCluster(uint32 uiPageSize, uint32 uiCluster, uint16 uiStep)
{
	// find the step between clusters allocated on page boundaries
	if (GetNoOfSectorsPerCluster() < uiPageSize)
	{
		uint32 uiSector;
		uint16 uiStepCount = 0;

		// find the 1st cluster that starts on a page boundary
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
uint16 CFatVolume::CalculateStepSize(uint32 uiPageSize)
{
	// find the step between clusters allocated on page boundaries
	if (GetNoOfSectorsPerCluster() < uiPageSize)
	{
		return (uint16)(uiPageSize / GetNoOfSectorsPerCluster());
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
fatEntry CFatVolume::GetEndOfClusterMarker(void)
{
	switch (GetFileSystemType())
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
EFatCode CFatVolume::FatReadFatSector(uint32 uiFatInfoSector)
{
	EFatCode	eResult;
	bool		bSuccess;

	if (!IsFatSectorLoaded(uiFatInfoSector))
	{
		eResult = FlushAndInvalidate();
		if (eResult != FAT_SUCCESS)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}

		bSuccess = Read(uiFatInfoSector, mauiFatSharedBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_READ_MEDIA;
		}
		muiFatSharedBufferSector = uiFatInfoSector;
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatReadFatSector(uint32 uiFatInfoSector, uint8* puiBuffer)
{
	bool		bSuccess;

	if (IsFatSectorLoaded(uiFatInfoSector))
	{
		memcpy(puiBuffer, mauiFatSharedBuffer, GetSectorSize());
		return FAT_SUCCESS;
	}
	else
	{
		bSuccess = Read(uiFatInfoSector, puiBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_READ_MEDIA;
		}
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatFlushFatSector(void)
{
	EFatCode	eResult;

	if (mbEntriesUpdated)
	{
		if (muiFatSharedBufferSector != FAT_UNKNOWN_SECTOR)
		{
			eResult = FatWriteFatSector(muiFatSharedBufferSector, mauiFatSharedBuffer);
			if (eResult != FAT_SUCCESS)
			{
				muiFatSharedBufferSector = FAT_UNKNOWN_SECTOR;
				return eResult;
			}

			mbEntriesUpdated = false;
		}
		else
		{
			return FAT_UNKNOWN_ERROR;
		}
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FlushAndInvalidate(void)
{
	EFatCode	eResult;

	eResult = FatFlushFatSector();
	muiFatSharedBufferSector = FAT_UNKNOWN_SECTOR;
	return eResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::ReadFat12Entry(uint32 uiClusterBytesRemainder, uint32 uiCluster, uint32 uiSector, fatEntry* puiFatEntry)
{
	fatEntry	uiFatEntry;
	bool		bNextSector;
	EFatCode	eResult;

	// clear uiFatEntry to make sure that the upper 16 bits are not set.
	uiFatEntry = 0;

	// read the 1st byte
	((uint8*)&uiFatEntry)[INT32_BYTE0] = mauiFatSharedBuffer[uiClusterBytesRemainder];

	// load the next sector (if necessary) and set the offset for the next byte in the uiBuffer
	if (uiClusterBytesRemainder == GetSectorSize() - 1)
	{
		// load the next sector into the uiBuffer
		eResult = FatReadFatSector(uiSector + 1);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// set flag to indicate that we had to load the next sector
		bNextSector = true;
	}
	else
	{
		bNextSector = false;
	}

	// read the 2nd byte
	((uint8*)&uiFatEntry)[INT32_BYTE1] = mauiFatSharedBuffer[bNextSector ? 0 : (uiClusterBytesRemainder + 1)];

	// Since a FAT12 sEntry is only 12 bits (1.5 bytes) we need to adjust the peResult.
	// For odd cluster numbers the FAT sEntry is stored in the upper 12 bits of the
	// 16 bits where it is stored, so we need to shift the value 4 bits to the right.
	// For even cluster numbers the FAT sEntry is stored in the lower 12 bits of the
	// 16 bits where it is stored, so we need to clear the upper 4 bits.
	if (uiCluster & 0x1)
	{
		uiFatEntry >>= 4;
	}
	else
	{
		uiFatEntry &= 0xFFF;
	}

	eResult = FatReadFatSector(uiSector);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}
	else
	{
		*puiFatEntry = uiFatEntry;
		return FAT_SUCCESS;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteFat12Entry(uint32 uiClusterBytesRemainder, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiLastFatEntry)
{
	bool		bNextSector;
	EFatCode	eResult;

	// write the 1st byte.
	if (uiCluster & 1)
	{
		//Odd cluster.
		mauiFatSharedBuffer[uiClusterBytesRemainder] &= 0x0F;
		mauiFatSharedBuffer[uiClusterBytesRemainder] |= LO8((uint16)FAT12_EOC << 4);	// set entry bits on 1st byte
	}
	else
	{
		//Even cluster.
		mauiFatSharedBuffer[uiClusterBytesRemainder] = LO8((uint16)FAT12_EOC);			// set entry bits on 1st byte
	}
	mbEntriesUpdated = true;

	// if the FAT sEntry spans a sector boundary flush the currently
	// loaded sector to the drive and load the next one.
	if (uiClusterBytesRemainder == GetSectorSize() - 1)
	{
		// load the next sector
		eResult = FatReadFatSector(uiSector + 1);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		bNextSector = true;
	}
	else
	{
		bNextSector = false;
	}

	// write the 2nd byte
	if (uiCluster & 1)
	{
		//Odd cluster.
		mauiFatSharedBuffer[bNextSector ? 0 : (uiClusterBytesRemainder + 1)] = HI8((uint16)FAT12_EOC << 4);
	}
	else
	{
		//Even cluster.
		mauiFatSharedBuffer[bNextSector ? 0 : (uiClusterBytesRemainder + 1)] &= 0xF0;
		mauiFatSharedBuffer[bNextSector ? 0 : (uiClusterBytesRemainder + 1)] |= HI8((uint16)FAT12_EOC);
	}
	mbEntriesUpdated = true;


	eResult = FatReadFatSector(uiSector);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	// if this is not the 1st cluster allocated update the last one to link to this one
	if (uiLastFatEntry != FAT12_EOC)
	{
		eResult = FatReadFatSector(uiPreviousSector);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// update the last sEntry to point to this one
		if (uiLastFatEntry & 1)
		{
			mauiFatSharedBuffer[uiPreviousOffset] &= 0x0F;							/* clear sEntry bits on 1st byte */
			mauiFatSharedBuffer[uiPreviousOffset] |= LO8((uint16)uiCluster << 4);		/* set sEntry bits on 1st byte */
		}
		else
		{
			mauiFatSharedBuffer[uiPreviousOffset] = LO8((uint16)uiCluster);			/* just copy the 1st byte */
		}
		mbEntriesUpdated = true;

		// if the FAT sEntry spans a sector boundary flush the currently
		// loaded sector to the drive and load the next one.
		if (uiPreviousOffset == GetSectorSize() - 1)
		{
			eResult = FatReadFatSector(uiPreviousOffset);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}

			// set the bNextSectorLoaded flag
			bNextSector = true;
		}

		// write the 2nd byte
		if (uiLastFatEntry & 1)
		{
			mauiFatSharedBuffer[bNextSector ? 0 : (uiPreviousOffset + 1)] = HI8((uint16)uiCluster << 4);
		}
		else
		{
			mauiFatSharedBuffer[bNextSector ? 0 : (uiPreviousOffset + 1)] &= 0xF0;
			mauiFatSharedBuffer[bNextSector ? 0 : (uiPreviousOffset + 1)] |= HI8((uint16)uiCluster);
		}
		mbEntriesUpdated = true;

		eResult = FatReadFatSector(uiSector);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteFat16Entry(uint32 uiClusterBytesRemainder, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiLastFatEntry)
{
	EFatCode	eResult;

	*((uint16*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) = FAT16_EOC;
	mbEntriesUpdated = true;

	// if this is not the first cluster allocated update the last one to link to this one.
	if (uiLastFatEntry != FAT16_EOC)
	{
		if (uiPreviousSector != uiSector)
		{
			eResult = FatReadFatSector(uiPreviousSector);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}
		}

		// update the last sEntry to point to this one
		*((uint16*)&mauiFatSharedBuffer[uiPreviousOffset]) = (uint16)uiCluster;
		mbEntriesUpdated = true;

		eResult = FatReadFatSector(uiSector);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteFat32Entry(uint32 uiClusterBytesRemainder, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiLastFatEntry)
{
	EFatCode	eResult;

	*((uint32*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) &= 0xF0000000;
	*((uint32*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) |= FAT32_EOC & 0x0FFFFFFF;
	mbEntriesUpdated = true;

	// if this is not the 1st cluster allocated update the last one to link to this one
	if (uiLastFatEntry != FAT32_EOC)
	{
		eResult = FatReadFatSector(uiPreviousSector);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// update the last sEntry to point to this one
		*((uint32*)&mauiFatSharedBuffer[uiPreviousOffset]) &= 0xF0000000;
		*((uint32*)&mauiFatSharedBuffer[uiPreviousOffset]) |= uiCluster & 0x0FFFFFFF;
		mbEntriesUpdated = true;

		eResult = FatReadFatSector(uiSector);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::FatAllocateCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 count, char zero, uint32 uiPageSize, EFatCode* peResult)
{
	uint32		uiFirstClusterSector;
	uint32		uiCurrentSector;
	uint32		uiClusterIndexInTable;					
	uint32		uiClusterBytesRemainder = 0;
	bool		bNextSectorLoaded = false;
	fatEntry	uiLastFatEntry = 0;			
	fatEntry	uiFatEntry;					
	uint32		uiFirstCluster;
	uint32		uiPreviousClusterSector = 0;
	uint32		last_entry_offset = 0;
	uint32		uiStartCluster;
	bool		bWrappedAround;
	uint16		step = 1;
	EFatCode	eResult;

	if ((count > 1) && (psParentDirectory != NULL))
	{
		*peResult = FAT_UNKNOWN_ERROR;
		return 0;
	}

	if (count == 0)
	{
		*peResult = FAT_UNKNOWN_ERROR;
		return 0;
	}

	uiFatEntry = 0;
	uiClusterIndexInTable = 0x2;
	uiFirstCluster = 0;

	if (HasNextFreeCluser())
	{
		uiClusterIndexInTable = GetNextFreeCluster();
	}

	step = CalculateStepSize(uiPageSize);
	uiClusterIndexInTable = FindNextPageCluster(uiPageSize, uiClusterIndexInTable, step);

	uiStartCluster = uiClusterIndexInTable;

	uiLastFatEntry = GetEndOfClusterMarker();

	uiClusterBytesRemainder = CalculateFatEntryOffset(uiClusterIndexInTable);
	uiFirstClusterSector = GetNoOfReservedSectors() + (uiClusterBytesRemainder / GetSectorSize());
	uiClusterBytesRemainder = uiClusterBytesRemainder % GetSectorSize();
	uiPreviousClusterSector = uiFirstClusterSector;

	for (;;)
	{
		*peResult = FatReadFatSector(uiFirstClusterSector);
		if (*peResult != FAT_SUCCESS)
		{
			return 0;
		}

		uiCurrentSector = uiFirstClusterSector;
		bWrappedAround = false;

		do
		{
			if ((uiClusterIndexInTable > GetNoOfClusters() + 1))
			{
				if (uiStartCluster > 2 && !bWrappedAround)
				{
					uiClusterIndexInTable = 2;
					bWrappedAround = true;

					// calculate the sector for the new cluster
					uiClusterBytesRemainder = CalculateFatEntryOffset(uiClusterIndexInTable);
					uiFirstClusterSector = GetNoOfReservedSectors();
					uiClusterBytesRemainder = uiClusterBytesRemainder % GetSectorSize();

					// break from this loop so that sector gets loaded
					break;
				}
				else if (bWrappedAround && (uiClusterIndexInTable >= uiStartCluster))
				{
					eResult = FatFlushFatSector();
					if (eResult != FAT_SUCCESS)
					{
						*peResult = FAT_INSUFFICIENT_DISK_SPACE;
						return 0;
					}

					// free any clusters that we've allocated so far
					if (!FatIsEOFEntry(uiLastFatEntry))
					{
						FatFreeClusterChain(uiLastFatEntry);
					}

					// return insufficient disk space error
					*peResult = FAT_INSUFFICIENT_DISK_SPACE;
					return 0;
				}
			}

			// copy the next FAT entry to the fatEntry variable
			switch (GetFileSystemType())
			{
				case FAT_FS_TYPE_FAT12:
				{
					eResult = ReadFat12Entry(uiClusterBytesRemainder, uiClusterIndexInTable, uiFirstClusterSector, &uiFatEntry);
					if (eResult != FAT_SUCCESS)
						return 0;
					break;
				}
				case FAT_FS_TYPE_FAT16:
				{
					uiFatEntry = *((uint16*)&mauiFatSharedBuffer[uiClusterBytesRemainder]);
					break;
				}
				case FAT_FS_TYPE_FAT32:
				{
					uiFatEntry = *((uint32*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) & 0x0FFFFFFF;
					break;
				}
			}

			// if the current FAT is free
			bool bIsFatFree = IsFreeFat(uiFatEntry);
			if (bIsFatFree)
			{
				// maintain the count of free cluster and the next cluster that may be free.
				SetNextFreeCluster(uiClusterIndexInTable + 1);
				SetTotalFreeClusters(GetTotalFreeClusters() - 1);

				// if this is the 1st cluster found remember it
				if (!uiFirstCluster)
				{
					uiFirstCluster = uiClusterIndexInTable;
				}

				// mark the FAT as the the new 1st link of the cluster chain (or the end of the chain if we're only allocating 1 cluster)
				switch (GetFileSystemType())
				{
					case FAT_FS_TYPE_FAT12:
					{
						eResult = WriteFat12Entry(uiClusterBytesRemainder, last_entry_offset, uiClusterIndexInTable, uiFirstClusterSector, uiPreviousClusterSector, uiLastFatEntry);
						if (eResult != FAT_SUCCESS)
							return 0;
						break;
					}
					case FAT_FS_TYPE_FAT16:
					{
						eResult = WriteFat16Entry(uiClusterBytesRemainder, last_entry_offset, uiClusterIndexInTable, uiFirstClusterSector, uiPreviousClusterSector, uiLastFatEntry);
						if (eResult != FAT_SUCCESS)
							return 0;
						break;
					}
					case FAT_FS_TYPE_FAT32:
					{
						eResult = WriteFat32Entry(uiClusterBytesRemainder, last_entry_offset, uiClusterIndexInTable, uiFirstClusterSector, uiPreviousClusterSector, uiLastFatEntry);
						if (eResult != FAT_SUCCESS)
							return 0;
						break;
					}
				}

				count--;

				// if we've found all the clusters that the user requested leave
				// and return cluster #
				if (count == 0)
				{
					// if we're allocating a cluster for a directory then initialize it properly
					if (psParentDirectory)
					{
						EFatCode uiResult;
						uiResult = FatInitializeDirectoryCluster(psParentDirectory, uiClusterIndexInTable);
						if (uiResult != FAT_SUCCESS)
						{
							muiFatSharedBufferSector = FAT_UNKNOWN_SECTOR;
							*peResult = uiResult;
							return 0;
						}
					}
					else
					{
						if (zero)
						{
							*peResult = FatZeroCluster(uiClusterIndexInTable);
							muiFatSharedBufferSector = FAT_UNKNOWN_SECTOR;
							if (*peResult != FAT_SUCCESS)
							{
								return 0;
							}
						}
					}

					*peResult = FlushAndInvalidate();
					if (*peResult != FAT_SUCCESS)
					{
						return 0;
					}
					return uiFirstCluster;
				}

				// remember the cluster number so we can mark the next fat entry with it
				uiLastFatEntry = uiClusterIndexInTable;
				uiPreviousClusterSector = uiFirstClusterSector;
				last_entry_offset = uiClusterBytesRemainder;
			}

			// increase the cluster number
			if (!uiFirstCluster)
			{
				uiClusterIndexInTable += step;
			}
			else
			{
				uiClusterIndexInTable++;
			}

			// calculate the offset of the cluster's FAT sEntry within it's sector
			// note: when we hit get past the end of the current sector uiClusterBytesRemainder
			// will roll back to zero (or possibly 1 for FAT12)
			uiClusterBytesRemainder = CalculateFatEntryOffset(uiClusterIndexInTable);
			uiFirstClusterSector = GetNoOfReservedSectors() + (uiClusterBytesRemainder / GetSectorSize());
			uiClusterBytesRemainder = uiClusterBytesRemainder % GetSectorSize();

		} while (uiCurrentSector == uiFirstClusterSector);
	}
}


// marks all the clusters in the cluster chain as free
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatFreeClusterChain(uint32 cluster)
{
	EFatCode	eResult;
	uint32		uiClusterOffsetInBytes = 0;					// the offset of the cluster sEntry within the FAT table
	uint32		uiClusterBytesRemainder;					// the offset of the cluster sEntry within it's sector
	uint32		uiFirstClusterSector;						// the sector where the sEntry is stored on the drive
	uint32		uiCurrentSector;							// the sector that's currently loaded in memory
	char		bFat12OddClusterBeingProcessed = false;		// indicates that the sEntry being processed is an odd cluster address (FAT12 only)
	char		bFat12MultiStepProgress = false;			// indicates that a multi-step operation is in progress (FAT12 only)

	// get the offset of the cluster sEntry within the FAT table,
	// the sector of the FAT table that contains the sEntry and the offset
	// of the fat sEntry within the sector
	uiClusterOffsetInBytes = CalculateFatEntryOffset(cluster);
	uiFirstClusterSector = GetNoOfReservedSectors() + (uiClusterOffsetInBytes / GetSectorSize());
	uiClusterBytesRemainder = uiClusterOffsetInBytes % GetSectorSize();

	// loop until we reach the EOC cluster or an error occurs.
	for (;;)
	{
		// load sector to memory
		if (!IsFatSectorLoaded(uiFirstClusterSector))
		{
			eResult = FatReadFatSector(uiFirstClusterSector);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}
		}

		// store the address of the sector that's (will be) loaded in memory
		uiCurrentSector = uiFirstClusterSector;

		// loop until a new sector needs to be loaded to continue
		// with the operation
		while (uiCurrentSector == uiFirstClusterSector)
		{
			// if cluster is less than 2 either we got a bug
			// or the file system is corrupted
			if (cluster < 2)
			{
				// leave critical section and return error code
				return FAT_INVALID_CLUSTER;
			}
			// read the cluster sEntry and mark it as free
			switch (GetFileSystemType())
			{
			case FAT_FS_TYPE_FAT12:
			{
				if (!bFat12MultiStepProgress)
				{
					// remember whether this is an odd cluster or not
					bFat12OddClusterBeingProcessed = (cluster & 0x1);

					// set the cluster to zero to make sure that the upper bytes are cleared
					// since we're only updating the lower 16 bits.
					cluster = 0;

					// read the 1st byte
					((uint8*)&cluster)[0] = mauiFatSharedBuffer[uiClusterBytesRemainder];

					// write the 1st byte
					//
					// note: since the value that we're writting is FREE_FAT which expands
					// to zero we can skip a step or two in the update process. I left the code
					// for the steps that we're skipping commented out for clarity.
					if (bFat12OddClusterBeingProcessed)
					{
						mauiFatSharedBuffer[uiClusterBytesRemainder] &= 0x0F;
					}
					else
					{
						mauiFatSharedBuffer[uiClusterBytesRemainder] = FREE_FAT;
					}
					mbEntriesUpdated = true;
				}

				if (uiClusterBytesRemainder == GetSectorSize() - 1)
				{
					// if the sEntry spans a sector boundary set bFat12MultiStepProgress to 1
					// so that we don't read the 1st byte again when we come back.
					// also increase the sector number and set the uiClusterBytesRemainder to 0 since
					// the next byte will be on offset zero when the next sector is loaded
					uiFirstClusterSector++;
					uiClusterBytesRemainder = 0;
					bFat12MultiStepProgress = 1;

					// continue with the next iteration of the loop. We'll come right back
					// here with the next sector loaded
					continue;
				}
				else if (!bFat12MultiStepProgress)
				{
					// increase the offset to point to the next byte
					uiClusterBytesRemainder++;
				}

				// read the 2nd byte
				((uint8*)&cluster)[1] = mauiFatSharedBuffer[uiClusterBytesRemainder];

				// Since a FAT12 sEntry is only 12 bits (1.5 bytes) we need to adjust the result.
				// For odd cluster numbers the FAT sEntry is stored in the upper 12 bits of the
				// 16 bits where it is stored, so we need to shift the value 4 bits to the right.
				// For even cluster numbers the FAT sEntry is stored in the lower 12 bits of the
				// 16 bits where it is stored, so we need to clear the upper 4 bits.
				if (bFat12OddClusterBeingProcessed)
				{
					cluster >>= 4;
				}
				else
				{
					cluster &= 0xFFF;
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
				if (bFat12OddClusterBeingProcessed)
				{
					mauiFatSharedBuffer[uiClusterBytesRemainder] = FREE_FAT;
				}
				else
				{
					mauiFatSharedBuffer[uiClusterBytesRemainder] &= 0xF0;	/* clear bits that 1st byte will be written to */
				}
				mbEntriesUpdated = true;

				// clear bFat12MultiStepProgress flag.
				bFat12MultiStepProgress = 0;
				break;
			}
			case FAT_FS_TYPE_FAT16:
			{
				cluster = *((uint16*)&mauiFatSharedBuffer[uiClusterBytesRemainder]);
				*((uint16*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) = FREE_FAT;
				break;
			}
			case FAT_FS_TYPE_FAT32:
			{
				// FAT32 entries are actually 28 bits so we need to leave the
				// upper nibble untouched
				cluster = *((uint32*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) & 0x0FFFFFFF;
				*((uint32*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) &= 0xF0000000;
				break;
			}
			}

			// increase the count of free clusters
			SetTotalFreeClusters(GetTotalFreeClusters() + 1);

			// if it's the EOF marker we're done, flush the uiBuffer and go
			if (FatIsEOFEntry(cluster))
			{
				eResult = FlushAndInvalidate();
				if (eResult != FAT_SUCCESS)
				{
					return eResult;
				}
				return FAT_SUCCESS;
			}

			// calculate the location of the next cluster in the chain
			uiClusterOffsetInBytes = CalculateFatEntryOffset(cluster);
			uiFirstClusterSector = GetNoOfReservedSectors() + (uiClusterOffsetInBytes / GetSectorSize());
			uiClusterBytesRemainder = uiClusterOffsetInBytes % GetSectorSize();
		}
	}
}


// gets the FAT structure for a given cluster number
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::GetNextClusterEntry(uint32 uiCurrentCluster, uint32* puiNextCluster)
{
	uint32		uiClusterOffsetInBytes = 0;		/* todo: this one may require 64 bits for large drives? */
	uint32		uiFirstClusterSector;
	uint32		uiClusterBytesRemainder;		/* todo: 16 bits should suffice for this value */
	EFatCode	eResult;

	// get the offset of the sEntry within the FAT table
	// for the requested cluster

	uiClusterOffsetInBytes = CalculateFatEntryOffset(uiCurrentCluster);

	// get the address of the sector that contains the FAT sEntry and
	// the offset of the FAT sEntry within that sector
	uiFirstClusterSector = GetNoOfReservedSectors() + (uiClusterOffsetInBytes / GetSectorSize());
	uiClusterBytesRemainder = uiClusterOffsetInBytes % GetSectorSize();

	eResult = FatReadFatSector(uiFirstClusterSector);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	// set the user supplied uiBuffer with the
	// value of the FAT sEntry
	switch (GetFileSystemType())
	{
		case FAT_FS_TYPE_FAT12:
		{
			// clear uiFatEntry to make sure that the upper 16
			// bits are not set.
			*puiNextCluster = 0;

			// read the 1st byte

			((uint8*)puiNextCluster)[0] = mauiFatSharedBuffer[uiClusterBytesRemainder];

			// load the next sector (if necessary) and set the offset
			// for the next byte in the uiBuffer
			if (uiClusterBytesRemainder == GetSectorSize() - 1)
			{
				// load the next sector into the uiBuffer
				eResult = FatReadFatSector(uiFirstClusterSector + 1);
				if (eResult != FAT_SUCCESS)
				{
					return eResult;
				}

				// the 2nd byte is now the 1st byte in the uiBuffer
				uiClusterBytesRemainder = 0;
			}
			else
			{
				// the 2nd byte is still right after the 1st one on
				// the uiBuffer
				uiClusterBytesRemainder++;
			}

			// read the 2nd byte
			((uint8*)puiNextCluster)[1] = mauiFatSharedBuffer[uiClusterBytesRemainder];

			// Since a FAT12 sEntry is only 12 bits (1.5 bytes) we need to adjust the result.
			// For odd cluster numbers the FAT sEntry is stored in the upper 12 bits of the
			// 16 bits where it is stored, so we need to shift the value 4 bits to the right.
			// For even cluster numbers the FAT sEntry is stored in the lower 12 bits of the
			// 16 bits where it is stored, so we need to clear the upper 4 bits.
			if (uiCurrentCluster & 0x1)
			{
				*puiNextCluster >>= 4;
			}
			else
			{
				*puiNextCluster &= 0xFFF;
			}
			break;
		}
		case FAT_FS_TYPE_FAT16:
		{
			*puiNextCluster = *((uint16*)&mauiFatSharedBuffer[uiClusterBytesRemainder]);
			break;
		}
		case FAT_FS_TYPE_FAT32:
		{
			*puiNextCluster = *((uint32*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) & 0x0FFFFFFF;
			break;
		}
	}

	return FAT_SUCCESS;
}


// updates the FAT sEntry for a given cluster
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatSetClusterEntry(uint32 uiCluster, fatEntry uiFatEntry)
{
	uint32		uiClusterOffsetInBytes = 0;
	uint32		uiFirstClusterSector;
	uint32		uiClusterBytesRemainder;
	EFatCode	eResult;

	uiClusterOffsetInBytes = CalculateFatEntryOffset(uiCluster);

	// get the address of the sector that contains the FAT sEntry
	// and the offset of the FAT sEntry within that sector
	uiFirstClusterSector = GetNoOfReservedSectors() + (uiClusterOffsetInBytes / GetSectorSize());
	uiClusterBytesRemainder = uiClusterOffsetInBytes % GetSectorSize();

	// read sector into uiBuffer
	if (!IsFatSectorLoaded(uiFirstClusterSector))
	{
		eResult = FatReadFatSector(uiFirstClusterSector);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}

	// set the FAT sEntry
	switch (GetFileSystemType())
	{
		case FAT_FS_TYPE_FAT12:
		{
			// write the 1st byte
			if (uiCluster & 1)
			{
				uiFatEntry <<= 4;												// odd entries occupy the upper 12 bits so we must shift
				mauiFatSharedBuffer[uiClusterBytesRemainder] &= 0x0F;						// clear sEntry bits on 1st byte
				mauiFatSharedBuffer[uiClusterBytesRemainder] |= LO8((uint16)uiFatEntry);	// set sEntry bits on 1st byte
			}
			else
			{
				mauiFatSharedBuffer[uiClusterBytesRemainder] = LO8((uint16)uiFatEntry);	/* just copy the 1st byte */
			}
			mbEntriesUpdated = true;

			// if the FAT sEntry spans a sector boundary flush the currently
			// loaded sector to the drive and load the next one.
			if (uiClusterBytesRemainder == GetSectorSize() - 1)
			{
				// increase the sector address
				uiFirstClusterSector++;

				// load the next sector
				eResult = FatReadFatSector(uiFirstClusterSector);
				if (eResult != FAT_SUCCESS)
				{
					return eResult;
				}

				// the next byte is now loacted at offset 0 on the uiBuffer
				uiClusterBytesRemainder = 0;
			}
			else
			{
				// the next byte is located next to the 1st one on the uiBuffer
				uiClusterBytesRemainder++;
			}

			// write the 2nd byte
			if (uiCluster & 1)
			{
				mauiFatSharedBuffer[uiClusterBytesRemainder] = HI8((uint16)uiFatEntry);		/* just copy the 1st byte */
			}
			else
			{
				mauiFatSharedBuffer[uiClusterBytesRemainder] &= 0xF0;						/* clear bits that 1st byte will be written to */
				mauiFatSharedBuffer[uiClusterBytesRemainder] |= HI8((uint16)uiFatEntry);	/* copy sEntry bits of 1st byte */
			}
			mbEntriesUpdated = true;
			break;
		}
		case FAT_FS_TYPE_FAT16:
		{
			*((uint16*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) = (uint16)uiFatEntry;
			mbEntriesUpdated = true;
			break;
		}
		case FAT_FS_TYPE_FAT32:
		{

			// since a FAT32 sEntry is actually 28 bits we need
			// to make sure that we don't modify the upper nibble.
			*((uint32*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) &= 0xF0000000;
			*((uint32*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) |= uiFatEntry & 0x0FFFFFFF;
			mbEntriesUpdated = true;
			break;
		}
	}

	eResult = FatFlushFatSector();
	return eResult;
}


// increase a cluster address by the amount of clusters indicated by count. This function will
// follow the FAT sEntry chain to fat the count-th cluster allocated to a file relative from the
// input cluster
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatIncreaseClusterAddress(uint32 uiCluster, uint16 count, uint32* puiNewCluster)
{
	uint32		uiClusterOffsetInBytes;
	uint32		uiClusterBytesRemainder;
	uint32		uiFirstClusterSector;
	uint32		uiCurrentSector;
	char		bFat12OddClusterBeingProcessed = 0;
	char		bFat12MultiStepProgress = 0;
	EFatCode	eResult;

	// if the count is zero we just return the same
	// uiCluster that we received
	if (!count)
	{
		*puiNewCluster = (uint32)uiCluster;
		return FAT_SUCCESS;
	}

	// get the offset of the uiCluster sEntry within the FAT table, the sector of the FAT table that contains the sEntry and the offset of the fat sEntry within the sector
	uiClusterOffsetInBytes = CalculateFatEntryOffset(uiCluster);
	uiFirstClusterSector = GetNoOfReservedSectors() + (uiClusterOffsetInBytes / GetSectorSize());
	uiClusterBytesRemainder = uiClusterOffsetInBytes % GetSectorSize();

	for (;;)
	{
		uiCurrentSector = uiFirstClusterSector;

		eResult = FatReadFatSector(uiCurrentSector);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// free all the fat entries on the current sector
		while (uiCurrentSector == uiFirstClusterSector)
		{
			// make sure we don't try to free an invalid uiCluster
			if (uiCluster < 2)
			{
				return FAT_INVALID_CLUSTER;
			}

			// read the uiCluster sEntry and mark it as free
			switch (GetFileSystemType())
			{
				case FAT_FS_TYPE_FAT12:
				{
					if (!bFat12MultiStepProgress)
					{
						// remember whether this is an odd uiCluster or not
						bFat12OddClusterBeingProcessed = (uiCluster & 0x1);

						// set the uiCluster to zero to make sure that the upper bytes are cleared
						// since we're only updating the lower 16 bits.
						uiCluster = 0;

						// read the 1st byte
						((uint8*)&uiCluster)[0] = mauiFatSharedBuffer[uiClusterBytesRemainder];
					}

					if (uiClusterBytesRemainder == GetSectorSize() - 1)
					{
						// if the sEntry spans a sector boundary set bFat12MultiStepProgress to 1
						// so that we don't read the 1st byte again when we come back.
						// also increase the sector number and set the uiClusterBytesRemainder to 0 since
						// the next byte will be on offset zero when the next sector is loaded
						uiFirstClusterSector++;
						uiClusterBytesRemainder = 0;
						bFat12MultiStepProgress = 1;

						// continue with the next iteration of the loop. We'll come right back
						// here with the next sector loaded
						continue;
					}
					else if (!bFat12MultiStepProgress)
					{
						// increase the offset to point to the next byte
						uiClusterBytesRemainder++;
					}

					// read the 2nd byte
					((uint8*)&uiCluster)[1] = mauiFatSharedBuffer[uiClusterBytesRemainder];

					// Since a FAT12 sEntry is only 12 bits (1.5 bytes) we need to adjust the result.
					// For odd uiCluster numbers the FAT sEntry is stored in the upper 12 bits of the
					// 16 bits where it is stored, so we need to shift the puiNewCluster 4 bits to the right.
					// For even uiCluster numbers the FAT sEntry is stored in the lower 12 bits of the
					// 16 bits where it is stored, so we need to clear the upper 4 bits.
					if (bFat12OddClusterBeingProcessed)
					{
						uiCluster >>= 4;
					}
					else
					{
						uiCluster &= 0xFFF;
					}

					// clear bFat12MultiStepProgress flag.
					bFat12MultiStepProgress = 0;
					break;
				}
				case FAT_FS_TYPE_FAT16:
				{
					uiCluster = *((uint16*)&mauiFatSharedBuffer[uiClusterBytesRemainder]);
					break;
				}
				case FAT_FS_TYPE_FAT32:
				{
					uiCluster = *((uint32*)&mauiFatSharedBuffer[uiClusterBytesRemainder]) & 0x0FFFFFFF;
					break;
				}
			}

			// if the last uiCluster marks the end of the chian we return
			// false as we cannot increase the address by the # of clusters
			// requested by the caller
			if (FatIsEOFEntry(uiCluster))
			{
				return FAT_UNKNOWN_ERROR;
			}

			// if we've followed the number of clusters requested by
			// the caller set the return puiNewCluster to the current uiCluster
			// and return success
			if (!--count)
			{
				*puiNewCluster = (uint32)uiCluster;
				return FAT_SUCCESS;
			}

			// calculate the location of the next uiCluster in the chain
			uiClusterOffsetInBytes = CalculateFatEntryOffset(uiCluster);
			uiFirstClusterSector = GetNoOfReservedSectors() + (uiClusterOffsetInBytes / GetSectorSize());
			uiClusterBytesRemainder = uiClusterOffsetInBytes % GetSectorSize();
		}
	}
}


// checks if a fat sEntry represents the
// last sEntry of a file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::FatIsEOFEntry(fatEntry uiFat)
{
	switch (GetFileSystemType())
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


// initializes a directory cluster
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatInitializeDirectoryCluster(SFatRawDirectoryEntry* parent, uint32 cluster)
{
	uint16					counter;
	uint32					uiCurrentSector;
	SFatRawDirectoryEntry*	entries;
	EFatCode				eResult;

	FlushAndInvalidate();

	// if this is a big endian system or the compiler does not support
	// struct packing we cannot use SFatRawDirectoryEntry to write directly
	// to the uiBuffer
	entries = (SFatRawDirectoryEntry*)mauiFatSharedBuffer;

	// initialize the 1st sector of the directory cluster with
	// the dot sEntry
	memset(mauiFatSharedBuffer, 0, GetSectorSize());
	entries->uEntry.sFatRawCommon.name[0x0] = '.';
	entries->uEntry.sFatRawCommon.name[0x1] = ' ';
	entries->uEntry.sFatRawCommon.name[0x2] = ' ';
	entries->uEntry.sFatRawCommon.name[0x3] = ' ';
	entries->uEntry.sFatRawCommon.name[0x4] = ' ';
	entries->uEntry.sFatRawCommon.name[0x5] = ' ';
	entries->uEntry.sFatRawCommon.name[0x6] = ' ';
	entries->uEntry.sFatRawCommon.name[0x7] = ' ';
	entries->uEntry.sFatRawCommon.name[0x8] = ' ';
	entries->uEntry.sFatRawCommon.name[0x9] = ' ';
	entries->uEntry.sFatRawCommon.name[0xA] = ' ';
	entries->uEntry.sFatRawCommon.attributes = FAT_ATTR_DIRECTORY;
	entries->uEntry.sFatRawCommon.size = 0x0;
	entries->uEntry.sFatRawCommon.reserved = 0;
	entries->uEntry.sFatRawCommon.first_cluster_lo = LO16(cluster);
	entries->uEntry.sFatRawCommon.first_cluster_hi = HI16(cluster);
	entries->uEntry.sFatRawCommon.create_date = GetSystemClockDate();
	entries->uEntry.sFatRawCommon.create_time = GetSystemClockTime();
	entries->uEntry.sFatRawCommon.modify_date = entries->uEntry.sFatRawCommon.create_date;
	entries->uEntry.sFatRawCommon.modify_time = entries->uEntry.sFatRawCommon.create_time;
	entries->uEntry.sFatRawCommon.access_date = entries->uEntry.sFatRawCommon.create_date;
	entries->uEntry.sFatRawCommon.create_time_tenth = 0xc6;

	// write the sEntry to the uiBuffer or move to the next sEntry
	// as required by target platform
	entries++;

	// initialize the dot dot sEntry
	entries->uEntry.sFatRawCommon.name[0x0] = '.';
	entries->uEntry.sFatRawCommon.name[0x1] = '.';
	entries->uEntry.sFatRawCommon.name[0x2] = ' ';
	entries->uEntry.sFatRawCommon.name[0x3] = ' ';
	entries->uEntry.sFatRawCommon.name[0x4] = ' ';
	entries->uEntry.sFatRawCommon.name[0x5] = ' ';
	entries->uEntry.sFatRawCommon.name[0x6] = ' ';
	entries->uEntry.sFatRawCommon.name[0x7] = ' ';
	entries->uEntry.sFatRawCommon.name[0x8] = ' ';
	entries->uEntry.sFatRawCommon.name[0x9] = ' ';
	entries->uEntry.sFatRawCommon.name[0xA] = ' ';
	entries->uEntry.sFatRawCommon.attributes = FAT_ATTR_DIRECTORY;
	entries->uEntry.sFatRawCommon.size = 0x0;
	entries->uEntry.sFatRawCommon.reserved = 0;
	entries->uEntry.sFatRawCommon.first_cluster_lo = parent->uEntry.sFatRawCommon.first_cluster_lo;
	entries->uEntry.sFatRawCommon.first_cluster_hi = parent->uEntry.sFatRawCommon.first_cluster_hi;
	entries->uEntry.sFatRawCommon.create_date = GetSystemClockDate();
	entries->uEntry.sFatRawCommon.create_time = GetSystemClockTime();
	entries->uEntry.sFatRawCommon.modify_date = entries->uEntry.sFatRawCommon.create_date;
	entries->uEntry.sFatRawCommon.modify_time = entries->uEntry.sFatRawCommon.create_time;
	entries->uEntry.sFatRawCommon.access_date = entries->uEntry.sFatRawCommon.create_date;
	entries->uEntry.sFatRawCommon.create_time_tenth = 0xc6;

	mbEntriesUpdated = true;

	// when the parent is the root directory the dotdot sEntry always points
	// to cluster 0, even in FAT32 when the root directory is not actually on
	// cluster 0 so we need to check if the parent is the root directory and
	// in that case set the 1st cluster to 0
	if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
	{
		uint32 uiParentCluster;

		uiParentCluster = GetFatClusterFromFatEntry(parent, true);
		if (GetRootCluster() == uiParentCluster)
		{
			entries->uEntry.sFatRawCommon.first_cluster_lo = 0;
			entries->uEntry.sFatRawCommon.first_cluster_hi = 0;
			mbEntriesUpdated = true;
		}
	}

	// write the 1st sector of the folder
	uiCurrentSector = CalculateFirstSectorOfCluster(cluster);
	muiFatSharedBufferSector = uiCurrentSector;
	eResult = FatFlushFatSector();
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}
	uiCurrentSector++;

	// clear the . and .. entries from the uiBuffer and
	// initialize the rest of the sectors of this cluster
	memset(mauiFatSharedBuffer, 0, sizeof(SFatRawDirectoryEntry) * 2);
	counter = GetNoOfSectorsPerCluster() - 1;
	while (counter--)
	{
		muiFatSharedBufferSector = uiCurrentSector;
		eResult = FatFlushFatSector();
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
		uiCurrentSector++;
	}

	eResult = FatFlushFatSector();
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatZeroCluster(uint32 cluster)
{
	bool	bSuccess;
	uint16	counter;
	uint32	uiCurrentSector;

	FlushAndInvalidate();

	// set all the bytes in the uiBuffer to zero
	memset(mauiFatSharedBuffer, 0, GetSectorSize());

	// calculate the address of the 1st sector
	// of the cluster
	uiCurrentSector = CalculateFirstSectorOfCluster(cluster);
	counter = GetNoOfSectorsPerCluster();

	// write the zeroed uiBuffer to every sector in the cluster
	while (counter--)
	{
		bSuccess = Write(uiCurrentSector, mauiFatSharedBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}
		uiCurrentSector++;
	}

	return FAT_SUCCESS;
}


// writes a sector of the FAT table to the active table and (if option is enabled)
// to all other FAT tables
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatWriteFatSector(uint32 uiSectorAddress, uint8* puiBuffer)
{
	bool bSuccess;

	// write the sector in the active FAT table
	bSuccess = Write(uiSectorAddress, puiBuffer);
	if (!bSuccess)
	{
		return FAT_CANNOT_WRITE_MEDIA;
	}

	// if we got more than one FAT table update the others as well
#if defined(FAT_MAINTAIN_TWO_FAT_TABLES)
	if (uiNoOfFatTables > 1)
	{
		int i;
		for (i = 1; i < uiNoOfFatTables; i++)
		{
			bSuccess = device->Write(uiSectorAddress + (fat_size * i), uiBuffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_WRITE_MEDIA;
			}
		}
	}
#endif
	return FAT_SUCCESS;
}


// finds the first file in a directory
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatFindFirstEntry(char* szParentPath, uint8 attributes, SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* query)
{
	EFatCode			eResult;
	SFatDirectoryEntry	sParentEntry;

	// if the path starts with a backslash then advance to
	// the next character
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
		eResult = FatGetFileEntry(szParentPath, &sParentEntry);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// try to get the 1st sEntry of the
		// query results
		eResult = FatQueryFirstEntry(&sParentEntry.raw, attributes, &query->sQueryState, 0);
	}
	// if the parent was not supplied then we
	// submit the query without it
	else
	{
		eResult = FatQueryFirstEntry(0, attributes, &query->sQueryState, 0);
	}

	// if we cant get the 1st sEntry then return the
	// error that we received from fat_Query_First_entry
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	// if there are no more entries
	if (query->sQueryState.sCurrentEntryRaw == 0)
	{
		*query->sCurrentEntry.name = 0;
		return FAT_SUCCESS;
	}

	// fill the current entry structure with data from
	// the current raw entry of the query
	FatFillDirectoryEntryFromRaw(&query->sCurrentEntry, query->sQueryState.sCurrentEntryRaw);

	// calculate the sector address of the entry - if query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is different.
	if (query->sQueryState.uiCurrentCluster == 0)
	{
		query->sCurrentEntry.uiSectorAddress = GetNoOfReservedSectors() +  (GetNoOfFatTables() * GetFatSize()) + query->sQueryState.uiCurrentSector;
	}
	else
	{
		query->sCurrentEntry.uiSectorAddress = CalculateFirstSectorOfCluster(query->sQueryState.uiCurrentCluster) + query->sQueryState.uiCurrentSector;
	}

	// calculate the offset of the entry within it's sector
	query->sCurrentEntry.uiSectorOffset = (uint16)((uintptr_t)query->sQueryState.sCurrentEntryRaw) - ((uintptr_t)mauiFatSharedBuffer);

	// store a copy of the original FAT directory entry within the SFatDirectoryEntry structure that is returned to users
	query->sCurrentEntry.raw = *query->sQueryState.sCurrentEntryRaw;

	// if long filenames are enabled copy the filename to the entry
	if (*query->sCurrentEntry.name != 0)
	{
		if (*query->sQueryState.long_filename != 0)
		{
			uint16	uiIndex;
			for (uiIndex = 0; uiIndex < 256; uiIndex++)
			{
				query->sCurrentEntry.name[uiIndex] = (uint8)query->sQueryState.long_filename[uiIndex];
				if (query->sQueryState.long_filename[uiIndex] == 0)
				{
					break;
				}
			}
		}
	}

	// if the user provided a pointer-to-pointer to the
	// result set it to the current sEntry.
	if (ppsDirectoryEntry)
	{
		*ppsDirectoryEntry = &query->sCurrentEntry;
	}

	// return success code
	return FAT_SUCCESS;
}

// finds the next file in the directory
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatFindNextEntry(SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* query)
{
	EFatCode				uiResult;

	// try to get the next entry of the query
	uiResult = FatQueryNextEntry(&query->sQueryState, false, false);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if there are no more entries
	if (query->sQueryState.sCurrentEntryRaw == 0)
	{
		// set the filename of the current sEntry to 0
		*query->sCurrentEntry.name = 0;
		return FAT_SUCCESS;
	}

	// fill the current sEntry structure with data from
	// the current raw sEntry of the query
	FatFillDirectoryEntryFromRaw(&query->sCurrentEntry, query->sQueryState.sCurrentEntryRaw);

	// calculate the sector address of the sEntry - if
	// query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is
	// different
	if (query->sQueryState.uiCurrentCluster == 0x0)
	{
		query->sCurrentEntry.uiSectorAddress = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize()) + query->sQueryState.uiCurrentSector;
	}
	else
	{
		query->sCurrentEntry.uiSectorAddress = CalculateFirstSectorOfCluster(query->sQueryState.uiCurrentCluster) + query->sQueryState.uiCurrentSector;
	}

	// calculate the offset of the sEntry within it's sector
	query->sCurrentEntry.uiSectorOffset = (uint16)((uintptr_t)query->sQueryState.sCurrentEntryRaw) - ((uintptr_t)mauiFatSharedBuffer);

	// store a copy of the original FAT directory sEntry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	query->sCurrentEntry.raw = *query->sQueryState.sCurrentEntryRaw;

	if (*query->sCurrentEntry.name != 0)
	{
		if (*query->sQueryState.long_filename != 0)
		{
			uint16 uiIndex;
			for (uiIndex = 0; uiIndex < 256; uiIndex++)
			{
				query->sCurrentEntry.name[uiIndex] = (uint8)query->sQueryState.long_filename[uiIndex];
				if (query->sQueryState.long_filename[uiIndex] == 0)
				{
					break;
				}
			}
		}
	}

	if (ppsDirectoryEntry)
	{
		*ppsDirectoryEntry = &query->sCurrentEntry;
	}

	return FAT_SUCCESS;
}

// fills a directory sEntry structure with data taken
// from a raw directory sEntry
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::FatFillDirectoryEntryFromRaw(SFatDirectoryEntry* sEntry, SFatRawDirectoryEntry* raw_entry) 
{

	// copy the filename and transform the filename from the internal structure to the public one
	FatGetShortNameFromEntry(sEntry->name, raw_entry->uEntry.sFatRawCommon.name);

	// copy other data from the internal sEntry structure to the public one
	sEntry->attributes = raw_entry->uEntry.sFatRawCommon.attributes;
	sEntry->size = raw_entry->uEntry.sFatRawCommon.size;
	sEntry->create_time = FatDecodeDateTime(raw_entry->uEntry.sFatRawCommon.create_date, raw_entry->uEntry.sFatRawCommon.create_time);
	sEntry->modify_time = FatDecodeDateTime(raw_entry->uEntry.sFatRawCommon.modify_date, raw_entry->uEntry.sFatRawCommon.modify_time);
	sEntry->access_time = FatDecodeDateTime(raw_entry->uEntry.sFatRawCommon.access_date, 0);
	sEntry->raw = *raw_entry;
}


// creates a directory
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatCreateDirectory(char* directory)
{
	EFatCode			uiResult;
	SFatDirectoryEntry	sEntry;

	// check that we got a valid pathname
	if (!directory || strlen(directory) > FAT_MAX_PATH)
	{
		return FAT_INVALID_FILENAME;
	}

	// try get the file sEntry
	uiResult = FatGetFileEntry(directory, &sEntry);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if we don'tfind a file or directory by that name
	// we can create it, otherwise return file already exists error
	if (*sEntry.name == 0)
	{
		// allocate memory for the file path
		size_t				path_len;
		char*				path_scanner;
		char				file_path[FAT_MAX_PATH + 1];
		SFatDirectoryEntry	sParentEntry;

		// get the name of the file path including
		// the filename
		path_len = strlen(directory);

		// set the pointer that will be used to scan
		// filename to the end of the filename
		path_scanner = directory + (path_len - 0x1);

		// if the filename ends with a backslash then it
		// is an invalid filename ( it's actually a directory
		// path )
		if (*path_scanner == BACKSLASH)
		{
			return FAT_INVALID_FILENAME;
		}
		
		// scan the filename starting at the end until
		// a backslash is found - when we exit this loop
		// path_scanner will point to the last character
		// of the filepath
		while (*(--path_scanner) != BACKSLASH);	/*scan file backwords until we get to */

		// calculate the length of the path part of the
		// filename
		path_len = (size_t)(path_scanner - directory);

		// copy the path part of the filename to
		// the file_path uiBuffer
		memcpy(file_path, directory, path_len);

		// set the null terminator of the file_path uiBuffer
		file_path[path_len] = 0x0;

		// increase pointer to the beggining of the filename
		// part of the path
		path_scanner++;

		// try to get the sEntry for the parent directory
		uiResult = FatGetFileEntry(file_path, &sParentEntry);

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

		// try to create the directory sEntry
		return FatCreateDirectoryEntry(&sParentEntry.raw, path_scanner, FAT_ATTR_DIRECTORY, 0, &sEntry);
	}

	// if we get here it means that a file or
	// directory with that name already exists.
	return FAT_FILENAME_ALREADY_EXISTS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::CreateFakeRootEntry(SFatDirectoryEntry* psEntry)
{
	// copy the file name to the psEntry and the raw
	// psEntry in their respective formats
	strcpy((char*)psEntry->name, "ROOT");
	GetShortNameForEntry(psEntry->raw.uEntry.sFatRawCommon.name, psEntry->name, 1);

	// set the general fields of the psEntry
	psEntry->attributes = psEntry->raw.uEntry.sFatRawCommon.attributes = FAT_ATTR_DIRECTORY;
	psEntry->size = psEntry->raw.uEntry.sFatRawCommon.size = 0x0;

	// since the psEntry does not physically exist the
	// address fields are set to zero as well
	psEntry->uiSectorAddress = 0;
	psEntry->uiSectorOffset = 0;

	// set the location of the root directory
	if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
	{
		// if this is a FAT32 volume then the root
		// directory is located on the data section just like
		// any other directory
		psEntry->raw.uEntry.sFatRawCommon.first_cluster_lo = LO16(GetRootCluster());
		psEntry->raw.uEntry.sFatRawCommon.first_cluster_hi = HI16(GetRootCluster());
	}
	else
	{
		// if the volume is FAT12/16 we set the cluster
		// address to zero and when time comes to get the
		// directory we'll calculate the address right after
		// the end of the FATs
		psEntry->raw.uEntry.sFatRawCommon.first_cluster_lo = 0;
		psEntry->raw.uEntry.sFatRawCommon.first_cluster_hi = 0;
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CFatVolume::FindNextPathItem(char* szPath, char* szCurrentLevelPath)
{
	char* szPathLevel;

	// set the pointer current level name uiBuffer
	szPathLevel = szCurrentLevelPath;

	// if the path starts with a backslash then advance to
	// the next character
	if (*szPath == '\\')
	{
		szPath++;
	}

	// copy the name of the current level psEntry
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
EFatCode CFatVolume::MatchesFileName(bool* pbMatch, bool* pbUsingLFN, char* szConstructedShortFileName, uint16* puiTargetFileLongName, char* szCurrentLevelPath, SFatQueryState* psQuery)
{
	// get an LFN version of the filename
	bool bLongFilename = false;
	bool bMatch;

	// format the current level filename to the 8.3 format if this is an invalid 8.3 filename try to get the LFN once we get a valid filename (either short or LFN) compare
	// it to the one on the current query psEntry.
	if (GetShortNameForEntry((uint8*)szConstructedShortFileName, (uint8*)szCurrentLevelPath, 1) == FAT_INVALID_FILENAME)
	{
		if (GetLongNameForEntry(puiTargetFileLongName, (uint8*)szCurrentLevelPath) == FAT_INVALID_FILENAME)
		{
			return FAT_INVALID_FILENAME;
		}
		bLongFilename = true;
		bMatch = FatCompareLongName(puiTargetFileLongName, psQuery->long_filename) || FatCompareShortName(szConstructedShortFileName, (char*)psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.name);
	}
	else
	{
		bMatch = FatCompareShortName(szConstructedShortFileName, (char*)psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.name);
	}

	*pbUsingLFN = bLongFilename;
	*pbMatch = bMatch;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatGetFileEntry(char* szPath, SFatDirectoryEntry* psEntry)
{
	EFatCode				eResult;
	bool					bMatch;
	char					szConstructedShortFileName[13];
	SFatRawDirectoryEntry*	psCurrentEntry;
	SFatQueryState			sQuery;
	bool					bLongFilename;

	uint16					auiConstructedLongFilename[FAT_MAX_PATH + 1];	/* stores the utf16 long filename */
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
	do
	{
		szPath = FindNextPathItem(szPath, szCurrentLevelPath);
		if (szPath == NULL)
		{
			return FAT_INVALID_FILENAME;
		}

		// try to find the first psEntry
		eResult = FatQueryFirstEntry(psCurrentEntry, 0, &sQuery, true);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// if the output of FatQueryFirstEntry indicates that there are no entries available...
		if (*sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.name == 0)
		{
			psEntry->name[0] = '\0';
			return FAT_SUCCESS;
		}

		eResult = MatchesFileName(&bMatch, &bLongFilename, szConstructedShortFileName, auiConstructedLongFilename, szCurrentLevelPath, &sQuery);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// if the file doesn't match then get the next file.
		while (!bMatch)
		{
			//  try to get the next file.
			eResult = FatQueryNextEntry(&sQuery, true, false);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}

			// if the output of FatQueryFirstEntry indicates that  there are no entries available then set the psEntry name to 0 and return success.
			if (IS_LAST_DIRECTORY_ENTRY(sQuery.sCurrentEntryRaw))
			{
				psEntry->name[0] = '\0';
				return FAT_SUCCESS;
			}

			// match the filename against the next psEntry.
			if (bLongFilename)
			{
				bMatch = FatCompareLongName(auiConstructedLongFilename, sQuery.long_filename) || FatCompareShortName(szConstructedShortFileName, (char*)sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.name);
			}
			else
			{
				bMatch = FatCompareShortName(szConstructedShortFileName, (char*)sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.name);
			}
		}

		// set the current entry to the entry that we've just found
		psCurrentEntry = sQuery.sCurrentEntryRaw;
	} 
	while (szPath[0] != '\0');

	// copy the filename and transform the filename
	// from the internal structure to the public one
	FatGetShortNameFromEntry(psEntry->name, sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.name);

	// copy other data from the internal psEntry structure
	// to the public one
	psEntry->attributes = sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.attributes;
	psEntry->size = sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.size;
	psEntry->create_time = FatDecodeDateTime(sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.create_date, sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.create_time);
	psEntry->modify_time = FatDecodeDateTime(sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.modify_date, sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.modify_time);
	psEntry->access_time = FatDecodeDateTime(sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.access_date, 0);

	// calculate the sector address of the psEntry - if
	// query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is
	// different
	if (sQuery.uiCurrentCluster == 0x0)
	{
		psEntry->uiSectorAddress = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize()) + sQuery.uiCurrentSector;
	}
	else
	{
		psEntry->uiSectorAddress = CalculateFirstSectorOfCluster(sQuery.uiCurrentCluster) + sQuery.uiCurrentSector;
	}

	// calculate the offset of the psEntry within it's sector
	psEntry->uiSectorOffset = (uint16)((uintptr_t)sQuery.sCurrentEntryRaw) - ((uintptr_t)mauiFatSharedBuffer);

	// store a copy of the original FAT directory psEntry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	psEntry->raw = *sQuery.sCurrentEntryRaw;
	return FAT_SUCCESS;
}


// initializes a query of a set of directory entries.
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatQueryFirstEntry(SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, bool bBufferLocked)
{
	uint32		uiFirstSector;
	EFatCode	eResult;

	// make sure the long filename is set to an empty string
	query->long_filename[0] = 0;

	// if the directory sEntry has the cluster # set to
	// zero it is the root directory so we need to
	// get the cluster # accordingly
	if (directory)
	{
		if (directory->uEntry.sFatRawCommon.first_cluster_hi == 0 && directory->uEntry.sFatRawCommon.first_cluster_lo == 0)
		{
			directory = NULL;
		}
	}

	// if no directory sEntry was provided
	// we'll use the root sEntry of the volume
	if (directory == NULL)
	{
		// calculate the cluster # from the
		if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
		{
			query->uiCurrentCluster = GetRootCluster();
			uiFirstSector = CalculateFirstSectorOfCluster(query->uiCurrentCluster);
		}
		else
		{
			query->uiCurrentCluster = 0x0;
			uiFirstSector = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize());
		}
	}
	// if a directory sEntry was provided
	else
	{
		// if the sEntry provided is not a directory
		// sEntry return an error code
		if (!(directory->uEntry.sFatRawCommon.attributes & FAT_ATTR_DIRECTORY))
		{
			return FAT_NOT_A_DIRECTORY;
		}

		query->uiCurrentCluster = GetFatClusterFromFatEntry(directory, GetFileSystemType() == FAT_FS_TYPE_FAT32);

		// get the 1st sector of the directory sEntry
		uiFirstSector = CalculateFirstSectorOfCluster(query->uiCurrentCluster);
	}

	// read the sector into the query state buffer
	eResult = FatReadFatSector(uiFirstSector, query->auiBuffer);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	// set the 1st and current sEntry pointers
	// on the query state to the 1st sEntry of the
	// directory
	query->Attributes = attributes;
	query->uiCurrentSector = 0;

	query->first_entry_raw = (SFatRawDirectoryEntry*)query->auiBuffer;
	query->sCurrentEntryRaw = (SFatRawDirectoryEntry*)query->auiBuffer;

	// find the 1st sEntry and return it's result code
	return FatQueryNextEntry(query, bBufferLocked, true);
}


// moves a query to the next sEntry
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatQueryNextEntry(SFatQueryState* query, bool bBufferLocked, bool bFirstEntry)
{
	bool		bPass;
	uint32		uiSectorAddress;
	EFatCode	eResult;

	do
	{
		// if the current entry is the last entry of the sector...
		if (!bFirstEntry)
		{
			if ((query->sCurrentEntryRaw - query->first_entry_raw) == GetSectorSize() - 0x20)
			{
				// if the current sector is the last of the current cluster then we must find the next
				// cluster... if CurrentCluster == 0 then this is the root directory of a FAT16/FAT12 volume, that
				// volume has a fixed size in sectors and is not allocated as a cluster chain so we don't do this
				if (query->uiCurrentCluster > 0 &&/*query->uiCurrentSector > 0x0 &&*/ query->uiCurrentSector == GetNoOfSectorsPerCluster() - 1)
				{
					fatEntry uiFat;

					// get the fat structure for the current cluster
					// and return UNKNOWN_ERROR if the operation fails
					if (GetNextClusterEntry(query->uiCurrentCluster, &uiFat) != FAT_SUCCESS)
					{
						return FAT_UNKNOWN_ERROR;
					}

					// if this is the last cluster of the directory...
					if (FatIsEOFEntry(uiFat))
					{
						// set the current sEntry to 0
						*query->sCurrentEntryRaw->uEntry.sFatRawCommon.name = 0;
						return FAT_SUCCESS;
					}

					// set the current cluster to the next
					// cluster of the directory sEntry
					query->uiCurrentCluster = uiFat;

					// reset the current sector
					query->uiCurrentSector = 0;

					// calculate the address of the next sector
					uiSectorAddress = CalculateFirstSectorOfCluster(query->uiCurrentCluster) + query->uiCurrentSector;
				}
				// if there are more sectors on the current cluster then
				else
				{
					query->uiCurrentSector++;

					// if this is the root directory of a FAT16/FAT12
					// volume and we have passed it's last sector then
					// there's no more entries...
					if (query->uiCurrentCluster == 0x0)
					{
						if (query->uiCurrentSector == GetRootDirectorySectors())
						{
							*query->sCurrentEntryRaw->uEntry.sFatRawCommon.name = 0;
							return FAT_SUCCESS;
						}
						uiSectorAddress = (GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize())) + query->uiCurrentSector;
					}
					else
					{
						// calculate the address of the next sector
						uiSectorAddress = CalculateFirstSectorOfCluster(query->uiCurrentCluster) + query->uiCurrentSector;
					}
				}

				// read the next sector into the query uiBuffer
				eResult = FatReadFatSector(uiSectorAddress, query->auiBuffer);
				if (eResult != FAT_SUCCESS)
				{
					return eResult;
				}

				// set the 1st and current entry pointers on the query state to the 1st sEntry of the directory
				query->first_entry_raw = (SFatRawDirectoryEntry*)query->auiBuffer;
				query->sCurrentEntryRaw = (SFatRawDirectoryEntry*)query->auiBuffer;
			}
			// if there are more entries on the current sector...
			else
			{
				// simply increase the current sEntry pointer
				query->sCurrentEntryRaw++;
			}
		}
		else
		{
			bFirstEntry = false;
		}

		// if this is a long filename sEntry...
		if (query->sCurrentEntryRaw->uEntry.sFatRawCommon.attributes == FAT_ATTR_LONG_NAME && !IS_FREE_DIRECTORY_ENTRY(query->sCurrentEntryRaw))
		{
			// if this entry is marked as the 1st LFN sEntry
			if (query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence & FAT_FIRST_LFN_ENTRY)
			{
				query->uiSequence = (query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence ^ FAT_FIRST_LFN_ENTRY) + 1;
				query->uiChecksum = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiChecksum;

				// insert null terminator at the end of the long filename
				((uint8*)&query->long_filename[((query->uiSequence - 2) * 13) + 0xD])[INT16_BYTE0] = 0;
				((uint8*)&query->long_filename[((query->uiSequence - 2) * 13) + 0xD])[INT16_BYTE1] = 0;
			}

			// if this is the LFN that we're expecting then
			// process it, otherwise we'll have to wait for
			// another 1st LFN sEntry otherwise read the LFN
			// chrs and save them on the query state struct
			if (query->uiChecksum == query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiChecksum &&
				(query->uiSequence == (query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence & (0xFF ^ FAT_FIRST_LFN_ENTRY)) + 1))
			{
				query->uiSequence = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence & (0xFF ^ FAT_FIRST_LFN_ENTRY);
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x0])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[0];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x0])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[1];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x1])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[2];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x1])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[3];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x2])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[4];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x2])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[5];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x3])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[6];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x3])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[7];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x4])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[8];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x4])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[9];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x5])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[0];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x5])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[1];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x6])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[2];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x6])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[3];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x7])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[4];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x7])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[5];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x8])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[6];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x8])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[7];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x9])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[8];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0x9])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[9];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0xA])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[10];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0xA])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[11];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0xB])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars3[0];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0xB])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars3[1];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0xC])[INT16_BYTE0] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars3[2];
				((uint8*)&query->long_filename[((query->uiSequence - 1) * 13) + 0xC])[INT16_BYTE1] = query->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars3[3];
			}
			else
			{
				query->uiChecksum = 0;
			}

			// make sure we never return this entry
			bPass = (query->Attributes == FAT_ATTR_LONG_NAME);
		}
		else
		{
			// check that the current entry passes the query attributes check
			bPass =
				(!(query->sCurrentEntryRaw->uEntry.sFatRawCommon.attributes & FAT_ATTR_HIDDEN) || (query->Attributes & FAT_ATTR_HIDDEN)) &&
				(!(query->sCurrentEntryRaw->uEntry.sFatRawCommon.attributes & FAT_ATTR_SYSTEM) || (query->Attributes & FAT_ATTR_SYSTEM)) &&
				(!(query->sCurrentEntryRaw->uEntry.sFatRawCommon.attributes & FAT_ATTR_VOLUME_ID) || (query->Attributes & FAT_ATTR_VOLUME_ID)) &&
				(!(query->sCurrentEntryRaw->uEntry.sFatRawCommon.attributes & FAT_ATTR_LONG_NAME) || (query->Attributes & FAT_ATTR_LONG_NAME));
		}
	}

	// repeat the process until we find a valid entry
	// that matches the attributes given
	while (!bPass || *query->sCurrentEntryRaw->uEntry.sFatRawCommon.name == 0xE5);

	// if we found an entry we need to check it's LFN checksum to make sure that the long filename that we've associated with it belongs to it. If it doesn't clear it.
	if (*query->sCurrentEntryRaw->uEntry.sFatRawCommon.name != 0x0)
	{
		if (query->uiChecksum != FatLongEntryChecksum((uint8*)query->sCurrentEntryRaw->uEntry.sFatRawCommon.name))
		{
			query->long_filename[0] = 0x0;
		}
	}

	// if this entry doesn't have an LFN entry but its marked as having
	// a lowercase name or extension then fill the long filename with the
	// lowercase version
	if (query->long_filename[0] == 0 && (query->sCurrentEntryRaw->uEntry.sFatRawCommon.reserved & (FAT_LOWERCASE_EXTENSION | FAT_LOWERCASE_BASENAME)))
	{
		int i = 0;
		for (uint16 uiResult = 0; uiResult < 8; uiResult++)
		{
			if (query->sCurrentEntryRaw->uEntry.sFatRawCommon.name[uiResult] != 0x20)
			{
				if (query->sCurrentEntryRaw->uEntry.sFatRawCommon.reserved & FAT_LOWERCASE_BASENAME)
				{
					query->long_filename[i++] = tolower(query->sCurrentEntryRaw->uEntry.sFatRawCommon.name[uiResult]);
				}
				else
				{
					query->long_filename[i++] = query->sCurrentEntryRaw->uEntry.sFatRawCommon.name[uiResult];
				}
			}
		}

		if (query->sCurrentEntryRaw->uEntry.sFatRawCommon.name[8] != 0x20)
		{
			query->long_filename[i++] = '.';

			for (uint16 uiResult = 8; uiResult < 11; uiResult++)
			{
				if (query->sCurrentEntryRaw->uEntry.sFatRawCommon.name[uiResult] != 0x20)
				{
					if (query->sCurrentEntryRaw->uEntry.sFatRawCommon.reserved & FAT_LOWERCASE_EXTENSION)
					{
						query->long_filename[i++] = tolower(query->sCurrentEntryRaw->uEntry.sFatRawCommon.name[uiResult]);
					}
					else
					{
						query->long_filename[i++] = query->sCurrentEntryRaw->uEntry.sFatRawCommon.name[uiResult];
					}
				}
			}
		}
		query->long_filename[i] = 0x0;
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatCreateDirectoryEntry(SFatRawDirectoryEntry* parent, char* name, uint8 attribs, uint32 uiEntryCluster, SFatDirectoryEntry* psNewEntry)
{
	EFatCode						eResult;
	uint16							uiLength;
	int16							char_index;
	uint16							illegal_char;
	uint16							uiEntriesCount = 0;
	uint32							sector;
	uint32							uiFirstSectorOfCluster = 0;
	uintptr_t						uiLastEntryAddress;
	fatEntry						uiFat;
	fatEntry						uiLastFat;
	SFatRawDirectoryEntry*			sParentEntry;

	int								iLFNEntriesNeeded = 0;
	int								iLFNEntriesFound;
	char							uiChecksum;

	// get the length of the filename
	uiLength = (uint16)strlen(name);

	// check that the character is a valid 8.3 filename, the
	// file is invalid if:
	//
	//	- name part is more than 8 chars (char_index > 8)
	//	- extension part is more than 3 (uiResult - char_index > 4)
	//	- it has more than one dot (FatIndexOf('.', name, char_index + 1) >= 0)
	if (uiLength > 255)
	{
		return FAT_FILENAME_TOO_LONG;
	}

	// all names are also invalid if they start or end with
	// a dot
	char_index = FatIndexOf('.', name, 0x0);

	if (char_index == 0 || char_index == (uiLength - 1))
	{
		return FAT_INVALID_FILENAME;
	}

	for (char_index = 0x0; char_index < uiLength; char_index++)
	{
		// if the character is less than 0x20 with the
		// exception of 0x5 then the filename is illegal
		if (name[char_index] < 0x1F)
		{
			return FAT_ILLEGAL_FILENAME;
		}

		// compare the character with a table of illegal
		// characters, if a match is found then the filename
		// is illegal
		for (illegal_char = 0x0; illegal_char < ILLEGAL_CHARS_COUNT; illegal_char++)
		{
			if (name[char_index] == ILLEGAL_CHARS[illegal_char] && name[char_index] != '.')
			{
				return FAT_ILLEGAL_FILENAME;
			}
		}
	}

	// initialize the raw sEntry
	// todo: check if no other functions are initializing
	// sNewEntry and initialize the whole thing
	memset(&psNewEntry->raw, 0, sizeof(psNewEntry->raw));

	// attempt to format the filename provided
	// to the format required by the directory sEntry
	// and copy it to it's field
	eResult = GetShortNameForEntry(psNewEntry->raw.uEntry.sFatRawCommon.name, (uint8*)name, 0);

	// if the above operation failed then the filename
	// is invalid
	if (eResult != FAT_SUCCESS && eResult != FAT_LFN_GENERATED)
	{
		return FAT_INVALID_FILENAME;
	}

	// if this is going to be an lfn sEntry we need to make
	// sure that the short filename is available
	if (eResult == FAT_LFN_GENERATED)
	{
		SFatQueryState query;
		uint16 name_suffix = 0;
		char is_valid_entry;
		char name_suffix_str[6];
		uint8 i, c;

		do
		{
			is_valid_entry = 1;

			memset(&query, 0, sizeof(query));
			eResult = FatQueryFirstEntry(parent, 0, &query, 0);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}

			sprintf(name_suffix_str, "~%i", name_suffix);

			for (i = 0; i < 8 - (char)strlen(name_suffix_str); i++)
			{
				if (psNewEntry->raw.uEntry.sFatRawCommon.name[i] == 0x20)
				{
					break;
				}
			}

			for (c = 0; c < (char)strlen(name_suffix_str); c++)
			{
				psNewEntry->raw.uEntry.sFatRawCommon.name[i++] = name_suffix_str[c];
			}

			// loop through all entries in the parent directory
			// and if we find one with the same name as hours mark the name
			// as invalid
			while (*query.sCurrentEntryRaw->uEntry.sFatRawCommon.name != 0)
			{
				if (memcmp(query.sCurrentEntryRaw->uEntry.sFatRawCommon.name, psNewEntry->raw.uEntry.sFatRawCommon.name, 11) == 0)
				{
					is_valid_entry = 0;
					break;
				}
				eResult = FatQueryNextEntry(&query, false, false);
				if (eResult != FAT_SUCCESS)
				{
					return eResult;
				}
			}

			// if the filename is taken we need to compute a new one
			if (!is_valid_entry)
			{
				// create the filename suffix and append it after
				// the last char or replace the end of the filename
				// with it.
				sprintf(name_suffix_str, "~%i", name_suffix++);

				for (i = 0; i < 8 - (char)strlen(name_suffix_str); i++)
				{
					if (psNewEntry->raw.uEntry.sFatRawCommon.name[i] == 0x20)
					{
						break;
					}
				}

				for (c = 0; c < (char)strlen(name_suffix_str); c++)
				{
					psNewEntry->raw.uEntry.sFatRawCommon.name[i++] = name_suffix_str[c];
				}
			}
		} while (!is_valid_entry);

		// calculate the # of entries needed to store the lfn
		// including the actual sEntry
		iLFNEntriesNeeded = ((strlen(name) + 12) / 13) + 1;
		iLFNEntriesFound = 0;
	}

	// if the new sEntry is a directory and no cluster was supplied
	// by the calling function then allocate a new cluster
	if (uiEntryCluster == 0 && (attribs & FAT_ATTR_DIRECTORY))
	{
		uiEntryCluster = FatAllocateDirectoryCluster(parent, &eResult);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}

	// set the sEntry attributes
	strcpy((char*)psNewEntry->name, name);
	psNewEntry->attributes = attribs;
	psNewEntry->size = 0x0;
	psNewEntry->raw.uEntry.sFatRawCommon.attributes = attribs;
	psNewEntry->raw.uEntry.sFatRawCommon.reserved = 0;
	psNewEntry->raw.uEntry.sFatRawCommon.size = 0x0;
	psNewEntry->raw.uEntry.sFatRawCommon.first_cluster_lo = LO16(uiEntryCluster);
	psNewEntry->raw.uEntry.sFatRawCommon.first_cluster_hi = HI16(uiEntryCluster);
	psNewEntry->raw.uEntry.sFatRawCommon.create_time_tenth = 0x0;
	psNewEntry->raw.uEntry.sFatRawCommon.create_date = GetSystemClockDate();
	psNewEntry->raw.uEntry.sFatRawCommon.create_time = GetSystemClockTime();
	psNewEntry->raw.uEntry.sFatRawCommon.modify_date = psNewEntry->raw.uEntry.sFatRawCommon.create_date;
	psNewEntry->raw.uEntry.sFatRawCommon.modify_time = psNewEntry->raw.uEntry.sFatRawCommon.create_time;
	psNewEntry->raw.uEntry.sFatRawCommon.access_date = psNewEntry->raw.uEntry.sFatRawCommon.create_date;
	psNewEntry->create_time = FatDecodeDateTime(psNewEntry->raw.uEntry.sFatRawCommon.create_date, psNewEntry->raw.uEntry.sFatRawCommon.create_time);
	psNewEntry->modify_time = FatDecodeDateTime(psNewEntry->raw.uEntry.sFatRawCommon.modify_date, psNewEntry->raw.uEntry.sFatRawCommon.modify_time);
	psNewEntry->access_time = FatDecodeDateTime(psNewEntry->raw.uEntry.sFatRawCommon.access_date, 0);

	// there's no fat sEntry that points to the 1st cluster of
	// a directory's cluster chain but we'll create a
	// fake fat sEntry from the 1st cluster data on the
	// directory sEntry so that we can handle the 1st
	// cluster with the same code as all other clusters
	// in the chain
	if (parent && (parent->uEntry.sFatRawCommon.first_cluster_lo != 0x0 || parent->uEntry.sFatRawCommon.first_cluster_hi != 0x0))
	{
		uiFat = GetFatClusterFromFatEntry(parent, GetFileSystemType() == FAT_FS_TYPE_FAT32);
	}

	// if no parent was specified then we create
	// the fake fat sEntry from the root directory's
	// cluster address found on the volume structure
	else
	{
		if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
		{
			uiFat = GetRootCluster();
		}
		else
		{
			uiFat = uiLastFat = 0x0;
			uiFirstSectorOfCluster = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize());
		}
	}

	FlushAndInvalidate();

	// for each cluster allocated to the parent
	// directory sEntry
	for (;;)
	{
		// calculate the address of the 1st sector
		// of the cluster - skip this step if uiResult equals
		// 1, this means that this is the 1st sector of the
		// root sEntry which doesn't start at the beggining
		// of the cluster
		if (uiFat != 0x0)
		{
			uiFirstSectorOfCluster = CalculateFirstSectorOfCluster(uiFat);
		}

		// set the current sector to the first
		// sector of the cluster
		sector = uiFirstSectorOfCluster;

		// calculate the address of the last directory sEntry on a sector when the sector is loaded into sec_buff
		uiLastEntryAddress = ((uintptr_t)mauiFatSharedBuffer + GetSectorSize()) - 0x20;

		// for each sector in the cluster
		while (uiFat == 0 || sector < (uiFirstSectorOfCluster + GetNoOfSectorsPerCluster()))
		{
			// read the current sector to RAM
			eResult = FatReadFatSector(sector);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}

			// set the parent sEntry pointer to the 1st sEntry of the current sector
			sParentEntry = (SFatRawDirectoryEntry*)mauiFatSharedBuffer;

			// for each directory sEntry in the sector...
			while ((uintptr_t)sParentEntry <= uiLastEntryAddress)
			{
				// make sure we don't exceed the limit of 0xFFFF entries
				// per directory
				if ((uiEntriesCount + iLFNEntriesNeeded) == 0xFFFF)
				{
					return FAT_DIRECTORY_LIMIT_EXCEEDED;
				}

				// increase the count of directory entries
				uiEntriesCount++;

				// if the directory sEntry is free
				if (IS_FREE_DIRECTORY_ENTRY(sParentEntry))
				{
					// we've found a free sEntry
					iLFNEntriesFound++;

					// if this is the last directory sEntry or if we've
					// found all the entries that we need then let's get
					// ready to write them
					if (IS_LAST_DIRECTORY_ENTRY(sParentEntry) || iLFNEntriesFound == iLFNEntriesNeeded)
					{
						// if there where any free entries before this
						// one then we need to rewind a bit
						while (iLFNEntriesFound-- > 1)
						{
							if ((uintptr_t)sParentEntry > (uintptr_t)mauiFatSharedBuffer)
							{
								sParentEntry--;
							}
							else
							{
								// if the last sEntry is on the same cluster we
								// can just decrease the sector number, otherwise we
								// need to get the sector address for the last cluster
								if (sector > uiFirstSectorOfCluster)
								{
									sector--;
								}
								else
								{
									if (uiLastFat == 0)
									{
										uiFirstSectorOfCluster = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize());
									}
									else
									{
										uiFat = uiLastFat;
										uiFirstSectorOfCluster = CalculateFirstSectorOfCluster(uiFat);
									}
									sector = uiFirstSectorOfCluster + GetNoOfSectorsPerCluster();
								}

								// read the last sector to the cache, calculate the last entry address and set our pointer to it
								eResult = FatReadFatSector(sector);
								if (eResult != FAT_SUCCESS)
								{
									return eResult;
								}
								uiLastEntryAddress = ((uintptr_t)mauiFatSharedBuffer + GetSectorSize()) - 0x20;
								sParentEntry = (SFatRawDirectoryEntry*)uiLastEntryAddress;
							}
						}

						// compute the checksum for this sEntry
						uiChecksum = FatLongEntryChecksum((uint8*)psNewEntry->raw.uEntry.sFatRawCommon.name);

						// now we can start writting
						iLFNEntriesFound = iLFNEntriesNeeded;
						while (iLFNEntriesFound--)
						{
							if (iLFNEntriesFound)
							{
								uint16 i, c;

								// set the required fields for this sEntry
								sParentEntry->uEntry.sFatRawLongFileName.uiSequence = (uint8)iLFNEntriesFound;
								sParentEntry->uEntry.sFatRawLongFileName.uiChecksum = uiChecksum;
								sParentEntry->uEntry.sFatRawCommon.attributes = FAT_ATTR_LONG_NAME;
								sParentEntry->uEntry.sFatRawLongFileName.uiFirstCluster = 0;
								sParentEntry->uEntry.sFatRawLongFileName.uiType = 0;

								// mark sEntry as the 1st sEntry if it is so
								if (iLFNEntriesFound == iLFNEntriesNeeded - 1)
								{
									sParentEntry->uEntry.sFatRawLongFileName.uiSequence = sParentEntry->uEntry.sFatRawLongFileName.uiSequence | FAT_FIRST_LFN_ENTRY;
								}

								// copy the lfn chars
								c = (uint16)strlen(name);
								i = ((iLFNEntriesFound - 1) * 13);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x0] = LO8((i + 0x0 > c) ? 0xFFFF : (uint16)name[i + 0x0]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x1] = HI8((i + 0x0 > c) ? 0xFFFF : (uint16)name[i + 0x0]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x2] = LO8((i + 0x1 > c) ? 0xFFFF : (uint16)name[i + 0x1]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x3] = HI8((i + 0x1 > c) ? 0xFFFF : (uint16)name[i + 0x1]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x4] = LO8((i + 0x2 > c) ? 0xFFFF : (uint16)name[i + 0x2]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x5] = HI8((i + 0x2 > c) ? 0xFFFF : (uint16)name[i + 0x2]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x6] = LO8((i + 0x3 > c) ? 0xFFFF : (uint16)name[i + 0x3]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x7] = HI8((i + 0x3 > c) ? 0xFFFF : (uint16)name[i + 0x3]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x8] = LO8((i + 0x4 > c) ? 0xFFFF : (uint16)name[i + 0x4]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x9] = HI8((i + 0x4 > c) ? 0xFFFF : (uint16)name[i + 0x4]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x0] = LO8((i + 0x5 > c) ? 0xFFFF : (uint16)name[i + 0x5]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x1] = HI8((i + 0x5 > c) ? 0xFFFF : (uint16)name[i + 0x5]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x2] = LO8((i + 0x6 > c) ? 0xFFFF : (uint16)name[i + 0x6]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x3] = HI8((i + 0x6 > c) ? 0xFFFF : (uint16)name[i + 0x6]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x4] = LO8((i + 0x7 > c) ? 0xFFFF : (uint16)name[i + 0x7]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x5] = HI8((i + 0x7 > c) ? 0xFFFF : (uint16)name[i + 0x7]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x6] = LO8((i + 0x8 > c) ? 0xFFFF : (uint16)name[i + 0x8]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x7] = HI8((i + 0x8 > c) ? 0xFFFF : (uint16)name[i + 0x8]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x8] = LO8((i + 0x9 > c) ? 0xFFFF : (uint16)name[i + 0x9]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x9] = HI8((i + 0x9 > c) ? 0xFFFF : (uint16)name[i + 0x9]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0xA] = LO8((i + 0xA > c) ? 0xFFFF : (uint16)name[i + 0xA]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars2[0xB] = HI8((i + 0xA > c) ? 0xFFFF : (uint16)name[i + 0xA]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars3[0x0] = LO8((i + 0xB > c) ? 0xFFFF : (uint16)name[i + 0xB]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars3[0x1] = HI8((i + 0xB > c) ? 0xFFFF : (uint16)name[i + 0xB]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars3[0x2] = LO8((i + 0xC > c) ? 0xFFFF : (uint16)name[i + 0xC]);
								sParentEntry->uEntry.sFatRawLongFileName.auiChars3[0x3] = HI8((i + 0xC > c) ? 0xFFFF : (uint16)name[i + 0xC]);
								mbEntriesUpdated = true;

								// continue to next sEntry
								if ((uintptr_t)sParentEntry < (uintptr_t)uiLastEntryAddress)
								{
									sParentEntry++;
								}
								else
								{
									if (uiFat == 0 || sector < uiFirstSectorOfCluster + GetNoOfSectorsPerCluster() - 1)
									{
										sector++;
										// make sure that we don't overflow the root directory
										// on FAT12/16 volumes
										if (!uiFat)
										{
											if (sector > uiFirstSectorOfCluster + GetRootDirectorySectors())
											{
												return FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED;
											}
										}
									}
									else
									{
										// get the next cluster, we'll remember the last one so we can update it bellow if it's the eof cluster
										uiLastFat = uiFat;
										eResult = GetNextClusterEntry(uiFat, &uiFat);
										if (eResult != FAT_SUCCESS)
										{
											return eResult;
										}

										// if this is the end of the FAT chain allocate
										// a new cluster to this folder and continue
										if (FatIsEOFEntry(uiFat))
										{
											fatEntry uiNewFat = FatAllocateDataCluster(1, 1, &eResult);
											if (eResult != FAT_SUCCESS)
											{
												return eResult;
											}

											eResult = FatSetClusterEntry(uiLastFat, uiNewFat);
											if (eResult != FAT_SUCCESS)
											{
												return eResult;
											}
											uiFat = uiNewFat;
										}

										eResult = FlushAndInvalidate();
										if (eResult != FAT_SUCCESS)
										{
											return eResult;
										}

										// continue working on the new cluster
										sector = CalculateFirstSectorOfCluster(uiFat);
									}

									// load the next sector
									eResult = FatReadFatSector(sector);
									if (eResult != FAT_SUCCESS)
									{
										return eResult;
									}
									sParentEntry = (SFatRawDirectoryEntry*)mauiFatSharedBuffer;
								}
							}
							else
							{
								*sParentEntry = psNewEntry->raw;
							}
						}

						psNewEntry->uiSectorAddress = sector;
						psNewEntry->uiSectorOffset = (uintptr_t)sParentEntry - (uintptr_t)mauiFatSharedBuffer;

						FlushAndInvalidate();

						// we're done!!!!!
						return FAT_SUCCESS;
					}
				}
				else
				{
					iLFNEntriesFound = 0;
				}

				// move the parent sEntry pointer to
				// the next sEntry in the sector
				sParentEntry++;
			}

			// move to the next sector in the cluster
			sector++;

			// make sure that we don't overflow the root directory
			// on FAT12/16 volumes
			if (!uiFat)
			{
				if (sector > uiFirstSectorOfCluster + GetRootDirectorySectors())
				{
					return FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED;
				}
			}
		}

		// get the next cluster, we'll remember the last one in
		// case we need to rewind to write lfn entries
		uiLastFat = uiFat;
		eResult = GetNextClusterEntry(uiFat, &uiFat);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// if this is the end of the FAT chain allocate
		// a new cluster to this folder and continue
		if (FatIsEOFEntry(uiFat))
		{
			fatEntry uiNewFat;

			// allocate the cluster
			uiNewFat = FatAllocateDataCluster(1, 1, &eResult);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}

			// link it to the cluster chain
			eResult = FatSetClusterEntry(uiLastFat, uiNewFat);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}
			uiFat = uiNewFat;
		}
	} 
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatFileDelete(char* filename)
{
	EFatCode				eResult;
	uint32					uiFirstCluster;
	SFatDirectoryEntry		sEntry;
	char					szPathPart[FAT_MAX_PATH];
	char*					szNamePart;
	uint8					checksum;
	SFatFileSystemQuery		query;


	// get the sEntry for the file
	eResult = FatGetFileEntry(filename, &sEntry);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	FlushAndInvalidate();

	// if the sEntry is located go ahead and delete it.
	if (sEntry.name != 0)
	{
		// compute the checksum for the file
		checksum = FatLongEntryChecksum((uint8*)sEntry.raw.uEntry.sFatRawCommon.name);

		// make sure we're not trying to delete a directory
		if (sEntry.attributes & FAT_ATTR_DIRECTORY)
		{
			return FAT_NOT_A_FILE;
		}

		// find the entry's first cluster address
		uiFirstCluster= GetFatClusterFromFatEntry(&sEntry.raw, GetFileSystemType() == FAT_FS_TYPE_FAT32);

		// free the file's cluster change
		if (uiFirstCluster)
		{
			eResult = FatFreeClusterChain(uiFirstCluster);
			if (eResult != FAT_SUCCESS)
				return eResult;
		}

		// mark the sEntry as deleted
		sEntry.raw.uEntry.sFatRawCommon.name[0] = FAT_DELETED_ENTRY;

		eResult = FatReadFatSector(sEntry.uiSectorAddress);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		memcpy(mauiFatSharedBuffer + sEntry.uiSectorOffset, &sEntry.raw, sizeof(sEntry.raw));
		muiFatSharedBufferSector = sEntry.uiSectorAddress;
		eResult = FatFlushFatSector();
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}

	// parse the filename
	FatParsePath(filename, szPathPart, &szNamePart);

	// get the 1st LFN sEntry of the parent directory
	eResult = FatFindFirstEntry(szPathPart, FAT_ATTR_LONG_NAME, 0, &query);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	// loop through each sEntry.
	while (*query.sCurrentEntry.raw.uEntry.sFatRawCommon.name != 0)
	{
		if (query.sCurrentEntry.raw.uEntry.sFatRawLongFileName.uiChecksum == checksum)
		{
			// mark the sEntry as deleted
			query.sCurrentEntry.raw.uEntry.sFatRawCommon.name[0] = FAT_DELETED_ENTRY;

			eResult = FatReadFatSector(query.sCurrentEntry.uiSectorAddress);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}

			memcpy(mauiFatSharedBuffer + query.sCurrentEntry.uiSectorOffset, &query.sCurrentEntry.raw, sizeof(sEntry.raw));
			muiFatSharedBufferSector = sEntry.uiSectorAddress;
			eResult = FatFlushFatSector();
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}
		}

		// get the next LFN sEntry
		eResult = FatFindNextEntry(0, &query);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatFileRename(char* szOriginalFilename, char* szNewFilename)
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

	// parse paths
	FatParsePath(szOriginalFilename, szOriginalParent, &szOriginalFilenamePart);
	FatParsePath(szNewFilename, szNewParent, &szNewFilenamePart);

	// try to get the new sEntry to see if it exists.
	FatGetFileEntry(szNewFilename, &sNewEntry);
	if (*sNewEntry.name != 0)
	{
		return FAT_FILENAME_ALREADY_EXISTS;
	}

	// get the directory sEntry
	FatGetFileEntry(szOriginalFilename, &sOriginalEntry);

	if (*sOriginalEntry.name != 0)
	{
		SFatDirectoryEntry parent;

		// compute the checksum for the file
		uiChecksum = FatLongEntryChecksum((uint8*)sOriginalEntry.raw.uEntry.sFatRawCommon.name);

		// get the cluster # for the sEntry
		uiEntryCluster = GetFatClusterFromFatEntry(&sOriginalEntry.raw, GetFileSystemType() == FAT_FS_TYPE_FAT32);

		// get the new parent sEntry
		eResult = FatGetFileEntry(szNewParent, &parent);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// create the new sEntry in the parent folder
		eResult = FatCreateDirectoryEntry(&parent.raw, szNewFilenamePart, sOriginalEntry.attributes, uiEntryCluster, &sNewEntry);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		// copy all info except name from the old sEntry to the new one
		sNewEntry.raw.uEntry.sFatRawCommon.access_date = sOriginalEntry.raw.uEntry.sFatRawCommon.access_date;
		sNewEntry.raw.uEntry.sFatRawCommon.attributes = sOriginalEntry.raw.uEntry.sFatRawCommon.attributes;
		sNewEntry.raw.uEntry.sFatRawCommon.create_date = sOriginalEntry.raw.uEntry.sFatRawCommon.create_date;
		sNewEntry.raw.uEntry.sFatRawCommon.create_time = sOriginalEntry.raw.uEntry.sFatRawCommon.create_time;
		sNewEntry.raw.uEntry.sFatRawCommon.create_time_tenth = sOriginalEntry.raw.uEntry.sFatRawCommon.create_time_tenth;
		sNewEntry.raw.uEntry.sFatRawCommon.first_cluster_hi = sOriginalEntry.raw.uEntry.sFatRawCommon.first_cluster_hi;
		sNewEntry.raw.uEntry.sFatRawCommon.first_cluster_lo = sOriginalEntry.raw.uEntry.sFatRawCommon.first_cluster_lo;
		sNewEntry.raw.uEntry.sFatRawCommon.modify_date = sOriginalEntry.raw.uEntry.sFatRawCommon.modify_date;
		sNewEntry.raw.uEntry.sFatRawCommon.modify_time = sOriginalEntry.raw.uEntry.sFatRawCommon.modify_time;
		sNewEntry.raw.uEntry.sFatRawCommon.reserved = sOriginalEntry.raw.uEntry.sFatRawCommon.reserved;
		sNewEntry.raw.uEntry.sFatRawCommon.size = sOriginalEntry.raw.uEntry.sFatRawCommon.size;

		// write modified entry to drive
		eResult = FatReadFatSector(sNewEntry.uiSectorAddress);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		memcpy(mauiFatSharedBuffer + sNewEntry.uiSectorOffset, &sNewEntry.raw, sizeof(sNewEntry.raw));
		mbEntriesUpdated = true;

		// mark the original sEntry as deleted.
		*sOriginalEntry.raw.uEntry.sFatRawCommon.name = FAT_DELETED_ENTRY;
		eResult = FatReadFatSector(sOriginalEntry.uiSectorAddress);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}

		memcpy(mauiFatSharedBuffer+ sOriginalEntry.uiSectorOffset, &sOriginalEntry.raw, sizeof(sOriginalEntry.raw));
		mbEntriesUpdated = true;
	}

	SFatFileSystemQuery query;

	// get the 1st LFN sEntry of the parent directory
	eResult = FatFindFirstEntry(szOriginalParent, FAT_ATTR_LONG_NAME, 0, &query);
	if (eResult != FAT_SUCCESS)
	{
		return eResult;
	}

	// loop through each sEntry.
	while (*query.sCurrentEntry.raw.uEntry.sFatRawCommon.name != 0)
	{
		if (query.sCurrentEntry.raw.uEntry.sFatRawLongFileName.uiChecksum == uiChecksum)
		{
			// mark the sEntry as deleted
			muiFatSharedBufferSector = FAT_UNKNOWN_SECTOR;
			query.sCurrentEntry.raw.uEntry.sFatRawCommon.name[0] = FAT_DELETED_ENTRY;
			eResult = FatReadFatSector(query.sCurrentEntry.uiSectorAddress);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}

			memcpy(mauiFatSharedBuffer+ query.sCurrentEntry.uiSectorOffset, &query.sCurrentEntry.raw, sizeof(query.sCurrentEntry.raw));
			mbEntriesUpdated = true;
		}
		// get the next LFN sEntry
		eResult = FatFindNextEntry(0, &query);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}

	FlushAndInvalidate();
	return FAT_SUCCESS;
}


// converts a 8.3 filename from the internal
// filesystem format to the user friendly convention
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::FatGetShortNameFromEntry(uint8* dest, const uint8* src)
{
	// copy the 1st character - 0xE5 is used on the
	// 1st character of the sEntry to indicate an unused
	// sEntry but it is also a valid KANJI lead byte for the
	// character set used in Japan. The special value 0x5 is
	// used so that this special file name case for Japan
	// can be handled properly and not cause the FAT code
	// to think that the sEntry is free.
	if (*src != 0x5)
	{
		*dest++ = *src++;
	}
	else
	{
		*dest++ = 0xE5;
	}

	// if there's a second character...
	if (*src != 0x20)
	{
		*dest++ = *src++;
		if (*src != 0x20)
		{
			*dest++ = *src++;
			if (*src != 0x20)
			{
				*dest++ = *src++;
				if (*src != 0x20)
				{
					*dest++ = *src++;
					if (*src != 0x20)
					{
						*dest++ = *src++;
						if (*src != 0x20)
						{
							*dest++ = *src++;
							if (*src != 0x20)
							{
								*dest++ = *src++;
							}
							else
							{
								src++;
							}
						}
						else
						{
							src += 0x2;
						}
					}
					else
					{
						src += 0x3;
					}
				}
				else
				{
					src += 0x4;
				}
			}
			else
			{
				src += 0x5;
			}
		}
		else
		{
			src += 0x6;
		}
	}
	else
	{
		src += 0x7;
	}

	// if there's an extension append it to
	// the output
	if (*src != 0x20)
	{
		*dest++ = '.';
		*dest++ = *src++;
		if (*src != 0x20)
		{
			*dest++ = *src++;
			if (*src != 0x20)
			{
				*dest++ = *src;
			}
		}
	}
	*dest = 0x0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::CalculateFirstSectorOfCluster(uint32 uiCluster)
{
	return (((uiCluster - 2) * GetNoOfSectorsPerCluster()) + GetFirstDataSector());
}


// trims leading and trailing spaces. If the result exceeds the max length the destination will be set to an empty string.
// todo: clean it up a bit
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::TrimPath(char* dest, char* src, size_t max)
{
	uint32 max_length;
	uint32 lead_spaces = 0x0;
	uint32 last_char = 0x0;
	uint32 i;
	char* dst = dest;

	max_length = strlen(src);

	// count the lead spaces
	for (i = 0; i < max_length && src[i] == 0x20; i++)
	{
		lead_spaces++;
	}

	// if the whole string is full of spaces
	// return an empty string
	if (max_length == lead_spaces) 
	{
		*dest = 0x0;
		return;
	}

	// calculate the index of the last non-space character
	for (last_char = max_length - 1; last_char > 0 && (src[last_char] == 0x20); last_char--);

	// copy the non-space characters to the
	// destination uiBuffer
	for (i = lead_spaces; i <= last_char; i++)
	{
		*dest++ = src[i];
		if (!max--)
		{
			*dst = 0x0;
			return;
		}
	}

	// set the null terminator
	*dest = 0x0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::FatParsePath(char* path, char* szPathPart, char** filename_part)
{
	*filename_part = path + strlen(path);

	while (*--(*filename_part) != '\\' && (*filename_part) != path);

	while (path != *filename_part)
	{
		*szPathPart++ = *path++;
	}
	*szPathPart = 0;
	(*filename_part)++;
}


// converts the filename to it's Unicode UTF16 representation
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CFatVolume::GetLongNameForEntry(uint16* dst, uint8* src)
{
	register int i;
	for (i = 0; i < (int)strlen((char*)src); i++)
	{
		dst[i] = (uint16)src[i];
	}
	dst[i] = 0x0;

	// todo: check that this is a valid filename
	// and that it only uses ASCII chars since we don't
	// support unicode at this time
	return FAT_SUCCESS;
}


// compares two short names after they
// have been formatted by GetShortNameForEntry
// returns 1 if both name are equal and 0 otherwise
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::FatCompareShortName(char* szName1, char* szName2)
{
	return memcmp(szName1, szName2, 11) == 0;
}


// performs an ASCII comparison on two UTF16 strings
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CFatVolume::FatCompareLongName(uint16* name1, uint16* name2)
{
	int		i;
	char	c1, c2;

	for (i = 0; i < 256; i++)
	{
		c1 = toupper((char)name1[i]);
		c2 = toupper((char)name2[i]);
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


// converts an 8.3 filename to the format required
// by the FAT directory sEntry structure
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::GetShortNameForEntry(uint8* dest, uint8* src, bool bLFNDisabled)
{
	char		tmp[13];
	bool		bUppercase = false;
	uint16		dot_index;
	uint16		length;
	uint16		i;

	// check that the name is actually a long filename
	// before processing it as such
	if (!bLFNDisabled)
	{
		uint8	c;
		char	is_lfn = 0;

		length = (uint16)strlen((char*)src);
		dot_index = FatIndexOf('.', (char*)src, 0x0);

		// if the file hs no extension and is longer than 8 chars
		// or if the name part has more than 8 chars or the extension more than 8
		// or if it has more than one dot then we need to handle it as a lfn
		if (dot_index < 0 && length > 8) is_lfn = 1;
		if (dot_index >= 0)
		{
			if (dot_index > 7 || (length - dot_index) > 4)
			{
				is_lfn = 1;
			}
			if (dot_index >= 0) if (FatIndexOf('.', (char*)src, 1) >= 0)
			{
				is_lfn = 1;
			}
		}
		else
		{
			if (length > 8) is_lfn = 1;
		}

		// if it has spaces or lowercase letters we must also handle it as a long filename
		if (!is_lfn)
		{
			for (i = 0; i < length; i++)
			{
				if (src[i] == 0x20 || src[i] != toupper(src[i]))
				{
					is_lfn = 1;
					break;
				}
			}
		}
		if (is_lfn)
		{
			// first we find the location of the LAST dot
			dot_index = length;
			for (i = length - 1; i; i--)
			{
				if (src[i] == '.')
				{
					dot_index = i;
					break;
				}
			}

			// now we copy the first 8 chars of the filename excluding dots and spaces and we pad it with spaces.
			c = 0;
			for (i = 0; i < 8; i++)
			{
				while (c < dot_index)
				{
					if (src[c] == 0x20 || src[c] == '.')
					{
						c++;
					}
					else
					{
						break;
					}
				}
				if (c < dot_index)
				{
					tmp[i] = toupper(src[c++]);
				}
				else
				{
					tmp[i] = 0x20;
				}
			}

			// do the same for the extension
			c = dot_index + 1;
			for (i = 8; i < 11; i++)
			{
				while (c < length)
				{
					if (src[c] == 0x20 || src[c] == '.')
					{
						c++;
					}
					else
					{
						break;
					}
				}
				if (c < length)
				{
					tmp[i] = toupper(src[c++]);
				}
				else
				{
					tmp[i] = 0x20;
				}
			}

			// now we copy it to the callers buffer and we're done
			for (i = 0; i < 11; i++)
			{
				*dest++ = tmp[i];
			}

			// return special code so the caller knows to store the long name.
			return FAT_LFN_GENERATED;
		}
	}

	// trim-off spaces - if the result is greater than 12 it will return an empty string.
	TrimPath(tmp, (char*)src, 12);

	// if the name length was invalid return error code.
	if (*tmp == 0 || strlen(tmp) > 12)
	{
		return FAT_INVALID_FILENAME;
	}

	// find the location of the dot.
	dot_index = (uintptr_t)strchr(tmp, (int)'.');

	// strchr gave us the address of the dot, we now convert it to a 1-based index.
	if (dot_index)
	{
		dot_index -= (uintptr_t)tmp - 0x1;
	}

	// get the length of the input string
	length = (uint16)strlen(tmp);

	// check that this is a valid 8.3 filename
	if ((length > 0x9 &&
		(dot_index == 0x0 || (dot_index) > 0x9)) ||
		(dot_index > 0x0 && (length - dot_index) > 0x5))
	{
		return FAT_INVALID_FILENAME;
	}
	
	// copy the 1st part of the filename to the destination bBuffer.
	for (i = 0x0; i < 0x8; i++)
	{
		if (dot_index == 0x0)
		{
			if (i < length)
			{
				if (bLFNDisabled && (tmp[i] != toupper(tmp[i])))
					bUppercase = true;

				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
		else
		{
			if (i < dot_index - 0x1)
			{
				if (bLFNDisabled && (tmp[i] != toupper(tmp[i])))
					bUppercase = true;

				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
	}

	// if there's not extension fill the extension characters with spaces.
	if (dot_index == 0x0)
	{
		for (i = 0x0; i < 0x3; i++)
			*dest++ = 0x20;
	}
	// if there is an extension...
	else
	{
		// copy the extension characters to the destination bBuffer.
		for (i = dot_index; i < dot_index + 0x3; i++)
		{
			if (i < length)
			{
				if (bLFNDisabled && (tmp[i] != toupper(tmp[i])))
					bUppercase = true;
				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
	}

	return bUppercase ? FAT_INVALID_FILENAME : FAT_SUCCESS;
}


// computes the short filename checksum
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CFatVolume::FatLongEntryChecksum(uint8* filename)
{
	uint16	len;
	uint8	sum = 0;

	for (len = 11; len != 0; len--)
	{
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *filename++;
	}
	return sum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CFatVolume::FatIndexOf(char chr, char* str, int index)
{
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
uint32 CFatVolume::GetBytesPerCluster(void) 
{ 
	return msVolume.uiNoOfBytesPerSector * msVolume.uiNoOfSectorsPerCluster;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::GetID(void) { return msVolume.uiID; }
uint32 CFatVolume::GetFatSize(void) { return msVolume.uiFatSize; }
uint32 CFatVolume::GetRootCluster(void) { return msVolume.uiRootCluster; }
uint32 CFatVolume::GetFirstDataSector(void) { return msVolume.uiFirstDataSector; }
uint32 CFatVolume::GetNoOfSectors(void) { return msVolume.uiNoOfSectors; }
uint32 CFatVolume::GetNoOfDataSectors(void) { return msVolume.uiNoOfDataSectors; }
uint32 CFatVolume::GetNoOfClusters(void) { return msVolume.uiNoOfClusters; }
uint32 CFatVolume::GetNoOfReservedSectors(void) { return msVolume.uiNoOfReservedSectors; }
uint32 CFatVolume::GetNextFreeCluster(void) { return msVolume.uiNextFreeCluster; }
uint32 CFatVolume::GetTotalFreeClusters(void) { return msVolume.uiTotalFreeClusters; }
uint32 CFatVolume::GetFsinfoSector(void) { return msVolume.uiFileSystemInfoSector; }
uint16 CFatVolume::GetRootDirectorySectors(void) { return msVolume.uiRootDirectorySectors; }
uint16 CFatVolume::GetSectorSize(void) { return msVolume.uiNoOfBytesPerSector; }
uint16 CFatVolume::GetNoOfSectorsPerCluster(void) { return msVolume.uiNoOfSectorsPerCluster; }
bool CFatVolume::IsUseLongFilenames(void) { return msVolume.bUseLongFilenames; }
EFatFileSystemType CFatVolume::GetFileSystemType(void) { return msVolume.eFileSystem; }
uint8 CFatVolume::GetNoOfFatTables(void) { return msVolume.uiNoOfFatTables; }
char* CFatVolume::GetLabel(void) { return msVolume.szLabel; }
uint8* CFatVolume::GetFatSharedBuffer(void) { return mauiFatSharedBuffer; }
uint32 CFatVolume::GetFatSharedBufferSector(void) { return muiFatSharedBufferSector; }
CFileDrive* CFatVolume::GetFileDrive(void) { return mpcDevice; }

