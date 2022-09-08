#ifndef __FAT_VOLUME_H__
#define __FAT_VOLUME_H__
#include "FileDrive.h"
#include "FatSharedBuffer.h"


struct SFatVolume
{
	uint32			uiID;
	uint32			uiFatSize;
	uint32			root_cluster;
	uint32			first_data_sector;
	uint32			no_of_sectors;
	uint32			no_of_data_sectors;
	uint32			no_of_clusters;
	uint32			no_of_reserved_sectors;
	uint32			next_free_cluster;
	uint32			total_free_clusters;
	uint32			fsinfo_sector;
	uint16			root_directory_sectors;
	uint16			no_of_bytes_per_serctor;
	uint16			no_of_sectors_per_cluster;
	bool			bUseLongFilenames;
	uint8			fs_type;
	uint8			no_of_fat_tables;
	char			label[12];
	CFileDrive*		device;
};


class CFatVolume
{
public:
	SFatVolume		msVolume;
	SFatVolume*		mpsVolume;

public:
	uint16	Mount(CFileDrive* device);
	uint16	Unmount(void);

	uint16 GetSectorSize(void);
};


#endif // !__FAT_VOLUME_H__

