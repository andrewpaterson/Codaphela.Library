#ifndef __FAT_VOLUME_H__
#define __FAT_VOLUME_H__
#include "FileDrive.h"
#include "FatSharedBuffer.h"


enum EFatFileSystemType
{
	FAT_FS_TYPE_UNSPECIFIED,
	FAT_FS_TYPE_FAT12,
	FAT_FS_TYPE_FAT16,		
	FAT_FS_TYPE_FAT32		
};


struct SFatVolume
{
	uint32				uiID;
	uint32				uiFatSize;
	uint32				uiRootCluster;
	uint32				uiFirstDataSector;
	uint32				uiNoOfSectors;
	uint32				uiNoOfDataSectors;
	uint32				uiNoOfClusters;
	uint32				uiNoOfReservedSectors;
	uint32				uiNextFreeCluster;
	uint32				uiTotalFreeClusters;
	uint32				uiFsinfoSector;
	uint16				uiRootDirectorySectors;
	uint16				uiNoOfBytesPerSector;
	uint16				uiNoOfSectorsPerCluster;
	bool				bUseLongFilenames;
	EFatFileSystemType	eFileSystem;
	uint8				uiNoOfFatTables;
	char				szLabel[12];
	CFileDrive*			mpcDevice;
};


class CFatVolume
{
public:
	SFatVolume		msVolume;
	SFatVolume*		mpsVolume;

public:
	uint16				Mount(CFileDrive* device);
	uint16				Unmount(void);

	bool				Read(uint64 uiSector, void* pvData);
	bool				Write(uint64 uiSector, void* pvData);
	bool				Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive);

	uint16				GetSectorSize(void);
	uint32				GetPageSize(void);

	bool				HasNextFreeCluser(void);
	bool				IsFreeFat(uint32 uifat);

	void				SetNextFreeCluster(uint32 uiCluster);
	void				SetTotalFreeClusters(uint32 uiTotalFreeClusters);

	uint32				GetID(void);
	uint32				GetFatSize(void);
	uint32				GetRootCluster(void);
	uint32				GetFirstDataSector(void);
	uint32				GetNoOfSectors(void);
	uint32				GetNoOfDataSectors(void);
	uint32				GetNoOfClusters(void);
	uint32				GetNoOfReservedSectors(void);
	uint32				GetNextFreeCluster(void);
	uint32				GetTotalFreeClusters(void);
	uint32				GetFsinfoSector(void);
	uint16				GetRootDirectorySectors(void);
	uint16				GetNoOfBytesPerSector(void);
	uint16				GetNoOfSectorsPerCluster(void);
	bool				IsUseLongFilenames(void);
	EFatFileSystemType	GetFileSystemType(void);
	uint8				GetNoOfFatTables(void);
	char*				GetLabel(void);
};


#endif // !__FAT_VOLUME_H__

