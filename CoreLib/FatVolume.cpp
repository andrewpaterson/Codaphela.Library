#include <string.h>
#include "FatInternals.h"
#include "FatStructure.h"
#include "FatVolume.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::Mount(CFileDrive* device)
{
	bool					bSuccess;
	SFatBIOSParameterBlock*				bpb;
	uint32					hidden_sectors = 0;
	SFatPartitionEntry*		partition_entry;
	char					partitions_tried = 0;
	char					label[12];
	uint32					fsinfo_sector;
	uint8*					buffer = fat_shared_buffer;

	mpsVolume = &msVolume;

	// set the null terminator.
	label[11] = 0;

	// save the storage device handle
	mpsVolume->mpcDevice = device;

	// mark the loaded sector as unknown
	fat_shared_buffer_sector = FAT_UNKNOWN_SECTOR;

	// retrieve the boot sector (sector 0) from the storage device
	bSuccess = mpsVolume->mpcDevice->Read(0x0, buffer);
	if (!bSuccess)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	// set the partition entry pointer
	partition_entry = (SFatPartitionEntry*)(buffer + 0x1BE);

	for (;;)
	{
		// if we've already tried to mount the mpsVolume as partitionless
		// try to mount the next partition
		if (partitions_tried)
		{
			// if we've already tried all 4 partitions then we're
			// out of luck
			if (partitions_tried > 4)
			{
				return FAT_INVALID_FAT_VOLUME;
			}

			// if we've tried to mount a partition mpsVolume (not the unpartioned one)
			// then we must reload sector 0 (MBR)
			if (partitions_tried > 1)
			{
				// retrieve the boot sector (sector 0) from the storage device
				bSuccess = mpsVolume->mpcDevice->Read(0x0, buffer);
				if (!bSuccess)
				{
					return FAT_CANNOT_READ_MEDIA;
				}

				// move to the next partition entry
				partition_entry++;
			}

			// remember how many sectors before this partition
			hidden_sectors = partition_entry->lba_first_sector;

			// make sure the partition doesn't exceeds the physical boundries
			// of the device
			if (partition_entry->lba_first_sector + partition_entry->total_sectors > mpsVolume->mpcDevice->GetTotalSectors())
			{
				partitions_tried++;
				continue;
			}

			// retrieve the 1st sector of partition
			bSuccess = mpsVolume->mpcDevice->Read(partition_entry->lba_first_sector, buffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_READ_MEDIA;
			}
		}

		// set our pointer to the BPB
		bpb = (SFatBIOSParameterBlock*)buffer;

		// if the sector size is larger than what this build
		// allows do not mount the mpsVolume
		if (bpb->BPB_BytsPerSec > MAX_SECTOR_LENGTH)
		{
			return FAT_SECTOR_SIZE_NOT_SUPPORTED;
		}

		// make sure BPB_BytsPerSec and BPB_SecPerClus are
		// valid before we divide by them
		if (!bpb->BPB_BytsPerSec || !bpb->BPB_SecPerClus)
		{
			partitions_tried++;
			continue;
		}

		// make sure that SecPerClus is a power of two
		uint16 uiResult = bpb->BPB_SecPerClus;
		while (uiResult != 0x1)
		{
			if (uiResult & 0x1)
			{
				partitions_tried++;
				continue;
			}
			uiResult >>= 1;
		}

		// get all the info we need from BPB
		mpsVolume->uiRootDirectorySectors = ((bpb->BPB_RootEntCnt * 32) + (bpb->BPB_BytsPerSec - 1)) / bpb->BPB_BytsPerSec;
		mpsVolume->uiFatSize = (bpb->BPB_FATSz16) ? bpb->BPB_FATSz16 : bpb->uFatEx.sFat32.BPB_FATSz32;
		mpsVolume->uiNoOfSectors = (bpb->BPB_TotSec16) ? bpb->BPB_TotSec16 : bpb->BPB_TotSec32;
		mpsVolume->uiNoOfDataSectors = mpsVolume->uiNoOfSectors - (bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * mpsVolume->uiFatSize) + mpsVolume->uiRootDirectorySectors);
		mpsVolume->uiNoOfClusters = mpsVolume->uiNoOfDataSectors / bpb->BPB_SecPerClus;
		mpsVolume->uiFirstDataSector = bpb->BPB_RsvdSecCnt + hidden_sectors + (bpb->BPB_NumFATs * mpsVolume->uiFatSize) + mpsVolume->uiRootDirectorySectors;
		mpsVolume->uiNoOfReservedSectors = bpb->BPB_RsvdSecCnt + hidden_sectors;
		mpsVolume->uiNoOfBytesPerSector = bpb->BPB_BytsPerSec;
		mpsVolume->uiNoOfSectorsPerCluster = bpb->BPB_SecPerClus;
		mpsVolume->uiNoOfFatTables = bpb->BPB_NumFATs;
		fsinfo_sector = bpb->uFatEx.sFat32.BPB_FSInfo;

		// determine the FAT file system type
		mpsVolume->eFileSystem = (mpsVolume->uiNoOfClusters < 4085) ? FAT_FS_TYPE_FAT12 : (mpsVolume->uiNoOfClusters < 65525) ? FAT_FS_TYPE_FAT16 : FAT_FS_TYPE_FAT32;

		// sanity check that the FAT table is big enough
		switch (mpsVolume->eFileSystem)
		{
		case FAT_FS_TYPE_FAT12:
			if (mpsVolume->uiFatSize < (((mpsVolume->uiNoOfClusters + (mpsVolume->uiNoOfClusters >> 1)) + bpb->BPB_BytsPerSec - 1) / bpb->BPB_BytsPerSec))
			{
				partitions_tried++;
				continue;
			}
			break;
		case FAT_FS_TYPE_FAT16:
			if (mpsVolume->uiFatSize < (((mpsVolume->uiNoOfClusters * 2) + bpb->BPB_BytsPerSec - 1) / bpb->BPB_BytsPerSec))
			{
				partitions_tried++;
				continue;
			}
			break;
		case FAT_FS_TYPE_FAT32:
			if (mpsVolume->uiFatSize < (((mpsVolume->uiNoOfClusters * 4) + bpb->BPB_BytsPerSec - 1) / bpb->BPB_BytsPerSec))
			{
				partitions_tried++;
				continue;
			}
			break;
		}

		// read the mpsVolume label from the boot sector
		if (mpsVolume->eFileSystem == FAT_FS_TYPE_FAT16)
		{
			mpsVolume->uiID = bpb->uFatEx.sFat16.BS_VolID;
			memcpy(label, bpb->uFatEx.sFat16.BS_VolLab, 11);
			strtrim(mpsVolume->szLabel, label, 11);
		}
		else
		{
			mpsVolume->uiID = bpb->uFatEx.sFat32.BS_VolID;
			memcpy(label, bpb->uFatEx.sFat32.BS_VolLab, 11);
			strtrim(mpsVolume->szLabel, label, 11);
		}

		// if the mpsVolume is FAT32 then copy the root
		// entry's cluster from the BPB_RootClus field
		// on the BPB
		if (mpsVolume->eFileSystem == FAT_FS_TYPE_FAT32)
		{
			mpsVolume->uiRootCluster = bpb->uFatEx.sFat32.BPB_RootClus;
		}
		else
		{
			mpsVolume->uiRootCluster = 0x0;
		}

		// ###############################################
		// NOTE!!!: bpb is no good from this point on!!!!
		// ###############################################

		// check that this is a valid FAT partition by comparing the media
		// byte in the BPB with the 1st byte of the fat table
		{
			uint8 media = bpb->BPB_Media;

			// read the 1st sector of the FAT table
			bSuccess = mpsVolume->mpcDevice->Read(mpsVolume->uiNoOfReservedSectors, buffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_READ_MEDIA;
			}

			// if the lower byte of the 1st FAT entry is not the same as
			// BPB_Media then this is not a valid mpsVolume
			if (buffer[0] != media)
			{
				partitions_tried++;
				continue;
			}
		}
		break;
	}

	// read mpsVolume label entry from the root directory (if any)
	{

		SFatQueryStateInternal query;
		query.buffer = buffer;
		if (fat_query_first_entry(this, 0, FAT_ATTR_VOLUME_ID, (SFatQueryState*)&query, 1) == FAT_SUCCESS)
		{
			if (*query.current_entry_raw->uEntry.sFatRawCommon.name != 0)
			{
				strtrim(mpsVolume->szLabel, (char*)query.current_entry_raw->uEntry.sFatRawCommon.name, 11);
			}
		}
	}
	mpsVolume->uiFsinfoSector = 0xFFFFFFFF;


	// if we find a valid fsinfo structure we'll use it
	if (mpsVolume->eFileSystem == FAT_FS_TYPE_FAT32)
	{
		SFatFileSystemInfo* fsinfo;


		// read the sector containing the FSInfo structure
		bSuccess = mpsVolume->mpcDevice->Read(hidden_sectors + fsinfo_sector, buffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_READ_MEDIA;
		}

		// set fsinfo pointer
		fsinfo = (SFatFileSystemInfo*)buffer;

		// check signatures before using
		if (fsinfo->LeadSig == 0x41615252 && fsinfo->StructSig == 0x61417272 && fsinfo->TrailSig == 0xAA550000)
		{
			mpsVolume->uiNextFreeCluster = fsinfo->Nxt_Free;

			// if this value is greater than or equal to the # of
			// clusters in the mpsVolume it cannot possible be valid
			if (fsinfo->Free_Count < mpsVolume->uiNoOfClusters)
			{
				mpsVolume->uiTotalFreeClusters = fsinfo->Free_Count;
			}
			else
			{
				mpsVolume->uiTotalFreeClusters = mpsVolume->uiNoOfClusters - 1;
			}
		}
		else
		{
			mpsVolume->uiNextFreeCluster = 0xFFFFFFFF;
			mpsVolume->uiTotalFreeClusters = mpsVolume->uiNoOfClusters - 1;
		}

		// remember fsinfo sector
		mpsVolume->uiFsinfoSector = hidden_sectors + fsinfo_sector;
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::Unmount(void)
{
	// if this is a FAT32 volume we'll update the fsinfo structure
	if (mpsVolume->eFileSystem == FAT_FS_TYPE_FAT32 && mpsVolume->uiFsinfoSector != 0xFFFFFFFF)
	{
		bool					bSuccess;
		SFatFileSystemInfo*		fsinfo;

		uint8* buffer = fat_shared_buffer;

		// mark the loaded sector as unknown
		fat_shared_buffer_sector = (FAT_UNKNOWN_SECTOR);

		// read the sector containing the FSInfo structure
		bSuccess = mpsVolume->mpcDevice->Read(GetFsinfoSector(), buffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_READ_MEDIA;
		}


		// set the pointer to the fsinfo structure
		fsinfo = (SFatFileSystemInfo*)buffer;

		// check the signatures before writting
		// note: when you mount a removable device in windows it will channge
		// these signatures, i guess it feels it cannot be trusted. So we're going
		// to rebuild them no matter what as they significantly speed up this
		// implementation. After the mpsVolume has been mounted elsewhere Free_Count cannot
		// be trusted. This implementation doesn't actually use it but if you only
		// mount the mpsVolume with us it will keep it up to date.

		/*if (fsinfo->LeadSig == 0x41615252 && fsinfo->StructSig == 0x61417272 && fsinfo->TrailSig == 0xAA550000)*/
		{
			// mark all values as unknown
			fsinfo->Nxt_Free = GetNextFreeCluster();
			fsinfo->Free_Count = GetTotalFreeClusters();
			fsinfo->LeadSig = 0x41615252;
			fsinfo->StructSig = 0x61417272;
			fsinfo->TrailSig = 0xAA550000;

			// write the fsinfo sector
			bSuccess = mpsVolume->mpcDevice->Write(GetFsinfoSector(), buffer);
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
	return mpsVolume->mpcDevice->Read(uiSector, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::Write(uint64 uiSector, void* pvData)
{
	return mpsVolume->mpcDevice->Write(uiSector, pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive)
{
	return mpsVolume->mpcDevice->Erase(uiStartSector, uiStopSectorInclusive);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatVolume::GetSectorSize(void)
{
	return mpsVolume->uiNoOfBytesPerSector;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::GetPageSize(void)
{
	return mpsVolume->mpcDevice->GetPageSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatVolume::HasNextFreeCluser(void)
{
	if (mpsVolume->uiNextFreeCluster != 0xFFFFFFFF)
	{
		if (mpsVolume->uiNextFreeCluster <= mpsVolume->uiNoOfClusters + 1)
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
	mpsVolume->uiNextFreeCluster = uiCluster;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetTotalFreeClusters(uint32 uiTotalFreeClusters)
{
	mpsVolume->uiTotalFreeClusters = uiTotalFreeClusters;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::GetID(void) { return mpsVolume->uiID; }
uint32 CFatVolume::GetFatSize(void) { return mpsVolume->uiFatSize; }
uint32 CFatVolume::GetRootCluster(void) { return mpsVolume->uiRootCluster; }
uint32 CFatVolume::GetFirstDataSector(void) { return mpsVolume->uiFirstDataSector; }
uint32 CFatVolume::GetNoOfSectors(void) { return mpsVolume->uiNoOfSectors; }
uint32 CFatVolume::GetNoOfDataSectors(void) { return mpsVolume->uiNoOfDataSectors; }
uint32 CFatVolume::GetNoOfClusters(void) { return mpsVolume->uiNoOfClusters; }
uint32 CFatVolume::GetNoOfReservedSectors(void) { return mpsVolume->uiNoOfReservedSectors; }
uint32 CFatVolume::GetNextFreeCluster(void) { return mpsVolume->uiNextFreeCluster; }
uint32 CFatVolume::GetTotalFreeClusters(void) { return mpsVolume->uiTotalFreeClusters; }
uint32 CFatVolume::GetFsinfoSector(void) { return mpsVolume->uiFsinfoSector; }
uint16 CFatVolume::GetRootDirectorySectors(void) { return mpsVolume->uiRootDirectorySectors; }
uint16 CFatVolume::GetNoOfBytesPerSector(void) { return mpsVolume->uiNoOfBytesPerSector; }
uint16 CFatVolume::GetNoOfSectorsPerCluster(void) { return mpsVolume->uiNoOfSectorsPerCluster; }
bool CFatVolume::IsUseLongFilenames(void) { return mpsVolume->bUseLongFilenames; }
EFatFileSystemType CFatVolume::GetFileSystemType(void) { return mpsVolume->eFileSystem; }
uint8 CFatVolume::GetNoOfFatTables(void) { return mpsVolume->uiNoOfFatTables; }
char* CFatVolume::GetLabel(void) { return mpsVolume->szLabel; }

