#ifndef __FAT_VOLUME_H__
#define __FAT_VOLUME_H__
#include "FileDrive.h"
#include "FatStructure.h"
#include "FatCommon.h"
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

	bool				IsFatSectorLoaded(uint32 uiSector);
	uint32				CalculateFatEntryOffset(EFatFileSystemType eFileSystemType, uint32 cluster);


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

public:
	uint32				FatAllocateCluster(SFatRawDirectoryEntry* parent, uint32 count, char zero, uint32 page_size, uint16* result);
	uint32				FatAllocateDirectoryCluster(SFatRawDirectoryEntry* parent, uint16* result);
	uint32				FatAllocateDataCluster(uint32 count, char zero, uint16* result);
	uint32				FatAllocateDataClusterEx(uint32 count, char zero, uint32 page_size, uint16* result);
	uint16				FatFreeClusterChain(uint32 cluster);
	uint16				FatGetClusterEntry(uint32 cluster, FatEntry* fat_entry);
	uint16				FatSetClusterEntry(uint32 cluster, FatEntry fat_entry);
	char				FatIncreaseClusterAddress(uint32 cluster, uint16 count, uint32* value);
	bool				FatIsEOFEntry(FatEntry fat);
	uint16				FatInitializeDirectoryCluster(SFatRawDirectoryEntry* parent, uint32 cluster, uint8* buffer);
	uint16				FatZeroCluster(uint32 cluster, uint8* buffer);
	bool				FatWriteFatSector(uint32 sector_address, uint8* buffer);

	uint16				FatFindFirstEntry(char* parent_path, uint8 attributes, SFatDirectoryEntry** dir_entry, SFatFileSystemQuery* q);
	uint16				FatFindNextEntry(SFatDirectoryEntry** dir_entry, SFatFileSystemQuery* q);
	void				FatFillDirectoryEntryFromRaw(SFatDirectoryEntry* entry, SFatRawDirectoryEntry* raw_entry);
	uint16				FatCreateDirectory(char* directory);
	uint16				FatGetFileEntry(char* path, SFatDirectoryEntry* entry);
	uint16				FatQueryFirstEntry(SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, char buffer_locked);
	uint16				FatCreateDirectoryEntry(SFatRawDirectoryEntry* parent, char* name, uint8 attribs, uint32 entry_cluster, SFatDirectoryEntry* new_entry);
	uint16				FatQueryNextEntry(SFatQueryState* query, char buffer_locked, char first_entry);

	void				FatGetShortNameFromEntry(uint8* dest, const uint8* src);
	uint32				CalculateFirstSectorOfCluster(uint32 cluster);
};


#endif // !__FAT_VOLUME_H__

