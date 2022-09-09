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

#include <stdlib.h>
#include <string.h>
#include "Fat.h"
#include "FatInternals.h"
#include "FatSharedBuffer.h"


bool IsFatSectorLoaded(uint32 uiSector)
{
	return fat_shared_buffer_sector != 0xFFFFFFFF && (fat_shared_buffer_sector == uiSector);
}


uint32 CalculateFatEntryOffset(uint8 fs_type, uint32 cluster)
{
	switch (fs_type)
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


uint32 fat_allocate_cluster(CFatVolume* volume, SFatRawDirectoryEntry* parent, uint32 count, char zero, uint32 page_size, uint16* result);
uint16 fat_initialize_directory_cluster(CFatVolume* volume, SFatRawDirectoryEntry* parent, uint32 cluster, uint8* buffer);
uint16 fat_zero_cluster(CFatVolume* volume, uint32 cluster, uint8* buffer);
bool fat_write_fat_sector(CFatVolume * volume, uint32 sector_address, uint8 * buffer);


// allocates a cluster for a directory - finds a free cluster, initializes it as
// required by a directory, marks it's FAT entry as EOC and returns the cluster address
//
// NOTE: this function used the volume/shared buffer (if enabled) so it must not be
// locked before calling this function
uint32 fat_allocate_directory_cluster(CFatVolume* volume, SFatRawDirectoryEntry* parent, uint16* result)
{
	return fat_allocate_cluster(volume, parent, 1, 1, 1, result);
}


// allocates the requested number of clusters - finds the free clusters, initializes it to zeroes,
// links them as a cluster chain (marking the last one as EOC) and returns the cluster number for
// the 1st cluster in the chain
//
// NOTE: this function used the volume/shared buffer (if enabled) so it must not be
// locked before calling this function
uint32 fat_allocate_data_cluster(CFatVolume* volume, uint32 count, char zero, uint16* result)
{
	return fat_allocate_cluster(volume, 0, count, zero, 1, result);
}


#if defined(FAT_OPTIMIZE_FOR_FLASH)
uint32 fat_allocate_data_cluster_ex(CFatVolume* volume, uint32 count, char zero, uint32 page_size, uint16* result)
{
	return fat_allocate_cluster(volume, 0, count, zero, page_size, result);
}
#endif


// performs the work for fat_allocate_data_cluster and fat_allocate_directory_cluster
//
// NOTE: this function used the volume/shared buffer (if enabled) so it must not be
// locked before calling this function
uint32 fat_allocate_cluster(CFatVolume* volume, SFatRawDirectoryEntry* parent, uint32 count, char zero, uint32 page_size, uint16* result)
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

#if defined(FAT_OPTIMIZE_FOR_FLASH)
	uint16		step = 1;
#endif

	uint8*		buffer = fat_shared_buffer;

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
	if (volume->HasNextFreeCluser())
	{
		cluster = volume->GetNextFreeCluster();
	}

	// find the step between clusters allocated on page boundaries
#if defined(FAT_OPTIMIZE_FOR_FLASH)
	if (volume->GetNoOfSectorsPerCluster() < page_size)
	{
		uint32 sector;
		uint16 step_count = 0;
		step = (uint16)(page_size / volume->GetNoOfSectorsPerCluster());
		/*
		// find the 1st cluster that starts on a page boundary
		*/
		sector = calculate_first_sector_of_cluster(volume, cluster);
		while (sector % page_size && step_count < step)
		{
			cluster++;
			step_count++;
			sector = calculate_first_sector_of_cluster(volume, cluster);
		}
	}
#endif

	// remember the 1st cluster of our search
	start_cluster = cluster;

	// set the last_fat_entry value to the eof marker
	switch (volume->GetFileSystemType())
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
	entry_offset = CalculateFatEntryOffset(volume->GetFileSystemType(), cluster);
	entry_sector = volume->GetNoOfReservedSectors() + (entry_offset / volume->GetNoOfBytesPerSector());
	entry_offset = entry_offset % volume->GetNoOfBytesPerSector();
	last_entry_sector = entry_sector;

	// for each sector of the FAT
	for (;;)
	{
		// read sector into sector cache
		if (!IsFatSectorLoaded(entry_sector))
		{
			bSuccess = volume->Read(entry_sector, buffer);
			if (!bSuccess)
			{
				fat_shared_buffer_sector = (0xFFFFFFFF);
				*result = FAT_CANNOT_READ_MEDIA;
				return 0;
			}
			fat_shared_buffer_sector = (entry_sector);
		}

		// save the current sector number
		current_sector = entry_sector;

		// for every FAT entry in the sector...
		do
		{
			// if we've reached the last cluster on the drive return insufficient
			// disk space error code.
			if ((cluster > volume->GetNoOfClusters() + 1) && start_cluster > 2 && !wrapped_around)
			{
				if (entries_updated)
				{
					// if the buffer is dirty flush the changes
					bSuccess = fat_write_fat_sector(volume, current_sector, buffer);
					if (!bSuccess)
					{
						fat_shared_buffer_sector = (0xFFFFFFFF);
						*result = FAT_CANNOT_READ_MEDIA;
						return 0;
					}

					// mark the buffer as clean
					entries_updated = 0;
				}

				// reset the cluster # to 2
				cluster = 2;
				wrapped_around = 1;

				// calculate the sector for the new cluster
				entry_offset = CalculateFatEntryOffset(volume->GetFileSystemType(), cluster);
				entry_sector = volume->GetNoOfReservedSectors();
				entry_offset = entry_offset % volume->GetNoOfBytesPerSector();

				// break from this loop so that sector gets loaded
				break;

			}
			else if ((cluster > volume->GetNoOfClusters() + 1) || (wrapped_around && cluster >= start_cluster))
			{
				if (entries_updated)
				{
					// if the buffer is dirty flush the changes, otherwise when we try
					// to free the cluster chain it would run into free clusters before
					// the EOC resulting in lost chains (or a hang in a debug build).
					bSuccess = fat_write_fat_sector(volume, current_sector, buffer);
					if (!bSuccess)
					{
						fat_shared_buffer_sector = (0xFFFFFFFF);
						*result = FAT_CANNOT_READ_MEDIA;
						return 0;
					}
				}

				// free any clusters that we've allocated so far
				if (!fat_is_eof_entry(volume, last_fat_entry))
				{
					// even if this returns error we cannot do anything with
					// it as we're already returning an error code so we can only
					// hope that the clusters get freed.
					uint16 uiResult;
					uiResult = fat_free_cluster_chain(volume, last_fat_entry);
				}

				// return insufficient disk space error
				*result = FAT_INSUFFICIENT_DISK_SPACE;
				return 0;
			}

			// copy the next FAT entry to the fat_entry variable
			switch (volume->GetFileSystemType())
			{
				case FAT_FS_TYPE_FAT12:
				{
					// clear fat_entry to make sure that the upper 16
					// bits are not set.
					fat_entry = 0;

					// read the 1st byte
					((uint8*)&fat_entry)[INT32_BYTE0] = buffer[entry_offset];

					// load the next sector (if necessary) and set the offset
					// for the next byte in the buffer
					if (entry_offset == volume->GetNoOfBytesPerSector() - 1)
					{
						if (entries_updated)
						{
							// if the buffer is dirty flush it before we use it to load
							// the next sector
							bSuccess = fat_write_fat_sector(volume, entry_sector, buffer);
							if (!bSuccess)
							{
								fat_shared_buffer_sector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}

							// mark the buffer as clean
							entries_updated = 0;
						}

						// load the next sector into the buffer
						bSuccess = volume->Read(entry_sector + 1, buffer);
						if (!bSuccess)
						{
							fat_shared_buffer_sector = (0xFFFFFFFF);
							*result = FAT_CANNOT_READ_MEDIA;
							return 0;
						}

						// set flag to indicate that we had to load the next sector
						next_sector_loaded = 1;
					}

					// read the 2nd byte
					((uint8*)&fat_entry)[INT32_BYTE1] = buffer[next_sector_loaded ? 0 : (entry_offset + 1)];

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
						bSuccess = volume->Read(entry_sector, buffer);
						if (!bSuccess)
						{
							fat_shared_buffer_sector = (0xFFFFFFFF);
							*result = FAT_CANNOT_READ_MEDIA;
							return 0;
						}
						next_sector_loaded = 0;
					}
					break;
				}
				case FAT_FS_TYPE_FAT16:
				{
					fat_entry = *((uint16*)&buffer[entry_offset]);
					break;
				}
				case FAT_FS_TYPE_FAT32:
				{
					fat_entry = *((uint32*)&buffer[entry_offset]) & 0x0FFFFFFF;
					break;
				}
			}

			// if the current FAT is free
			if (volume->IsFreeFat(fat_entry))
			{
				// maintain the count of free cluster and the next
				// cluster that may be free
				volume->SetNextFreeCluster(cluster + 1);
				volume->SetTotalFreeClusters(volume->GetTotalFreeClusters() - 1);

				// if this is the 1st cluster found remember it
				if (!first_cluster)
				{
					first_cluster = cluster;
				}

				// mark the FAT as the the new 1st link of the cluster chain
				// (or the end of the chain if we're only allocating 1 cluster)
				switch (volume->GetFileSystemType())
				{
					case FAT_FS_TYPE_FAT12:
					{
						// write the 1st byte
						if (cluster & 0x1)
						{
							buffer[entry_offset] &= 0x0F;								/* clear entry bits on 1st byte */
							buffer[entry_offset] |= LO8((uint16)FAT12_EOC << 4);		/* set entry bits on 1st byte */
						}
						else
						{
							buffer[entry_offset] = LO8((uint16)FAT12_EOC);			/* just copy the 1st byte */
						}

						// if the FAT entry spans a sector boundary flush the currently
						// loaded sector to the drive and load the next one.
						if (entry_offset == volume->GetNoOfBytesPerSector() - 1)
						{
							// flush the updated sector to the drive
							bSuccess = fat_write_fat_sector(volume, entry_sector, buffer);
							if (!bSuccess)
							{
								fat_shared_buffer_sector = (0xFFFFFFFF);
								*result = FAT_CANNOT_WRITE_MEDIA;
								return 0;
							}

							// mark the buffer as clean
							entries_updated = 0;

							// load the next sector
							bSuccess = volume->Read(entry_sector + 1, buffer);
							if (!bSuccess)
							{
								fat_shared_buffer_sector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}

							// set the next_sector_loaded flag
							next_sector_loaded = 1;
						}

						// write the 2nd byte
						if (cluster & 0x1)
						{
							buffer[next_sector_loaded ? 0 : (entry_offset + 1)] = HI8((uint16)FAT12_EOC << 4);
						}
						else
						{
							buffer[next_sector_loaded ? 0 : (entry_offset + 1)] &= 0xF0;
							buffer[next_sector_loaded ? 0 : (entry_offset + 1)] |= HI8((uint16)FAT12_EOC);
						}

						// if the next sector has been loaded then flush it
						if (next_sector_loaded)
						{
							// flush buffer to disk
							bSuccess = fat_write_fat_sector(volume, entry_sector + 1, buffer);
							if (!bSuccess)
							{
								fat_shared_buffer_sector = (0xFFFFFFFF);
								*result = FAT_CANNOT_READ_MEDIA;
								return 0;
							}

							// note: we don't reload the current sector because this being the last entry
							// the next sector is about to be reloaded anyways
							next_sector_loaded = 0;

							// reload the current sector
							bSuccess = volume->Read(entry_sector, buffer);
							if (!bSuccess)
							{
								fat_shared_buffer_sector = (0xFFFFFFFF);
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
									// flush buffer to disk
									bSuccess = fat_write_fat_sector(volume, entry_sector, buffer);
									if (!bSuccess)
									{
										fat_shared_buffer_sector = (0xFFFFFFFF);
										*result = FAT_CANNOT_READ_MEDIA;
										return 0;
									}

									// mark buffer as clean
									entries_updated = 0;
								}

								// load last_entry sector
								bSuccess = volume->Read(last_entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}
							}

							// update the last entry to point to this one
							if (last_fat_entry & 0x1)
							{
								buffer[last_entry_offset] &= 0x0F;							/* clear entry bits on 1st byte */
								buffer[last_entry_offset] |= LO8((uint16)cluster << 4);		/* set entry bits on 1st byte */
							}
							else
							{
								buffer[last_entry_offset] = LO8((uint16)cluster);	/* just copy the 1st byte */
							}

							// if the FAT entry spans a sector boundary flush the currently
							// loaded sector to the drive and load the next one.
							if (last_entry_offset == volume->GetNoOfBytesPerSector() - 1)
							{
								// flush the updated sector to the drive
								bSuccess = fat_write_fat_sector(volume, last_entry_sector, buffer);
								if (!bSuccess)
								{
									*result = FAT_CANNOT_WRITE_MEDIA;
									return 0;
								}

								// mark the buffer as clean
								entries_updated = 0;

								// load the next sector
								bSuccess = volume->Read(last_entry_sector + 1, buffer);
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
								buffer[next_sector_loaded ? 0 : (last_entry_offset + 1)] = HI8((uint16)cluster << 4);
							}
							else
							{
								buffer[next_sector_loaded ? 0 : (last_entry_offset + 1)] &= 0xF0;
								buffer[next_sector_loaded ? 0 : (last_entry_offset + 1)] |= HI8((uint16)cluster);
							}

							// if the next sector has been loaded then flush it
							if (next_sector_loaded)
							{
								// flush buffer to disk
								bSuccess = fat_write_fat_sector(volume, last_entry_sector + 1, buffer);
								if (!bSuccess)
								{
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}

								// note: we don't reload the current sector because this being the last entry
								// the next sector is about to be reloaded anyways
								next_sector_loaded = 0;

								// reload the last entry sector
								bSuccess = volume->Read(last_entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = 0xFFFFFFFF;
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
									// flush buffer to disk
									bSuccess = fat_write_fat_sector(volume, last_entry_sector, buffer);
									if (!bSuccess)
									{
										fat_shared_buffer_sector = (0xFFFFFFFF);
										*result = FAT_CANNOT_READ_MEDIA;
										return 0;
									}

									// mark buffer as clean
									entries_updated = 0;
								}
								/*
								// reload current sector
								*/
								bSuccess = volume->Read(entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}
							}
						}
						break;
					}
					case FAT_FS_TYPE_FAT16:
					{
						*((uint16*)&buffer[entry_offset]) = FAT16_EOC;

						// mark the buffer as dirty
						entries_updated = 1;

						// if this is not the first cluster allocated update
						// the last one to link to this one
						if (last_fat_entry != FAT16_EOC)
						{
							if (last_entry_sector != entry_sector)
							{
								// flush buffer to disk
								bSuccess = fat_write_fat_sector(volume, entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}
								// mark buffer as clean
								entries_updated = 0;

								// load last_entry sector
								bSuccess = volume->Read(last_entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}
							}

							// update the last entry to point to this one
							* ((uint16*)&buffer[last_entry_offset]) = (uint16)cluster;

							if (last_entry_sector != entry_sector)
							{
								// flush buffer to disk
								bSuccess = fat_write_fat_sector(volume, last_entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}

								// reload current sector
								bSuccess = volume->Read(entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}
							}
						}
						break;
					}
					case FAT_FS_TYPE_FAT32:
					{
						*((uint32*)&buffer[entry_offset]) &= 0xF0000000;
						*((uint32*)&buffer[entry_offset]) |= FAT32_EOC & 0x0FFFFFFF;

						// mark the buffer as dity
						entries_updated = 1;

						// if this is not the 1st cluster allocated update
						// the last one to link to this one
						if (last_fat_entry != FAT32_EOC)
						{
							if (last_entry_sector != entry_sector)
							{
								// flush buffer to disk
								bSuccess = fat_write_fat_sector(volume, entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}

								// mark buffer as clean
								entries_updated = 0;

								// load last_entry sector
								bSuccess = volume->Read(last_entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}
							}

							// update the last entry to point to this one
							*((uint32*)&buffer[last_entry_offset]) &= 0xF0000000;
							*((uint32*)&buffer[last_entry_offset]) |= cluster & 0x0FFFFFFF;

							if (last_entry_sector != entry_sector)
							{
								// flush buffer to disk
								bSuccess = fat_write_fat_sector(volume, last_entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = (0xFFFFFFFF);
									*result = FAT_CANNOT_READ_MEDIA;
									return 0;
								}

								// reload current sector
								bSuccess = volume->Read(entry_sector, buffer);
								if (!bSuccess)
								{
									fat_shared_buffer_sector = (0xFFFFFFFF);
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
						// flush buffer to disk
						bSuccess = fat_write_fat_sector(volume, entry_sector, buffer);
						if (!bSuccess)
						{
							fat_shared_buffer_sector = (0xFFFFFFFF);
							*result = FAT_CANNOT_READ_MEDIA;
							return 0;
						}
					}

					// if we're allocating a cluster for a
					// directory then initialize it properly
					if (parent)
					{
						uint16 uiResult;
						uiResult = fat_initialize_directory_cluster(volume, parent, cluster, buffer);
						if (uiResult != FAT_SUCCESS)
						{
							fat_shared_buffer_sector = (0xFFFFFFFF);
							*result = uiResult;
							return 0;
						}
					}
					else
					{
						if (zero)
						{
							uint16 uiResult;
							uiResult = fat_zero_cluster(volume, cluster, buffer);
							if (uiResult != FAT_SUCCESS)
							{
								fat_shared_buffer_sector = (0xFFFFFFFF);
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
#if defined(FAT_OPTIMIZE_FOR_FLASH)
			if (!first_cluster)
			{
				cluster += step;
			}
			else
			{
				cluster++;
			}
#else
			cluster++;
#endif
			// calculate the offset of the cluster's FAT entry within it's sector
			// note: when we hit get past the end of the current sector entry_offset
			// will roll back to zero (or possibly 1 for FAT12)
			entry_offset = CalculateFatEntryOffset(volume->GetFileSystemType(), cluster);
			entry_sector = volume->GetNoOfReservedSectors() + (entry_offset / volume->GetNoOfBytesPerSector());
			entry_offset = entry_offset % volume->GetNoOfBytesPerSector();

		} while (current_sector == entry_sector);

		// if any changes where made to the fat entry currently cached sector
		// flush the changes to the drive before moving to next sector
		if (entries_updated)
		{
			bSuccess = fat_write_fat_sector(volume, current_sector, buffer);
			if (!bSuccess)
			{
				fat_shared_buffer_sector = (0xFFFFFFFF);
				*result = FAT_CANNOT_READ_MEDIA;
				return 0;
			}
			entries_updated = 0;
		}
	}
}


// marks all the clusters in the cluster chain as free
uint16 fat_free_cluster_chain(CFatVolume* volume, uint32 cluster)
{
	bool	bSuccess;
	uint32	fat_offset = 0;			/* the offset of the cluster entry within the FAT table */
	uint32	entry_offset;			/* the offset of the cluster entry within it's sector */
	uint32	entry_sector;			/* the sector where the entry is stored on the drive */
	uint32	current_sector;			/* the sector that's currently loaded in memory */
	char	is_odd_cluster = 0;		/* indicates that the entry being processed is an odd cluster address (FAT12 only) */
	char	op_in_progress = 0;		/* indicates that a multi-step operation is in progress (FAT12 only) */
	uint8*	buffer = fat_shared_buffer;

	// get the offset of the cluster entry within the FAT table,
	// the sector of the FAT table that contains the entry and the offset
	// of the fat entry within the sector
	fat_offset = CalculateFatEntryOffset(volume->GetFileSystemType(), cluster);
	entry_sector = volume->GetNoOfReservedSectors() + (fat_offset / volume->GetNoOfBytesPerSector());
	entry_offset = fat_offset % volume->GetNoOfBytesPerSector();

	// loop until we reach the EOC cluster or an error occurs.
	while (1)
	{
		// load sector to memory
		if (!IsFatSectorLoaded(entry_sector))
		{
			bSuccess = volume->Read(entry_sector, buffer);
			if (!bSuccess)
			{
				fat_shared_buffer_sector = (0xFFFFFFFF);
				return FAT_CANNOT_READ_MEDIA;
			}
			fat_shared_buffer_sector = (entry_sector);
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
			switch (volume->GetFileSystemType())
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
						((uint8*)&cluster)[0] = buffer[entry_offset];

						// write the 1st byte
						//
						// note: since the value that we're writting is FREE_FAT which expands
						// to zero we can skip a step or two in the update process. I left the code
						// for the steps that we're skipping commented out for clarity.
						if (is_odd_cluster)
						{
							buffer[entry_offset] &= 0x0F;
						}
						else
						{
							buffer[entry_offset] = FREE_FAT;
						}
					}

					if (entry_offset == volume->GetNoOfBytesPerSector() - 1)
					{
						/*
						// flush current sector to drive
						*/
						bSuccess = fat_write_fat_sector(volume, current_sector, buffer);
						if (!bSuccess)
						{
							fat_shared_buffer_sector = 0xFFFFFFFF;
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
					((uint8*)&cluster)[1] = buffer[entry_offset];

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
						buffer[entry_offset] = FREE_FAT;
					}
					else
					{
						buffer[entry_offset] &= 0xF0;									/* clear bits that 1st byte will be written to */
					}

					// clear op_in_progress flag.
					op_in_progress = 0;
					break;
				}
				case FAT_FS_TYPE_FAT16:
				{
					cluster = (uint32) * ((uint16*)&buffer[entry_offset]);
					*((uint16*)&buffer[entry_offset]) = FREE_FAT;
					break;
				}
				case FAT_FS_TYPE_FAT32:
				{
					// FAT32 entries are actually 28 bits so we need to leave the
					// upper nibble untouched
					cluster = *((uint32*)&buffer[entry_offset]) & 0x0FFFFFFF;
					*((uint32*)&buffer[entry_offset]) &= 0xF0000000;
					break;
				}
			}

			// increase the count of free clusters
			volume->SetTotalFreeClusters(volume->GetTotalFreeClusters() + 1);

			// if it's the EOF marker we're done, flush the buffer and go
			if (fat_is_eof_entry(volume, cluster))
			{
				bSuccess = fat_write_fat_sector(volume, current_sector, buffer);
				if (!bSuccess)
				{
					fat_shared_buffer_sector = 0xFFFFFFFF;
					return FAT_CANNOT_READ_MEDIA;
				}

				return FAT_SUCCESS;
			}

			// calculate the location of the next cluster in the chain
			fat_offset = CalculateFatEntryOffset(volume->GetFileSystemType(), cluster);
			entry_sector = volume->GetNoOfReservedSectors() + (fat_offset / volume->GetNoOfBytesPerSector());
			entry_offset = fat_offset % volume->GetNoOfBytesPerSector();
		}

		// flush FAT table changes
		bSuccess = fat_write_fat_sector(volume, current_sector, buffer);
		if (!bSuccess)
		{
			fat_shared_buffer_sector = 0xFFFFFFFF;
			return FAT_CANNOT_READ_MEDIA;
		}
	}
}


// gets the FAT structure for a given cluster number
uint16 fat_get_cluster_entry(CFatVolume* volume, uint32 cluster, FatEntry* fat_entry)
{
	bool	bSuccess;
	uint32	fat_offset = 0;		/* todo: this one may require 64 bits for large drives? */
	uint32	entry_sector;
	uint32	entry_offset;		/* todo: 16 bits should suffice for this value */

	uint8* buffer = fat_shared_buffer;

	// get the offset of the entry within the FAT table
	// for the requested cluster
	switch (volume->GetFileSystemType())
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
	entry_sector = volume->GetNoOfReservedSectors() + (fat_offset / volume->GetNoOfBytesPerSector());
	entry_offset = fat_offset % volume->GetNoOfBytesPerSector();

	// load sector into the buffer
	if (!IsFatSectorLoaded(entry_sector))
	{
		bSuccess = volume->Read(entry_sector, buffer);
		if (!bSuccess)
		{
			fat_shared_buffer_sector = 0xFFFFFFFF;
			return FAT_CANNOT_READ_MEDIA;
		}
		fat_shared_buffer_sector = (entry_sector);
	}

	// set the user supplied buffer with the
	// value of the FAT entry
	switch (volume->GetFileSystemType())
	{
		case FAT_FS_TYPE_FAT12:
		{
			// clear fat_entry to make sure that the upper 16
			// bits are not set.
			*fat_entry = 0;

			// read the 1st byte
			((uint8*)fat_entry)[0] = buffer[entry_offset];

			// load the next sector (if necessary) and set the offset
			// for the next byte in the buffer
			if (entry_offset == volume->GetNoOfBytesPerSector() - 1)
			{
				// load the next sector into the buffer
				bSuccess = volume->Read(entry_sector + 1, buffer);
				if (!bSuccess)
				{
					fat_shared_buffer_sector = 0xFFFFFFFF;
					return FAT_CANNOT_READ_MEDIA;
				}
				fat_shared_buffer_sector = (entry_sector + 1);

				// the 2nd byte is now the 1st byte in the buffer
				entry_offset = 0;
			}
			else
			{
				// the 2nd byte is still right after the 1st one on
				// the buffer
				entry_offset++;
			}

			// read the 2nd byte
			((uint8*)fat_entry)[1] = buffer[entry_offset];

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
			*fat_entry = (uint32) * ((uint16*)&buffer[entry_offset]);
			break;
		}
		case FAT_FS_TYPE_FAT32:
		{
			*fat_entry = *((uint32*)&buffer[entry_offset]) & 0x0FFFFFFF;
			break;
		}
	}

	return FAT_SUCCESS;
}


// updates the FAT entry for a given cluster
uint16 fat_set_cluster_entry(CFatVolume* volume, uint32 cluster, FatEntry fat_entry)
{
	bool	bSuccess;
	uint32	fat_offset = 0;
	uint32	entry_sector;
	uint32	entry_offset;
	uint8*	buffer = fat_shared_buffer;

	// get the offset of the entry in the FAT table for the requested cluster
	switch (volume->GetFileSystemType())
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
	entry_sector = volume->GetNoOfReservedSectors() + (fat_offset / volume->GetNoOfBytesPerSector());
	entry_offset = fat_offset % volume->GetNoOfBytesPerSector();


	// read sector into buffer
	if (!IsFatSectorLoaded(entry_sector))
	{
		bSuccess = volume->Read(entry_sector, buffer);
		if (!bSuccess)
		{
			fat_shared_buffer_sector = 0xFFFFFFFF;
			return FAT_CANNOT_READ_MEDIA;
		}
		fat_shared_buffer_sector = (entry_sector);
	}

	// set the FAT entry
	switch (volume->GetFileSystemType())
	{
		case FAT_FS_TYPE_FAT12:
		{
			// write the 1st byte
			if (cluster & 0x1)
			{
				fat_entry <<= 4;									/* odd entries occupy the upper 12 bits so we must shift */
				buffer[entry_offset] &= 0x0F;						/* clear entry bits on 1st byte */
				buffer[entry_offset] |= LO8((uint16)fat_entry);		/* set entry bits on 1st byte */
			}
			else
			{
				buffer[entry_offset] = LO8((uint16)fat_entry);	/* just copy the 1st byte */
			}

			// if the FAT entry spans a sector boundary flush the currently
			// loaded sector to the drive and load the next one.
			if (entry_offset == volume->GetNoOfBytesPerSector() - 1)
			{
				// flush the updated sector to the drive
				bSuccess = fat_write_fat_sector(volume, entry_sector, buffer);
				if (!bSuccess)
				{
					fat_shared_buffer_sector = (0xFFFFFFFF);
					return FAT_CANNOT_WRITE_MEDIA;
				}

				// increase the sector address
				entry_sector++;

				// load the next sector
				bSuccess = volume->Read(entry_sector, buffer);
				if (!bSuccess)
				{
					fat_shared_buffer_sector = (0xFFFFFFFF);
					return FAT_CANNOT_READ_MEDIA;
				}
				fat_shared_buffer_sector = (entry_sector);

				// the next byte is now loacted at offset 0 on the buffer
				entry_offset = 0;
			}
			else
			{
				// the next byte is located next to the 1st one on the buffer
				entry_offset++;
			}

			// write the 2nd byte
			if (cluster & 0x1)
			{
				buffer[entry_offset] = HI8((uint16)fat_entry);		/* just copy the 1st byte */
			}
			else
			{
				buffer[entry_offset] &= 0xF0;						/* clear bits that 1st byte will be written to */
				buffer[entry_offset] |= HI8((uint16)fat_entry);		/* copy entry bits of 1st byte */
			}
			break;
		}
		case FAT_FS_TYPE_FAT16:
		{
			*((uint16*)&buffer[entry_offset]) = (uint16)fat_entry;
			break;
		}
		case FAT_FS_TYPE_FAT32:
		{

			// since a FAT32 entry is actually 28 bits we need
			// to make sure that we don't modify the upper nibble.
			*((uint32*)&buffer[entry_offset]) &= 0xF0000000;
			*((uint32*)&buffer[entry_offset]) |= fat_entry & 0x0FFFFFFF;
			break;
		}
	}

	// write the updated sector to the
	// storage device
	bSuccess = fat_write_fat_sector(volume, entry_sector, buffer);
	if (!bSuccess)
	{
		fat_shared_buffer_sector = 0xFFFFFFFF;
		return FAT_CANNOT_WRITE_MEDIA;
	}

	return FAT_SUCCESS;
}


// increase a cluster address by the amount of clusters indicated by count. This function will
// follow the FAT entry chain to fat the count-th cluster allocated to a file relative from the
// current_cluster cluster
char fat_increase_cluster_address(CFatVolume* volume, uint32 cluster, uint16 count, uint32* value)
{
	bool	bSuccess;
	uint32	fat_offset = 0;
	uint32	entry_offset;
	uint32	entry_sector;
	uint32	current_sector;
	char	is_odd_cluster = 0;
	char	op_in_progress = 0;

	uint8* buffer = fat_shared_buffer;

	/*
	// if the count is zero we just return the same
	// cluster that we received
	*/
	if (!count)
	{
		*value = (uint32)cluster;
		return 1;
	}
	/*
	// get the offset of the cluster entry within the FAT table,
	// the sector of the FAT table that contains the entry and the offset
	// of the fat entry within the sector
	*/
	fat_offset = CalculateFatEntryOffset(volume->GetFileSystemType(), cluster);
	entry_sector = volume->GetNoOfReservedSectors() + (fat_offset / volume->GetNoOfBytesPerSector());
	entry_offset = fat_offset % volume->GetNoOfBytesPerSector();

	while (1)
	{
		current_sector = entry_sector;

		// read sector into hte buffer
		if (!IsFatSectorLoaded(current_sector))
		{
			bSuccess = volume->Read(current_sector, buffer);
			if (!bSuccess)
			{
				fat_shared_buffer_sector = 0xFFFFFFFF;
				return 0;
			}
			fat_shared_buffer_sector = (current_sector);
		}

		// free all the fat entries on the current sector
		while (current_sector == entry_sector)
		{
			// make sure we don't try to free an invalid cluster
			if (cluster < 2)
			{
				return FAT_INVALID_CLUSTER;
			}

			// read the cluster entry and mark it as free
			switch (volume->GetFileSystemType())
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
						((uint8*)&cluster)[0] = buffer[entry_offset];
					}

					if (entry_offset == volume->GetNoOfBytesPerSector() - 1)
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
					((uint8*)&cluster)[1] = buffer[entry_offset];

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

					// clear op_in_progress flag.
					op_in_progress = 0;
					break;
				}
				case FAT_FS_TYPE_FAT16:
				{
					cluster = (uint32) *((uint16*)&buffer[entry_offset]);
					break;
				}
				case FAT_FS_TYPE_FAT32:
				{
					cluster = *((uint32*)&buffer[entry_offset]) & 0x0FFFFFFF;
					break;
				}
			}

			// if the last cluster marks the end of the chian we return
			// false as we cannot increase the address by the # of clusters
			// requested by the caller
			if (fat_is_eof_entry(volume, cluster))
			{
				return 0;
			}

			// if we've followed the number of clusters requested by
			// the caller set the return value to the current cluster
			// and return true
			if (!--count)
			{
				*value = (uint32)cluster;
				return 1;
			}

			// calculate the location of the next cluster in the chain
			fat_offset = CalculateFatEntryOffset(volume->GetFileSystemType(), cluster);
			entry_sector = volume->GetNoOfReservedSectors() + (fat_offset / volume->GetNoOfBytesPerSector());
			entry_offset = fat_offset % volume->GetNoOfBytesPerSector();
		}
	}
}


// checks if a fat entry represents the
// last entry of a file
char fat_is_eof_entry(CFatVolume* volume, FatEntry fat)
{
	switch (volume->GetFileSystemType())
	{
		case FAT_FS_TYPE_FAT12: 
			return fat >= 0x0FF8;

		case FAT_FS_TYPE_FAT16: 
			return fat >= 0xFFF8;

		case FAT_FS_TYPE_FAT32: 
			return fat >= 0x0FFFFFF8;
	}

	return 0;
}


// initializes a directory cluster
uint16 fat_initialize_directory_cluster(CFatVolume* volume, SFatRawDirectoryEntry* parent, uint32 cluster, uint8* buffer)
{
	bool						bSuccess;
	uint16						counter;
	uint32						current_sector;
	SFatRawDirectoryEntry*		entries;

	// if this is a big endian system or the compiler does not support
	// struct packing we cannot use SFatRawDirectoryEntry to write directly
	// to the buffer
	entries = (SFatRawDirectoryEntry*)buffer;

	fat_shared_buffer_sector = (0xFFFFFFFF);

	// initialize the 1st sector of the directory cluster with
	// the dot entry
	memset(buffer, 0, volume->GetNoOfBytesPerSector());
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
	entries->uEntry.sFatRawCommon.create_date = rtc_get_fat_date();
	entries->uEntry.sFatRawCommon.create_time = rtc_get_fat_time();
	entries->uEntry.sFatRawCommon.modify_date = entries->uEntry.sFatRawCommon.create_date;
	entries->uEntry.sFatRawCommon.modify_time = entries->uEntry.sFatRawCommon.create_time;
	entries->uEntry.sFatRawCommon.access_date = entries->uEntry.sFatRawCommon.create_date;
	entries->uEntry.sFatRawCommon.create_time_tenth = 0xc6;

	// write the entry to the buffer or move to the next entry
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
	entries->uEntry.sFatRawCommon.create_date = rtc_get_fat_date();
	entries->uEntry.sFatRawCommon.create_time = rtc_get_fat_time();
	entries->uEntry.sFatRawCommon.modify_date = entries->uEntry.sFatRawCommon.create_date;
	entries->uEntry.sFatRawCommon.modify_time = entries->uEntry.sFatRawCommon.create_time;
	entries->uEntry.sFatRawCommon.access_date = entries->uEntry.sFatRawCommon.create_date;
	entries->uEntry.sFatRawCommon.create_time_tenth = 0xc6;

	// when the parent is the root directory the dotdot entry always points
	// to cluster 0, even in FAT32 when the root directory is not actually on
	// cluster 0 so we need to check if the parent is the root directory and
	// in that case set the 1st cluster to 0
	if (volume->GetFileSystemType() == FAT_FS_TYPE_FAT32)
	{
		uint32 parent_cluster;
		((uint16*)&parent_cluster)[INT32_WORD0] = parent->uEntry.sFatRawCommon.first_cluster_lo;
		((uint16*)&parent_cluster)[INT32_WORD1] = parent->uEntry.sFatRawCommon.first_cluster_hi;

		if (volume->GetRootCluster() == parent_cluster)
		{
			entries->uEntry.sFatRawCommon.first_cluster_lo = 0;
			entries->uEntry.sFatRawCommon.first_cluster_hi = 0;
		}
	}
	/*
	// write the 1st sector of the folder
	*/
	current_sector = calculate_first_sector_of_cluster(volume, cluster);
	bSuccess = volume->Write(current_sector++, buffer);
	if (!bSuccess)
	{
		return FAT_CANNOT_WRITE_MEDIA;
	}

	// clear the . and .. entries from the buffer and
	// initialize the rest of the sectors of this cluster
	memset(buffer, 0, sizeof(SFatRawDirectoryEntry) * 2);
	counter = volume->GetNoOfSectorsPerCluster() - 1;
	while (counter--)
	{
		bSuccess = volume->Write(current_sector++, buffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}
	}

	return FAT_SUCCESS;
}


// sets all sectors in a cluster to zeroes
uint16 fat_zero_cluster(CFatVolume* volume, uint32 cluster, uint8* buffer)
{
	bool	bSuccess;
	uint16	counter;
	uint32	current_sector;

	fat_shared_buffer_sector = 0xFFFFFFFF;

	// set all the bytes in the buffer to zero
	memset(buffer, 0, volume->GetNoOfBytesPerSector());

	// calculate the address of the 1st sector
	// of the cluster
	current_sector = calculate_first_sector_of_cluster(volume, cluster);
	counter = volume->GetNoOfSectorsPerCluster();

	// write the zeroed buffer to every sector in the cluster
	while (counter--)
	{
		bSuccess = volume->Write(current_sector++, buffer);
		if (!bSuccess)
		{
			return FAT_CANNOT_WRITE_MEDIA;
		}
	}

	return FAT_SUCCESS;
}



/*
// writes a sector of the FAT table to the active table and (if option is enabled)
// to all other FAT tables
*/
bool fat_write_fat_sector(CFatVolume* volume, uint32 sector_address, uint8* buffer)
{
	bool bSuccess;

	/*
	// write the sector in the active FAT table
	*/
	bSuccess = volume->Write(sector_address, buffer);
	if (!bSuccess)
	{
		return false;
	}
	/*
	// if we got more than one FAT table update the others as well
	*/
#if defined(FAT_MAINTAIN_TWO_FAT_TABLES)
	if (volume->uiNoOfFatTables > 1)
	{
		int i;
		for (i = 1; i < volume->uiNoOfFatTables; i++)
		{
			bSuccess = volume->device->Write(sector_address + (volume->fat_size * i), buffer);
			if (!bSuccess)
			{
				return false;
			}
		}
	}
#endif
	return true;
}

