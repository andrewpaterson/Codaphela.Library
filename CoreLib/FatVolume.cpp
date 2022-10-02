#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "BaseLib/MemCompare.h"
#include "FatTime.h"
#include "FatVolume.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::Mount(CFileDrive* device)
{
	SFatBIOSParameterBlock* psBPB;
	uint32					uiHiddenSectors = 0;
	SFatPartitionEntry*		psPartitionEntry;
	uint8					uiPartitionsTried = 0;
	uint32					uiFileSystemInfoSector;
	uint8*					pvMBRSector;
	uint8*					pvLBASector;

	mcSectorCache.Init(device, 3);

	// set the null terminator.

	// save the storage device handle
	mpcDevice = device;

	pvMBRSector = (uint8*)mcSectorCache.ReadSector(0);
	if (pvMBRSector == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	mcSectorCache.Lock(pvMBRSector);

	// set the partition sEntry pointer
	psPartitionEntry = (SFatPartitionEntry*)(pvMBRSector + 0x1BE);

	for (;;)
	{
		// if we've already tried to mount the volume as partitionless try to mount the next partition
		if (uiPartitionsTried > 0)
		{
			// if we've already tried all 4 partitions then we're out of luck
			if (uiPartitionsTried > 4)
			{
				mcSectorCache.Unlock(pvMBRSector);
				return FAT_INVALID_FAT_VOLUME;
			}

			// if we've tried to mount a partition mpsVolume (not the unpartioned one) then we must reload sector 0 (MBR)
			if (uiPartitionsTried > 1)
			{
				// move to the next partition sEntry
				psPartitionEntry++;
			}

			// remember how many sectors before this partition
			uiHiddenSectors = psPartitionEntry->lba_first_sector;

			// make sure the partition doesn't exceeds the physical boundries of the device
			if (psPartitionEntry->lba_first_sector + psPartitionEntry->total_sectors > mpcDevice->GetTotalSectors())
			{
				uiPartitionsTried++;
				continue;
			}

			// retrieve the 1st sector of partition
			pvLBASector = (uint8*)mcSectorCache.ReadSector(psPartitionEntry->lba_first_sector);
			if (pvLBASector == NULL)
			{
				mcSectorCache.Unlock(pvMBRSector);
				return FAT_CANNOT_READ_MEDIA;
			}

			// set our pointer to the BPB
			psBPB = (SFatBIOSParameterBlock*)pvLBASector;
		}
		else
		{
			psBPB = (SFatBIOSParameterBlock*)pvMBRSector;
		}


		// if the sector size is larger than what this build
		// allows do not mount the mpsVolume
		if (psBPB->BPB_BytsPerSec > MAX_SECTOR_LENGTH)
		{
			mcSectorCache.Unlock(pvMBRSector);
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
		while (uiResult != 1)
		{
			if (uiResult & 1)
			{
				uiPartitionsTried++;
				continue;
			}
			uiResult >>= 1;
		}

		// get all the info we need from BPB
		uiRootDirectorySectors = ((psBPB->BPB_RootEntCnt * 32) + (psBPB->BPB_BytsPerSec - 1)) / psBPB->BPB_BytsPerSec;
		uiFatSize = (psBPB->BPB_FATSz16) ? psBPB->BPB_FATSz16 : psBPB->uFatEx.sFat32.BPB_FATSz32;
		uiNoOfSectors = (psBPB->BPB_TotSec16) ? psBPB->BPB_TotSec16 : psBPB->BPB_TotSec32;
		uiNoOfDataSectors = uiNoOfSectors - (psBPB->BPB_RsvdSecCnt + (psBPB->BPB_NumFATs * uiFatSize) + uiRootDirectorySectors);
		uiNoOfClusters = uiNoOfDataSectors / psBPB->BPB_SecPerClus;
		uiFirstDataSector = psBPB->BPB_RsvdSecCnt + uiHiddenSectors + (psBPB->BPB_NumFATs * uiFatSize) + uiRootDirectorySectors;
		uiNoOfReservedSectors = psBPB->BPB_RsvdSecCnt + uiHiddenSectors;
		uiBytesPerSector = psBPB->BPB_BytsPerSec;
		uiNoOfSectorsPerCluster = psBPB->BPB_SecPerClus;
		uiNoOfFatTables = psBPB->BPB_NumFATs;
		uiFileSystemInfoSector = psBPB->uFatEx.sFat32.BPB_FSInfo;
		uiBytesPerCluster =  uiBytesPerSector * uiNoOfSectorsPerCluster;

		// determine the FAT file system type
		eFileSystem = (uiNoOfClusters < 4085) ? FAT_FS_TYPE_FAT12 : (uiNoOfClusters < 65525) ? FAT_FS_TYPE_FAT16 : FAT_FS_TYPE_FAT32;

		// sanity check that the FAT table is big enough
		switch (eFileSystem)
		{
		case FAT_FS_TYPE_FAT12:
			if (uiFatSize < (((uiNoOfClusters + (uiNoOfClusters >> 1)) + psBPB->BPB_BytsPerSec - 1) / psBPB->BPB_BytsPerSec))
			{
				uiPartitionsTried++;
				continue;
			}
			break;
		case FAT_FS_TYPE_FAT16:
			if (uiFatSize < (((uiNoOfClusters * 2) + psBPB->BPB_BytsPerSec - 1) / psBPB->BPB_BytsPerSec))
			{
				uiPartitionsTried++;
				continue;
			}
			break;
		case FAT_FS_TYPE_FAT32:
			if (uiFatSize < (((uiNoOfClusters * 4) + psBPB->BPB_BytsPerSec - 1) / psBPB->BPB_BytsPerSec))
			{
				uiPartitionsTried++;
				continue;
			}
			break;
		}

		// read the mpsVolume label from the boot sector
		if (eFileSystem == FAT_FS_TYPE_FAT16)
		{
			uiID = psBPB->uFatEx.sFat16.BS_VolID;
			mszLabel.Init(psBPB->uFatEx.sFat16.BS_VolLab, 0, 10);
			mszLabel.StripWhiteSpace();
		}
		else if (eFileSystem == FAT_FS_TYPE_FAT32)
		{
			uiID = psBPB->uFatEx.sFat32.BS_VolID;
			mszLabel.Init(psBPB->uFatEx.sFat32.BS_VolLab, 0, 10);
			mszLabel.StripWhiteSpace();
		}
		else
		{
			mszLabel.Init();
		}

		// if the mpsVolume is FAT32 then copy the root sEntry's cluster from the BPB_RootClus field on the BPB 
		if (eFileSystem == FAT_FS_TYPE_FAT32)
		{
			uiRootCluster = psBPB->uFatEx.sFat32.BPB_RootClus;
		}
		else
		{
			uiRootCluster = 0;
		}

		uiRootSector = CalculateRootSector();

		uint8	uiMedia = psBPB->BPB_Media;
		uint32	uiFATSector;
		uint8* pvFATSector;

		uiFATSector = uiNoOfReservedSectors;
		pvFATSector = (uint8*)mcSectorCache.ReadSector(uiFATSector);
		if (pvFATSector == NULL)
		{
			mcSectorCache.Unlock(pvMBRSector);
			return FAT_CANNOT_READ_MEDIA;
		}

		// if the lower byte of the 1st FAT sEntry is not the same as BPB_Media then this is not a valid volume
		if (pvFATSector[0] != uiMedia)
		{
			uiPartitionsTried++;
			continue;
		}
		break;
	}

	// read volume label sEntry from the root directory (if any)
	SFatQueryState	query;
	uint8* pvFSInfoSector;

	if (QueryFirstEntry(0, FAT_ATTR_VOLUME_ID, &query, 1) == FAT_SUCCESS)
	{
		if (*query.sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName != 0)
		{
			CChars	szLabel;
			szLabel.Init((char*)query.sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName, 0, 10);
			szLabel.StripWhiteSpace();
		}
	}
	uiFileSystemInfoSector = 0xFFFFFFFF;

	// if we find a valid psFileSystemInfo structure we'll use it
	if (eFileSystem == FAT_FS_TYPE_FAT32)
	{
		SFatFileSystemInfo* psFileSystemInfo;

		// read the sector containing the FSInfo structure
		pvFSInfoSector = (uint8*)mcSectorCache.ReadSector(psPartitionEntry->lba_first_sector);
		if (pvFSInfoSector == NULL)
		{
			mcSectorCache.Unlock(pvMBRSector);
			return FAT_CANNOT_READ_MEDIA;
		}

		// set psFileSystemInfo pointer
		psFileSystemInfo = (SFatFileSystemInfo*)pvFSInfoSector;

		// check signatures before using
		if (psFileSystemInfo->LeadSig == 0x41615252 && psFileSystemInfo->StructSig == 0x61417272 && psFileSystemInfo->TrailSig == 0xAA550000)
		{
			uiNextFreeCluster = psFileSystemInfo->Nxt_Free;

			// if this value is greater than or equal to the # of clusters in the mpsVolume it cannot possible be valid
			if (psFileSystemInfo->Free_Count < uiNoOfClusters)
			{
				uiTotalFreeClusters = psFileSystemInfo->Free_Count;
			}
			else
			{
				uiTotalFreeClusters = uiNoOfClusters - 1;
			}
		}
		else
		{
			uiNextFreeCluster = 0xFFFFFFFF;
			uiTotalFreeClusters = uiNoOfClusters - 1;
		}

		// remember psFileSystemInfo sector
		uiFileSystemInfoSector = uiHiddenSectors + uiFileSystemInfoSector;
	}

	mcSectorCache.Unlock(pvMBRSector);
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
	uint8*					pvBuffer;

	// if this is a FAT32 volume we'll update the psFileSystemInfo structure
	if (eFileSystem == FAT_FS_TYPE_FAT32 && uiFileSystemInfoSector != 0xFFFFFFFF)
	{
		// read the sector containing the FSInfo structure
		pvBuffer = ReadInfoSector(GetFileSystemInfoSector());
		if (pvBuffer == NULL)
		{
			return FAT_CANNOT_READ_MEDIA;
		}


		// set the pointer to the psFileSystemInfo structure
		psFileSystemInfo = (SFatFileSystemInfo*)pvBuffer;

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

		SetInfoSectorDirty(pvBuffer);

		// write the psFileSystemInfo sector
		eResult = Flush();
		RETURN_ON_FAT_FAILURE(eResult);
	}

	mcSectorCache.Kill();
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
EFatCode CFatVolume::QueryFirstEntry(SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, bool bBufferLocked)
{
	uint32		uiFirstSector;

	// make sure the long filename is set to an empty string
	query->long_filename[0] = 0;

	// if the directory sEntry has the cluster # set to
	// zero it is the root directory so we need to
	// get the cluster # accordingly
	if (directory)
	{
		if (directory->uEntry.sFatRawCommon.uiFirstClusterHighWord == 0 && directory->uEntry.sFatRawCommon.uiFirstClusterLowWord == 0)
		{
			directory = NULL;
		}
	}

	// if no directory entry was provided we'll use the root entry of the volume.
	if (directory == NULL)
	{
		query->uiCurrentCluster = GetRootCluster();
		uiFirstSector = GetRootSector();
	}
	// if a directory sEntry was provided
	else
	{
		// if the sEntry provided is not a directory
		// sEntry return an error code
		if (!(directory->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_DIRECTORY))
		{
			return FAT_NOT_A_DIRECTORY;
		}

		query->uiCurrentCluster = GetFirstClusterFromFatEntry(directory, GetFileSystemType() == FAT_FS_TYPE_FAT32);

		// get the 1st sector of the directory sEntry
		uiFirstSector = CalculateFirstSectorOfCluster(query->uiCurrentCluster);
	}

	// read the sector into the query state buffer
	query->puiBuffer = ReadInfoSector(uiFirstSector, true);
	if (query->puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	// set the 1st and current sEntry pointers on the query state to the 1st sEntry of the directory.
	query->Attributes = attributes;
	query->uiCurrentSector = 0;

	query->sFirstEntryRaw = (SFatRawDirectoryEntry*)query->puiBuffer;
	query->sCurrentEntryRaw = (SFatRawDirectoryEntry*)query->puiBuffer;

	// find the 1st sEntry and return it's result code
	return QueryNextEntry(query, bBufferLocked, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::QueryNextEntry(SFatQueryState* psQuery, bool bBufferLocked, bool bFirstEntry)
{
	bool		bPass;
	uint32		uiSectorAddress;
	uint16		uiEntryCount;
	uint16		uiEntriesPerSector;

	uiEntryCount = 0;
	uiEntriesPerSector = (GetSectorSize() - 0x20) / sizeof(SFatRawDirectoryEntry);

	uiEntryCount = psQuery->sCurrentEntryRaw - psQuery->sFirstEntryRaw;

	do
	{
		// if the current entry is the last entry of the sector...
		if (!bFirstEntry)
		{
			if (uiEntryCount == uiEntriesPerSector)
			{
				uiEntryCount = 0;
				// if the current sector is the last of the current cluster then we must find the next
				// cluster... if CurrentCluster == 0 then this is the root directory of a FAT16/FAT12 volume, that
				// volume has a fixed size in sectors and is not allocated as a cluster chain so we don't do this
				if (psQuery->uiCurrentCluster > 0 && psQuery->uiCurrentSector == NumSectorsPerCluster() - 1)
				{
					fatEntry uiFat;

					// get the fat structure for the current cluster
					// and return UNKNOWN_ERROR if the operation fails
					if (GetNextClusterEntry(psQuery->uiCurrentCluster, &uiFat) != FAT_SUCCESS)
					{
						mcSectorCache.Unlock(psQuery->puiBuffer);
						return FAT_UNKNOWN_ERROR;
					}

					// if this is the last cluster of the directory...
					if (FatIsEOFEntry(uiFat))
					{
						mcSectorCache.Unlock(psQuery->puiBuffer);
						// set the current sEntry to 0
						*psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName = 0;
						return FAT_SUCCESS;
					}

					// set the current cluster to the next
					// cluster of the directory sEntry
					psQuery->uiCurrentCluster = uiFat;

					// reset the current sector
					psQuery->uiCurrentSector = 0;

					// calculate the address of the next sector
					uiSectorAddress = CalculateFirstSectorOfCluster(psQuery->uiCurrentCluster) + psQuery->uiCurrentSector;
				}
				else
				{
					// if there are more sectors on the current cluster then
					psQuery->uiCurrentSector++;

					// if this is the root directory of a FAT16/FAT12
					// volume and we have passed it's last sector then
					// there's no more entries...
					if (psQuery->uiCurrentCluster == 0)
					{
						if (psQuery->uiCurrentSector == GetRootDirectorySectors())
						{
							mcSectorCache.Unlock(psQuery->puiBuffer);
							*psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName = 0;
							return FAT_SUCCESS;
						}
						uiSectorAddress = (GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize())) + psQuery->uiCurrentSector;
					}
					else
					{
						// calculate the address of the next sector
						uiSectorAddress = CalculateFirstSectorOfCluster(psQuery->uiCurrentCluster) + psQuery->uiCurrentSector;
					}
				}

				// read the next sector into the psQuery uiBuffer
				mcSectorCache.Unlock(psQuery->puiBuffer);
				psQuery->puiBuffer = ReadInfoSector(uiSectorAddress, true);
				if (psQuery->puiBuffer == NULL)
				{
					return FAT_CANNOT_READ_MEDIA;
				}

				// set the 1st and current entry pointers on the psQuery state to the 1st sEntry of the directory
				psQuery->sFirstEntryRaw = (SFatRawDirectoryEntry*)psQuery->puiBuffer;
				psQuery->sCurrentEntryRaw = (SFatRawDirectoryEntry*)psQuery->puiBuffer;
			}
			// if there are more entries on the current sector...
			else
			{
				psQuery->sCurrentEntryRaw++;
				uiEntryCount++;
			}
		}
		else
		{
			bFirstEntry = false;
			//uiEntryCount++; //?
		}

		// if this is a long filename sEntry...
		if (psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes == FAT_ATTR_LONG_NAME && !IS_FREE_DIRECTORY_ENTRY(psQuery->sCurrentEntryRaw))
		{
			// if this entry is marked as the 1st LFN sEntry
			if (psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence & FAT_FIRST_LFN_ENTRY)
			{
				psQuery->uiSequence = (psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence ^ FAT_FIRST_LFN_ENTRY) + 1;
				psQuery->uiChecksum = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiChecksum;

				// insert null terminator at the end of the long filename
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 2) * 13) + 0xD])[INT16_BYTE0] = 0;
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 2) * 13) + 0xD])[INT16_BYTE1] = 0;
			}

			// if this is the LFN that we're expecting then process it, otherwise we'll have to wait for another 1st LFN sEntry otherwise read the LFN
			// characters and save them on the psQuery state struct.
			if (psQuery->uiChecksum == psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiChecksum &&
				(psQuery->uiSequence == (psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence & (0xFF ^ FAT_FIRST_LFN_ENTRY)) + 1))
			{
				psQuery->uiSequence = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.uiSequence & (0xFF ^ FAT_FIRST_LFN_ENTRY);
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x0])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[0];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x0])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[1];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x1])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[2];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x1])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[3];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x2])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[4];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x2])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[5];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x3])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[6];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x3])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[7];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x4])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[8];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x4])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars1[9];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x5])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[0];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x5])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[1];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x6])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[2];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x6])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[3];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x7])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[4];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x7])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[5];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x8])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[6];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x8])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[7];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x9])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[8];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0x9])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[9];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0xA])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[10];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0xA])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars2[11];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0xB])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars3[0];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0xB])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars3[1];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0xC])[INT16_BYTE0] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars3[2];
				((uint8*)&psQuery->long_filename[((psQuery->uiSequence - 1) * 13) + 0xC])[INT16_BYTE1] = psQuery->sCurrentEntryRaw->uEntry.sFatRawLongFileName.auiChars3[3];
			}
			else
			{
				psQuery->uiChecksum = 0;
			}

			// make sure we never return this entry
			bPass = (psQuery->Attributes == FAT_ATTR_LONG_NAME);
		}
		else
		{
			// check that the current entry passes the psQuery attributes check
			bPass =
				(!(psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_HIDDEN) || (psQuery->Attributes & FAT_ATTR_HIDDEN)) &&
				(!(psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_SYSTEM) || (psQuery->Attributes & FAT_ATTR_SYSTEM)) &&
				(!(psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_VOLUME_ID) || (psQuery->Attributes & FAT_ATTR_VOLUME_ID)) &&
				(!(psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes & FAT_ATTR_LONG_NAME) || (psQuery->Attributes & FAT_ATTR_LONG_NAME));
		}
	}
	while (!bPass || *psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName == FAT_DELETED_ENTRY);

	// if we found an entry we need to check it's LFN checksum to make sure that the long filename that we've associated with it belongs to it. If it doesn't clear it.
	if (*psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName != 0x0)
	{
		if (psQuery->uiChecksum != FatLongEntryChecksum((uint8*)psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName))
		{
			psQuery->long_filename[0] = 0x0;
		}
	}

	// if this entry doesn't have an LFN entry but its marked as having
	// a lowercase name or extension then fill the long filename with the
	// lowercase version
	if (psQuery->long_filename[0] == 0 && (psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.uiReserved & (FAT_LOWERCASE_EXTENSION | FAT_LOWERCASE_BASENAME)))
	{
		int i = 0;
		for (uint16 uiResult = 0; uiResult < 8; uiResult++)
		{
			if (psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult] != 0x20)
			{
				if (psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.uiReserved & FAT_LOWERCASE_BASENAME)
				{
					psQuery->long_filename[i++] = tolower(psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult]);
				}
				else
				{
					psQuery->long_filename[i++] = psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult];
				}
			}
		}

		if (psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[8] != 0x20)
		{
			psQuery->long_filename[i++] = '.';

			for (uint16 uiResult = 8; uiResult < 11; uiResult++)
			{
				if (psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult] != 0x20)
				{
					if (psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.uiReserved & FAT_LOWERCASE_EXTENSION)
					{
						psQuery->long_filename[i++] = tolower(psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult]);
					}
					else
					{
						psQuery->long_filename[i++] = psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName[uiResult];
					}
				}
			}
		}
		psQuery->long_filename[i] = 0x0;
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::GetNextClusterEntry(uint32 uiCurrentCluster, uint32* puiNextCluster)
{
	uint32		uiFirstClusterSector;
	uint32		uiOffsetInSector;
	uint8*		puiBuffer;

	CalculateFATIndexAndOffset(&uiOffsetInSector, uiCurrentCluster, &uiFirstClusterSector);

	puiBuffer = ReadInfoSector(uiFirstClusterSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	switch (GetFileSystemType())
	{
		case FAT_FS_TYPE_FAT12:
		{
			// clear FatEntry to make sure that the upper 16
			// bits are not set.
			*puiNextCluster = 0;

			// read the 1st byte

			((uint8*)puiNextCluster)[0] = puiBuffer[uiOffsetInSector];

			// load the next sector (if necessary) and set the offset for the next byte in the buffer
			if (uiOffsetInSector == GetSectorSize() - 1)
			{
				// load the next sector into the uiBuffer
				puiBuffer = ReadInfoSector(uiFirstClusterSector + 1);
				if (puiBuffer == NULL)
				{
					return FAT_CANNOT_READ_MEDIA;
				}

				// the 2nd byte is now the 1st byte in the uiBuffer
				uiOffsetInSector = 0;
			}
			else
			{
				// the 2nd byte is still right after the 1st one on
				// the uiBuffer
				uiOffsetInSector++;
			}

			// read the 2nd byte
			((uint8*)puiNextCluster)[1] = puiBuffer[uiOffsetInSector];

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
			*puiNextCluster = *((uint16*)&puiBuffer[uiOffsetInSector]);
			break;
		}
		case FAT_FS_TYPE_FAT32:
		{
			*puiNextCluster = *((uint32*)&puiBuffer[uiOffsetInSector]) & 0x0FFFFFFF;
			break;
		}
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::Allocate(uint32 uiClustersNeeded, uint32 uiCluster, uint32* puiNewClusterInVolume)
{
	uint32		uiNextCluster;
	uint32		uiPageSize;
	uint32		uiStartSector;
	uint32		uiEndSector;
	EFatCode	eResult;
	bool		bSuccess;

	uiPageSize = GetPageSize();

	if (uiPageSize > NumSectorsPerCluster())
	{
		uint32 clusters_per_page = uiPageSize / NumSectorsPerCluster();

		if (uiClustersNeeded % clusters_per_page)
		{
			uiClustersNeeded += clusters_per_page - (uiClustersNeeded % clusters_per_page);
		}

		if ((uiClustersNeeded % clusters_per_page) != 0)
		{
			return FAT_UNKNOWN_ERROR;
		}

		eResult = FAT_UNKNOWN_ERROR;
		*puiNewClusterInVolume = AllocateDataClusterEx(uiClustersNeeded, 0, uiPageSize, &eResult);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}
	else
	{
		eResult = FAT_UNKNOWN_ERROR;
		*puiNewClusterInVolume = AllocateDataCluster(uiClustersNeeded, 1, &eResult);
		if (eResult != FAT_SUCCESS)
		{
			return eResult;
		}
	}

	// find out how many clusters are allocated sequentially to this file following the current cursor location
	while (!FatIsEOFEntry(uiCluster))
	{
		// calculate the start and end address the cluster
		uiStartSector = CalculateFirstSectorOfCluster(uiCluster);
		uiEndSector = uiStartSector + NumSectorsPerCluster();

		// find the last sequential sector after this address
		while (!FatIsEOFEntry(uiCluster))
		{
			eResult = GetNextClusterEntry(uiCluster, &uiNextCluster);
			if (eResult != FAT_SUCCESS)
			{
				return eResult;
			}

			if (uiNextCluster == (uiCluster + 1))
			{
				uiEndSector += NumSectorsPerCluster();
				uiCluster = uiNextCluster;
			}
			else
			{
				uiCluster = uiNextCluster;
				break;
			}
		}

		// pre-erase the clusters
		bSuccess = Erase(uiStartSector, uiEndSector - 1);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}
	}
	return FAT_SUCCESS;
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
uint32 CFatVolume::AllocateDirectoryCluster(SFatRawDirectoryEntry* parent, EFatCode* peResult)
{
	return AllocateCluster(parent, 1, 1, 1, peResult);
}


// allocates the requested number of clusters - finds the free clusters, initializes it to zeroes,
// links them as a cluster chain (marking the last one as EOC) and returns the cluster number for
// the 1st cluster in the chain
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::AllocateDataCluster(uint32 uiClusterCount, bool bEraseCluster, EFatCode* peResult)
{
	return AllocateCluster(0, uiClusterCount, bEraseCluster, 1, peResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::AllocateDataClusterEx(uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult)
{
	return AllocateCluster(0, uiClusterCount, bEraseCluster, uiPageSize, peResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::FindNextPageCluster(uint32 uiPageSize, uint32 uiCluster, uint16 uiStep)
{
	if (NumSectorsPerCluster() < uiPageSize)
	{
		uint32 uiSector;
		uint16 uiStepCount = 0;

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
	switch (eFileSystem)
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
bool CFatVolume::HasNextFreeCluser(void)
{
	if (uiNextFreeCluster != 0xFFFFFFFF)
	{
		if (uiNextFreeCluster <= uiNoOfClusters + 1)
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
EFatCode CFatVolume::InitialiseCluster(uint32 uiCluster)
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
EFatCode CFatVolume::InitializeDirectoryCluster(SFatRawDirectoryEntry* psDirectoryParent, uint32 uiCluster, uint32 uiSector)
{
	uint32					uiFirstClusterSector;
	SFatRawDirectoryEntry*	entries;
	uint8*					puiBuffer;
	bool					bSuccess;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	entries = (SFatRawDirectoryEntry*)puiBuffer;

	// initialize the 1st sector of the directory cluster with the dot sEntry
	memset(puiBuffer, 0, GetSectorSize());
	entries->uEntry.sFatRawCommon.szShortName[0x0] = '.';
	entries->uEntry.sFatRawCommon.szShortName[0x1] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x2] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x3] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x4] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x5] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x6] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x7] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x8] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x9] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0xA] = ' ';
	entries->uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_DIRECTORY;
	entries->uEntry.sFatRawCommon.uiSize = 0x0;
	entries->uEntry.sFatRawCommon.uiReserved = 0;
	entries->uEntry.sFatRawCommon.uiFirstClusterLowWord = LO16(uiCluster);
	entries->uEntry.sFatRawCommon.uiFirstClusterHighWord = HI16(uiCluster);
	entries->uEntry.sFatRawCommon.uiCreateDate = GetSystemClockDate();
	entries->uEntry.sFatRawCommon.uiCreateTime = GetSystemClockTime();
	entries->uEntry.sFatRawCommon.uiModifyDate = entries->uEntry.sFatRawCommon.uiCreateDate;
	entries->uEntry.sFatRawCommon.uiModifyTime = entries->uEntry.sFatRawCommon.uiCreateTime;
	entries->uEntry.sFatRawCommon.uiAccessDate = entries->uEntry.sFatRawCommon.uiCreateDate;
	entries->uEntry.sFatRawCommon.uiCreateTimeTenths = 0xc6;

	entries++;

	// initialize the dot dot sEntry.
	entries->uEntry.sFatRawCommon.szShortName[0x0] = '.';
	entries->uEntry.sFatRawCommon.szShortName[0x1] = '.';
	entries->uEntry.sFatRawCommon.szShortName[0x2] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x3] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x4] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x5] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x6] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x7] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x8] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0x9] = ' ';
	entries->uEntry.sFatRawCommon.szShortName[0xA] = ' ';
	entries->uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_DIRECTORY;
	entries->uEntry.sFatRawCommon.uiSize = 0x0;
	entries->uEntry.sFatRawCommon.uiReserved = 0;
	entries->uEntry.sFatRawCommon.uiFirstClusterLowWord = psDirectoryParent->uEntry.sFatRawCommon.uiFirstClusterLowWord;
	entries->uEntry.sFatRawCommon.uiFirstClusterHighWord = psDirectoryParent->uEntry.sFatRawCommon.uiFirstClusterHighWord;
	entries->uEntry.sFatRawCommon.uiCreateDate = GetSystemClockDate();
	entries->uEntry.sFatRawCommon.uiCreateTime = GetSystemClockTime();
	entries->uEntry.sFatRawCommon.uiModifyDate = entries->uEntry.sFatRawCommon.uiCreateDate;
	entries->uEntry.sFatRawCommon.uiModifyTime = entries->uEntry.sFatRawCommon.uiCreateTime;
	entries->uEntry.sFatRawCommon.uiAccessDate = entries->uEntry.sFatRawCommon.uiCreateDate;
	entries->uEntry.sFatRawCommon.uiCreateTimeTenths = 0xc6;
	mcSectorCache.Dirty(puiBuffer);

	// when the parent is the root directory the dotdot sEntry always points
	// to cluster 0, even in FAT32 when the root directory is not actually on
	// cluster 0 so we need to check if the parent is the root directory and
	// in that case set the 1st cluster to 0
	if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
	{
		uint32 uiParentCluster;

		uiParentCluster = GetFirstClusterFromFatEntry(psDirectoryParent, true);
		if (GetRootCluster() == uiParentCluster)
		{
			entries->uEntry.sFatRawCommon.uiFirstClusterLowWord = 0;
			entries->uEntry.sFatRawCommon.uiFirstClusterHighWord = 0;
		}
	}

	// write the 1st sector of the folder
	uiFirstClusterSector = CalculateFirstSectorOfCluster(uiCluster);
	puiBuffer = ReadInfoSector(uiFirstClusterSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}


	// clear the . and .. entries from the uiBuffer and
	// initialize the rest of the sectors of this cluster
	memset(puiBuffer, 0, sizeof(SFatRawDirectoryEntry) * 2);

	uiFirstClusterSector++;
	bSuccess = mpcDevice->Erase(uiFirstClusterSector, NumSectorsPerCluster() - 2);
	if (!bSuccess)
	{
		return FAT_CANNOT_WRITE_MEDIA;
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::InitialiseAllocatedFatCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 uiCluster, bool bZero, uint32 uiSector)
{
	EFatCode	eResult;

	// if we're allocating a cluster for a directory then initialize it properly
	if (psParentDirectory)
	{
		eResult = InitializeDirectoryCluster(psParentDirectory, uiCluster, uiSector);
		RETURN_ON_FAT_FAILURE(eResult);
	}
	else
	{
		if (bZero)
		{
			eResult = InitialiseCluster(uiCluster);
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
	uint8*		puiBuffer;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	if (uiCluster & 1)
	{
		puiBuffer[uiOffsetInSector] &= 0x0F;
		puiBuffer[uiOffsetInSector] |= LO8((uint16)FAT12_EOC << 4);
	}
	else
	{
		puiBuffer[uiOffsetInSector] = LO8((uint16)FAT12_EOC);
	}
	mcSectorCache.Dirty(puiBuffer);

	if (uiOffsetInSector == GetSectorSize() - 1)
	{
		puiBuffer = ReadInfoSector(uiSector + 1);
		if (puiBuffer == NULL)
		{
			return FAT_CANNOT_READ_MEDIA;
		}
		bNextSector = true;
	}
	else
	{
		bNextSector = false;
	}

	if (uiCluster & 1)
	{
		puiBuffer[bNextSector ? 0 : (uiOffsetInSector + 1)] = HI8((uint16)FAT12_EOC << 4);
	}
	else
	{
		//Even cluster.
		puiBuffer[bNextSector ? 0 : (uiOffsetInSector + 1)] &= 0xF0;
		puiBuffer[bNextSector ? 0 : (uiOffsetInSector + 1)] |= HI8((uint16)FAT12_EOC);
	}
	mcSectorCache.Dirty(puiBuffer);

	// if this is not the 1st cluster allocated update the last one to link to this one
	if (uiLastFatEntry != FAT12_EOC)
	{
		puiBuffer = ReadInfoSector(uiPreviousSector);
		if (puiBuffer == NULL)
		{
			return FAT_CANNOT_READ_MEDIA;
		}

		if (uiLastFatEntry & 1)
		{
			puiBuffer[uiPreviousOffset] &= 0x0F;							/* clear sEntry bits on 1st byte */
			puiBuffer[uiPreviousOffset] |= LO8((uint16)uiCluster << 4);		/* set sEntry bits on 1st byte */
		}
		else
		{
			puiBuffer[uiPreviousOffset] = LO8((uint16)uiCluster);			/* just copy the 1st byte */
		}
		mcSectorCache.Dirty(puiBuffer);

		// if the FAT sEntry spans a sector boundary flush the currently
		// loaded sector to the drive and load the next one.
		if (uiPreviousOffset == GetSectorSize() - 1)
		{
			puiBuffer = ReadInfoSector(uiPreviousSector + 1);
			if (puiBuffer == NULL)
			{
				return FAT_CANNOT_READ_MEDIA;
			}

			// set the bNextSectorLoaded flag
			bNextSector = true;
		}

		// write the 2nd byte
		if (uiLastFatEntry & 1)
		{
			puiBuffer[bNextSector ? 0 : (uiPreviousOffset + 1)] = HI8((uint16)uiCluster << 4);
		}
		else
		{
			puiBuffer[bNextSector ? 0 : (uiPreviousOffset + 1)] &= 0xF0;
			puiBuffer[bNextSector ? 0 : (uiPreviousOffset + 1)] |= HI8((uint16)uiCluster);
		}
		mcSectorCache.Dirty(puiBuffer);
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteFat16Entry(uint32 uiOffsetInSector, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiLastFatEntry)
{
	uint8*		puiBuffer;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	*((uint16*)&puiBuffer[uiOffsetInSector]) = FAT16_EOC;
	mcSectorCache.Dirty(puiBuffer);

	if (uiLastFatEntry != FAT16_EOC)
	{
		if (uiPreviousSector != uiSector)
		{
			puiBuffer = ReadInfoSector(uiPreviousSector);
			if (puiBuffer == NULL)
			{
				return FAT_CANNOT_READ_MEDIA;
			}
		}

		*((uint16*)&puiBuffer[uiPreviousOffset]) = (uint16)uiCluster;
		mcSectorCache.Dirty(puiBuffer);
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteFat32Entry(uint32 uiOffsetInSector, uint32 uiLastEntryOffset, uint32 uiClusterIndexInTable, uint32 uiSector, uint32 uiLastSector, fatEntry uiLastFatEntry)
{
	uint8*	puiBuffer;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	*((uint32*)&puiBuffer[uiOffsetInSector]) &= 0xF0000000;
	*((uint32*)&puiBuffer[uiOffsetInSector]) |= FAT32_EOC;
	mcSectorCache.Dirty(puiBuffer);

	// if this is not the 1st cluster allocated update the last one to link to this one
	if (uiLastFatEntry != FAT32_EOC)
	{
		puiBuffer = ReadInfoSector(uiSector);
		if (puiBuffer == NULL)
		{
			return FAT_CANNOT_READ_MEDIA;
		}

		// update the last sEntry to point to this one
		*((uint32*)&puiBuffer[uiLastEntryOffset]) &= 0xF0000000;
		*((uint32*)&puiBuffer[uiLastEntryOffset]) |= uiClusterIndexInTable & 0x0FFFFFFF;
		mcSectorCache.Dirty(puiBuffer);
	}

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::WriteAllocatedFatEntry(uint32 uiClusterIndexInTable, uint32 uiOffsetInSector, uint32 uiSector, fatEntry uiLastFatEntry, uint32 uiLastSector, uint32 uiLastEntryOffset)
{
	// maintain the count of free cluster and the next cluster that may be free.
	SetNextFreeCluster(uiClusterIndexInTable + 1);
	SetTotalFreeClusters(GetTotalFreeClusters() - 1);

	// mark the FAT as the the new 1st link of the cluster chain (or the end of the chain if we're only allocating 1 cluster)
	switch (GetFileSystemType())
	{
	case FAT_FS_TYPE_FAT12:
	{
		return WriteFat12Entry(uiOffsetInSector, uiLastEntryOffset, uiClusterIndexInTable, uiSector, uiLastSector, uiLastFatEntry);
	}
	case FAT_FS_TYPE_FAT16:
	{
		return WriteFat16Entry(uiOffsetInSector, uiLastEntryOffset, uiClusterIndexInTable, uiSector, uiLastSector, uiLastFatEntry);
	}
	case FAT_FS_TYPE_FAT32:
	{
		return WriteFat32Entry(uiOffsetInSector, uiLastEntryOffset, uiClusterIndexInTable, uiSector, uiLastSector, uiLastFatEntry);
	}
	}

	return FAT_UNKNOWN_ERROR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::AllocateCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult)
{
	uint32		uiFirstClusterSector;
	uint32		uiCurrentSector;
	fatEntry	uiCluster;
	uint32		uiOffsetInSector = 0;
	bool		bNextSectorLoaded = false;
	fatEntry	uiLastClusterIndex;
	fatEntry	uiFatEntry;
	uint32		uiFirstEmptyCluster;
	uint32		uiLastClusterSector = 0;
	uint32		uiLastEntryOffset = 0;
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

	uiStartCluster = uiCluster;
	uiLastClusterIndex = GetEndOfClusterMarker();

	CalculateFATIndexAndOffset(&uiOffsetInSector, uiCluster, &uiFirstClusterSector);
	uiLastClusterSector = uiFirstClusterSector;

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

				// if we've found all the clusters that the user requested leave and return the cluster number.
				if (uiClusterCount == 0)
				{
					*peResult = InitialiseAllocatedFatCluster(psParentDirectory, uiCluster, bEraseCluster, uiFirstClusterSector);
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
	uint8*		puiBuffer;
	EFatCode	eResult;

	// if this is the 1st cluster cluster allocated to the file then we must modify the file's entry
	if ((uiCluster == 0))
	{
		// modify the file entry to point to the new cluster
		psEntry->raw.uEntry.sFatRawCommon.uiFirstClusterLowWord = LO16(uiNewClusterInVolume);
		psEntry->raw.uEntry.sFatRawCommon.uiFirstClusterHighWord = HI16(uiNewClusterInVolume);
		psEntry->raw.uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_ARCHIVE;

		// try load the sector that contains the entry
		puiBuffer = ReadInfoSector(psEntry->uiSectorAddress);
		if (puiBuffer)
		{
			return FAT_CANNOT_READ_MEDIA;
		}

		// copy the modified file entry to the sector buffer
		memcpy(puiBuffer + psEntry->uiSectorOffset, &psEntry->raw, sizeof(SFatRawDirectoryEntry));
		SetInfoSectorDirty(puiBuffer);
	}
	else
	{
		// if there are clusters allocated to the file update the last FAT entry
		// of the file to point to the newly allocated cluster(s)

		uint32 uiEndClusterIndex;

		if (uiClustersInFile != 0)
		{
			eResult = IncreaseClusterAddress(uiCluster, uiClustersInFile, &uiEndClusterIndex);
			RETURN_ON_FAT_FAILURE(eResult);
		}
		else
		{
			uiEndClusterIndex = uiEndClusterIndex;
		}

		// set the FAT entry for the last cluster to the beggining of the newly allocated cluster chain (ie. link them).
		eResult = SetClusterEntry(uiEndClusterIndex, uiNewClusterInVolume);
		RETURN_ON_FAT_FAILURE(eResult);
	}
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::SetFat12ClusterEntry(uint32 uiClusterIndex, fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector)
{
	uint8* puiBuffer;

	puiBuffer = ReadInfoSector(uiFirstClusterSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	// write the 1st byte
	if (uiClusterIndex & 1)
	{
		uiClusterInVolume <<= 4;													// odd entries occupy the upper 12 bits so we must shift
		puiBuffer[uiOffsetInSector] &= 0x0F;								// clear sEntry bits on 1st byte
		puiBuffer[uiOffsetInSector] |= LO8((uint16)uiClusterInVolume);	// set sEntry bits on 1st byte
	}
	else
	{
		puiBuffer[uiOffsetInSector] = LO8((uint16)uiClusterInVolume);		// just copy the 1st byte
	}
	mcSectorCache.Dirty(puiBuffer);

	// if the FAT sEntry spans a sector boundary flush the currently
	// loaded sector to the drive and load the next one.
	if (uiOffsetInSector == GetSectorSize() - 1)
	{
		// load the next sector
		puiBuffer = ReadInfoSector(uiFirstClusterSector + 1);
		if (puiBuffer == NULL)
		{
			return FAT_CANNOT_READ_MEDIA;
		}

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
		puiBuffer[uiOffsetInSector] = HI8((uint16)uiClusterInVolume);		/* just copy the 1st byte */
	}
	else
	{
		puiBuffer[uiOffsetInSector] &= 0xF0;						/* clear bits that 1st byte will be written to */
		puiBuffer[uiOffsetInSector] |= HI8((uint16)uiClusterInVolume);	/* copy sEntry bits of 1st byte */
	}
	mcSectorCache.Dirty(puiBuffer);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::SetFat16ClusterEntry(fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector)
{
	uint8* puiBuffer;

	puiBuffer = ReadInfoSector(uiFirstClusterSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	*((uint16*)&puiBuffer[uiOffsetInSector]) = (uint16)uiClusterInVolume;
	mcSectorCache.Dirty(puiBuffer);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::SetFat32ClusterEntry(fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector)
{
	uint8* puiBuffer;

	puiBuffer = ReadInfoSector(uiFirstClusterSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	*((uint32*)&puiBuffer[uiOffsetInSector]) &= 0xF0000000;
	*((uint32*)&puiBuffer[uiOffsetInSector]) |= uiClusterInVolume & 0x0FFFFFFF;
	mcSectorCache.Dirty(puiBuffer);

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

	// set the FAT sEntry
	switch (GetFileSystemType())
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


// increase a cluster address by the amount of clusters indicated by count. This function will
// follow the FAT sEntry chain to fat the count-th cluster allocated to a file relative from the
// input cluster
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::IncreaseFat12ClusterAddress(uint32* puiClusterIndex, uint32* puiSector, uint32* puiOffsetInSector, bool* pbFat12OddClusterBeingProcessed, bool* pbFat12MultiStepProgress)
{
	uint8*	puiBuffer;

	puiBuffer = ReadInfoSector(*puiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	if (!*pbFat12MultiStepProgress)
	{
		*pbFat12OddClusterBeingProcessed = *puiClusterIndex & 1;

		*puiClusterIndex = 0;
		((uint8*)&puiClusterIndex)[0] = puiBuffer[*puiOffsetInSector];
	}

	if (*puiOffsetInSector == GetSectorSize() - 1)
	{
		*puiSector++;
		*puiOffsetInSector = 0;
		*pbFat12MultiStepProgress = true;

		return FAT_SUCCESS;
	}
	else if (!*pbFat12MultiStepProgress)
	{
		*puiOffsetInSector++;
	}

	((uint8*)&puiClusterIndex)[1] = puiBuffer[*puiOffsetInSector];

	if (*pbFat12OddClusterBeingProcessed)
	{
		*puiClusterIndex >>= 4;
	}
	else
	{
		*puiClusterIndex &= 0xFFF;
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
	uint8* puiBuffer;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	*puiClusterIndex = *((uint16*)&puiBuffer[uiOffsetInSector]);
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::IncreaseFat32ClusterAddress(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector)
{
	uint8* puiBuffer;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	*puiClusterIndex = *((uint32*)&puiBuffer[uiOffsetInSector]) & 0x0FFFFFFF;
	return FAT_SUCCESS;
}


// increase a cluster address by the amount of clusters indicated by count. This function will
// follow the FAT entry chain to fat the count-th cluster allocated to a file relative from the
// input cluster
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::IncreaseClusterAddress(uint32 uiClusterIndex, uint16 uiClusterCount, uint32* puiNewClusterIndex)
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

			// read the uiCluster sEntry and mark it as free
			switch (GetFileSystemType())
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
EFatCode CFatVolume::Free(SFatDirectoryEntry* psEntry)
{
	uint32		uiCluster;
	EFatCode	eResult;

	uiCluster = GetFirstClusterFromFatEntry(&psEntry->raw, eFileSystem == FAT_FS_TYPE_FAT32);

	// if the file is not already empty then  we'll empty it
	if (uiCluster != 0)
	{
		// update the entry to point to cluster 0
		psEntry->raw.uEntry.sFatRawCommon.uiFirstClusterLowWord = 0;
		psEntry->raw.uEntry.sFatRawCommon.uiFirstClusterHighWord = 0;
		psEntry->raw.uEntry.sFatRawCommon.uiSize = 0x0;

		// read the sector that contains the entry
		uint8* puiBuffer;

		puiBuffer = ReadInfoSector(psEntry->uiSectorAddress);
		if (puiBuffer == NULL)
		{
			return FAT_CANNOT_READ_MEDIA;
		}

		memcpy(puiBuffer + psEntry->uiSectorOffset, &psEntry->raw, sizeof(SFatRawDirectoryEntry));
		SetInfoSectorDirty(puiBuffer);

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
	switch (GetFileSystemType())
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
uint8* CFatVolume::ReadInfoSector(uint32 uiFatInfoSector, bool bLock)
{
	uint8* pvBuffer;

	pvBuffer = (uint8*)mcSectorCache.ReadSector(uiFatInfoSector);
	if ((pvBuffer != NULL) && bLock)
	{
		mcSectorCache.Lock(pvBuffer);
	}
	return pvBuffer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetInfoSectorDirty(uint8* pvCachedSector)
{
	mcSectorCache.Dirty(pvCachedSector);
}


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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::CalculateRootSector(void)
{
	uint32 uiCurrentCluster;

	if (GetFileSystemType() == FAT_FS_TYPE_FAT32)
	{
		uiCurrentCluster = GetRootCluster();
		return CalculateFirstSectorOfCluster(uiCurrentCluster);
	}
	else
	{
		return GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize());
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CFatVolume::CalculateFirstSectorOfCluster(uint32 uiCluster)
{
	return (((uiCluster - 2) * NumSectorsPerCluster()) + GetFirstDataSector());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::ReadFat12Entry(uint32 uiOffsetInSector, uint32 uiCluster, uint32 uiSector, fatEntry* puiFatEntry)
{
	fatEntry	uiFatEntry;
	bool		bNextSector;
	uint8*		puiBuffer;

	uiFatEntry = 0;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	// read the 1st byte
	((uint8*)&uiFatEntry)[INT32_BYTE0] = puiBuffer[uiOffsetInSector];

	// load the next sector (if necessary) and set the offset for the next byte in the uiBuffer
	if (uiOffsetInSector == GetSectorSize() - 1)
	{
		// load the next sector into the uiBuffer
		puiBuffer = ReadInfoSector(uiSector + 1);
		if (puiBuffer == NULL)
		{
			return FAT_CANNOT_READ_MEDIA;
		}

		// set flag to indicate that we had to load the next sector
		bNextSector = true;
	}
	else
	{
		bNextSector = false;
	}

	// read the 2nd byte
	((uint8*)&uiFatEntry)[INT32_BYTE1] = puiBuffer[bNextSector ? 0 : (uiOffsetInSector + 1)];

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

	*puiFatEntry = uiFatEntry;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::ReadFat16Entry(uint32 uiOffsetInSector, uint32 uiSector, fatEntry* puiFatEntry)
{
	uint8*	puiBuffer;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	*puiFatEntry = *((uint16*)&puiBuffer[uiOffsetInSector]);
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::ReadFat32Entry(uint32 uiOffsetInSector, uint32 uiSector, fatEntry* puiFatEntry)
{
	uint8* puiBuffer;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	*puiFatEntry = *((uint32*)&puiBuffer[uiOffsetInSector]) & 0x0FFFFFFF;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::ReadFatEntry(uint32 uiOffsetInSector, uint32 uiClusterIndex, uint32 uiSector, fatEntry* puiFatEntry)
{
	EFatCode	eResult;

	switch (GetFileSystemType())
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
EFatCode CFatVolume::FreeFat12Chain(bool* pbFat12MultiStepProgress, uint32* puiClusterIndex, uint32* puiSector, uint32* puiOffsetInSector)
{
	uint8* puiBuffer;

	puiBuffer = ReadInfoSector(*puiSector);

	if (!*pbFat12MultiStepProgress)
	{
		// remember whether this is an odd cluster or not
		*pbFat12MultiStepProgress = *puiClusterIndex & 1;

		// set the cluster to zero to make sure that the upper bytes are cleared
		// since we're only updating the lower 16 bits.
		*puiClusterIndex = 0;

		// read the 1st byte
		((uint8*)puiClusterIndex)[0] = puiBuffer[*puiOffsetInSector];

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
		SetInfoSectorDirty(puiBuffer);
	}

	if (*puiOffsetInSector == GetSectorSize() - 1)
	{
		// if the sEntry spans a sector boundary set bFat12MultiStepProgress to 1
		// so that we don't read the 1st byte again when we come back.
		// also increase the sector number and set the uiOffsetInSector to 0 since
		// the next byte will be on offset zero when the next sector is loaded
		*puiSector++;
		*puiOffsetInSector = 0;
		*pbFat12MultiStepProgress = true;

		// continue with the next iteration of the loop. We'll come right back
		// here with the next sector loaded
		return FAT_SUCCESS;
	}
	else if (!*pbFat12MultiStepProgress)
	{
		// increase the offset to point to the next byte
		*puiOffsetInSector++;
	}

	// read the 2nd byte
	((uint8*)puiClusterIndex)[1] = puiBuffer[*puiOffsetInSector];

	// Since a FAT12 sEntry is only 12 bits (1.5 bytes) we need to adjust the result.
	// For odd cluster numbers the FAT sEntry is stored in the upper 12 bits of the
	// 16 bits where it is stored, so we need to shift the value 4 bits to the right.
	// For even cluster numbers the FAT sEntry is stored in the lower 12 bits of the
	// 16 bits where it is stored, so we need to clear the upper 4 bits.
	if (*pbFat12MultiStepProgress)
	{
		*puiClusterIndex >>= 4;
	}
	else
	{
		*puiClusterIndex &= 0xFFF;
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
	SetInfoSectorDirty(puiBuffer);

	// clear bFat12MultiStepProgress flag.
	*pbFat12MultiStepProgress = false;
	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FreeFat16Chain(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector)
{
	uint8* puiBuffer;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	*puiClusterIndex = *((uint16*)&puiBuffer[uiOffsetInSector]);
	*((uint16*)&puiBuffer[uiOffsetInSector]) = FREE_FAT;
	SetInfoSectorDirty(puiBuffer);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FreeFat32Chain(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector)
{
	uint8* puiBuffer;

	puiBuffer = ReadInfoSector(uiSector);
	if (puiBuffer == NULL)
	{
		return FAT_CANNOT_READ_MEDIA;
	}

	// FAT32 entries are actually 28 bits so we need to leave the  upper nibble untouched.
	*puiClusterIndex = *((uint32*)&puiBuffer[uiOffsetInSector]) & 0x0FFFFFFF;
	*((uint32*)&puiBuffer[uiOffsetInSector]) &= 0xF0000000;
	SetInfoSectorDirty(puiBuffer);

	return FAT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FreeChain(uint32 uiClusterIndex)
{
	EFatCode	eResult;
	uint32		uiOffsetInSector;							// the offset of the cluster sEntry within it's sector
	uint32		uiFirstClusterSector;						// the sector where the sEntry is stored on the drive
	uint32		uiCurrentSector;							// the sector that's currently loaded in memory
	bool		bFat12OddClusterBeingProcessed = false;		// indicates that the sEntry being processed is an odd cluster address (FAT12 only)
	bool		bFat12MultiStepProgress = false;			// indicates that a multi-step operation is in progress (FAT12 only)
	uint8*		puiBuffer;

	// get the offset of the cluster sEntry within the FAT table,
	// the sector of the FAT table that contains the sEntry and the offset
	// of the fat sEntry within the sector
	CalculateFATIndexAndOffset(&uiOffsetInSector, uiClusterIndex, &uiFirstClusterSector);

	// loop until we reach the EOC cluster or an error occurs.
	for (;;)
	{
		puiBuffer = ReadInfoSector(uiFirstClusterSector);

		// store the address of the sector that's (will be) loaded in memory
		uiCurrentSector = uiFirstClusterSector;

		// loop until a new sector needs to be loaded to continue
		// with the operation
		while (uiCurrentSector == uiFirstClusterSector)
		{
			// if cluster is less than 2 either we got a bug
			// or the file system is corrupted
			if (uiClusterIndex < 2)
			{
				// leave critical section and return error code
				return FAT_INVALID_CLUSTER;
			}

			// read the cluster sEntry and mark it as free
			switch (GetFileSystemType())
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
	switch (eFileSystem)
	{
		case FAT_FS_TYPE_FAT32:
			return (uifat & 0x0FFFFFFF) == 0;

		case FAT_FS_TYPE_FAT16:
			return (uifat & 0xFFFF) == 0;

		case FAT_FS_TYPE_FAT12:
			return (uifat & 0x0FFF) == 0;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetNextFreeCluster(uint32 uiCluster)
{
	uiNextFreeCluster = uiCluster;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatVolume::SetTotalFreeClusters(uint32 uiTotalFreeClusters)
{
	uiTotalFreeClusters = uiTotalFreeClusters;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::CreateFakeRootEntry(SFatDirectoryEntry* psEntry)
{
	uint32	uiRootCluster;

	// copy the file name to the psEntry and the raw
	// psEntry in their respective formats
	strcpy((char*)psEntry->name, "ROOT");
	GetShortNameForEntry((uint8*)psEntry->raw.uEntry.sFatRawCommon.szShortName, psEntry->name, 1);

	// set the general fields of the psEntry
	psEntry->attributes = psEntry->raw.uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_DIRECTORY;
	psEntry->size = psEntry->raw.uEntry.sFatRawCommon.uiSize = 0x0;

	// since the psEntry does not physically exist the
	// address fields are set to zero as well
	psEntry->uiSectorAddress = 0;
	psEntry->uiSectorOffset = 0;

	uiRootCluster = GetRootCluster();
	psEntry->raw.uEntry.sFatRawCommon.uiFirstClusterLowWord = LO16(uiRootCluster);
	psEntry->raw.uEntry.sFatRawCommon.uiFirstClusterHighWord = HI16(uiRootCluster);

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


// converts the filename to it's Unicode UTF16 representation
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char GetLongNameForEntry(uint16* puiDest, uint8* puiSource)
{
	register int i;
	for (i = 0; i < (int)strlen((char*)puiSource); i++)
	{
		puiDest[i] = (uint16)puiSource[i];
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

	if (GetShortNameForEntry((uint8*)szConstructedShortFileName, (uint8*)szCurrentLevelPath, 1) == FAT_INVALID_FILENAME)
	{
		if (GetLongNameForEntry(puiTargetFileLongName, (uint8*)szCurrentLevelPath) == FAT_INVALID_FILENAME)
		{
			return FAT_INVALID_FILENAME;
		}
		bLongFilename = true;
		bMatch = FatCompareLongName(puiTargetFileLongName, psQuery->long_filename) || FatCompareShortName(szConstructedShortFileName, (char*)psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
	}
	else
	{
		bMatch = FatCompareShortName(szConstructedShortFileName, (char*)psQuery->sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
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
EFatCode CFatVolume::GetShortNameForEntry(uint8* puiDest, uint8* puiSource, bool bLFNDisabled)
{
	char		szName[13];
	bool		bUppercase = false;
	uint16		uiDotIndex;
	uint16		uiLength;
	uint16		i;

	// check that the name is actually a long filename
	// before processing it as such
	if (!bLFNDisabled)
	{
		uint8	c;
		bool	bLFN = false;

		uiLength = (uint16)strlen((char*)puiSource);
		uiDotIndex = FatIndexOf('.', (char*)puiSource, 0);

		// if the file hs no extension and is longer than 8 chars
		// or if the name part has more than 8 chars or the extension more than 8
		// or if it has more than one dot then we need to handle it as a lfn
		if (uiDotIndex < 0 && uiLength > 8) bLFN = true;
		if (uiDotIndex >= 0)
		{
			if (uiDotIndex > 7 || (uiLength - uiDotIndex) > 4)
			{
				bLFN = true;
			}
			if (uiDotIndex >= 0)
			{
				if (FatIndexOf('.', (char*)puiSource, 1) >= 0)
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
				if (puiSource[i] == ' ' || puiSource[i] != toupper(puiSource[i]))
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
				if (puiSource[i] == '.')
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
					if (puiSource[c] == ' ' || puiSource[c] == '.')
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
					szName[i] = toupper(puiSource[c++]);
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
					if (puiSource[c] == ' ' || puiSource[c] == '.')
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
					szName[i] = toupper(puiSource[c++]);
				}
				else
				{
					szName[i] = ' ';
				}
			}

			// now we copy it to the callers buffer and we're done
			for (i = 0; i < 11; i++)
			{
				*puiDest++ = szName[i];
			}

			// return special code so the caller knows to store the long name.
			return FAT_LFN_GENERATED;
		}
	}

	// trim-off spaces - if the result is greater than 12 it will return an empty string.
	TrimPath(szName, (char*)puiSource, 12);

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

				*puiDest++ = toupper(szName[i]);
			}
			else
			{
				*puiDest++ = ' ';
			}
		}
		else
		{
			if (i < uiDotIndex - 1)
			{
				if (bLFNDisabled && (szName[i] != toupper(szName[i])))
					bUppercase = true;

				*puiDest++ = toupper(szName[i]);
			}
			else
			{
				*puiDest++ = ' ';
			}
		}
	}

	// if there's not extension fill the extension characters with spaces.
	if (uiDotIndex == 0)
	{
		for (i = 0; i < 3; i++)
			*puiDest++ = ' ';
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
				*puiDest++ = toupper(szName[i]);
			}
			else
			{
				*puiDest++ = ' ';
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

		// try to get the 1st sEntry of the
		// query results
		eResult = QueryFirstEntry(&sParentEntry.raw, uiAttributes, &psQuery->sQueryState, 0);
	}
	// if the parent was not supplied then we
	// submit the query without it
	else
	{
		eResult = QueryFirstEntry(0, uiAttributes, &psQuery->sQueryState, 0);
	}

	// if we cant get the 1st sEntry then return the
	// error that we received from fat_Query_First_entry
	RETURN_ON_FAT_FAILURE(eResult);

	// if there are no more entries
	if (psQuery->sQueryState.sCurrentEntryRaw == 0)
	{
		*psQuery->sCurrentEntry.name = 0;
		return FAT_SUCCESS;
	}

	// fill the current entry structure with data from
	// the current raw entry of the query
	FillDirectoryEntryFromRawEntry(&psQuery->sCurrentEntry, psQuery->sQueryState.sCurrentEntryRaw);

	// calculate the sector address of the entry - if query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is different.
	if (psQuery->sQueryState.uiCurrentCluster == 0)
	{
		psQuery->sCurrentEntry.uiSectorAddress = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize()) + psQuery->sQueryState.uiCurrentSector;
	}
	else
	{
		psQuery->sCurrentEntry.uiSectorAddress = CalculateFirstSectorOfCluster(psQuery->sQueryState.uiCurrentCluster) + psQuery->sQueryState.uiCurrentSector;
	}

	// calculate the offset of the entry within it's sector
	psQuery->sCurrentEntry.uiSectorOffset = (uint16)((uintptr_t)psQuery->sQueryState.sCurrentEntryRaw) - ((uintptr_t)psQuery->sQueryState.auiBuffer);

	// store a copy of the original FAT directory entry within the SFatDirectoryEntry structure that is returned to users
	psQuery->sCurrentEntry.raw = *psQuery->sQueryState.sCurrentEntryRaw;

	// if long filenames are enabled copy the filename to the entry
	if (*psQuery->sCurrentEntry.name != 0)
	{
		if (*psQuery->sQueryState.long_filename != 0)
		{
			uint16	uiIndex;
			for (uiIndex = 0; uiIndex < 256; uiIndex++)
			{
				psQuery->sCurrentEntry.name[uiIndex] = (uint8)psQuery->sQueryState.long_filename[uiIndex];
				if (psQuery->sQueryState.long_filename[uiIndex] == 0)
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
	EFatCode				uiResult;

	// try to get the next entry of the query.
	uiResult = QueryNextEntry(&psQuery->sQueryState, false, false);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if there are no more entries
	if (psQuery->sQueryState.sCurrentEntryRaw == 0)
	{
		// set the filename of the current sEntry to 0.
		*psQuery->sCurrentEntry.name = 0;
		return FAT_SUCCESS;
	}

	// fill the current sEntry structure with data from the current raw sEntry of the query.
	FillDirectoryEntryFromRawEntry(&psQuery->sCurrentEntry, psQuery->sQueryState.sCurrentEntryRaw);

	// calculate the sector address of the sEntry - if query->CurrentCluster equals zero then this is the root
	// directory of a FAT12/FAT16 volume and the calculation is different.
	if (psQuery->sQueryState.uiCurrentCluster == 0x0)
	{
		psQuery->sCurrentEntry.uiSectorAddress = GetNoOfReservedSectors() + (GetNoOfFatTables() * GetFatSize()) + psQuery->sQueryState.uiCurrentSector;
	}
	else
	{
		psQuery->sCurrentEntry.uiSectorAddress = CalculateFirstSectorOfCluster(psQuery->sQueryState.uiCurrentCluster) + psQuery->sQueryState.uiCurrentSector;
	}

	// calculate the offset of the sEntry within it's sector
	psQuery->sCurrentEntry.uiSectorOffset = (uint16)((uintptr_t)psQuery->sQueryState.sCurrentEntryRaw) - ((uintptr_t)psQuery->sQueryState.auiBuffer);

	// store a copy of the original FAT directory sEntry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	psQuery->sCurrentEntry.raw = *psQuery->sQueryState.sCurrentEntryRaw;

	if (*psQuery->sCurrentEntry.name != 0)
	{
		if (*psQuery->sQueryState.long_filename != 0)
		{
			uint16 uiIndex;
			for (uiIndex = 0; uiIndex < 256; uiIndex++)
			{
				psQuery->sCurrentEntry.name[uiIndex] = (uint8)psQuery->sQueryState.long_filename[uiIndex];
				if (psQuery->sQueryState.long_filename[uiIndex] == 0)
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
	ConvertFATShortInternalNameInto8Dot3Format(psEntry->name, (uint8*)psRawEntry->uEntry.sFatRawCommon.szShortName);

	// copy other data from the internal entry structure to the public one
	psEntry->attributes = psRawEntry->uEntry.sFatRawCommon.uiAttributes;
	psEntry->size = psRawEntry->uEntry.sFatRawCommon.uiSize;
	psEntry->uiCreateTime = FatDecodeDateTime(psRawEntry->uEntry.sFatRawCommon.uiCreateDate, psRawEntry->uEntry.sFatRawCommon.uiCreateTime);
	psEntry->uiModifyTime = FatDecodeDateTime(psRawEntry->uEntry.sFatRawCommon.uiModifyDate, psRawEntry->uEntry.sFatRawCommon.uiModifyTime);
	psEntry->access_time = FatDecodeDateTime(psRawEntry->uEntry.sFatRawCommon.uiAccessDate, 0);
	psEntry->raw = *psRawEntry;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::CreateFATEntry(SFatRawDirectoryEntry* psParentDirectory, char* szName, uint8 uiAttributes, uint32 uiEntryCluster, SFatDirectoryEntry* psNewEntry)
{
	EFatCode				eResult;
	uint16					uiLength;
	int16					char_index;
	uint16					illegal_char;
	uint16					uiDirectoryEntries = 0;
	uint32					uiSector;
	uint32					uiFirstSectorOfCluster = 0;
	fatEntry				uiFat;
	fatEntry				uiLastFat;
	SFatRawDirectoryEntry*	psParentEntry;

	int						iLFNEntriesNeeded = 0;
	int						iLFNEntriesFound;
	char					uiChecksum;
	uint8*					puiBuffer;
	uintptr_t				puiLastEntryAddress;

	// get the length of the filename
	uiLength = (uint16)strlen(szName);

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
	char_index = FatIndexOf('.', szName, 0);

	if (char_index == 0 || char_index == (uiLength - 1))
	{
		return FAT_INVALID_FILENAME;
	}

	for (char_index = 0x0; char_index < uiLength; char_index++)
	{
		// if the character is less than 0x20 with the exception of 0x5 then the filename is illegal.
		if (szName[char_index] < 0x1F)
		{
			return FAT_ILLEGAL_FILENAME;
		}

		// compare the character with a table of illegal characters, if a match is found then the filename is illegal.
		for (illegal_char = 0x0; illegal_char < ILLEGAL_CHARS_COUNT; illegal_char++)
		{
			if (szName[char_index] == ILLEGAL_CHARS[illegal_char] && szName[char_index] != '.')
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
	eResult = GetShortNameForEntry((uint8*)psNewEntry->raw.uEntry.sFatRawCommon.szShortName, (uint8*)szName, false);

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
			eResult = QueryFirstEntry(psParentDirectory, 0, &query, 0);
			RETURN_ON_FAT_FAILURE(eResult);

			sprintf(name_suffix_str, "~%i", name_suffix);

			for (i = 0; i < 8 - (char)strlen(name_suffix_str); i++)
			{
				if (psNewEntry->raw.uEntry.sFatRawCommon.szShortName[i] == 0x20)
				{
					break;
				}
			}

			for (c = 0; c < (char)strlen(name_suffix_str); c++)
			{
				psNewEntry->raw.uEntry.sFatRawCommon.szShortName[i++] = name_suffix_str[c];
			}

			// loop through all entries in the parent directory
			// and if we find one with the same name as hours mark the name
			// as invalid
			while (*query.sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName != 0)
			{
				if (memcmp(query.sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName, psNewEntry->raw.uEntry.sFatRawCommon.szShortName, 11) == 0)
				{
					is_valid_entry = 0;
					break;
				}
				eResult = QueryNextEntry(&query, false, false);
				RETURN_ON_FAT_FAILURE(eResult);
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
					if (psNewEntry->raw.uEntry.sFatRawCommon.szShortName[i] == 0x20)
					{
						break;
					}
				}

				for (c = 0; c < (char)strlen(name_suffix_str); c++)
				{
					psNewEntry->raw.uEntry.sFatRawCommon.szShortName[i++] = name_suffix_str[c];
				}
			}
		} while (!is_valid_entry);

		// calculate the # of entries needed to store the lfn
		// including the actual sEntry
		iLFNEntriesNeeded = ((strlen(szName) + 12) / 13) + 1;
		iLFNEntriesFound = 0;
	}

	// if the new sEntry is a directory and no cluster was supplied
	// by the calling function then allocate a new cluster
	if (uiEntryCluster == 0 && (uiAttributes & FAT_ATTR_DIRECTORY))
	{
		uiEntryCluster = AllocateDirectoryCluster(psParentDirectory, &eResult);
		RETURN_ON_FAT_FAILURE(eResult);
	}

	// set the sEntry attributes
	strcpy((char*)psNewEntry->name, szName);
	psNewEntry->attributes = uiAttributes;
	psNewEntry->size = 0x0;
	psNewEntry->raw.uEntry.sFatRawCommon.uiAttributes = uiAttributes;
	psNewEntry->raw.uEntry.sFatRawCommon.uiReserved = 0;
	psNewEntry->raw.uEntry.sFatRawCommon.uiSize = 0x0;
	psNewEntry->raw.uEntry.sFatRawCommon.uiFirstClusterLowWord = LO16(uiEntryCluster);
	psNewEntry->raw.uEntry.sFatRawCommon.uiFirstClusterHighWord = HI16(uiEntryCluster);
	psNewEntry->raw.uEntry.sFatRawCommon.uiCreateTimeTenths = 0x0;
	psNewEntry->raw.uEntry.sFatRawCommon.uiCreateDate = GetSystemClockDate();
	psNewEntry->raw.uEntry.sFatRawCommon.uiCreateTime = GetSystemClockTime();
	psNewEntry->raw.uEntry.sFatRawCommon.uiModifyDate = psNewEntry->raw.uEntry.sFatRawCommon.uiCreateDate;
	psNewEntry->raw.uEntry.sFatRawCommon.uiModifyTime = psNewEntry->raw.uEntry.sFatRawCommon.uiCreateTime;
	psNewEntry->raw.uEntry.sFatRawCommon.uiAccessDate = psNewEntry->raw.uEntry.sFatRawCommon.uiCreateDate;
	psNewEntry->uiCreateTime = FatDecodeDateTime(psNewEntry->raw.uEntry.sFatRawCommon.uiCreateDate, psNewEntry->raw.uEntry.sFatRawCommon.uiCreateTime);
	psNewEntry->uiModifyTime = FatDecodeDateTime(psNewEntry->raw.uEntry.sFatRawCommon.uiModifyDate, psNewEntry->raw.uEntry.sFatRawCommon.uiModifyTime);
	psNewEntry->access_time = FatDecodeDateTime(psNewEntry->raw.uEntry.sFatRawCommon.uiAccessDate, 0);

	// there's no fat entry that points to the 1st cluster of a directory's cluster chain but we'll create a fake fat sEntry from the 1st cluster data on the
	// directory sEntry so that we can handle the 1st cluster with the same code as all other clusters in the chain.
	if (psParentDirectory && (psParentDirectory->uEntry.sFatRawCommon.uiFirstClusterLowWord != 0 || psParentDirectory->uEntry.sFatRawCommon.uiFirstClusterHighWord != 0))
	{
		uiFat = GetFirstClusterFromFatEntry(psParentDirectory, GetFileSystemType() == FAT_FS_TYPE_FAT32);
	}

	// if no parent was specified then we create the fake fat sEntry from the root directory's cluster address found on the volume structure
	else
	{
		uiFat = GetRootCluster();
		uiFirstSectorOfCluster = GetRootSector();
	}

	for (;;)
	{
		if (uiFat != 0)
		{
			uiFirstSectorOfCluster = CalculateFirstSectorOfCluster(uiFat);
		}

		// set the current sector to the first
		// sector of the cluster
		uiSector = uiFirstSectorOfCluster;

		// for each sector in the cluster
		while (uiFat == 0 || uiSector < (uiFirstSectorOfCluster + NumSectorsPerCluster()))
		{
			// read the current sector to RAM
			puiBuffer = ReadInfoSector(uiSector);
			if (puiBuffer == NULL)
			{
				return FAT_CANNOT_READ_MEDIA;
			}
			puiLastEntryAddress = ((uintptr_t)puiBuffer + GetSectorSize()) - 0x20;

			// set the parent sEntry pointer to the 1st sEntry of the current sector.
			psParentEntry = (SFatRawDirectoryEntry*)puiBuffer;

			// for each directory entry in the sector...
			while ((uintptr_t)psParentEntry <= puiLastEntryAddress)
			{
				// make sure we don't exceed the limit of 0xFFFF entries per directory.
				if ((uiDirectoryEntries + iLFNEntriesNeeded) == 0xFFFF)
				{
					return FAT_DIRECTORY_LIMIT_EXCEEDED;
				}

				uiDirectoryEntries++;

				// if the directory entry is free.
				if (IS_FREE_DIRECTORY_ENTRY(psParentEntry))
				{
					// we've found a free sEntry
					iLFNEntriesFound++;

					// if this is the last directory sEntry or if we've found all the entries that we need then let's get ready to write them.
					if (IS_LAST_DIRECTORY_ENTRY(psParentEntry) || iLFNEntriesFound == iLFNEntriesNeeded)
					{
						// if there where any free entries before this one then we need to rewind a bit.
						while (iLFNEntriesFound-- > 1)
						{
							if ((uintptr_t)psParentEntry > (uintptr_t)puiBuffer)
							{
								psParentEntry--;
							}
							else
							{
								// if the last sEntry is on the same cluster we
								// can just decrease the sector number, otherwise we
								// need to get the sector address for the last cluster
								if (uiSector > uiFirstSectorOfCluster)
								{
									uiSector--;
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
									uiSector = uiFirstSectorOfCluster + NumSectorsPerCluster();
								}

								// read the last sector to the cache, calculate the last entry address and set our pointer to it
								puiBuffer = ReadInfoSector(uiSector);
								if (puiBuffer == NULL)
								{
									return FAT_CANNOT_READ_MEDIA;
								}
								puiLastEntryAddress = ((uintptr_t)puiBuffer + GetSectorSize()) - 0x20;

								psParentEntry = (SFatRawDirectoryEntry*)((uintptr_t)puiBuffer + GetSectorSize()) - 0x20;
							}
						}

						// compute the checksum for this sEntry
						uiChecksum = FatLongEntryChecksum((uint8*)psNewEntry->raw.uEntry.sFatRawCommon.szShortName);

						// now we can start writting
						iLFNEntriesFound = iLFNEntriesNeeded;
						while (iLFNEntriesFound--)
						{
							if (iLFNEntriesFound)
							{
								uint16 i, c;

								// set the required fields for this sEntry
								psParentEntry->uEntry.sFatRawLongFileName.uiSequence = (uint8)iLFNEntriesFound;
								psParentEntry->uEntry.sFatRawLongFileName.uiChecksum = uiChecksum;
								psParentEntry->uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_LONG_NAME;
								psParentEntry->uEntry.sFatRawLongFileName.uiFirstCluster = 0;
								psParentEntry->uEntry.sFatRawLongFileName.uiType = 0;

								// mark sEntry as the 1st sEntry if it is so
								if (iLFNEntriesFound == iLFNEntriesNeeded - 1)
								{
									psParentEntry->uEntry.sFatRawLongFileName.uiSequence = psParentEntry->uEntry.sFatRawLongFileName.uiSequence | FAT_FIRST_LFN_ENTRY;
								}

								// copy the lfn chars
								c = (uint16)strlen(szName);
								i = ((iLFNEntriesFound - 1) * 13);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x0] = LO8((i + 0x0 > c) ? 0xFFFF : (uint16)szName[i + 0x0]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x1] = HI8((i + 0x0 > c) ? 0xFFFF : (uint16)szName[i + 0x0]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x2] = LO8((i + 0x1 > c) ? 0xFFFF : (uint16)szName[i + 0x1]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x3] = HI8((i + 0x1 > c) ? 0xFFFF : (uint16)szName[i + 0x1]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x4] = LO8((i + 0x2 > c) ? 0xFFFF : (uint16)szName[i + 0x2]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x5] = HI8((i + 0x2 > c) ? 0xFFFF : (uint16)szName[i + 0x2]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x6] = LO8((i + 0x3 > c) ? 0xFFFF : (uint16)szName[i + 0x3]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x7] = HI8((i + 0x3 > c) ? 0xFFFF : (uint16)szName[i + 0x3]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x8] = LO8((i + 0x4 > c) ? 0xFFFF : (uint16)szName[i + 0x4]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars1[0x9] = HI8((i + 0x4 > c) ? 0xFFFF : (uint16)szName[i + 0x4]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x0] = LO8((i + 0x5 > c) ? 0xFFFF : (uint16)szName[i + 0x5]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x1] = HI8((i + 0x5 > c) ? 0xFFFF : (uint16)szName[i + 0x5]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x2] = LO8((i + 0x6 > c) ? 0xFFFF : (uint16)szName[i + 0x6]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x3] = HI8((i + 0x6 > c) ? 0xFFFF : (uint16)szName[i + 0x6]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x4] = LO8((i + 0x7 > c) ? 0xFFFF : (uint16)szName[i + 0x7]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x5] = HI8((i + 0x7 > c) ? 0xFFFF : (uint16)szName[i + 0x7]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x6] = LO8((i + 0x8 > c) ? 0xFFFF : (uint16)szName[i + 0x8]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x7] = HI8((i + 0x8 > c) ? 0xFFFF : (uint16)szName[i + 0x8]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x8] = LO8((i + 0x9 > c) ? 0xFFFF : (uint16)szName[i + 0x9]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0x9] = HI8((i + 0x9 > c) ? 0xFFFF : (uint16)szName[i + 0x9]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0xA] = LO8((i + 0xA > c) ? 0xFFFF : (uint16)szName[i + 0xA]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars2[0xB] = HI8((i + 0xA > c) ? 0xFFFF : (uint16)szName[i + 0xA]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars3[0x0] = LO8((i + 0xB > c) ? 0xFFFF : (uint16)szName[i + 0xB]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars3[0x1] = HI8((i + 0xB > c) ? 0xFFFF : (uint16)szName[i + 0xB]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars3[0x2] = LO8((i + 0xC > c) ? 0xFFFF : (uint16)szName[i + 0xC]);
								psParentEntry->uEntry.sFatRawLongFileName.auiChars3[0x3] = HI8((i + 0xC > c) ? 0xFFFF : (uint16)szName[i + 0xC]);
								mcSectorCache.Dirty(puiBuffer);

								// continue to next sEntry
								if ((uintptr_t)psParentEntry < (uintptr_t)puiBuffer)
								{
									psParentEntry++;
								}
								else
								{
									if (uiFat == 0 || uiSector < uiFirstSectorOfCluster + NumSectorsPerCluster() - 1)
									{
										uiSector++;

										// make sure that we don't overflow the root directory on FAT12/16 volumes
										if (!uiFat)
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
										uiLastFat = uiFat;
										eResult = GetNextClusterEntry(uiFat, &uiFat);
										RETURN_ON_FAT_FAILURE(eResult);

										// if this is the end of the FAT chain allocate
										// a new cluster to this folder and continue
										if (FatIsEOFEntry(uiFat))
										{
											fatEntry uiNewFat = AllocateDataCluster(1, 1, &eResult);
											RETURN_ON_FAT_FAILURE(eResult);

											eResult = SetClusterEntry(uiLastFat, uiNewFat);
											RETURN_ON_FAT_FAILURE(eResult);

											uiFat = uiNewFat;
										}

										// continue working on the new cluster
										uiSector = CalculateFirstSectorOfCluster(uiFat);
									}

									// load the next sector
									puiBuffer = ReadInfoSector(uiSector);
									if (puiBuffer == NULL)
									{
										return FAT_CANNOT_READ_MEDIA;
									}
									puiLastEntryAddress = ((uintptr_t)puiBuffer + GetSectorSize()) - 0x20;

									psParentEntry = (SFatRawDirectoryEntry*)puiBuffer;
								}
							}
							else
							{
								*psParentEntry = psNewEntry->raw;
							}
						}

						psNewEntry->uiSectorAddress = uiSector;
						psNewEntry->uiSectorOffset = (uintptr_t)psParentEntry - (uintptr_t)puiBuffer;

						return FAT_SUCCESS;
					}
				}
				else
				{
					iLFNEntriesFound = 0;
				}

				// move the parent sEntry pointer to the next sEntry in the sector.
				psParentEntry++;
			}

			// move to the next sector in the cluster.
			uiSector++;

			// make sure that we don't overflow the root directory
			// on FAT12/16 volumes
			if (!uiFat)
			{
				if (uiSector > uiFirstSectorOfCluster + GetRootDirectorySectors())
				{
					return FAT_ROOT_DIRECTORY_LIMIT_EXCEEDED;
				}
			}
		}

		// get the next cluster, we'll remember the last one in
		// case we need to rewind to write lfn entries
		uiLastFat = uiFat;
		eResult = GetNextClusterEntry(uiFat, &uiFat);
		RETURN_ON_FAT_FAILURE(eResult);

		// if this is the end of the FAT chain allocate
		// a new cluster to this folder and continue
		if (FatIsEOFEntry(uiFat))
		{
			fatEntry uiNewFat;

			uiNewFat = AllocateDataCluster(1, 1, &eResult);
			RETURN_ON_FAT_FAILURE(eResult);

			eResult = SetClusterEntry(uiLastFat, uiNewFat);
			RETURN_ON_FAT_FAILURE(eResult);

			uiFat = uiNewFat;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode CFatVolume::FatCreateDirectory(char* szDirectory)
{
	EFatCode			uiResult;
	SFatDirectoryEntry	sEntry;

	// check that we got a valid pathname
	if (!szDirectory || strlen(szDirectory) > FAT_MAX_PATH)
	{
		return FAT_INVALID_FILENAME;
	}

	// try get the file sEntry
	uiResult = GetFileEntry(szDirectory, &sEntry);
	if (uiResult != FAT_SUCCESS)
	{
		return uiResult;
	}

	// if we don'tfind a file or directory by that name
	// we can create it, otherwise return file already exists error
	if (*sEntry.name == 0)
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

		if (*sParentEntry.name == 0)
		{
			return FAT_DIRECTORY_DOES_NOT_EXIST;
		}

		// try to create the directory sEntry
		return CreateFATEntry(&sParentEntry.raw, szPathScanner, FAT_ATTR_DIRECTORY, 0, &sEntry);
	}

	// if we get here it means that a file or
	// directory with that name already exists.
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
	do
	{
		szPath = FindNextPathItem(szPath, szCurrentLevelPath);
		if (szPath == NULL)
		{
			return FAT_INVALID_FILENAME;
		}

		// try to find the first psEntry
		eResult = QueryFirstEntry(psCurrentEntry, 0, &sQuery, true);
		RETURN_ON_FAT_FAILURE(eResult);

		// if the output of FatQueryFirstEntry indicates that there are no entries available...
		if (*sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName == 0)
		{
			psEntry->name[0] = '\0';
			return FAT_SUCCESS;
		}

		eResult = MatchesFileName(&bMatch, &bLongFilename, szConstructedShortFileName, auiConstructedLongFilename, szCurrentLevelPath, &sQuery);
		RETURN_ON_FAT_FAILURE(eResult);

		// if the file doesn't match then get the next file.
		while (!bMatch)
		{
			//  try to get the next file.
			eResult = QueryNextEntry(&sQuery, true, false);
			RETURN_ON_FAT_FAILURE(eResult);

			// if the output of FatQueryFirstEntry indicates that  there are no entries available then set the psEntry name to 0 and return success.
			if (IS_LAST_DIRECTORY_ENTRY(sQuery.sCurrentEntryRaw))
			{
				psEntry->name[0] = '\0';
				return FAT_SUCCESS;
			}

			// match the filename against the next psEntry.
			if (bLongFilename)
			{
				bMatch = FatCompareLongName(auiConstructedLongFilename, sQuery.long_filename) || FatCompareShortName(szConstructedShortFileName, (char*)sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
			}
			else
			{
				bMatch = FatCompareShortName(szConstructedShortFileName, (char*)sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);
			}
		}

		// set the current entry to the entry that we've just found
		psCurrentEntry = sQuery.sCurrentEntryRaw;
	} 
	while (szPath[0] != '\0');

	// copy the filename and transform the filename
	// from the internal structure to the public one
	ConvertFATShortInternalNameInto8Dot3Format(psEntry->name, (uint8*)sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.szShortName);

	// copy other data from the internal psEntry structure
	// to the public one
	psEntry->attributes = sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.uiAttributes;
	psEntry->size = sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.uiSize;
	psEntry->uiCreateTime = FatDecodeDateTime(sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.uiCreateDate, sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.uiCreateTime);
	psEntry->uiModifyTime = FatDecodeDateTime(sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.uiModifyDate, sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.uiModifyTime);
	psEntry->access_time = FatDecodeDateTime(sQuery.sCurrentEntryRaw->uEntry.sFatRawCommon.uiAccessDate, 0);

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
	psEntry->uiSectorOffset = (uint16)((uintptr_t)sQuery.sCurrentEntryRaw) - ((uintptr_t)sQuery.auiBuffer);

	// store a copy of the original FAT directory psEntry
	// within the SFatDirectoryEntry structure that is returned
	// to users
	psEntry->raw = *sQuery.sCurrentEntryRaw;
	return FAT_SUCCESS;
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
uint32 CFatVolume::GetClusterSize(void) { return uiBytesPerCluster; }
uint32 CFatVolume::GetID(void) { return uiID; }
uint32 CFatVolume::GetFatSize(void) { return uiFatSize; }
uint32 CFatVolume::GetRootCluster(void) { return uiRootCluster; }
uint32 CFatVolume::GetRootSector(void) { return uiRootSector; }
uint32 CFatVolume::GetFirstDataSector(void) { return uiFirstDataSector; }
uint32 CFatVolume::GetNoOfSectors(void) { return uiNoOfSectors; }
uint32 CFatVolume::GetNoOfDataSectors(void) { return uiNoOfDataSectors; }
uint32 CFatVolume::GetNumClusters(void) { return uiNoOfClusters; }
uint32 CFatVolume::GetNoOfReservedSectors(void) { return uiNoOfReservedSectors; }
uint32 CFatVolume::GetNextFreeCluster(void) { return uiNextFreeCluster; }
uint32 CFatVolume::GetTotalFreeClusters(void) { return uiTotalFreeClusters; }
uint32 CFatVolume::GetFileSystemInfoSector(void) { return uiFileSystemInfoSector; }
uint16 CFatVolume::GetRootDirectorySectors(void) { return uiRootDirectorySectors; }
uint16 CFatVolume::GetSectorSize(void) { return uiBytesPerSector; }
uint16 CFatVolume::NumSectorsPerCluster(void) { return uiNoOfSectorsPerCluster; }
bool CFatVolume::IsUseLongFilenames(void) { return bUseLongFilenames; }
EFatFileSystemType CFatVolume::GetFileSystemType(void) { return eFileSystem; }
uint8 CFatVolume::GetNoOfFatTables(void) { return uiNoOfFatTables; }
char* CFatVolume::GetLabel(void) { return mszLabel.Text(); }
CFileDrive* CFatVolume::GetFileDrive(void) { return mpcDevice; }

