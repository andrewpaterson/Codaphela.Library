#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "FatTime.h"
#include "FatVolume.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::Mount(CFileDrive* device)
{
	bool					bSuccess;
	SFatBIOSParameterBlock*	psBPB;
	uint32					uiHiddenSectors = 0;
	SFatPartitionEntry*		sPartitionEntry;
	uint8					uiPartitionsTtried = 0;
	char					szLabel[12];
	uint32					uiFileSystemInfoSector;
	uint8*					uiBuffer = mauiFatSharedBuffer;

	muiFatSharedBufferSector = FAT_UNKNOWN_SECTOR;

	// set the null terminator.
	szLabel[11] = 0;

	// save the storage device handle
	mpcDevice = device;

	// mark the loaded sector as unknown
	SetFatSharedBufferSector(FAT_UNKNOWN_SECTOR);

	// retrieve the boot sector (sector 0) from the storage device
	bSuccess = mpcDevice->Read(0x0, uiBuffer);
	if (!bSuccess)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	// set the partition entry pointer
	sPartitionEntry = (SFatPartitionEntry*)(uiBuffer + 0x1BE);

	for (;;)
	{
		// if we've already tried to mount the mpsVolume as partitionless
		// try to mount the next partition
		if (uiPartitionsTtried)
		{
			// if we've already tried all 4 partitions then we're
			// out of luck
			if (uiPartitionsTtried > 4)
			{
				return FAT_INVALID_FAT_VOLUME;
			}

			// if we've tried to mount a partition mpsVolume (not the unpartioned one)
			// then we must reload sector 0 (MBR)
			if (uiPartitionsTtried > 1)
			{
				// retrieve the boot sector (sector 0) from the storage device
				bSuccess = mpcDevice->Read(0x0, uiBuffer);
				if (!bSuccess)
				{
					return FAT_CANNOT_READ_MEDIA;
				}

				// move to the next partition entry
				sPartitionEntry++;
			}

			// remember how many sectors before this partition
			uiHiddenSectors = sPartitionEntry->lba_first_sector;

			// make sure the partition doesn't exceeds the physical boundries
			// of the device
			if (sPartitionEntry->lba_first_sector + sPartitionEntry->total_sectors > mpcDevice->GetTotalSectors())
			{
				uiPartitionsTtried++;
				continue;
			}

			// retrieve the 1st sector of partition
			bSuccess = mpcDevice->Read(sPartitionEntry->lba_first_sector, uiBuffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_READ_MEDIA;
			}
		}

		// set our pointer to the BPB
		psBPB = (SFatBIOSParameterBlock*)uiBuffer;

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
			uiPartitionsTtried++;
			continue;
		}

		// make sure that SecPerClus is a power of two
		uint16 uiResult = psBPB->BPB_SecPerClus;
		while (uiResult != 0x1)
		{
			if (uiResult & 0x1)
			{
				uiPartitionsTtried++;
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
				uiPartitionsTtried++;
				continue;
			}
			break;
		case FAT_FS_TYPE_FAT16:
			if (msVolume.uiFatSize < (((msVolume.uiNoOfClusters * 2) + psBPB->BPB_BytsPerSec - 1) / psBPB->BPB_BytsPerSec))
			{
				uiPartitionsTtried++;
				continue;
			}
			break;
		case FAT_FS_TYPE_FAT32:
			if (msVolume.uiFatSize < (((msVolume.uiNoOfClusters * 4) + psBPB->BPB_BytsPerSec - 1) / psBPB->BPB_BytsPerSec))
			{
				uiPartitionsTtried++;
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
		// entry's cluster from the BPB_RootClus field
		// on the BPB
		if (msVolume.eFileSystem == FAT_FS_TYPE_FAT32)
		{
			msVolume.uiRootCluster = psBPB->uFatEx.sFat32.BPB_RootClus;
		}
		else
		{
			msVolume.uiRootCluster = 0x0;
		}

		// ###############################################
		// NOTE!!!: psBPB is no good from this point on!!!!
		// ###############################################

		// check that this is a valid FAT partition by comparing the uiMedia
		// byte in the BPB with the 1st byte of the fat table
		{
			uint8 uiMedia = psBPB->BPB_Media;

			// read the 1st sector of the FAT table
			bSuccess = mpcDevice->Read(msVolume.uiNoOfReservedSectors, uiBuffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_READ_MEDIA;
			}

			// if the lower byte of the 1st FAT entry is not the same as
			// BPB_Media then this is not a valid mpsVolume
			if (uiBuffer[0] != uiMedia)
			{
				uiPartitionsTtried++;
				continue;
			}
		}
		break;
	}

	// read mpsVolume label entry from the root directory (if any)
	{

		SFatQueryStateInternal query;
		query.uiBuffer = uiBuffer;
		if (FatQueryFirstEntry(0, FAT_ATTR_VOLUME_ID, (SFatQueryState*)&query, 1) == FAT_SUCCESS)
		{
			if (*query.current_entry_raw->uEntry.sFatRawCommon.name != 0)
			{
				TrimPath(msVolume.szLabel, (char*)query.current_entry_raw->uEntry.sFatRawCommon.name, 11);
			}
		}
	}
	msVolume.uiFileSystemInfoSector = 0xFFFFFFFF;


	// if we find a valid psFileSystemInfo structure we'll use it
	if (msVolume.eFileSystem == FAT_FS_TYPE_FAT32)
	{
		SFatFileSystemInfo* psFileSystemInfo;

		// read the sector containing the FSInfo structure
		bSuccess = mpcDevice->Read(uiHiddenSectors + uiFileSystemInfoSector, uiBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_READ_MEDIA;
		}

		// set psFileSystemInfo pointer
		psFileSystemInfo = (SFatFileSystemInfo*)uiBuffer;

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
uint16 CFatVolume::Unmount(void)
{
	// if this is a FAT32 volume we'll update the psFileSystemInfo structure
	if (msVolume.eFileSystem == FAT_FS_TYPE_FAT32 && msVolume.uiFileSystemInfoSector != 0xFFFFFFFF)
	{
		bool					bSuccess;
		SFatFileSystemInfo*		psFileSystemInfo;
		uint8*					uiBuffer = mauiFatSharedBuffer;

		// mark the loaded sector as unknown
		SetFatSharedBufferSector(FAT_UNKNOWN_SECTOR);

		// read the sector containing the FSInfo structure
		bSuccess = mpcDevice->Read(GetFsinfoSector(), uiBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_READ_MEDIA;
		}


		// set the pointer to the psFileSystemInfo structure
		psFileSystemInfo = (SFatFileSystemInfo*)uiBuffer;

		// check the signatures before writting
		// note: when you mount a removable device in windows it will channge
		// these signatures, i guess it feels it cannot be trusted. So we're going
		// to rebuild them no matter what as they significantly speed up this
		// implementation. After the mpsVolume has been mounted elsewhere Free_Count cannot
		// be trusted. This implementation doesn't actually use it but if you only
		// mount the mpsVolume with us it will keep it up to date.

		/*if (psFileSystemInfo->LeadSig == 0x41615252 && psFileSystemInfo->StructSig == 0x61417272 && psFileSystemInfo->TrailSig == 0xAA550000)*/
		{
			// mark all values as unknown
			psFileSystemInfo->Nxt_Free = GetNextFreeCluster();
			psFileSystemInfo->Free_Count = GetTotalFreeClusters();
			psFileSystemInfo->LeadSig = 0x41615252;
			psFileSystemInfo->StructSig = 0x61417272;
			psFileSystemInfo->TrailSig = 0xAA550000;

			// write the psFileSystemInfo sector
			bSuccess = mpcDevice->Write(GetFsinfoSector(), uiBuffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_READ_MEDIA;
			}
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
bool CFatVolume::Write(uint64 uiSector, void* pvData)
{
	return mpcDevice->Write(uiSector, pvData);
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
uint16 CFatVolume::GetSectorSize(void)
{
	return msVolume.uiNoOfBytesPerSector;
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
	return ((GetFileSystemType() == FAT_FS_TYPE_FAT32) ? !(uifat & 0x0FFFFFFF) : (GetFileSystemType() == FAT_FS_TYPE_FAT16) ? !(uifat & 0xFFFF) : !(uifat & 0x0FFF));
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
	return muiFatSharedBufferSector != 0xFFFFFFFF && (muiFatSharedBufferSector == uiSector);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::CalculateFatEntryOffset(EFatFileSystemType eFileSystemType, uint32 cluster)
{
	switch (eFileSystemType)
	{
	case FAT_FS_TYPE_FAT12:
		return cluster + (cluster >> 1);

	case FAT_FS_TYPE_FAT16:
		return cluster * (uint32)2;

	case FAT_FS_TYPE_FAT32:
		return cluster * (uint32)4;
	}

	return 0xFFFFFFFF;
}


// allocates a cluster for a directory - finds a free cluster, initializes it as
// required by a directory, marks it's FAT entry as EOC and returns the cluster address
//
// NOTE: this function used the volume/shared uiBuffer (if enabled) so it must not be
// locked before calling this function
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::FatAllocateDirectoryCluster(SFatRawDirectoryEntry* parent, uint16* result)
{
	return FatAllocateCluster(parent, 1, 1, 1, result);
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
uint32 CFatVolume::FatAllocateDataCluster(uint32 count, char zero, uint16* result)
{
	return FatAllocateCluster(0, count, zero, 1, result);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::FatAllocateDataClusterEx(uint32 count, char zero, uint32 page_size, uint16* result)
{
	return FatAllocateCluster(0, count, zero, page_size, result);
}


// performs the work for FatAllocateDataCluster and FatAllocateDirectoryCluster
//
// NOTE: this function used the volume/shared uiBuffer (if enabled) so it must not be
// locked before calling this function
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::FatAllocateCluster(SFatRawDirectoryEntry* parent, uint32 count, char zero, uint32 page_size, uint16* result)
{
	bool		bSuccess;
	uint32		entry_sector;			/* the address of the cached sector */
	uint32		current_sector;			/* address of the sector loaded in memory */
	uint32		cluster;				/* cluster number */
	uint32		entry_offset = 0;		/* offset of fat entry within it's sector */
	char		entries_updated;		/* indicates that the cached sector is dirty */
	char		next_sector_loaded = 0;	/* indicates that the next sector has been loaded */
	FatEntry	last_fat_entry = 0;		/* stores the value of the last cluster found or EOC if no clusters found yet */
	FatEntry	fat_entry;				/* temp value to store cluster numbers read from FAT table */
	uint32		first_cluster;
	uint32		last_entry_sector = 0;
	uint32		last_entry_offset = 0;
	uint32		start_cluster;
	char		wrapped_around = 0;
	uint16		step = 1;

	uint8* uiBuffer = mauiFatSharedBuffer;

	// the zero and parent parameters should only be set when
	// allocating only 1 cluster. Also this function should never
	// be called with count = 0 as it won't return a cluster # which
	// will cause undefined behaviour so the caller needs to make sure
	// that it's requesting at least 1 cluster.
	if (count > 1 && (parent != 0 || zero != 0))
	{
		return FAT_UNKNOWN_ERROR;
	}

	if (count == 0)
	{
		return FAT_UNKNOWN_ERROR;
	}

	fat_entry = 0;
	entries_updated = 0;
	*result = FAT_SUCCESS;
	cluster = 0x2;
	first_cluster = 0;

	// if we got a hint of the 1st free cluster then take it
	if (HasNextFreeCluser())
	{
		cluster = GetNextFreeCluster();
	}

	// find the step between clusters allocated on page boundaries
	if (GetNoOfSectorsPerCluster() < page_size)
	{
		uint32 sector;
		uint16 step_count = 0;
		step = (uint16)(page_size / GetNoOfSectorsPerCluster());
		/*
		// find the 1st cluster that starts on a page boundary
		*/
		sector = CalculateFirstSectorOfCluster(cluster);
		while (sector % page_size && step_count < step)
		{
			cluster++;
			step_count++;
			sector = CalculateFirstSectorOfCluster(cluster);
		}
	}

	// remember the 1st cluster of our search
	start_cluster = cluster;

	// set the last_fat_entry value to the eof marker
	switch (GetFileSystemType())
	{
	case FAT_FS_TYPE_FAT12:
		last_fat_entry = FAT12_EOC;
		break;

	case FAT_FS_TYPE_FAT16:
		last_fat_entry = FAT16_EOC;
		break;

	case FAT_FS_TYPE_FAT32:
		last_fat_entry = FAT32_EOC;
		break;
	}

	// calculate the offset of the FAT entry within it's sector
	// and the sector number
	entry_offset = CalculateFatEntryOffset(GetFileSystemType(), cluster);
	entry_sector = GetNoOfReservedSectors() + (entry_offset / GetNoOfBytesPerSector());
	entry_offset = entry_offset % GetNoOfBytesPerSector();
	last_entry_sector = entry_sector;

	// for each sector of the FAT
	for (;;)
	{
		// read sector into sector cache
		if (!IsFatSectorLoaded(entry_sector))
		{
			bSuccess = Read(entry_sector, uiBuffer);
			if (!bSuccess)
			{
				muiFatSharedBufferSector = (0xFFFFFFFF);
				*result = FAT_CANNOT_READ_MEDIA;
				return 0;
			}
			muiFatSharedBufferSector = (entry_sector);
		}

		// save the current sector number
		current_sector = entry_sector;

		// for every FAT entry in the sector...
		do
		{
			// if we've reached the last cluster on the drive return insufficient
			// disk space error code.
			if ((cluster > GetNoOfClusters() + 1) && start_cluster > 2 && !wrapped_around)
			{
				if (entries_updated)
				{
					// if the uiBuffer is dirty flush the changes
					bSuccess = FatWriteFatSector(current_sector, uiBuffer);
					if (!bSuccess)
					{
						muiFatSharedBufferSector = (0xFFFFFFFF);
						*result = FAT_CANNOT_READ_MEDIA;
						return 0;
					}

					// mark the uiBuffer as clean
					entries_updated = 0;
				}

				// reset the cluster # to 2
				cluster = 2;
				wrapped_around = 1;

				// calculate the sector for the new cluster
				entry_offset = CalculateFatEntryOffset(GetFileSystemType(), cluster);
				entry_sector = GetNoOfReservedSectors();
				entry_offset = entry_offset % GetNoOfBytesPerSector();

				// break from this loop so that sector gets loaded
				break;

			}
			else if ((cluster > GetNoOfClusters() + 1) || (wrapped_around && cluster >= start_cluster))
			{
				if (entries_updated)
				{
					// if the uiBuffer is dirty flush the changes, otherwise when we try
					// to free the cluster chain it would run into free clusters before
					// the EOC resulting in lost chains (or a hang in a debug build).
					bSuccess = FatWriteFatSector(current_sector, uiBuffer);
					if (!bSuccess)
					{
						muiFatSharedBufferSector = (0xFFFFFFFF);
						*result = FAT_CANNOT_READ_MEDIA;
						return 0;
					}
				}

				// free any clusters that we've allocated so far
				if (!FatIsEOFEntry(last_fat_entry))
				{
					// even if this returns error we cannot do anything with
					// it as we're already returning an error code so we can only
					// hope that the clusters get freed.
					uint16 uiResult;
					uiResult = FatFreeClusterChain(last_fat_entry);
				}

				// return insufficient disk space error
				*result = FAT_INSUFFICIENT_DISK_SPACE;
				return 0;
			}

			// copy the next FAT entry to the fat_entry variable
			switch (GetFileSystemType())
			{
			case FAT_FS_TYPE_FAT12:
			{
				// clear fat_entry to make sure that the upper 16
				// bits are not set.
				fat_entry = 0;

				// read the 1st byte
				((uint8*)&fat_entry)[INT32_BYTE0] = uiBuffer[entry_offset];

				// load the next sector (if necessary) and set the offset
				// for the next byte in the uiBuffer
				if (entry_offset == GetNoOfBytesPerSector() - 1)
				{
					if (entries_updated)
					{
						// if the uiBuffer is dirty flush it before we use it to load
						// the next sector
						bSuccess = FatWriteFatSector(entry_sector, uiBuffer);
						if (!bSuccess)
						{
							muiFatSharedBufferSector = (0xFFFFFFFF);
							*result = FAT_CANNOT_READ_MEDIA;
							return 0;
						}

						// mark the uiBuffer as clean
						entries_updated = 0;
					}

					// load the next sector into the uiBuffer
					bSuccess = Read(entry_sector + 1, uiBuffer);
					if (!bSuccess)
					{
						muiFatSharedBufferSector = (0xFFFFFFFF);
						*result = FAT_CANNOT_READ_MEDIA;
						return 0;
					}

					// set flag to indicate that we had to load the next sector
					next_sector_loaded = 1;
				}

				// read the 2nd byte
				((uint8*)&fat_entry)[INT32_BYTE1] = uiBuffer[next_sector_loaded ? 0 : (entry_offset + 1)];

				// Since a FAT12 entry is only 12 bits (1.5 bytes) we need to adjust the result.
				// For odd cluster numbers the FAT entry is stored in the upper 12 bits of the
				// 16 bits where it is stored, so we need to shift the value 4 bits to the right.
				// For even cluster numbers the FAT entry is stored in the lower 12 bits of the
				// 16 bits where it is stored, so we need to clear the upper 4 bits.
				if (cluster & 0x1)
				{
					fat_entry >>= 4;
				}
				else
				{
					fat_entry &= 0xFFF;
				}

				// if we had to load the next sector into the cache we need
				// to reload the current sector before continuing
				if (next_sector_loaded)
				{
					bSuccess = Read(entry_sector, uiBuffer);
					if (!bSuccess)
					{
						muiFatSharedBufferSector = (0xFFFFFFFF);
						*result = FAT_CANNOT_READ_MEDIA;
						return 0;
					}
					next_sector_loaded = 0;
				}
				break;
			}
			case FAT_FS_TYPE_FAT16:
			{
				fat_entry = *((uint16*)&uiBuffer[entry_offset]);
				break;
			}
			case FAT_FS_TYPE_FAT32:
			{
				fat_entry = *((uint32*)&uiBuffer[entry_offset]) & 0x0FFFFFFF;
				break;
			}
			}

			// if the current FAT is free
			if (IsFreeFat(fat_entry))
			{
				// maintain the count of free cluster and the next
				// cluster that may be free
				SetNextFreeCluster(cluster + 1);
				SetTotalFreeClusters(GetTotalFreeClusters() - 1);

				// if this is the 1st cluster found remember it
				if (!first_cluster)
				{
					first_cluster = cluster;
				}

				// mark the FAT as the the new 1st link of the cluster chain
				// (or the end of the chain if we're only allocating 1 cluster)
				switch (GetFileSystemType())
				{
				case FAT_FS_TYPE_FAT12:
				{
					// write the 1st byte
					if (cluster & 0x1)
					{
						uiBuffer[entry_offset] &= 0x0F;								/* clear entry bits on 1st byte */
						uiBuffer[entry_offset] |= LO8((uint16)FAT12_EOC << 4);		/* set entry bits on 1st byte */
					}
					else
					{
						uiBuffer[entry_offset] = LO8((uint16)FAT12_EOC);			/* just copy the 1st byte */
					}

					// if the FAT entry spans a sector boundary flush the currently
					// loaded sector to the drive and load the next one.
					if (entry_offset == GetNoOfBytesPerSector() - 1)
					{
						// flush the updated sector to the drive
						bSuccess = FatWriteFatSector(entry_sector, uiBuffer);
						if (!bSuccess)
						{
							muiFatSharedBufferSector = (0xFFFFFFFF);
							*result = FAT_CANNOT_WRITE_MEDIA;
							return 0;
						}

						// mark the uiBuffer as clean
						entries_updated = 0;

						// load the next sector
						bSuccess = Read(entry_sector + 1, uiBuffer);
						if (!bSuccess)
						{
							muiFatSharedBufferSector = (0xFFFFFFFF);
							*result = FAT_CANNOT_READ_MEDIA;
							return 0;
						}

						// set the next_sector_loaded flag
						next_sector_loaded = 1;
					}

					// write the 2nd byte
					if (cluster & 0x1)
					{
						uiBuffer[next_sector_loaded ? 0 : (entry_offset + 1)] = HI8((uint16)FAT12_EOC << 4);
					}
					else
					{
						uiBuffer[next_sector_loaded ? 0 : (entry_offset + 1)] &= 0xF0;
						uiBuffer[next_sector_loaded ? 0 : (entry_offset + 1)] |= HI8((uint16)FAT12_EOC);
					}

					// if the next sector has been loaded then flush it
					if (next_sector_loaded)
					{
						// flush uiBuffer to disk
						bSuccess = FatWriteFatSector(entry_sector + 1, uiBuffer);
						if (!bSuccess)
						{
							muiFatSharedBufferSector = (0xFFFFFFFF);
							*result = FAT_CANNOT_READ_MEDIA;
							return 0;
						}

						// note: we don't reload the current sector because this being the last entry
						// the next sector is about to be reloaded anyways
						next_sector_loaded = 0;

						// reload the current sector
						bSuccess = Read(entry_sector, uiBuffer);
						if (!bSuccess)
						{
							muiFatSharedBufferSector = (0xFFFFFFFF);
							*result = FAT_CANNOT_READ_MEDIA;
							return 0;
						}
						entries_updated = 0;
					}
					else
					{
						entries_updated = 1;
					}

					// if this is not the 1st cluster allocated update
					// the last one to link to this one
					if (last_fat_entry != FAT12_EOC)
					{
						if (last_entry_sector != entry_sector)
						{
							if (entries_updated)
							{
								// flush uiBuffer to disk
								bSuccess = FatWriteFatSector(entry_sector, uiBuffer);
								if (!bSuccess)
								{
									muiFatSharedBufferSector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}

								// mark uiBuffer as clean
								entries_updated = 0;
							}

							// load last_entry sector
							bSuccess = Read(last_entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}
						}

						// update the last entry to point to this one
						if (last_fat_entry & 0x1)
						{
							uiBuffer[last_entry_offset] &= 0x0F;							/* clear entry bits on 1st byte */
							uiBuffer[last_entry_offset] |= LO8((uint16)cluster << 4);		/* set entry bits on 1st byte */
						}
						else
						{
							uiBuffer[last_entry_offset] = LO8((uint16)cluster);	/* just copy the 1st byte */
						}

						// if the FAT entry spans a sector boundary flush the currently
						// loaded sector to the drive and load the next one.
						if (last_entry_offset == GetNoOfBytesPerSector() - 1)
						{
							// flush the updated sector to the drive
							bSuccess = FatWriteFatSector(last_entry_sector, uiBuffer);
							if (!bSuccess)
							{
								*result = FAT_CANNOT_WRITE_MEDIA;
								return 0;
							}

							// mark the uiBuffer as clean
							entries_updated = 0;

							// load the next sector
							bSuccess = Read(last_entry_sector + 1, uiBuffer);
							if (!bSuccess)
							{
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}

							// set the next_sector_loaded flag
							next_sector_loaded = 1;
						}

						// write the 2nd byte
						if (last_fat_entry & 0x1)
						{
							uiBuffer[next_sector_loaded ? 0 : (last_entry_offset + 1)] = HI8((uint16)cluster << 4);
						}
						else
						{
							uiBuffer[next_sector_loaded ? 0 : (last_entry_offset + 1)] &= 0xF0;
							uiBuffer[next_sector_loaded ? 0 : (last_entry_offset + 1)] |= HI8((uint16)cluster);
						}

						// if the next sector has been loaded then flush it
						if (next_sector_loaded)
						{
							// flush uiBuffer to disk
							bSuccess = FatWriteFatSector(last_entry_sector + 1, uiBuffer);
							if (!bSuccess)
							{
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}

							// note: we don't reload the current sector because this being the last entry
							// the next sector is about to be reloaded anyways
							next_sector_loaded = 0;

							// reload the last entry sector
							bSuccess = Read(last_entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = 0xFFFFFFFF;
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}
						}
						else
						{
							entries_updated = 1;
						}
						if (last_entry_sector != entry_sector)
						{
							if (entries_updated)
							{
								// flush uiBuffer to disk
								bSuccess = FatWriteFatSector(last_entry_sector, uiBuffer);
								if (!bSuccess)
								{
									muiFatSharedBufferSector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}

								// mark uiBuffer as clean
								entries_updated = 0;
							}
							/*
							// reload current sector
							*/
							bSuccess = Read(entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}
						}
					}
					break;
				}
				case FAT_FS_TYPE_FAT16:
				{
					*((uint16*)&uiBuffer[entry_offset]) = FAT16_EOC;

					// mark the uiBuffer as dirty
					entries_updated = 1;

					// if this is not the first cluster allocated update
					// the last one to link to this one
					if (last_fat_entry != FAT16_EOC)
					{
						if (last_entry_sector != entry_sector)
						{
							// flush uiBuffer to disk
							bSuccess = FatWriteFatSector(entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}
							// mark uiBuffer as clean
							entries_updated = 0;

							// load last_entry sector
							bSuccess = Read(last_entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}
						}

						// update the last entry to point to this one
						*((uint16*)&uiBuffer[last_entry_offset]) = (uint16)cluster;

						if (last_entry_sector != entry_sector)
						{
							// flush uiBuffer to disk
							bSuccess = FatWriteFatSector(last_entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}

							// reload current sector
							bSuccess = Read(entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}
						}
					}
					break;
				}
				case FAT_FS_TYPE_FAT32:
				{
					*((uint32*)&uiBuffer[entry_offset]) &= 0xF0000000;
					*((uint32*)&uiBuffer[entry_offset]) |= FAT32_EOC & 0x0FFFFFFF;

					// mark the uiBuffer as dity
					entries_updated = 1;

					// if this is not the 1st cluster allocated update
					// the last one to link to this one
					if (last_fat_entry != FAT32_EOC)
					{
						if (last_entry_sector != entry_sector)
						{
							// flush uiBuffer to disk
							bSuccess = FatWriteFatSector(entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}

							// mark uiBuffer as clean
							entries_updated = 0;

							// load last_entry sector
							bSuccess = Read(last_entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}
						}

						// update the last entry to point to this one
						*((uint32*)&uiBuffer[last_entry_offset]) &= 0xF0000000;
						*((uint32*)&uiBuffer[last_entry_offset]) |= cluster & 0x0FFFFFFF;

						if (last_entry_sector != entry_sector)
						{
							// flush uiBuffer to disk
							bSuccess = FatWriteFatSector(last_entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}

							// reload current sector
							bSuccess = Read(entry_sector, uiBuffer);
							if (!bSuccess)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}
						}
					}
					break;
				}
				}

				// if we've found all the clusters that the user requested leave
				// and return cluster #
				if (!--count)
				{
					if (entries_updated)
					{
						// flush uiBuffer to disk
						bSuccess = FatWriteFatSector(entry_sector, uiBuffer);
						if (!bSuccess)
						{
							muiFatSharedBufferSector = (0xFFFFFFFF);
							*result = FAT_CANNOT_READ_MEDIA;
							return 0;
						}
					}

					// if we're allocating a cluster for a
					// directory then initialize it properly
					if (parent)
					{
						uint16 uiResult;
						uiResult = FatInitializeDirectoryCluster(parent, cluster, uiBuffer);
						if (uiResult != FAT_SUCCESS)
						{
							muiFatSharedBufferSector = (0xFFFFFFFF);
							*result = uiResult;
							return 0;
						}
					}
					else
					{
						if (zero)
						{
							uint16 uiResult;
							uiResult = FatZeroCluster(cluster, uiBuffer);
							if (uiResult != FAT_SUCCESS)
							{
								muiFatSharedBufferSector = (0xFFFFFFFF);
								*result = uiResult;
								return 0;
							}
						}
					}
					return first_cluster;
				}

				// remember the cluster number so we can mark the
				// next fat entry with it
				last_fat_entry = cluster;
				last_entry_sector = entry_sector;
				last_entry_offset = entry_offset;
			}

			// increase the cluster number
			if (!first_cluster)
			{
				cluster += step;
			}
			else
			{
				cluster++;
			}

			// calculate the offset of the cluster's FAT entry within it's sector
			// note: when we hit get past the end of the current sector entry_offset
			// will roll back to zero (or possibly 1 for FAT12)
			entry_offset = CalculateFatEntryOffset(GetFileSystemType(), cluster);
			entry_sector = GetNoOfReservedSectors() + (entry_offset / GetNoOfBytesPerSector());
			entry_offset = entry_offset % GetNoOfBytesPerSector();

		} while (current_sector == entry_sector);

		// if any changes where made to the fat entry currently cached sector
		// flush the changes to the drive before moving to next sector
		if (entries_updated)
		{
			bSuccess = FatWriteFatSector(current_sector, uiBuffer);
			if (!bSuccess)
			{
				muiFatSharedBufferSector = (0xFFFFFFFF);
				*result = FAT_CANNOT_READ_MEDIA;
				return 0;
			}
			entries_updated = 0;
		}
	}
}


// marks all the clusters in the cluster chain as free
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatFreeClusterChain(uint32 cluster)
{
	bool	bSuccess;
	uint32	fat_offset = 0;			/* the offset of the cluster entry within the FAT table */
	uint32	entry_offset;			/* the offset of the cluster entry within it's sector */
	uint32	entry_sector;			/* the sector where the entry is stored on the drive */
	uint32	current_sector;			/* the sector that's currently loaded in memory */
	char	is_odd_cluster = 0;		/* indicates that the entry being processed is an odd cluster address (FAT12 only) */
	char	op_in_progress = 0;		/* indicates that a multi-step operation is in progress (FAT12 only) */
	uint8* uiBuffer = mauiFatSharedBuffer;

	// get the offset of the cluster entry within the FAT table,
	// the sector of the FAT table that contains the entry and the offset
	// of the fat entry within the sector
	fat_offset = CalculateFatEntryOffset(GetFileSystemType(), cluster);
	entry_sector = GetNoOfReservedSectors() + (fat_offset / GetNoOfBytesPerSector());
	entry_offset = fat_offset % GetNoOfBytesPerSector();

	// loop until we reach the EOC cluster or an error occurs.
	while (1)
	{
		// load sector to memory
		if (!IsFatSectorLoaded(entry_sector))
		{
			bSuccess = Read(entry_sector, uiBuffer);
			if (!bSuccess)
			{
				muiFatSharedBufferSector = (0xFFFFFFFF);
				return FAT_CANNOT_READ_MEDIA;
			}
			muiFatSharedBufferSector = (entry_sector);
		}

		// store the address of the sector that's (will be) loaded in memory
		current_sector = entry_sector;

		// loop until a new sector needs to be loaded to continue
		// with the operation
		while (current_sector == entry_sector)
		{
			// if cluster is less than 2 either we got a bug
			// or the file system is corrupted
			if (cluster < 2)
			{
				// leave critical section and return error code
				return FAT_INVALID_CLUSTER;
			}
			// read the cluster entry and mark it as free
			switch (GetFileSystemType())
			{
			case FAT_FS_TYPE_FAT12:
			{
				if (!op_in_progress)
				{
					// remember whether this is an odd cluster or not
					is_odd_cluster = (cluster & 0x1);

					// set the cluster to zero to make sure that the upper bytes are cleared
					// since we're only updating the lower 16 bits.
					cluster = 0;

					// read the 1st byte
					((uint8*)&cluster)[0] = uiBuffer[entry_offset];

					// write the 1st byte
					//
					// note: since the value that we're writting is FREE_FAT which expands
					// to zero we can skip a step or two in the update process. I left the code
					// for the steps that we're skipping commented out for clarity.
					if (is_odd_cluster)
					{
						uiBuffer[entry_offset] &= 0x0F;
					}
					else
					{
						uiBuffer[entry_offset] = FREE_FAT;
					}
				}

				if (entry_offset == GetNoOfBytesPerSector() - 1)
				{
					/*
					// flush current sector to drive
					*/
					bSuccess = FatWriteFatSector(current_sector, uiBuffer);
					if (!bSuccess)
					{
						muiFatSharedBufferSector = 0xFFFFFFFF;
						return FAT_CANNOT_READ_MEDIA;
					}

					// if the entry spans a sector boundary set op_in_progress to 1
					// so that we don't read the 1st byte again when we come back.
					// also increase the sector number and set the entry_offset to 0 since
					// the next byte will be on offset zero when the next sector is loaded
					entry_sector++;
					entry_offset = 0;
					op_in_progress = 1;

					// continue with the next iteration of the loop. We'll come right back
					// here with the next sector loaded
					continue;
				}
				else if (!op_in_progress)
				{
					// increase the offset to point to the next byte
					entry_offset++;
				}

				// read the 2nd byte
				((uint8*)&cluster)[1] = uiBuffer[entry_offset];

				// Since a FAT12 entry is only 12 bits (1.5 bytes) we need to adjust the result.
				// For odd cluster numbers the FAT entry is stored in the upper 12 bits of the
				// 16 bits where it is stored, so we need to shift the value 4 bits to the right.
				// For even cluster numbers the FAT entry is stored in the lower 12 bits of the
				// 16 bits where it is stored, so we need to clear the upper 4 bits.
				if (is_odd_cluster)
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
				if (is_odd_cluster)
				{
					uiBuffer[entry_offset] = FREE_FAT;
				}
				else
				{
					uiBuffer[entry_offset] &= 0xF0;	/* clear bits that 1st byte will be written to */
				}

				// clear op_in_progress flag.
				op_in_progress = 0;
				break;
			}
			case FAT_FS_TYPE_FAT16:
			{
				cluster = (uint32) * ((uint16*)&uiBuffer[entry_offset]);
				*((uint16*)&uiBuffer[entry_offset]) = FREE_FAT;
				break;
			}
			case FAT_FS_TYPE_FAT32:
			{
				// FAT32 entries are actually 28 bits so we need to leave the
				// upper nibble untouched
				cluster = *((uint32*)&uiBuffer[entry_offset]) & 0x0FFFFFFF;
				*((uint32*)&uiBuffer[entry_offset]) &= 0xF0000000;
				break;
			}
			}

			// increase the count of free clusters
			SetTotalFreeClusters(GetTotalFreeClusters() + 1);

			// if it's the EOF marker we're done, flush the uiBuffer and go
			if (FatIsEOFEntry(cluster))
			{
				bSuccess = FatWriteFatSector(current_sector, uiBuffer);
				if (!bSuccess)
				{
					muiFatSharedBufferSector = 0xFFFFFFFF;
					return FAT_CANNOT_READ_MEDIA;
				}

				return FAT_SUCCESS;
			}

			// calculate the location of the next cluster in the chain
			fat_offset = CalculateFatEntryOffset(GetFileSystemType(), cluster);
			entry_sector = GetNoOfReservedSectors() + (fat_offset / GetNoOfBytesPerSector());
			entry_offset = fat_offset % GetNoOfBytesPerSector();
		}

		// flush FAT table changes
		bSuccess = FatWriteFatSector(current_sector, uiBuffer);
		if (!bSuccess)
		{
			muiFatSharedBufferSector = 0xFFFFFFFF;
			return FAT_CANNOT_READ_MEDIA;
		}
	}
}


// gets the FAT structure for a given cluster number
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatGetClusterEntry(uint32 cluster, FatEntry* fat_entry)
{
	bool	bSuccess;
	uint32	fat_offset = 0;		/* todo: this one may require 64 bits for large drives? */
	uint32	entry_sector;
	uint32	entry_offset;		/* todo: 16 bits should suffice for this value */

	uint8* uiBuffer = mauiFatSharedBuffer;

	// get the offset of the entry within the FAT table
	// for the requested cluster
	switch (GetFileSystemType())
	{
	case FAT_FS_TYPE_FAT12:
		fat_offset = cluster + (cluster >> 1);
		break;

	case FAT_FS_TYPE_FAT16:
		fat_offset = cluster * ((uint32)2);
		break;

	case FAT_FS_TYPE_FAT32:
		fat_offset = cluster * ((uint32)4);
		break;
	}

	// get the address of the sector that contains the FAT entry and
	// the offset of the FAT entry within that sector
	entry_sector = GetNoOfReservedSectors() + (fat_offset / GetNoOfBytesPerSector());
	entry_offset = fat_offset % GetNoOfBytesPerSector();

	// load sector into the uiBuffer
	if (!IsFatSectorLoaded(entry_sector))
	{
		bSuccess = Read(entry_sector, uiBuffer);
		if (!bSuccess)
		{
			muiFatSharedBufferSector = 0xFFFFFFFF;
			return FAT_CANNOT_READ_MEDIA;
		}
		muiFatSharedBufferSector = (entry_sector);
	}

	// set the user supplied uiBuffer with the
	// value of the FAT entry
	switch (GetFileSystemType())
	{
	case FAT_FS_TYPE_FAT12:
	{
		// clear fat_entry to make sure that the upper 16
		// bits are not set.
		*fat_entry = 0;

		// read the 1st byte
		((uint8*)fat_entry)[0] = uiBuffer[entry_offset];

		// load the next sector (if necessary) and set the offset
		// for the next byte in the uiBuffer
		if (entry_offset == GetNoOfBytesPerSector() - 1)
		{
			// load the next sector into the uiBuffer
			bSuccess = Read(entry_sector + 1, uiBuffer);
			if (!bSuccess)
			{
				muiFatSharedBufferSector = 0xFFFFFFFF;
				return FAT_CANNOT_READ_MEDIA;
			}
			muiFatSharedBufferSector = (entry_sector + 1);

			// the 2nd byte is now the 1st byte in the uiBuffer
			entry_offset = 0;
		}
		else
		{
			// the 2nd byte is still right after the 1st one on
			// the uiBuffer
			entry_offset++;
		}

		// read the 2nd byte
		((uint8*)fat_entry)[1] = uiBuffer[entry_offset];

		// Since a FAT12 entry is only 12 bits (1.5 bytes) we need to adjust the result.
		// For odd cluster numbers the FAT entry is stored in the upper 12 bits of the
		// 16 bits where it is stored, so we need to shift the value 4 bits to the right.
		// For even cluster numbers the FAT entry is stored in the lower 12 bits of the
		// 16 bits where it is stored, so we need to clear the upper 4 bits.

		if (cluster & 0x1)
		{
			*fat_entry >>= 4;
		}
		else
		{
			*fat_entry &= 0xFFF;
		}
		break;
	}
	case FAT_FS_TYPE_FAT16:
	{
		*fat_entry = (uint32) * ((uint16*)&uiBuffer[entry_offset]);
		break;
	}
	case FAT_FS_TYPE_FAT32:
	{
		*fat_entry = *((uint32*)&uiBuffer[entry_offset]) & 0x0FFFFFFF;
		break;
	}
	}

	return FAT_SUCCESS;
}


// updates the FAT entry for a given cluster
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatSetClusterEntry(uint32 cluster, FatEntry fat_entry)
{
	bool	bSuccess;
	uint32	fat_offset = 0;
	uint32	entry_sector;
	uint32	entry_offset;
	uint8* uiBuffer = mauiFatSharedBuffer;

	// get the offset of the entry in the FAT table for the requested cluster
	switch (GetFileSystemType())
	{
	case FAT_FS_TYPE_FAT12:
		fat_offset = cluster + (cluster >> 1);
		break;

	case FAT_FS_TYPE_FAT16:
		fat_offset = cluster * ((uint32)2);
		break;

	case FAT_FS_TYPE_FAT32:
		fat_offset = cluster * ((uint32)4);
		break;
	}

	// get the address of the sector that contains the FAT entry
	// and the offset of the FAT entry within that sector
	entry_sector = GetNoOfReservedSectors() + (fat_offset / GetNoOfBytesPerSector());
	entry_offset = fat_offset % GetNoOfBytesPerSector();


	// read sector into uiBuffer
	if (!IsFatSectorLoaded(entry_sector))
	{
		bSuccess = Read(entry_sector, uiBuffer);
		if (!bSuccess)
		{
			muiFatSharedBufferSector = 0xFFFFFFFF;
			return FAT_CANNOT_READ_MEDIA;
		}
		muiFatSharedBufferSector = (entry_sector);
	}

	// set the FAT entry
	switch (GetFileSystemType())
	{
	case FAT_FS_TYPE_FAT12:
	{
		// write the 1st byte
		if (cluster & 0x1)
		{
			fat_entry <<= 4;									/* odd entries occupy the upper 12 bits so we must shift */
			uiBuffer[entry_offset] &= 0x0F;						/* clear entry bits on 1st byte */
			uiBuffer[entry_offset] |= LO8((uint16)fat_entry);		/* set entry bits on 1st byte */
		}
		else
		{
			uiBuffer[entry_offset] = LO8((uint16)fat_entry);	/* just copy the 1st byte */
		}

		// if the FAT entry spans a sector boundary flush the currently
		// loaded sector to the drive and load the next one.
		if (entry_offset == GetNoOfBytesPerSector() - 1)
		{
			// flush the updated sector to the drive
			bSuccess = FatWriteFatSector(entry_sector, uiBuffer);
			if (!bSuccess)
			{
				muiFatSharedBufferSector = (0xFFFFFFFF);
				return FAT_CANNOT_WRITE_MEDIA;
			}

			// increase the sector address
			entry_sector++;

			// load the next sector
			bSuccess = Read(entry_sector, uiBuffer);
			if (!bSuccess)
			{
				muiFatSharedBufferSector = (0xFFFFFFFF);
				return FAT_CANNOT_READ_MEDIA;
			}
			muiFatSharedBufferSector = (entry_sector);

			// the next byte is now loacted at offset 0 on the uiBuffer
			entry_offset = 0;
		}
		else
		{
			// the next byte is located next to the 1st one on the uiBuffer
			entry_offset++;
		}

		// write the 2nd byte
		if (cluster & 0x1)
		{
			uiBuffer[entry_offset] = HI8((uint16)fat_entry);		/* just copy the 1st byte */
		}
		else
		{
			uiBuffer[entry_offset] &= 0xF0;						/* clear bits that 1st byte will be written to */
			uiBuffer[entry_offset] |= HI8((uint16)fat_entry);		/* copy entry bits of 1st byte */
		}
		break;
	}
	case FAT_FS_TYPE_FAT16:
	{
		*((uint16*)&uiBuffer[entry_offset]) = (uint16)fat_entry;
		break;
	}
	case FAT_FS_TYPE_FAT32:
	{

		// since a FAT32 entry is actually 28 bits we need
		// to make sure that we don't modify the upper nibble.
		*((uint32*)&uiBuffer[entry_offset]) &= 0xF0000000;
		*((uint32*)&uiBuffer[entry_offset]) |= fat_entry & 0x0FFFFFFF;
		break;
	}
	}

	// write the updated sector to the
	// storage device
	bSuccess = FatWriteFatSector(entry_sector, uiBuffer);
	if (!bSuccess)
	{
		muiFatSharedBufferSector = 0xFFFFFFFF;
		return FAT_CANNOT_WRITE_MEDIA;
	}

	return FAT_SUCCESS;
}


// increase a cluster address by the amount of clusters indicated by count. This function will
// follow the FAT entry chain to fat the count-th cluster allocated to a file relative from the
// current_cluster cluster
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatIncreaseClusterAddress(uint32 uiCluster, uint16 count, uint32* puiNewCluster)
{
	bool	bSuccess;
	uint32	fat_offset = 0;
	uint32	entry_offset;
	uint32	entry_sector;
	uint32	current_sector;
	char	is_odd_cluster = 0;
	char	op_in_progress = 0;

	uint8* uiBuffer = mauiFatSharedBuffer;

	// if the count is zero we just return the same
	// uiCluster that we received
	if (!count)
	{
		*puiNewCluster = (uint32)uiCluster;
		return FAT_SUCCESS;
	}

	// get the offset of the uiCluster entry within the FAT table,
	// the sector of the FAT table that contains the entry and the offset
	// of the fat entry within the sector
	fat_offset = CalculateFatEntryOffset(GetFileSystemType(), uiCluster);
	entry_sector = GetNoOfReservedSectors() + (fat_offset / GetNoOfBytesPerSector());
	entry_offset = fat_offset % GetNoOfBytesPerSector();

	for (;;)
	{
		current_sector = entry_sector;

		// read sector into the buffer
		if (!IsFatSectorLoaded(current_sector))
		{
			bSuccess = Read(current_sector, uiBuffer);
			if (!bSuccess)
			{
				muiFatSharedBufferSector = 0xFFFFFFFF;
				return FAT_SUCCESS;
			}
			muiFatSharedBufferSector = current_sector;
		}

		// free all the fat entries on the current sector
		while (current_sector == entry_sector)
		{
			// make sure we don't try to free an invalid uiCluster
			if (uiCluster < 2)
			{
				return FAT_INVALID_CLUSTER;
			}

			// read the uiCluster entry and mark it as free
			switch (GetFileSystemType())
			{
				case FAT_FS_TYPE_FAT12:
				{
					if (!op_in_progress)
					{
						// remember whether this is an odd uiCluster or not
						is_odd_cluster = (uiCluster & 0x1);

						// set the uiCluster to zero to make sure that the upper bytes are cleared
						// since we're only updating the lower 16 bits.
						uiCluster = 0;

						// read the 1st byte
						((uint8*)&uiCluster)[0] = uiBuffer[entry_offset];
					}

					if (entry_offset == GetNoOfBytesPerSector() - 1)
					{
						// if the entry spans a sector boundary set op_in_progress to 1
						// so that we don't read the 1st byte again when we come back.
						// also increase the sector number and set the entry_offset to 0 since
						// the next byte will be on offset zero when the next sector is loaded
						entry_sector++;
						entry_offset = 0;
						op_in_progress = 1;

						// continue with the next iteration of the loop. We'll come right back
						// here with the next sector loaded
						continue;
					}
					else if (!op_in_progress)
					{
						// increase the offset to point to the next byte
						entry_offset++;
					}

					// read the 2nd byte
					((uint8*)&uiCluster)[1] = uiBuffer[entry_offset];

					// Since a FAT12 entry is only 12 bits (1.5 bytes) we need to adjust the result.
					// For odd uiCluster numbers the FAT entry is stored in the upper 12 bits of the
					// 16 bits where it is stored, so we need to shift the puiNewCluster 4 bits to the right.
					// For even uiCluster numbers the FAT entry is stored in the lower 12 bits of the
					// 16 bits where it is stored, so we need to clear the upper 4 bits.
					if (is_odd_cluster)
					{
						uiCluster >>= 4;
					}
					else
					{
						uiCluster &= 0xFFF;
					}

					// clear op_in_progress flag.
					op_in_progress = 0;
					break;
				}
				case FAT_FS_TYPE_FAT16:
				{
					uiCluster = (uint32) * ((uint16*)&uiBuffer[entry_offset]);
					break;
				}
				case FAT_FS_TYPE_FAT32:
				{
					uiCluster = *((uint32*)&uiBuffer[entry_offset]) & 0x0FFFFFFF;
					break;
				}
			}

			// if the last uiCluster marks the end of the chian we return
			// false as we cannot increase the address by the # of clusters
			// requested by the caller
			if (FatIsEOFEntry(uiCluster))
			{
				return 0;
			}

			// if we've followed the number of clusters requested by
			// the caller set the return puiNewCluster to the current uiCluster
			// and return true
			if (!--count)
			{
				*puiNewCluster = (uint32)uiCluster;
				return FAT_SUCCESS;
			}

			// calculate the location of the next uiCluster in the chain
			fat_offset = CalculateFatEntryOffset(GetFileSystemType(), uiCluster);
			entry_sector = GetNoOfReservedSectors() + (fat_offset / GetNoOfBytesPerSector());
			entry_offset = fat_offset % GetNoOfBytesPerSector();
		}
	}
}


// checks if a fat entry represents the
// last entry of a file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::FatIsEOFEntry(FatEntry fat)
{
	switch (GetFileSystemType())
	{
	case FAT_FS_TYPE_FAT12:
		return fat >= 0x0FF8;

	case FAT_FS_TYPE_FAT16:
		return fat >= 0xFFF8;

	case FAT_FS_TYPE_FAT32:
		return fat >= 0x0FFFFFF8;
	}

	return false;
}


// initializes a directory cluster
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatInitializeDirectoryCluster(SFatRawDirectoryEntry* parent, uint32 cluster, uint8* uiBuffer)
{
	bool						bSuccess;
	uint16						counter;
	uint32						current_sector;
	SFatRawDirectoryEntry* entries;

	// if this is a big endian system or the compiler does not support
	// struct packing we cannot use SFatRawDirectoryEntry to write directly
	// to the uiBuffer
	entries = (SFatRawDirectoryEntry*)uiBuffer;

	muiFatSharedBufferSector = (0xFFFFFFFF);

	// initialize the 1st sector of the directory cluster with
	// the dot entry
	memset(uiBuffer, 0, GetNoOfBytesPerSector());
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

	// write the entry to the uiBuffer or move to the next entry
	// as required by target platform
	entries++;

	// initialize the dot dot entry
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

	// when the parent is the root directory the dotdot entry always points
	// to cluster 0, even in FAT32 when the root directory is not actually on
	// cluster 0 so we need to check if the parent is the root directory and
	// in that case set the 1st cluster to 0
	if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
	{
		uint32 parent_cluster;
		((uint16*)&parent_cluster)[INT32_WORD0] = parent->uEntry.sFatRawCommon.first_cluster_lo;
		((uint16*)&parent_cluster)[INT32_WORD1] = parent->uEntry.sFatRawCommon.first_cluster_hi;

		if (GetRootCluster() == parent_cluster)
		{
			entries->uEntry.sFatRawCommon.first_cluster_lo = 0;
			entries->uEntry.sFatRawCommon.first_cluster_hi = 0;
		}
	}
	/*
	// write the 1st sector of the folder
	*/
	current_sector = CalculateFirstSectorOfCluster(cluster);
	bSuccess = Write(current_sector++, uiBuffer);
	if (!bSuccess)
	{
		return FAT_CANNOT_WRITE_MEDIA;
	}

	// clear the . and .. entries from the uiBuffer and
	// initialize the rest of the sectors of this cluster
	memset(uiBuffer, 0, sizeof(SFatRawDirectoryEntry) * 2);
	counter = GetNoOfSectorsPerCluster() - 1;
	while (counter--)
	{
		bSuccess = Write(current_sector++, uiBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}
	}

	return FAT_SUCCESS;
}


// sets all sectors in a cluster to zeroes
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatZeroCluster(uint32 cluster, uint8* uiBuffer)
{
	bool	bSuccess;
	uint16	counter;
	uint32	current_sector;

	muiFatSharedBufferSector = 0xFFFFFFFF;

	// set all the bytes in the uiBuffer to zero
	memset(uiBuffer, 0, GetNoOfBytesPerSector());

	// calculate the address of the 1st sector
	// of the cluster
	current_sector = CalculateFirstSectorOfCluster(cluster);
	counter = GetNoOfSectorsPerCluster();

	// write the zeroed uiBuffer to every sector in the cluster
	while (counter--)
	{
		bSuccess = Write(current_sector++, uiBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}
	}

	return FAT_SUCCESS;
}



// writes a sector of the FAT table to the active table and (if option is enabled)
// to all other FAT tables
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::FatWriteFatSector(uint32 sector_address, uint8* uiBuffer)
{
	bool bSuccess;

	/*
	// write the sector in the active FAT table
	*/
	bSuccess = Write(sector_address, uiBuffer);
	if (!bSuccess)
	{
		return false;
	}
	/*
	// if we got more than one FAT table update the others as well
	*/
#if defined(FAT_MAINTAIN_TWO_FAT_TABLES)
	if (uiNoOfFatTables > 1)
	{
		int i;
		for (i = 1; i < uiNoOfFatTables; i++)
		{
			bSuccess = device->Write(sector_address + (fat_size * i), uiBuffer);
			if (!bSuccess)
			{
				return false;
			}
		}
	}
#endif
	return true;
}






















// finds the first file in a directory
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatFindFirstEntry(char* parent_path, uint8 attributes, SFatDirectoryEntry** dir_entry, SFatFileSystemQuery* q)
{
	uint16					uiResult;
	SFatDirectoryEntry		parent_entry;
	SFatFileSystemQuery* query = q;

	// make sure the query has a uiBuffer
	if (!q->state.uiBuffer)
	{
		q->state.uiBuffer = q->state.buff;
	}

	// if the path starts with a backlash then advance to
	// the next character
	if (parent_path)
	{
		if (*parent_path == '\\')
		{
			parent_path++;
		}
	}

	// if a parent was specified...
	if (parent_path != NULL)
	{
		// try to get the entry for the parent
		uiResult = FatGetFileEntry(parent_path, &parent_entry);

		// if we were unable to get the parent entry
		// then return the error that we received from
		// FatGetFileEntry
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// try to get the 1st entry of the
		// query results
		uiResult = FatQueryFirstEntry(&parent_entry.raw, attributes, &query->state, 0);
	}
	// if the parent was not supplied then we
	// submit the query without it
	else
	{
		uiResult = FatQueryFirstEntry(0, attributes, &query->state, 0);
	}

	// if we cant get the 1st entry then return the
	// error that we received from fat_Query_First_entry
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if there are no more entries
	if (query->state.current_entry_raw == 0)
	{
		*query->current_entry.name = 0;
		return FAT_SUCCESS;
	}

	// fill the current entry structure with data from
	// the current raw entry of the query
	FatFillDirectoryEntryFromRaw(&query->current_entry, query->state.current_entry_raw);

	// calculate the sector address of the entry - if
	// query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is
	// different
	if (query->state.current_cluster == 0x0)
	{
		query->current_entry.sector_addr = GetNoOfReservedSectors() +
			(GetNoOfFatTables() * GetFatSize()) +
			query->state.current_sector;
	}
	else
	{
		query->current_entry.sector_addr = CalculateFirstSectorOfCluster(query->state.current_cluster) + query->state.current_sector;
	}

	// calculate the offset of the entry within it's sector
	query->current_entry.sector_offset = (uint16)((uintptr_t)query->state.current_entry_raw) - ((uintptr_t)query->state.uiBuffer);
	// store a copy of the original FAT directory entry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	query->current_entry.raw = *query->state.current_entry_raw;

	// if long filenames are enabled copy the filename to the entry
	if (*query->current_entry.name != 0)
	{
		if (*query->state.long_filename != 0)
		{
			for (uiResult = 0; uiResult < 256; uiResult++)
			{
				query->current_entry.name[uiResult] = (uint8)query->state.long_filename[uiResult];
				if (query->state.long_filename[uiResult] == 0)
				{
					break;
				}
			}
		}
	}

	// if the user provided a pointer-to-pointer to the
	// result set it to the current entry.
	if (dir_entry)
	{
		*dir_entry = &query->current_entry;
	}

	// return success code
	return FAT_SUCCESS;
}

// finds the next file in the directory
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatFindNextEntry(SFatDirectoryEntry** dir_entry, SFatFileSystemQuery* q)
{
	uint16					uiResult;
	SFatFileSystemQuery* query = q;

	// try to get the next entry of the query
	uiResult = FatQueryNextEntry(&query->state, 0, 0);

	// if we received an error from FatQueryNextEntry
	// then we return the error code to the calling function
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if there are no more entries
	if (query->state.current_entry_raw == 0)
	{
		// set the filename of the current entry to 0
		*query->current_entry.name = 0;
		return FAT_SUCCESS;
	}

	// fill the current entry structure with data from
	// the current raw entry of the query
	FatFillDirectoryEntryFromRaw(&query->current_entry, query->state.current_entry_raw);

	// calculate the sector address of the entry - if
	// query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is
	// different
	if (query->state.current_cluster == 0x0)
	{
		query->current_entry.sector_addr = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize()) + query->state.current_sector;
	}
	else
	{
		query->current_entry.sector_addr = CalculateFirstSectorOfCluster(query->state.current_cluster) + query->state.current_sector;
	}

	// calculate the offset of the entry within it's sector
	query->current_entry.sector_offset = (uint16)((uintptr_t)query->state.current_entry_raw) - ((uintptr_t)query->state.uiBuffer);

	// store a copy of the original FAT directory entry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	query->current_entry.raw = *query->state.current_entry_raw;

	if (*query->current_entry.name != 0)
	{
		if (*query->state.long_filename != 0)
		{
			for (uiResult = 0; uiResult < 256; uiResult++)
			{
				query->current_entry.name[uiResult] = (uint8)query->state.long_filename[uiResult];
				if (query->state.long_filename[uiResult] == 0)
					break;
			}
		}
	}

	if (dir_entry)
	{
		*dir_entry = &query->current_entry;
	}

	return FAT_SUCCESS;
}

// fills a directory entry structure with data taken
// from a raw directory entry
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::FatFillDirectoryEntryFromRaw(SFatDirectoryEntry* entry, SFatRawDirectoryEntry* raw_entry) 
{

	// copy the filename and transform the filename
	// from the internal structure to the public one
	FatGetShortNameFromEntry(entry->name, raw_entry->uEntry.sFatRawCommon.name);

	// copy other data from the internal entry structure
	// to the public one
	entry->attributes = raw_entry->uEntry.sFatRawCommon.attributes;
	entry->size = raw_entry->uEntry.sFatRawCommon.size;
	entry->create_time = FatDecodeDateTime(raw_entry->uEntry.sFatRawCommon.create_date, raw_entry->uEntry.sFatRawCommon.create_time);
	entry->modify_time = FatDecodeDateTime(raw_entry->uEntry.sFatRawCommon.modify_date, raw_entry->uEntry.sFatRawCommon.modify_time);
	entry->access_time = FatDecodeDateTime(raw_entry->uEntry.sFatRawCommon.access_date, 0);
	entry->raw = *raw_entry;
}


// creates a directory
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatCreateDirectory(char* directory)
{
	uint16				uiResult;
	SFatDirectoryEntry	entry;

	// check that we got a valid pathname
	if (!directory || strlen(directory) > FAT_MAX_PATH)
	{
		return FAT_INVALID_FILENAME;
	}

	// try get the file entry
	uiResult = FatGetFileEntry(directory, &entry);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if we don'tfind a file or directory by that name
	// we can create it, otherwise return file already exists error
	if (*entry.name == 0)
	{
		// allocate memory for the file path
		size_t				path_len;
		char*				path_scanner;
		char				file_path[FAT_MAX_PATH + 1];
		SFatDirectoryEntry	parent_entry;

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

		// try to get the entry for the parent directory
		uiResult = FatGetFileEntry(file_path, &parent_entry);

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
		return FatCreateDirectoryEntry(&parent_entry.raw, path_scanner, FAT_ATTR_DIRECTORY, 0, &entry);
	}

	// if we get here it means that a file or
	// directory with that name already exists.
	return FAT_FILENAME_ALREADY_EXISTS;
}


// gets a SFatDirectoryEntry by it's full path
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatGetFileEntry(char* path, SFatDirectoryEntry* entry)
{
	uint16					uiResult;
	char					match;
	uint8					target_file[13];
	uint8*					pLevel;
	SFatRawDirectoryEntry*	current_entry;
	SFatQueryStateInternal	query;
	char					using_lfn;

	uint16					target_file_long[FAT_MAX_PATH + 1];	/* stores the utf16 long filename */
	uint8					current_level[FAT_MAX_PATH + 1];

	query.uiBuffer = mauiFatSharedBuffer;

	// if the path starts with a backlash then advance to
	// the next character
	if (*path == '\\')
	{
		path++;
	}

	if (*path != 0)
	{
		// set current_entry to 0, in this state
		// it represents the root directory of the
		// volume
		current_entry = 0;
	}
	// if the caller did not supply a path then the
	// request is for the root directory, since there's
	// no physical entry for the root directory we must
	// create a fake one
	else
	{
		// copy the file name to the entry and the raw
		// entry in their respective formats
		strcpy((char*)entry->name, "ROOT");
		GetShortNameForEntry(entry->raw.uEntry.sFatRawCommon.name, entry->name, 1);

		// set the general fields of the entry
		entry->attributes = entry->raw.uEntry.sFatRawCommon.attributes = FAT_ATTR_DIRECTORY;
		entry->size = entry->raw.uEntry.sFatRawCommon.size = 0x0;

		// since the entry does not physically exist the
		// address fields are set to zero as well
		entry->sector_addr = 0x0;
		entry->sector_offset = 0x0;

		// set the location of the root directory
		if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
		{
			// if this is a FAT32 volume then the root
			// directory is located on the data section just like
			// any other directory
			entry->raw.uEntry.sFatRawCommon.first_cluster_lo = LO16(GetRootCluster());
			entry->raw.uEntry.sFatRawCommon.first_cluster_hi = HI16(GetRootCluster());
		}
		else
		{
			// if the volume is FAT12/16 we set the cluster
			// address to zero and when time comes to get the
			// directory we'll calculate the address right after
			// the end of the FATs
			entry->raw.uEntry.sFatRawCommon.first_cluster_lo = 0x0;
			entry->raw.uEntry.sFatRawCommon.first_cluster_hi = 0x0;
		}

		return FAT_SUCCESS;
	}

	// mark the cached sector as unknown
	muiFatSharedBufferSector = (FAT_UNKNOWN_SECTOR);

	// for each level on the path....
	do
	{
		// set the pointer current level name uiBuffer
		pLevel = current_level;

		// if the path starts with a backlash then advance to
		// the next character
		if (*path == '\\')
		{
			path++;
		}

		// copy the name of the current level entry
		uiResult = 0;
		while (*path != 0x0 && *path != '\\')
		{
			if (uiResult++ > FAT_MAX_PATH)
			{
				return FAT_INVALID_FILENAME;
			}
			*pLevel++ = *path++;
		}
		*pLevel = 0x0;

		// try to find the first entry
		uiResult = FatQueryFirstEntry(current_entry, 0, (SFatQueryState*)&query, 1);

		// if we could not find the entry then
		// return an error code
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// if the output of FatQueryFirstEntry indicates that
		// there are no entries available...
		if (*query.current_entry_raw->uEntry.sFatRawCommon.name == 0x0)
		{
			// set the name of the entry to 0
			*entry->name = 0;
			return FAT_SUCCESS;
		}

		// get an LFN version of the filename
		using_lfn = 0;

		// format the current level filename to the 8.3 format
		// if this is an invalid 8.3 filename try to get the LFN
		// once we get a valid filename (either short or LFN) compare
		// it to the one on the current query entry
		if (GetShortNameForEntry(target_file, current_level, 1) == FAT_INVALID_FILENAME)
		{
			if (GetLongNameForEntry(target_file_long, current_level) == FAT_INVALID_FILENAME)
			{
				return FAT_INVALID_FILENAME;
			}
			using_lfn = 1;
			match = FatCompareLongName(target_file_long, query.long_filename)
				|| FatCompareShortName(target_file, query.current_entry_raw->uEntry.sFatRawCommon.name);
		}
		else
		{
			match = FatCompareShortName(target_file, query.current_entry_raw->uEntry.sFatRawCommon.name);
		}

		// if the file doesn't match then get the
		// next file
		while (!match)
		{
			//  try to get the next file
			uiResult = FatQueryNextEntry((SFatQueryState*)&query, 1, 0);

			// if we received an error message then return
			// it to the calling function
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}

			// if the output of FatQueryFirstEntry indicates that
			// there are no entries available then set the entry name to 0
			// and return success
			if (IS_LAST_DIRECTORY_ENTRY(query.current_entry_raw))
			{
				*entry->name = 0;
				return FAT_SUCCESS;
			}

			// match the filename against the next entry
			if (using_lfn)
			{
				match = FatCompareLongName(target_file_long, query.long_filename) || FatCompareShortName(target_file, query.current_entry_raw->uEntry.sFatRawCommon.name);
			}
			else
			{
				match = FatCompareShortName(target_file, query.current_entry_raw->uEntry.sFatRawCommon.name);
			}
		}

		// set the current entry to the entry
		// that we've just found
		current_entry = query.current_entry_raw;
	} while (*path != 0x0);

	// copy the filename and transform the filename
	// from the internal structure to the public one
	FatGetShortNameFromEntry(entry->name, query.current_entry_raw->uEntry.sFatRawCommon.name);

	// copy other data from the internal entry structure
	// to the public one
	entry->attributes = query.current_entry_raw->uEntry.sFatRawCommon.attributes;
	entry->size = query.current_entry_raw->uEntry.sFatRawCommon.size;
	entry->create_time = FatDecodeDateTime(query.current_entry_raw->uEntry.sFatRawCommon.create_date, query.current_entry_raw->uEntry.sFatRawCommon.create_time);
	entry->modify_time = FatDecodeDateTime(query.current_entry_raw->uEntry.sFatRawCommon.modify_date, query.current_entry_raw->uEntry.sFatRawCommon.modify_time);
	entry->access_time = FatDecodeDateTime(query.current_entry_raw->uEntry.sFatRawCommon.access_date, 0);

	// calculate the sector address of the entry - if
	// query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is
	// different
	if (query.current_cluster == 0x0)
	{
		entry->sector_addr = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize()) + query.current_sector;
	}
	else
	{
		entry->sector_addr = CalculateFirstSectorOfCluster(query.current_cluster) + query.current_sector;
	}

	// calculate the offset of the entry within it's sector
	entry->sector_offset = (uint16)((uintptr_t)query.current_entry_raw) - ((uintptr_t)query.uiBuffer);

	// store a copy of the original FAT directory entry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	entry->raw = *query.current_entry_raw;
	return FAT_SUCCESS;
}


// initializes a query of a set of directory
// entries
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatQueryFirstEntry(SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, char buffer_locked)
{
	bool	bSuccess;
	uint32	first_sector;

	// make sure the long filename is set to an empty string
	query->long_filename[0] = 0;

	// if the directory entry has the cluster # set to
	// zero it is the root directory so we need to
	// get the cluster # accordingly
	if (directory)
	{
		if (directory->uEntry.sFatRawCommon.first_cluster_hi == 0 && directory->uEntry.sFatRawCommon.first_cluster_lo == 0)
		{
			directory = 0;
		}
	}

	// if no directory entry was provided
	// we'll use the root entry of the volume
	if (directory == 0)
	{
		// calculate the cluster # from the
		if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
		{
			query->current_cluster = GetRootCluster();
			first_sector = CalculateFirstSectorOfCluster(query->current_cluster);
		}
		else
		{
			query->current_cluster = 0x0;
			first_sector = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize());
		}
	}
	// if a directory entry was provided
	else
	{
		// if the entry provided is not a directory
		// entry return an error code
		if (!(directory->uEntry.sFatRawCommon.attributes & FAT_ATTR_DIRECTORY))
		{
			return FAT_NOT_A_DIRECTORY;
		}

		// set the CurrentCluster field of the query
		// state structure to the values found on the
		// directory entry structure
		((uint16*)&query->current_cluster)[INT32_WORD0] = directory->uEntry.sFatRawCommon.first_cluster_lo;

		// read the upper word of the cluster address
		// only if this is a FAT32 volume
		if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
		{
			((uint8*)&query->current_cluster)[INT32_BYTE2] = LO8(directory->uEntry.sFatRawCommon.first_cluster_hi);
			((uint8*)&query->current_cluster)[INT32_BYTE3] = HI8(directory->uEntry.sFatRawCommon.first_cluster_hi);
		}
		else
		{
			((uint16*)&query->current_cluster)[INT32_WORD1] = 0;
		}

		// get the 1st sector of the directory entry
		first_sector = CalculateFirstSectorOfCluster(query->current_cluster);
	}

	// read the sector into the query
	// state uiBuffer
	bSuccess = Read(first_sector, query->uiBuffer);
	if (!bSuccess)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	// set the 1st and current entry pointers
	// on the query state to the 1st entry of the
	// directory
	query->Attributes = attributes;
	query->current_sector = 0;

	query->first_entry_raw = (SFatRawDirectoryEntry*)query->uiBuffer;
	query->current_entry_raw = (SFatRawDirectoryEntry*)query->uiBuffer;

	// find the 1st entry and return it's result code
	return FatQueryNextEntry(query, buffer_locked, 1);
}


// moves a query to the next entry
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatQueryNextEntry(SFatQueryState* query, char buffer_locked, char first_entry)
{
	char	pass;
	bool	bSuccess;
	uint32	sector_address;

	do
	{
		// if the current entry is the last entry of
		// the sector...
		if (!first_entry)
		{
			if (((uintptr_t)query->current_entry_raw - (uintptr_t)query->first_entry_raw) == GetNoOfBytesPerSector() - 0x20)
			{
				// if the current sector is the last of the current cluster then we must find the next
				// cluster... if CurrentCluster == 0 then this is the root directory of a FAT16/FAT12 volume, that
				// volume has a fixed size in sectors and is not allocated as a cluster chain so we don't do this
				if (query->current_cluster > 0 &&/*query->current_sector > 0x0 &&*/ query->current_sector == GetNoOfSectorsPerCluster() - 1)
				{
					FatEntry fat;

					// get the fat structure for the current cluster
					// and return UNKNOWN_ERROR if the operation fails
					if (FatGetClusterEntry(query->current_cluster, &fat) != FAT_SUCCESS)
					{
						return FAT_UNKNOWN_ERROR;
					}

					// if this is the last cluster of the directory...
					if (FatIsEOFEntry(fat))
					{
						// set the current entry to 0
						*query->current_entry_raw->uEntry.sFatRawCommon.name = 0;
						return FAT_SUCCESS;
					}

					// set the current cluster to the next
					// cluster of the directory entry
					query->current_cluster = fat;

					// reset the current sector
					query->current_sector = 0x0;

					// calculate the address of the next sector
					sector_address = CalculateFirstSectorOfCluster(query->current_cluster) + query->current_sector;
				}
				// if there are more sectors on the current cluster then
				else
				{
					query->current_sector++;

					// if this is the root directory of a FAT16/FAT12
					// volume and we have passed it's last sector then
					// there's no more entries...
					if (query->current_cluster == 0x0)
					{
						if (query->current_sector == GetRootDirectorySectors())
						{
							*query->current_entry_raw->uEntry.sFatRawCommon.name = 0;
							return FAT_SUCCESS;
						}
						sector_address =
							(GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize())) + query->current_sector;
					}
					else
					{
						// calculate the address of the next sector
						sector_address = CalculateFirstSectorOfCluster(query->current_cluster) + query->current_sector;
					}
				}

				// read the next sector into the query uiBuffer
				bSuccess = Read(sector_address, query->uiBuffer);
				if (!bSuccess)
				{
					return FAT_CANNOT_READ_MEDIA;
				}

				// set the 1st and current entry pointers
				// on the query state to the 1st entry of the
				// directory
				query->first_entry_raw = (SFatRawDirectoryEntry*)query->uiBuffer;
				query->current_entry_raw = (SFatRawDirectoryEntry*)query->uiBuffer;
			}
			// if there are more entries on the current sector...
			else
			{
				// simply increase the current entry pointer
				query->current_entry_raw++;
			}
		}
		else
		{
			first_entry = 0;
		}

		// if this is a long filename entry...
		if (query->current_entry_raw->uEntry.sFatRawCommon.attributes == FAT_ATTR_LONG_NAME && !IS_FREE_DIRECTORY_ENTRY(query->current_entry_raw))
		{
			// if this enntry is marked as the 1st LFN entry
			if (query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_sequence & FAT_FIRST_LFN_ENTRY)
			{
				query->lfn_sequence = (query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_sequence ^ FAT_FIRST_LFN_ENTRY) + 1;
				query->lfn_checksum = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_checksum;

				// insert null terminator at the end of the long filename
				((uint8*)&query->long_filename[((query->lfn_sequence - 2) * 13) + 0xD])[INT16_BYTE0] = 0;
				((uint8*)&query->long_filename[((query->lfn_sequence - 2) * 13) + 0xD])[INT16_BYTE1] = 0;
			}

			// if this is the LFN that we're expecting then
			// process it, otherwise we'll have to wait for
			// another 1st LFN entry otherwise read the LFN
			// chrs and save them on the query state struct
			if (query->lfn_checksum == query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_checksum &&
				(query->lfn_sequence == (query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_sequence & (0xFF ^ FAT_FIRST_LFN_ENTRY)) + 1))
			{
				query->lfn_sequence = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_sequence & (0xFF ^ FAT_FIRST_LFN_ENTRY);
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x0])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[0];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x0])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[1];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x1])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[2];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x1])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[3];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x2])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[4];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x2])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[5];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x3])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[6];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x3])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[7];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x4])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[8];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x4])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_1[9];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x5])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[0];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x5])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[1];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x6])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[2];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x6])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[3];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x7])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[4];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x7])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[5];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x8])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[6];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x8])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[7];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x9])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[8];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0x9])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[9];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xA])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[10];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xA])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_2[11];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xB])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_3[0];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xB])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_3[1];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xC])[INT16_BYTE0] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_3[2];
				((uint8*)&query->long_filename[((query->lfn_sequence - 1) * 13) + 0xC])[INT16_BYTE1] = query->current_entry_raw->uEntry.sFatRawLongFileName.lfn_chars_3[3];
			}
			else
			{
				query->lfn_checksum = 0;
			}

			// make sure we never return this entry
			pass = (query->Attributes == FAT_ATTR_LONG_NAME);
		}
		else
		{
			// check that the current entry passes the query
			// attributes check
			pass =
				(!(query->current_entry_raw->uEntry.sFatRawCommon.attributes & FAT_ATTR_HIDDEN) || (query->Attributes & FAT_ATTR_HIDDEN)) &&
				(!(query->current_entry_raw->uEntry.sFatRawCommon.attributes & FAT_ATTR_SYSTEM) || (query->Attributes & FAT_ATTR_SYSTEM)) &&
				(!(query->current_entry_raw->uEntry.sFatRawCommon.attributes & FAT_ATTR_VOLUME_ID) || (query->Attributes & FAT_ATTR_VOLUME_ID)) &&
				(!(query->current_entry_raw->uEntry.sFatRawCommon.attributes & FAT_ATTR_LONG_NAME) || (query->Attributes & FAT_ATTR_LONG_NAME));
		}
	}

	// repeat the process until we find a valid entry
	// that matches the attributes given
	while (!pass || *query->current_entry_raw->uEntry.sFatRawCommon.name == 0xE5);

	// if we found an entry we need to check it's LFN checksum
	// to make sure that the long filename that we've associated
	// with it belongs to it. If it doesn't clear it.
	if (*query->current_entry_raw->uEntry.sFatRawCommon.name != 0x0)
	{
		if (query->lfn_checksum != FatLongEntryChecksum((uint8*)query->current_entry_raw->uEntry.sFatRawCommon.name))
		{
			query->long_filename[0] = 0x0;
		}
	}

	// if this entry doesn't have an LFN entry but its marked as having
	// a lowercase name or extension then fill the long filename with the
	// lowercase version
	if (query->long_filename[0] == 0 && (query->current_entry_raw->uEntry.sFatRawCommon.reserved & (FAT_LOWERCASE_EXTENSION | FAT_LOWERCASE_BASENAME)))
	{
		int i = 0;
		for (uint16 uiResult = 0; uiResult < 8; uiResult++)
		{
			if (query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult] != 0x20)
			{
				if (query->current_entry_raw->uEntry.sFatRawCommon.reserved & FAT_LOWERCASE_BASENAME)
				{
					query->long_filename[i++] = tolower(query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult]);
				}
				else
				{
					query->long_filename[i++] = query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult];
				}
			}
		}

		if (query->current_entry_raw->uEntry.sFatRawCommon.name[8] != 0x20)
		{
			query->long_filename[i++] = '.';

			for (uint16 uiResult = 8; uiResult < 11; uiResult++)
			{
				if (query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult] != 0x20)
				{
					if (query->current_entry_raw->uEntry.sFatRawCommon.reserved & FAT_LOWERCASE_EXTENSION)
					{
						query->long_filename[i++] = tolower(query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult]);
					}
					else
					{
						query->long_filename[i++] = query->current_entry_raw->uEntry.sFatRawCommon.name[uiResult];
					}
				}
			}
		}
		query->long_filename[i] = 0x0;
	}

	return FAT_SUCCESS;
}

// creates a FAT directory entry
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatCreateDirectoryEntry(SFatRawDirectoryEntry* parent, char* name, uint8 attribs, uint32 entry_cluster, SFatDirectoryEntry* new_entry)
{
	uint16							uiResult;
	int16							char_index;
	uint16							illegal_char;
	uint16							entries_count = 0;
	uint32							sector;
	uint32							first_sector_of_cluster = 0;
	uintptr_t						last_entry_address;
	FatEntry						fat;
	FatEntry						last_fat;
	SFatRawDirectoryEntry*			parent_entry;

	int								no_of_lfn_entries_needed = 0;
	int								no_of_lfn_entries_found;
	char							lfn_checksum;

	uint8* uiBuffer = mauiFatSharedBuffer;

	// get the length of the filename
	uiResult = (uint16)strlen(name);

	// check that the character is a valid 8.3 filename, the
	// file is invalid if:
	//
	//	- name part is more than 8 chars (char_index > 8)
	//	- extension part is more than 3 (uiResult - char_index > 4)
	//	- it has more than one dot (FatIndexOf('.', name, char_index + 1) >= 0)
	if (uiResult > 255)
	{
		return FAT_FILENAME_TOO_LONG;
	}

	// all names are also invalid if they start or end with
	// a dot
	char_index = FatIndexOf('.', name, 0x0);

	if (char_index == 0 || char_index == (uiResult - 1))
	{
		return FAT_INVALID_FILENAME;
	}

	for (char_index = 0x0; char_index < uiResult; char_index++)
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

	// initialize the raw entry
	// todo: check if no other functions are initializing
	// new_entry and initialize the whole thing
	memset(&new_entry->raw, 0, sizeof(new_entry->raw));

	// attempt to format the filename provided
	// to the format required by the directory entry
	// and copy it to it's field
	uiResult = GetShortNameForEntry(new_entry->raw.uEntry.sFatRawCommon.name, (uint8*)name, 0);

	// if the above operation failed then the filename
	// is invalid
	if (uiResult != FAT_SUCCESS && uiResult != FAT_LFN_GENERATED)
	{
		return FAT_INVALID_FILENAME;
	}

	// if this is going to be an lfn entry we need to make
	// sure that the short filename is available
	if (uiResult == FAT_LFN_GENERATED)
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
			query.uiBuffer = query.buff;
			uiResult = FatQueryFirstEntry(parent, 0, &query, 0);
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}

			sprintf(name_suffix_str, "~%i", name_suffix);

			for (i = 0; i < 8 - (char)strlen(name_suffix_str); i++)
			{
				if (new_entry->raw.uEntry.sFatRawCommon.name[i] == 0x20)
				{
					break;
				}
			}

			for (c = 0; c < (char)strlen(name_suffix_str); c++)
			{
				new_entry->raw.uEntry.sFatRawCommon.name[i++] = name_suffix_str[c];
			}

			// loop through all entries in the parent directory
			// and if we find one with the same name as hours mark the name
			// as invalid
			while (*query.current_entry_raw->uEntry.sFatRawCommon.name != 0)
			{
				if (memcmp(query.current_entry_raw->uEntry.sFatRawCommon.name, new_entry->raw.uEntry.sFatRawCommon.name, 11) == 0)
				{
					is_valid_entry = 0;
					break;
				}
				uiResult = FatQueryNextEntry(&query, 0, 0);
				if (uiResult != FAT_SUCCESS)
				{
					return uiResult;
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
					if (new_entry->raw.uEntry.sFatRawCommon.name[i] == 0x20)
					{
						break;
					}
				}

				for (c = 0; c < (char)strlen(name_suffix_str); c++)
				{
					new_entry->raw.uEntry.sFatRawCommon.name[i++] = name_suffix_str[c];
				}
			}
		} while (!is_valid_entry);

		// calculate the # of entries needed to store the lfn
		// including the actual entry
		no_of_lfn_entries_needed = ((strlen(name) + 12) / 13) + 1;
		no_of_lfn_entries_found = 0;
	}

	// if the new entry is a directory and no cluster was supplied
	// by the calling function then allocate a new cluster
	if (entry_cluster == 0 && (attribs & FAT_ATTR_DIRECTORY))
	{
		entry_cluster = FatAllocateDirectoryCluster(parent, &uiResult);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}
	}

	// set the entry attributes
	strcpy((char*)new_entry->name, name);
	new_entry->attributes = attribs;
	new_entry->size = 0x0;
	new_entry->raw.uEntry.sFatRawCommon.attributes = attribs;
	new_entry->raw.uEntry.sFatRawCommon.reserved = 0;
	new_entry->raw.uEntry.sFatRawCommon.size = 0x0;
	new_entry->raw.uEntry.sFatRawCommon.first_cluster_lo = LO16(entry_cluster);
	new_entry->raw.uEntry.sFatRawCommon.first_cluster_hi = HI16(entry_cluster);
	new_entry->raw.uEntry.sFatRawCommon.create_time_tenth = 0x0;
	new_entry->raw.uEntry.sFatRawCommon.create_date = GetSystemClockDate();
	new_entry->raw.uEntry.sFatRawCommon.create_time = GetSystemClockTime();
	new_entry->raw.uEntry.sFatRawCommon.modify_date = new_entry->raw.uEntry.sFatRawCommon.create_date;
	new_entry->raw.uEntry.sFatRawCommon.modify_time = new_entry->raw.uEntry.sFatRawCommon.create_time;
	new_entry->raw.uEntry.sFatRawCommon.access_date = new_entry->raw.uEntry.sFatRawCommon.create_date;
	new_entry->create_time = FatDecodeDateTime(new_entry->raw.uEntry.sFatRawCommon.create_date, new_entry->raw.uEntry.sFatRawCommon.create_time);
	new_entry->modify_time = FatDecodeDateTime(new_entry->raw.uEntry.sFatRawCommon.modify_date, new_entry->raw.uEntry.sFatRawCommon.modify_time);
	new_entry->access_time = FatDecodeDateTime(new_entry->raw.uEntry.sFatRawCommon.access_date, 0);

	// there's no fat entry that points to the 1st cluster of
	// a directory's cluster chain but we'll create a
	// fake fat entry from the 1st cluster data on the
	// directory entry so that we can handle the 1st
	// cluster with the same code as all other clusters
	// in the chain
	if (parent && (parent->uEntry.sFatRawCommon.first_cluster_lo != 0x0 || parent->uEntry.sFatRawCommon.first_cluster_hi != 0x0))
	{
		// read the low word of the cluster address
		// and read the high word of the 1st cluster address
		// ONLY if the file system type is FAT32
		((uint16*)&fat)[INT32_WORD0] = parent->uEntry.sFatRawCommon.first_cluster_lo;
		((uint16*)&fat)[INT32_WORD1] = (GetFileSystemType() == FAT_FS_TYPE_FAT32) ? parent->uEntry.sFatRawCommon.first_cluster_hi : 0x0;
	}
	// if no parent was specified then we create
	// the fake fat entry from the root directory's
	// cluster address found on the volume structure
	else
	{
		if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
		{
			fat = GetRootCluster();
		}
		else
		{
			fat = last_fat = 0x0;
			first_sector_of_cluster =
				GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize());
		}
	}

	// mark the cached sector as unknown
	muiFatSharedBufferSector = (FAT_UNKNOWN_SECTOR);

	// for each cluster allocated to the parent
	// directory entry
	do
	{
		// calculate the address of the 1st sector
		// of the cluster - skip this step if uiResult equals
		// 1, this means that this is the 1st sector of the
		// root entry which doesn't start at the beggining
		// of the cluster
		if (fat != 0x0)
		{
			first_sector_of_cluster = CalculateFirstSectorOfCluster(fat);
		}

		// set the current sector to the first
		// sector of the cluster
		sector = first_sector_of_cluster;

		// calculate the address of the last directory
		// entry on a sector when the sector is loaded
		// into sec_buff
		last_entry_address = ((uintptr_t)uiBuffer + GetNoOfBytesPerSector()) - 0x20;

		// for each sector in the cluster
		while (fat == 0 || sector < (first_sector_of_cluster + GetNoOfSectorsPerCluster()))
		{
			// read the current sector to RAM
			bool bSuccess;

			bSuccess = Read(sector, uiBuffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_READ_MEDIA;
			}

			// set the parent entry pointer to the 1st
			// entry of the current sector
			parent_entry = (SFatRawDirectoryEntry*)uiBuffer;


			// for each directory entry in the sector...
			while ((uintptr_t)parent_entry <= last_entry_address)
			{
				// make sure we don't exceed the limit of 0xFFFF entries
				// per directory
				if ((entries_count + no_of_lfn_entries_needed) == 0xFFFF)
				{
					return FAT_DIRECTORY_LIMIT_EXCEEDED;
				}

				// increase the count of directory entries
				entries_count++;

				// if the directory entry is free
				if (IS_FREE_DIRECTORY_ENTRY(parent_entry))
				{
					// we've found a free entry
					no_of_lfn_entries_found++;

					// if this is the last directory entry or if we've
					// found all the entries that we need then let's get
					// ready to write them
					if (IS_LAST_DIRECTORY_ENTRY(parent_entry) || no_of_lfn_entries_found == no_of_lfn_entries_needed)
					{
						// if there where any free entries before this
						// one then we need to rewind a bit
						while (no_of_lfn_entries_found-- > 1)
						{
							if ((uintptr_t)parent_entry > (uintptr_t)uiBuffer)
							{
								parent_entry--;
							}
							else
							{
								// if the last entry is on the same cluster we
								// can just decrease the sector number, otherwise we
								// need to get the sector address for the last cluster
								if (sector > first_sector_of_cluster)
								{
									sector--;
								}
								else
								{
									if (last_fat == 0)
									{
										first_sector_of_cluster =
											GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize());
									}
									else
									{
										fat = last_fat;
										first_sector_of_cluster = CalculateFirstSectorOfCluster(fat);
									}
									sector = first_sector_of_cluster + GetNoOfSectorsPerCluster();
								}

								// read the last sector to the cache, calculate the last
								// entry address and set our pointer to it
								bSuccess = Read(sector, uiBuffer);
								if (!bSuccess)
								{
									return FAT_CANNOT_READ_MEDIA;
								}
								last_entry_address = ((uintptr_t)uiBuffer + GetNoOfBytesPerSector()) - 0x20;
								parent_entry = (SFatRawDirectoryEntry*)last_entry_address;
							}
						}

						// compute the checksum for this entry
						lfn_checksum = FatLongEntryChecksum((uint8*)new_entry->raw.uEntry.sFatRawCommon.name);

						// now we can start writting
						no_of_lfn_entries_found = no_of_lfn_entries_needed;
						while (no_of_lfn_entries_found--)
						{
							if (no_of_lfn_entries_found)
							{
								uint16 i, c;

								// set the required fields for this entry
								parent_entry->uEntry.sFatRawLongFileName.lfn_sequence = (uint8)no_of_lfn_entries_found;
								parent_entry->uEntry.sFatRawLongFileName.lfn_checksum = lfn_checksum;
								parent_entry->uEntry.sFatRawCommon.attributes = FAT_ATTR_LONG_NAME;
								parent_entry->uEntry.sFatRawLongFileName.lfn_first_cluster = 0;
								parent_entry->uEntry.sFatRawLongFileName.lfn_type = 0;

								// mark entry as the 1st entry if it is so
								if (no_of_lfn_entries_found == no_of_lfn_entries_needed - 1)
								{
									parent_entry->uEntry.sFatRawLongFileName.lfn_sequence = parent_entry->uEntry.sFatRawLongFileName.lfn_sequence | FAT_FIRST_LFN_ENTRY;
								}

								// copy the lfn chars
								c = (uint16)strlen(name);
								i = ((no_of_lfn_entries_found - 1) * 13);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x0] = LO8((i + 0x0 > c) ? 0xFFFF : (uint16)name[i + 0x0]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x1] = HI8((i + 0x0 > c) ? 0xFFFF : (uint16)name[i + 0x0]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x2] = LO8((i + 0x1 > c) ? 0xFFFF : (uint16)name[i + 0x1]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x3] = HI8((i + 0x1 > c) ? 0xFFFF : (uint16)name[i + 0x1]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x4] = LO8((i + 0x2 > c) ? 0xFFFF : (uint16)name[i + 0x2]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x5] = HI8((i + 0x2 > c) ? 0xFFFF : (uint16)name[i + 0x2]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x6] = LO8((i + 0x3 > c) ? 0xFFFF : (uint16)name[i + 0x3]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x7] = HI8((i + 0x3 > c) ? 0xFFFF : (uint16)name[i + 0x3]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x8] = LO8((i + 0x4 > c) ? 0xFFFF : (uint16)name[i + 0x4]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_1[0x9] = HI8((i + 0x4 > c) ? 0xFFFF : (uint16)name[i + 0x4]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x0] = LO8((i + 0x5 > c) ? 0xFFFF : (uint16)name[i + 0x5]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x1] = HI8((i + 0x5 > c) ? 0xFFFF : (uint16)name[i + 0x5]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x2] = LO8((i + 0x6 > c) ? 0xFFFF : (uint16)name[i + 0x6]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x3] = HI8((i + 0x6 > c) ? 0xFFFF : (uint16)name[i + 0x6]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x4] = LO8((i + 0x7 > c) ? 0xFFFF : (uint16)name[i + 0x7]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x5] = HI8((i + 0x7 > c) ? 0xFFFF : (uint16)name[i + 0x7]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x6] = LO8((i + 0x8 > c) ? 0xFFFF : (uint16)name[i + 0x8]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x7] = HI8((i + 0x8 > c) ? 0xFFFF : (uint16)name[i + 0x8]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x8] = LO8((i + 0x9 > c) ? 0xFFFF : (uint16)name[i + 0x9]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0x9] = HI8((i + 0x9 > c) ? 0xFFFF : (uint16)name[i + 0x9]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0xA] = LO8((i + 0xA > c) ? 0xFFFF : (uint16)name[i + 0xA]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_2[0xB] = HI8((i + 0xA > c) ? 0xFFFF : (uint16)name[i + 0xA]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_3[0x0] = LO8((i + 0xB > c) ? 0xFFFF : (uint16)name[i + 0xB]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_3[0x1] = HI8((i + 0xB > c) ? 0xFFFF : (uint16)name[i + 0xB]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_3[0x2] = LO8((i + 0xC > c) ? 0xFFFF : (uint16)name[i + 0xC]);
								parent_entry->uEntry.sFatRawLongFileName.lfn_chars_3[0x3] = HI8((i + 0xC > c) ? 0xFFFF : (uint16)name[i + 0xC]);

								// continue to next entry
								if ((uintptr_t)parent_entry < (uintptr_t)last_entry_address)
								{
									parent_entry++;
								}
								else
								{
									// flush this sector to the storage device and
									// load the next sector
									bSuccess = Write(sector, uiBuffer);
									if (!bSuccess)
									{
										return FAT_CANNOT_WRITE_MEDIA;
									}

									if (fat == 0 || sector < first_sector_of_cluster + GetNoOfSectorsPerCluster() - 1)
									{
										sector++;
										// make sure that we don't overflow the root directory
										// on FAT12/16 volumes
										if (!fat)
										{
											if (sector > first_sector_of_cluster + GetRootDirectorySectors())
											{
												return FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED;
											}
										}
									}
									else
									{
										// get the next cluster, we'll remember the last one so
										// we can update it bellow if it's the eof cluster
										last_fat = fat;
										uiResult = FatGetClusterEntry(fat, &fat);
										if (uiResult != FAT_SUCCESS)
										{
											return uiResult;
										}

										// if this is the end of the FAT chain allocate
										// a new cluster to this folder and continue
										if (FatIsEOFEntry(fat))
										{
											FatEntry newfat = FatAllocateDataCluster(1, 1, &uiResult);
											if (uiResult != FAT_SUCCESS)
											{
												return uiResult;
											}

											uiResult = FatSetClusterEntry(last_fat, newfat);
											if (uiResult != FAT_SUCCESS)
											{
												return uiResult;
											}
											fat = newfat;
										}
										// mark the loaded sector as unknown
										muiFatSharedBufferSector = (FAT_UNKNOWN_SECTOR);

										// continue working on the new cluster
										sector = CalculateFirstSectorOfCluster(fat);
									}

									// load the next sector
									bSuccess = Read(sector, uiBuffer);
									if (!bSuccess)
									{
										return FAT_CANNOT_READ_MEDIA;
									}
									parent_entry = (SFatRawDirectoryEntry*)uiBuffer;
								}
							}
							else
							{
								*parent_entry = new_entry->raw;
							}
						}

						// flush this sector to the storage device and
						// load the next sector
						bSuccess = Write(sector, uiBuffer);
						if (!bSuccess)
						{
							return FAT_CANNOT_WRITE_MEDIA;
						}
						new_entry->sector_addr = sector;
						new_entry->sector_offset = (uintptr_t)parent_entry - (uintptr_t)uiBuffer;

						// we're done!!!!!
						return FAT_SUCCESS;
					}
				}
				else
				{
					no_of_lfn_entries_found = 0;
				}

				// move the parent entry pointer to
				// the next entry in the sector
				parent_entry++;
			}

			// move to the next sector in the cluster
			sector++;

			// make sure that we don't overflow the root directory
			// on FAT12/16 volumes
			if (!fat)
			{
				if (sector > first_sector_of_cluster + GetRootDirectorySectors())
				{
					return FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED;
				}
			}
		}

		// get the next cluster, we'll remember the last one in
		// case we need to rewind to write lfn entries
		last_fat = fat;
		uiResult = FatGetClusterEntry(fat, &fat);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// if this is the end of the FAT chain allocate
		// a new cluster to this folder and continue
		if (FatIsEOFEntry(fat))
		{
			FatEntry newfat;

			// allocate the cluster
			newfat = FatAllocateDataCluster(1, 1, &uiResult);
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}

			// link it to the cluster chain
			uiResult = FatSetClusterEntry(last_fat, newfat);
			if (uiResult != FAT_SUCCESS)
			{
				return uiResult;
			}
			fat = newfat;
		}

		// mark the loaded sector as unknown
		muiFatSharedBufferSector = (FAT_UNKNOWN_SECTOR);
	} while (1);
}


// Deletes a file.
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatFileDelete(char* filename)
{
	uint16					uiResult;
	uint32					first_cluster;
	SFatDirectoryEntry		entry;
	uint8					uiBuffer[MAX_SECTOR_LENGTH];
	bool					bSuccess;

	char					path_part[256];
	char*					name_part;
	uint8					checksum;
	SFatFileSystemQuery		query;


	// get the entry for the file
	uiResult = FatGetFileEntry(filename, &entry);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if the entry is located go ahead and delete it.
	if (entry.name != 0)
	{
		// compute the checksum for the file
		checksum = FatLongEntryChecksum((uint8*)entry.raw.uEntry.sFatRawCommon.name);

		// make sure we're not trying to delete a directory
		if (entry.attributes & FAT_ATTR_DIRECTORY)
		{
			return FAT_NOT_A_FILE;
		}

		// find the entry's first cluster address
		((uint16*)&first_cluster)[INT32_WORD0] = entry.raw.uEntry.sFatRawCommon.first_cluster_lo;
		((uint16*)&first_cluster)[INT32_WORD1] = (GetFileSystemType() == FAT_FS_TYPE_FAT32) ? entry.raw.uEntry.sFatRawCommon.first_cluster_hi : 0;

		// free the file's cluster change
		if (first_cluster)
		{
			uiResult = FatFreeClusterChain(first_cluster);
			if (uiResult != FAT_SUCCESS)
				return uiResult;
		}

		// mark the entry as deleted
		entry.raw.uEntry.sFatRawCommon.name[0] = FAT_DELETED_ENTRY;

		bSuccess = Read(entry.sector_addr, uiBuffer);
		if (!bSuccess)
		{
			return STORAGE_UNKNOWN_ERROR;
		}

		memcpy(uiBuffer + entry.sector_offset, &entry.raw, sizeof(entry.raw));

		bSuccess = Write(entry.sector_addr, uiBuffer);
		if (!bSuccess)
		{
			return STORAGE_UNKNOWN_ERROR;
		}
	}

	// parse the filename
	FatParsePath(filename, path_part, &name_part);

	// get the 1st LFN entry of the parent directory
	uiResult = FatFindFirstEntry(path_part, FAT_ATTR_LONG_NAME, 0, &query);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// loop through each entry.
	while (*query.current_entry.raw.uEntry.sFatRawCommon.name != 0)
	{
		if (query.current_entry.raw.uEntry.sFatRawLongFileName.lfn_checksum == checksum)
		{
			// mark the entry as deleted
			query.current_entry.raw.uEntry.sFatRawCommon.name[0] = FAT_DELETED_ENTRY;

			bSuccess = Read(query.current_entry.sector_addr, uiBuffer);
			if (!bSuccess)
			{
				return STORAGE_UNKNOWN_ERROR;
			}

			memcpy(uiBuffer + query.current_entry.sector_offset, &query.current_entry.raw, sizeof(entry.raw));

			bSuccess = Write(query.current_entry.sector_addr, uiBuffer);
			if (!bSuccess)
			{
				return STORAGE_UNKNOWN_ERROR;
			}
		}

		// get the next LFN entry
		FatFindNextEntry(0, &query);
	}

	return FAT_SUCCESS;
}


// renames a file
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::FatFileRename(char* original_filename, char* new_filename)
{
	uint16					uiResult;
	uint32					entry_cluster;
	char					new_parent[256];
	char*					original_filename_part;
	SFatDirectoryEntry		original_entry;
	SFatDirectoryEntry		new_entry;
	bool					bSuccess;
	uint8					checksum = 0;
	char					original_parent[256];
	char* new_filename_part;
	uint8* uiBuffer = mauiFatSharedBuffer;

	// parse paths
	FatParsePath(original_filename, original_parent, &original_filename_part);
	FatParsePath(new_filename, new_parent, &new_filename_part);

	// try to get the new entry to see if it exists.
	FatGetFileEntry(new_filename, &new_entry);
	if (*new_entry.name != 0)
	{
		return FAT_FILENAME_ALREADY_EXISTS;
	}

	// get the directory entry
	FatGetFileEntry(original_filename, &original_entry);

	if (*original_entry.name != 0)
	{
		SFatDirectoryEntry parent;

		// compute the checksum for the file
		checksum = FatLongEntryChecksum((uint8*)original_entry.raw.uEntry.sFatRawCommon.name);

		// get the cluster # for the entry
		((uint16*)&entry_cluster)[INT32_WORD0] = original_entry.raw.uEntry.sFatRawCommon.first_cluster_lo;
		((uint16*)&entry_cluster)[INT32_WORD1] = original_entry.raw.uEntry.sFatRawCommon.first_cluster_hi;

		// get the new parent entry
		uiResult = FatGetFileEntry(new_parent, &parent);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// create the new entry in the parent folder
		uiResult = FatCreateDirectoryEntry(&parent.raw, new_filename_part, original_entry.attributes, entry_cluster, &new_entry);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// copy all info except name from the old entry to the new one
		new_entry.raw.uEntry.sFatRawCommon.access_date = original_entry.raw.uEntry.sFatRawCommon.access_date;
		new_entry.raw.uEntry.sFatRawCommon.attributes = original_entry.raw.uEntry.sFatRawCommon.attributes;
		new_entry.raw.uEntry.sFatRawCommon.create_date = original_entry.raw.uEntry.sFatRawCommon.create_date;
		new_entry.raw.uEntry.sFatRawCommon.create_time = original_entry.raw.uEntry.sFatRawCommon.create_time;
		new_entry.raw.uEntry.sFatRawCommon.create_time_tenth = original_entry.raw.uEntry.sFatRawCommon.create_time_tenth;
		new_entry.raw.uEntry.sFatRawCommon.first_cluster_hi = original_entry.raw.uEntry.sFatRawCommon.first_cluster_hi;
		new_entry.raw.uEntry.sFatRawCommon.first_cluster_lo = original_entry.raw.uEntry.sFatRawCommon.first_cluster_lo;
		new_entry.raw.uEntry.sFatRawCommon.modify_date = original_entry.raw.uEntry.sFatRawCommon.modify_date;
		new_entry.raw.uEntry.sFatRawCommon.modify_time = original_entry.raw.uEntry.sFatRawCommon.modify_time;
		new_entry.raw.uEntry.sFatRawCommon.reserved = original_entry.raw.uEntry.sFatRawCommon.reserved;
		new_entry.raw.uEntry.sFatRawCommon.size = original_entry.raw.uEntry.sFatRawCommon.size;

		// write modified entry to drive
		muiFatSharedBufferSector = (FAT_UNKNOWN_SECTOR);
		bSuccess = Read(new_entry.sector_addr, uiBuffer);
		if (!bSuccess)
		{
			return STORAGE_UNKNOWN_ERROR;
		}

		memcpy(uiBuffer + new_entry.sector_offset, &new_entry.raw, sizeof(new_entry.raw));
		bSuccess = Write(new_entry.sector_addr, uiBuffer);
		if (!bSuccess)
		{
			return STORAGE_UNKNOWN_ERROR;
		}

		// mark the original entry as deleted.
		*original_entry.raw.uEntry.sFatRawCommon.name = FAT_DELETED_ENTRY;
		bSuccess = Read(original_entry.sector_addr, uiBuffer);
		if (!bSuccess)
		{
			return STORAGE_UNKNOWN_ERROR;
		}

		memcpy(uiBuffer + original_entry.sector_offset, &original_entry.raw, sizeof(original_entry.raw));
		bSuccess = Write(original_entry.sector_addr, uiBuffer);
		if (!bSuccess)
		{
			return STORAGE_UNKNOWN_ERROR;
		}
	}
	{
		SFatFileSystemQuery query;

		// get the 1st LFN entry of the parent directory
		uiResult = FatFindFirstEntry(original_parent, FAT_ATTR_LONG_NAME, 0, &query);
		if (uiResult != FAT_SUCCESS)
		{
			return uiResult;
		}

		// loop through each entry.
		while (*query.current_entry.raw.uEntry.sFatRawCommon.name != 0)
		{
			if (query.current_entry.raw.uEntry.sFatRawLongFileName.lfn_checksum == checksum)
			{
				// mark the entry as deleted
				muiFatSharedBufferSector = (FAT_UNKNOWN_SECTOR);
				query.current_entry.raw.uEntry.sFatRawCommon.name[0] = FAT_DELETED_ENTRY;
				bSuccess = Read(query.current_entry.sector_addr, uiBuffer);
				if (!bSuccess)
				{
					return STORAGE_UNKNOWN_ERROR;
				}
				memcpy(uiBuffer + query.current_entry.sector_offset, &query.current_entry.raw, sizeof(query.current_entry.raw));
				bSuccess = Write(query.current_entry.sector_addr, uiBuffer);
				if (!bSuccess)
				{
					return STORAGE_UNKNOWN_ERROR;
				}
			}
			// get the next LFN entry
			FatFindNextEntry(0, &query);
		}
	}

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
	// 1st character of the entry to indicate an unused
	// entry but it is also a valid KANJI lead byte for the
	// character set used in Japan. The special value 0x5 is
	// used so that this special file name case for Japan
	// can be handled properly and not cause the FAT code
	// to think that the entry is free.
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
uint32 CFatVolume::CalculateFirstSectorOfCluster(uint32 cluster)
{
	return (((cluster - 0x2) * GetNoOfSectorsPerCluster()) + GetFirstDataSector());
}




// treams leading and trailing spaces. If the result
// exceeds the max length the destination will be set
// to an empty string
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
void CFatVolume::FatParsePath(char* path, char* path_part, char** filename_part)
{
	*filename_part = path + strlen(path);

	while (*--(*filename_part) != '\\' && (*filename_part) != path);

	while (path != *filename_part)
	{
		*path_part++ = *path++;
	}
	*path_part = 0;
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
char CFatVolume::FatCompareShortName(uint8* name1, uint8* name2)
{
	return memcmp(name1, name2, 11) == 0;
}


// performs an ASCII comparison on two UTF16 strings
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CFatVolume::FatCompareLongName(uint16* name1, uint16* name2)
{
	register short i;
	for (i = 0; i < 256; i++)
	{
		if (toupper((char)name1[i]) != toupper((char)name2[i]))
		{
			return 0;
		}

		if ((char)name1[i] == 0x0)
		{
			return 1;
		}
	}
	return 1;
}


// converts an 8.3 filename to the format required
// by the FAT directory entry structure
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::GetShortNameForEntry(uint8* dest, uint8* src, char lfn_disabled)
{

	char		tmp[13];
	char		has_uppercase = 0;
	uint16		dot_index;
	uint16		length;
	uint16		i;

	// check that the name is actually a long filename
	// before processing it as such
	if (!lfn_disabled)
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

		// if it has spaces or lowercase letters we must also
		// handle it as a long filename
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

			// now we copy the first 8 chars of the filename
			// excluding dots and spaces and we pad it with
			// spaces
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

			// return special code so the caller knows
			// to store the long name
			return FAT_LFN_GENERATED;
		}
	}

	// trim-off spaces - if the result is
	// greater than 12 it will return an empty
	// string
	TrimPath(tmp, (char*)src, 12);

	// if the name length was invalid return
	// error code
	if (*tmp == 0 || strlen(tmp) > 12)
	{
		return FAT_INVALID_FILENAME;
	}

	// find the location of the dot
	dot_index = (uintptr_t)strchr(tmp, (int)'.');

	// strchr gave us the address of the dot, we now
	// convert it to a 1-based index
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
	
	// copy the 1st part of the filename to the
	// destination bBuffer
	for (i = 0x0; i < 0x8; i++)
	{
		if (dot_index == 0x0)
		{
			if (i < length)
			{
				if (lfn_disabled && (tmp[i] != toupper(tmp[i])))
					has_uppercase = 1;

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
				if (lfn_disabled && (tmp[i] != toupper(tmp[i])))
					has_uppercase = 1;

				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
	}

	// if there's not extension fill the extension
	// characters with spaces
	if (dot_index == 0x0)
	{
		for (i = 0x0; i < 0x3; i++)
			*dest++ = 0x20;
	}
	// if there is an extension...
	else
	{
		// copy the extension characters to the
		// destination bBuffer
		for (i = dot_index; i < dot_index + 0x3; i++)
		{
			if (i < length)
			{
				if (lfn_disabled && (tmp[i] != toupper(tmp[i])))
					has_uppercase = 1;
				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
	}

	return has_uppercase ? FAT_INVALID_FILENAME : FAT_SUCCESS;
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
uint16 CFatVolume::GetNoOfBytesPerSector(void) { return msVolume.uiNoOfBytesPerSector; }
uint16 CFatVolume::GetNoOfSectorsPerCluster(void) { return msVolume.uiNoOfSectorsPerCluster; }
bool CFatVolume::IsUseLongFilenames(void) { return msVolume.bUseLongFilenames; }
EFatFileSystemType CFatVolume::GetFileSystemType(void) { return msVolume.eFileSystem; }
uint8 CFatVolume::GetNoOfFatTables(void) { return msVolume.uiNoOfFatTables; }
char* CFatVolume::GetLabel(void) { return msVolume.szLabel; }
uint8* CFatVolume::GetFatSharedBuffer(void) { return mauiFatSharedBuffer; }
uint32 CFatVolume::GetFatSharedBufferSector(void) { return muiFatSharedBufferSector; }

