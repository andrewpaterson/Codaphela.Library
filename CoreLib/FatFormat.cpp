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

#include <ctype.h>
#include <string.h>
#include "FileDrive.h"
#include "FatFile.h"
#include "FatTime.h"
#include "FatFormat.h"


 // structure of the disk size to sectors per
 // cluster lookup table
struct SDiskSizeToSectorsPerCluster
{
	uint32	DiskSize;
	uint8	SecPerClusVal;
};


// this is the table for FAT12 drives.
static const SDiskSizeToSectorsPerCluster gasFatDiskSizeTableFat12[] =
{
	{ 8200, 2 },  		/* disks up to 4 MB, 1k cluster */
	{ 16368, 4 },		/* disks up to 8 MB, 2k cluster */
	{ 32704, 8 },		/* disks up to 16 MB, 4k cluster */
	/* The entries after this point are not used unless FAT16 is forced */
	{ 65376, 16 },		/* disks up to 32 MB, 8k cluster */
	{ 130720, 32 },		/* disks up to 64 MB, 16k cluster */
	{ 261408, 64 },		/* disks up to 128 MB, 32k cluster */
	{ 0xFFFFFFFF, 0 }	/* any disk greater than 2GB, 0 value for SecPerClusVal trips an error */
};


//* This is the table for FAT16 drives. NOTE that this table includes
//* entries for disk sizes larger than 512 MB even though typically
//* only the entries for disks < 512 MB in size are used.
//* The way this table is accessed is to look for the first entry
//* in the table for which the disk size is less than or equal
//* to the DiskSize field in that table entry.  For this table to
//* work properly uiReservedSectorCount must be 1, uiNumFileAllocationTables
//* must be 2, and uiRootEntryCount must be 512. Any of these values
//* being different may require the first table entries DiskSize value
//* to be changed otherwise the cluster count may be to low for FAT16.
static const SDiskSizeToSectorsPerCluster gasFatDiskSizeTableFat16[] =
{
   { 8400, 0 }, 		/* disks up to 4.1 MB, the 0 value for SecPerClusVal trips an error */
   { 32680, 2 },  		/* disks up to 16 MB, 1k cluster */
   { 262144, 4 },		/* disks up to 128 MB, 2k cluster */
   { 524288, 8 },		/* disks up to 256 MB, 4k cluster */
   { 1048576, 16 },		/* disks up to 512 MB, 8k cluster */
   /* The entries after this point are not used unless FAT16 is forced */
   { 2097152, 32 },		/* disks up to 1 GB, 16k cluster */
   { 4194304, 64 },		/* disks up to 2 GB, 32k cluster */
   { 0xFFFFFFFF, 0 }	/* any disk greater than 2GB, 0 value for SecPerClusVal trips an error */
};


// This is the table for FAT32 drives. NOTE that this table includes
// entries for disk sizes smaller than 512 MB even though typically
// only the entries for disks >= 512 MB in size are used.
// The way this table is accessed is to look for the first entry
// in the table for which the disk size is less than or equal
// to the DiskSize field in that table entry. For this table to
// work properly uiReservedSectorCount must be 32, and uiNumFileAllocationTables
// must be 2. Any of these values being different may require the first
// table entries DiskSize value to be changed otherwise the cluster count
// may be to low for FAT32.
 

static const SDiskSizeToSectorsPerCluster gasFatDiskSizeTableFat32[] =
{
	{ 66600, 0 },		/* disks up to 32.5 MB, the 0 value for SecPerClusVal trips an error */
	{ 532480, 1 },		/* disks up to 260 MB, .5k cluster */
	{ 16777216, 8 },	/* disks up to 8 GB, 4k cluster */
	{ 33554432, 16 },	/* disks up to 16 GB, 8k cluster */
	{ 67108864, 32 },	/* disks up to 32 GB, 16k cluster */
	{ 0xFFFFFFFF, 64 }	/* disks greater than 32GB, 32k cluster */
};


// find the best cluster size of given file system and # of 512 bytes sectors
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 FatCalculateClusterSize(uint8 fs_type, uint32 uiTotalSectors, char forced_fs_type)
{
	uint8 i;
	switch (fs_type)
	{
	case FAT_FS_TYPE_FAT12:
		for (i = 0; i < forced_fs_type ? 7 : 3; i++)
		{
			if (gasFatDiskSizeTableFat12[i].DiskSize >= uiTotalSectors)
			{
				return gasFatDiskSizeTableFat12[i].SecPerClusVal;
			}
		}
		break;

	case FAT_FS_TYPE_FAT16:
		for (i = 0; i < forced_fs_type ? 8 : 5; i++)
		{
			if (gasFatDiskSizeTableFat16[i].DiskSize >= uiTotalSectors)
			{
				return gasFatDiskSizeTableFat16[i].SecPerClusVal;
			}
		}
		break;

	case FAT_FS_TYPE_FAT32:
		for (i = 0; i < 6; i++)
		{
			if (gasFatDiskSizeTableFat32[i].DiskSize >= uiTotalSectors)
			{
				return gasFatDiskSizeTableFat32[i].SecPerClusVal;
			}
		}
		break;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFatVolumeLabel(char* szDestination, char* szVolumeLabel)
{
	size_t	c;
	uint8	i;

	c = strlen(szVolumeLabel);
	if (c)
	{
		for (i = 0; i < 11; i++)
		{
			if (i < c)
			{
				szDestination[i] = toupper(szVolumeLabel[i]);
			}
			else
			{
				szDestination[i] = 0x20;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CalcuateFatTableSizeInSectorsAndNumberOfClusters(uint32* puiNoOfClusters, uint32* puiFatTableSizeInSectors, EFatFileSystemType fs_type, uint32 uiBytesPerSector, uint32 uiNoOfReservedSectors, uint32 uiNoOfFatTables, uint32 uiTotalSectors, uint32 uiSectorsPerCluster)
{
// calculate the FAT table size (ignoring the fact that it won't fit
// since we're allocating all disk space to clusters
	for (;;)
	{
		switch (fs_type)
		{
			case FAT_FS_TYPE_FAT12:
			{
				*puiFatTableSizeInSectors = (((*puiNoOfClusters + 2) + ((*puiNoOfClusters + 2) >> 1)) + uiBytesPerSector - 1) / uiBytesPerSector;
				break;
			}
			case FAT_FS_TYPE_FAT16:
			{
				*puiFatTableSizeInSectors = (((*puiNoOfClusters + 2) * 2) + uiBytesPerSector - 1) / uiBytesPerSector;
				break;
			}
			case FAT_FS_TYPE_FAT32:
			{
				*puiFatTableSizeInSectors = (((*puiNoOfClusters + 2) * 4) + uiBytesPerSector - 1) / uiBytesPerSector;
				break;
			}
		}

		// if the FAT table fits then we're done
		if (uiNoOfReservedSectors + (*puiFatTableSizeInSectors * uiNoOfFatTables) + (*puiNoOfClusters * uiSectorsPerCluster) <= uiTotalSectors)
		{
			break;
		}

		(*puiNoOfClusters)--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EFatCode FatFormat(EFatFileSystemType fs_type, char* const volume_label, uint32 uiSectorsPerCluster, CFileDrive* device)
{
	bool						bSuccess;
	uint32						i;
	uint32						c;
	uint32						uiTotalSectors;
	uint32						root_dir_sectors;
	uint32						uiFatTableSizeInSectors = 0;

	uint32						backup_boot_sector;
	uint32						uiFileSystemInfoSector;
	uint32						no_of_root_entries;
	uint32						uiBytesPerSector;
	uint32						uiNoOfReservedSectors;
	uint32						root_cluster;
	uint32						uiNoOfClusters;
	uint32						uiNoOfFatTables;
	uint8						media_type;
	uint32						root_entry_sector;
	uint32						root_entry_offset = 0;

	SFatBIOSParameterBlock*		bpb;
	SFatFileSystemInfo*			psFileSystemInfo;
	SFatRawDirectoryEntry*		psEntry;

	uint8						uiBuffer[MAX_SECTOR_LENGTH];

	// todo: check for illegal characters in volume label
	if (strlen(volume_label) > 11)
	{
		return FAT_INVALID_VOLUME_LABEL;
	}

	memset(uiBuffer, 0, MAX_SECTOR_LENGTH);

	// get the total capacity of the storage device in bytes.
	media_type = 0xF8;
	uiFileSystemInfoSector = 1;
	uiNoOfFatTables = 2;
	backup_boot_sector = 6;
	root_cluster = 2;
	uiTotalSectors = (uint32)device->GetTotalSectors();
	uiBytesPerSector = device->GetSectorSize();

	// if the user didn't specify a file system type find the most appropriate one.
	if (fs_type == FAT_FS_TYPE_UNSPECIFIED)
	{
		// if the user specifies a cluster size he/she must also
		// specify a file system type. Also we can only calculate cluster
		// size for drives with a 512 bytes sector
		if (uiSectorsPerCluster || uiBytesPerSector != 0x200)
		{
			return FAT_INVALID_FORMAT_PARAMETERS;
		}

		// first try FAT12
		uiSectorsPerCluster = FatCalculateClusterSize(FAT_FS_TYPE_FAT12, uiTotalSectors, 0);

		if (uiSectorsPerCluster)
		{
			fs_type = FAT_FS_TYPE_FAT12;
		}
		else
		{
			// try FAT16
			uiSectorsPerCluster = FatCalculateClusterSize(FAT_FS_TYPE_FAT16, uiTotalSectors, 0);

			if (uiSectorsPerCluster)
			{
				fs_type = FAT_FS_TYPE_FAT16;
			}
			else
			{
				// try FAT32
				uiSectorsPerCluster = FatCalculateClusterSize(FAT_FS_TYPE_FAT32, uiTotalSectors, 0);

				if (uiSectorsPerCluster)
				{
					fs_type = FAT_FS_TYPE_FAT32;
				}
			}
		}
	}

	// if we don't have a valid fs type return error
	if (fs_type < FAT_FS_TYPE_FAT12 || fs_type > FAT_FS_TYPE_FAT32)
	{
		return FAT_INVALID_FORMAT_PARAMETERS;
	}

	// if we don't have a cluster size try to get one
	if (!uiSectorsPerCluster)
	{
		// if sector size is not 512 bytes user must specify cluster size
		if (uiSectorsPerCluster || uiBytesPerSector != 0x200)
		{
			return FAT_INVALID_FORMAT_PARAMETERS;
		}
		uiSectorsPerCluster = FatCalculateClusterSize(fs_type, uiTotalSectors, 1);
	}

	// if we still don't have it return error
	if (!uiSectorsPerCluster)
	{
		return FAT_INVALID_FORMAT_PARAMETERS;
	}

	// calculate no of root entries and reserved sectors
	no_of_root_entries = (fs_type == FAT_FS_TYPE_FAT32) ? 0 : 512;
	uiNoOfReservedSectors = (fs_type == FAT_FS_TYPE_FAT32) ? 32 : 1;

	// calculate the count of clusters on the volume without accounting
	// for the space that will be used by the FAT tables
	root_dir_sectors = ((no_of_root_entries * 32) + (uiBytesPerSector - 1)) / uiBytesPerSector;
	uiNoOfClusters = (uiTotalSectors - uiNoOfReservedSectors - root_dir_sectors) / uiSectorsPerCluster;

	CalcuateFatTableSizeInSectorsAndNumberOfClusters(&uiNoOfClusters, &uiFatTableSizeInSectors, fs_type, uiBytesPerSector, uiNoOfReservedSectors, uiNoOfFatTables, uiTotalSectors, uiSectorsPerCluster);

	// check that the filesystem type/cluster size supplied is correct.
	switch (fs_type)
	{
		case FAT_FS_TYPE_FAT12:
		{
			if (uiNoOfClusters > FAT12_MAX_CLUSTERS)
			{
				return FAT_INVALID_FORMAT_PARAMETERS;
			}
			break;
		}
		case FAT_FS_TYPE_FAT16:
		{
			if (uiNoOfClusters > FAT16_MAX_CLUSTERS || uiNoOfClusters < (FAT12_MAX_CLUSTERS + 1))
			{
				return FAT_INVALID_FORMAT_PARAMETERS;
			}
			break;
		}
		case FAT_FS_TYPE_FAT32:
		{
			if (uiNoOfClusters < (FAT16_MAX_CLUSTERS + 1))
			{
				return FAT_INVALID_FORMAT_PARAMETERS;
			}
			break;
		}
	}

	root_cluster = 3;

	// calculate the offset of the cluster's FAT entry within it's sector
	// note: when we hit get past the end of the current sector entry_offset
	// will roll back to zero (or possibly 1 for FAT12)
	switch (fs_type)
	{
		case FAT_FS_TYPE_FAT12: 
			root_entry_offset = root_cluster + (root_cluster >> 1); 
			break;

		case FAT_FS_TYPE_FAT16: 
			root_entry_offset = root_cluster * ((uint32)2); 
			break;

		case FAT_FS_TYPE_FAT32: 
			root_entry_offset = root_cluster * ((uint32)4); 
			break;
	}

	root_entry_sector = (root_entry_offset / uiBytesPerSector);
	root_entry_offset = (root_entry_offset % uiBytesPerSector) / 4;

	// set common Bios Parameter Block (BPB) fields.
	bpb = (SFatBIOSParameterBlock*)uiBuffer;

	memcpy(bpb->szOEMName, "FAT32LIB", 8);
	bpb->uiJumpInstrustion[0] = 0xEB;
	bpb->uiJumpInstrustion[1] = 0x00;
	bpb->uiJumpInstrustion[2] = 0x90;
	bpb->uiNumHeads = 0x1;
	bpb->uiNumHiddenSectors = 0x0;
	bpb->uiSectorsPerTrack = 0;
	bpb->uiMediaType = media_type;
	bpb->uiNumFileAllocationTables = uiNoOfFatTables;
	bpb->uiBytsPerSector = uiBytesPerSector;
	bpb->uiRootEntryCount = no_of_root_entries;
	bpb->uiReservedSectorCount = uiNoOfReservedSectors;
	bpb->uiSectorsPerCluster = uiSectorsPerCluster;
	bpb->uiTotalSectorsFat16 = (fs_type == FAT_FS_TYPE_FAT32 || uiTotalSectors > 0xFFFF) ? 0 : uiTotalSectors;
	bpb->uiTotalSectorsFat32 = (fs_type == FAT_FS_TYPE_FAT32 || uiTotalSectors > 0xFFFF) ? uiTotalSectors : 0;
	bpb->uiFATSzFat16 = (fs_type == FAT_FS_TYPE_FAT32) ? 0 : uiFatTableSizeInSectors;

	if (fs_type == FAT_FS_TYPE_FAT32)
	{
		// set FAT32 specific fields
		bpb->uFatEx.sFat32.uiDriverNumber = 0;
		bpb->uFatEx.sFat32.uiReserved1 = 0;
		bpb->uFatEx.sFat32.uiBootSignature = 0x29;
		bpb->uFatEx.sFat32.uiFATSzFat32 = uiFatTableSizeInSectors;
		bpb->uFatEx.sFat32.uiExtendedFlags = 0;
		bpb->uFatEx.sFat32.uiFileSystemVersion = 0;
		bpb->uFatEx.sFat32.uiRootCluster = root_cluster;
		bpb->uFatEx.sFat32.uiFileSystemInformation = uiFileSystemInfoSector;
		bpb->uFatEx.sFat32.uiBackupBootSector = backup_boot_sector;
		time((time_t*)&bpb->uFatEx.sFat32.uiVolumeID);
		memset(bpb->uFatEx.sFat32.auiReserved, 0, 12);
		memcpy(bpb->uFatEx.sFat32.szVolumeLabel, "NO NAME    ", 11);
		memcpy(bpb->uFatEx.sFat32.szFileSystemType, "FAT32   ", 8);

		SetFatVolumeLabel(bpb->uFatEx.sFat32.szVolumeLabel, volume_label);
	}
	else
	{
		// set FAT12/FAT16 specific fields
		bpb->uFatEx.sFat16.uiDriverNumber = 0;
		bpb->uFatEx.sFat16.uiReserved1 = 0;
		bpb->uFatEx.sFat16.uiBootSignature = 0x29;
		time((time_t*)&bpb->uFatEx.sFat16.uiVolumeID);
		memcpy(bpb->uFatEx.sFat16.szVolumeLabel, "NO NAME    ", 11);
		memcpy(bpb->uFatEx.sFat16.szFileSystemType, "FAT     ", 8);

		// set the volume label
		SetFatVolumeLabel(bpb->uFatEx.sFat16.szVolumeLabel, volume_label);
	}

	// set the boot sector signature
	uiBuffer[510] = 0x55;
	uiBuffer[511] = 0xAA;
	bpb = 0;

	// write boot sector to sector 0
	bSuccess = device->Write(0x0, uiBuffer);
	if (!bSuccess)
	{
		return FAT_CANNOT_WRITE_MEDIA;
	}

	// if this is a FAT32 volume write a backup of boot sector
	// and FSInfo structure
	if (fs_type == FAT_FS_TYPE_FAT32)
	{
		// write a copy of the boot sector to sector # uiBackupBootSector
		bSuccess = device->Write(backup_boot_sector, uiBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}

		// initialize the FSInfo structure
		psFileSystemInfo = (SFatFileSystemInfo*)uiBuffer;
		memset(uiBuffer, 0, MAX_SECTOR_LENGTH);
		psFileSystemInfo->uiLeadSignature = 0x41615252;
		psFileSystemInfo->uiStructSignature = 0x61417272;
		psFileSystemInfo->uiNumFreeClusters = uiNoOfClusters - 1;
		psFileSystemInfo->uiNextFreeCluster = 3;
		psFileSystemInfo->uiTrailSignature = 0xAA550000;
		psFileSystemInfo = 0;

		// write the FSInfo structor to sector # uiFileSystemInformation
		bSuccess = device->Write(uiFileSystemInfoSector, uiBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_READ_MEDIA;
		}
	}

	for (c = 0; c < uiNoOfFatTables; c++)
	{
		// zero out the FAT table and set the entries for clusters 0 and 1
		// start by zeroing the whole uiBuffer
		memset(uiBuffer, 0, MAX_SECTOR_LENGTH);

		// loop through each sector on the fat
		for (i = 0; i < uiFatTableSizeInSectors; i++)
		{
			if (!i)
			{
				// if this is the 1st sector of the FAT set the values for
				// clusters 0 and 1
				if (fs_type == FAT_FS_TYPE_FAT12)
				{
					uint16 value;

					/* even */
					value = 0xFF8;									/* the value to be written */
					*((uint16*)&uiBuffer[0]) &= 0xF000;				/* clear bits for entry */
					*((uint16*)&uiBuffer[0]) |= (value & 0x0FFF);	/* set bits for entry */


					// odd - we need to write this one 1 byte at a time since
					// unaligned word access is inefficient and may cause a fatal
					// exception on some platforms.
					value = FAT12_EOC;							/* the value to be written */
					value <<= 4;								/* an odd entry occupies the upper 12-bits of the word */

					uiBuffer[1 + 0] &= 0x0F;					/* clear entry bits on 1st byte */
					uiBuffer[1 + 0] |= LO8(value);				/* set entry bits on 1st byte */

					uiBuffer[1 + 1] = 0x00;						/* clear the 2nd byte */
					uiBuffer[1 + 1] = HI8(value);				/* set the 2nd byte */

				}
				else if (fs_type == FAT_FS_TYPE_FAT16)
				{
					((uint16*)uiBuffer)[0] = (FAT16_EOC & 0xFF00) | media_type;
					((uint16*)uiBuffer)[1] = FAT16_EOC;
				}
				else
				{
					((uint32*)uiBuffer)[0] = (FAT32_EOC & 0x0FFFFF00) | media_type;
					((uint32*)uiBuffer)[1] = FAT32_EOC;
				}
			}
			if (i == root_entry_sector && fs_type == FAT_FS_TYPE_FAT32)
			{
				((uint32*)uiBuffer)[root_entry_offset] = FAT32_EOC;
			}

			// write the sector to all the FAT tables
			bSuccess = device->Write(uiNoOfReservedSectors + i + (c * uiFatTableSizeInSectors), uiBuffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_WRITE_MEDIA;
			}

			if (i == 0 || i == root_entry_sector)
			{
				memset(uiBuffer, 0, MAX_SECTOR_LENGTH);
			}
		}
	}

	// zero the root directory (or it's 1st cluster in the case of FAT32
	// note: since cluster #2 is located where the root directory would be on
	// a FAT12/FAT16 volume we can use the same code for both, only far
	// FAT32 we only initialize 1 cluster
	//
	// this is the formula to get the 1st sector of a cluster:
	//
	// ((root_cluster - 2) * uiSectorsPerCluster) + uiNoOfReservedSectors + (uiNoOfFatTables * fatsz) + root_dir_sectors;
	//
	// since we're using cluster 2 as the root cluster and root_dir_sectors == 0 for FAT32 this is equal to:
	//
	// uiNoOfReservedSectors + (uiNoOfFatTables * fatsz)
	//
	// so the address for the root directory is the same on FAT32 and on FAT12/16 as long as we're
	// using cluster #2
	memset(uiBuffer, 0, MAX_SECTOR_LENGTH);
	c = (fs_type == FAT_FS_TYPE_FAT32) ? uiSectorsPerCluster : root_dir_sectors;
	for (i = 1; i < c; i++)
	{
		if (fs_type == FAT_FS_TYPE_FAT32)
		{
			uint32 root_sector = ((root_cluster - 2) * uiSectorsPerCluster) + uiNoOfReservedSectors + (uiNoOfFatTables * uiFatTableSizeInSectors) + root_dir_sectors;
			bSuccess = device->Write(root_sector, uiBuffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_WRITE_MEDIA;
			}
		}
		else
		{
			bSuccess = device->Write(uiNoOfReservedSectors + (uiNoOfFatTables * uiFatTableSizeInSectors) + i, uiBuffer);
			if (!bSuccess)
			{
				return FAT_CANNOT_WRITE_MEDIA;
			}

		}
	}

	// initialize the volume label entry
	psEntry = (SFatRawDirectoryEntry*)uiBuffer;
	psEntry->uEntry.sFatRawCommon.uiAttributes = FAT_ATTR_VOLUME_ID;
	psEntry->uEntry.sFatRawCommon.uiFirstClusterHighWord = 0;
	psEntry->uEntry.sFatRawCommon.uiFirstClusterLowWord = 0;
	psEntry->uEntry.sFatRawCommon.uiReserved = 0;
	psEntry->uEntry.sFatRawCommon.uiSize = 0;
	psEntry->uEntry.sFatRawCommon.uiCreateDate = GetSystemClockDate();
	psEntry->uEntry.sFatRawCommon.uiCreateTime = GetSystemClockTime();
	psEntry->uEntry.sFatRawCommon.uiModifyDate = psEntry->uEntry.sFatRawCommon.uiCreateDate;
	psEntry->uEntry.sFatRawCommon.uiModifyTime = psEntry->uEntry.sFatRawCommon.uiCreateTime;
	psEntry->uEntry.sFatRawCommon.uiAccessDate = psEntry->uEntry.sFatRawCommon.uiCreateDate;
	psEntry->uEntry.sFatRawCommon.uiCreateTimeTenths = 0;

	// set the volume label
	if ((c = strlen(volume_label)))
	{
		for (i = 0; i < 11; i++)
		{
			if (i < c)
			{
				psEntry->uEntry.sFatRawCommon.szShortName[i] = (volume_label[i]);
			}
			else
			{
				psEntry->uEntry.sFatRawCommon.szShortName[i] = 0x20;
			}
		}
	}

	// write the volume label entry to the root dir
	if (fs_type == FAT_FS_TYPE_FAT32)
	{
		bSuccess = device->Write(((root_cluster - 2) * uiSectorsPerCluster) + uiNoOfReservedSectors + (uiNoOfFatTables * uiFatTableSizeInSectors) + root_dir_sectors, uiBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}
	}
	else
	{
		bSuccess = device->Write(uiNoOfReservedSectors + (uiNoOfFatTables * uiFatTableSizeInSectors), uiBuffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}
	}
	return FAT_SUCCESS;
}

