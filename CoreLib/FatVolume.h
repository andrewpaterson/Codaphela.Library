#ifndef __FAT_VOLUME_H__
#define __FAT_VOLUME_H__
#include "FileDrive.h"
#include "FatCommon.h"


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
	uint32				uiFileSystemInfoSector;
	uint16				uiRootDirectorySectors;
	uint16				uiNoOfBytesPerSector;
	uint16				uiNoOfSectorsPerCluster;
	bool				bUseLongFilenames;
	EFatFileSystemType	eFileSystem;
	uint8				uiNoOfFatTables;
	char				szLabel[12];
};


class CFatVolume
{
public:
	SFatVolume		msVolume;
	CFileDrive*		mpcDevice;

	uint8			mauiFatSharedBuffer[MAX_SECTOR_LENGTH];
	uint32			muiFatSharedBufferSector;

public:
	EFatCode			Mount(CFileDrive* device);
	EFatCode			Unmount(void);

	bool				Read(uint64 uiSector, void* pvData);
	bool				Write(uint64 uiSector, void* pvData);
	bool				Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive);

	EFatCode			FatFileDelete(char* filename);
	EFatCode			FatFileRename(char* original_filename, char* new_filename);

	uint16				GetSectorSize(void);
	uint32				GetPageSize(void);

	bool				HasNextFreeCluser(void);
	bool				IsFreeFat(uint32 uifat);

	void				SetNextFreeCluster(uint32 uiCluster);
	void				SetTotalFreeClusters(uint32 uiTotalFreeClusters);
	void				SetFatSharedBufferSector(uint32 uiSector);

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
	uint8*				GetFatSharedBuffer(void);
	uint32				GetFatSharedBufferSector(void);

public:
	uint32				FatAllocateCluster(SFatRawDirectoryEntry* parent, uint32 count, char zero, uint32 page_size, EFatCode* result);
	uint32				FatAllocateDirectoryCluster(SFatRawDirectoryEntry* parent, EFatCode* result);
	uint32				FatAllocateDataCluster(uint32 count, char zero, EFatCode* result);
	uint32				FatAllocateDataClusterEx(uint32 count, char zero, uint32 page_size, EFatCode* result);
	EFatCode			FatFreeClusterChain(uint32 cluster);
	EFatCode			FatGetClusterEntry(uint32 cluster, FatEntry* fat_entry);
	EFatCode			FatSetClusterEntry(uint32 cluster, FatEntry fat_entry);
	EFatCode			FatIncreaseClusterAddress(uint32 cluster, uint16 count, uint32* value);
	bool				FatIsEOFEntry(FatEntry fat);
	EFatCode			FatInitializeDirectoryCluster(SFatRawDirectoryEntry* parent, uint32 cluster, uint8* uiBuffer);
	EFatCode			FatZeroCluster(uint32 cluster, uint8* uiBuffer);
	bool				FatWriteFatSector(uint32 uiSectorAddress, uint8* puiBuffer);

	EFatCode			FatFindFirstEntry(char* parent_path, uint8 attributes, SFatDirectoryEntry** dir_entry, SFatFileSystemQuery* q);
	EFatCode			FatFindNextEntry(SFatDirectoryEntry** dir_entry, SFatFileSystemQuery* q);
	void				FatFillDirectoryEntryFromRaw(SFatDirectoryEntry* entry, SFatRawDirectoryEntry* raw_entry);
	EFatCode			FatCreateDirectory(char* directory);
	EFatCode			FatGetFileEntry(char* path, SFatDirectoryEntry* entry);
	EFatCode			FatQueryFirstEntry(SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, char buffer_locked);
	EFatCode			FatCreateDirectoryEntry(SFatRawDirectoryEntry* parent, char* name, uint8 attribs, uint32 uiEntryCluster, SFatDirectoryEntry* psNewEntry);
	EFatCode			FatQueryNextEntry(SFatQueryState* query, char buffer_locked, char first_entry);

	void				FatGetShortNameFromEntry(uint8* dest, const uint8* src);
	uint32				CalculateFirstSectorOfCluster(uint32 cluster);

protected:
	void				TrimPath(char* dest, char* src, size_t max);
	void				FatParsePath(char* path, char* szPathPart, char** filename_part);
	char				GetLongNameForEntry(uint16* dst, uint8* src);
	char				FatCompareShortName(uint8* name1, uint8* name2);
	char				FatCompareLongName(uint16* name1, uint16* name2);
	EFatCode			GetShortNameForEntry(uint8* dest, uint8* src, char lfn_disabled);
	uint8				FatLongEntryChecksum(uint8* filename);
	int					FatIndexOf(char chr, char* str, int index);
};


#endif // !__FAT_VOLUME_H__

