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
	bool			mbEntriesUpdated;

public:
	EFatCode			Mount(CFileDrive* device);
	EFatCode			Unmount(void);

	bool				Read(uint64 uiSector, void* pvData);
	bool				Read(uint64 uiSector, uint32 uiNumSectors, void* pvData);
	bool				Write(uint64 uiSector, void* pvData);
	bool				Write(uint64 uiSector, uint32 uiNumSectors, void* pvData);
	bool				Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive);

	EFatCode			FatFileDelete(char* filename);
	EFatCode			FatFileRename(char* original_filename, char* new_filename);

	uint32				GetPageSize(void);
	uint32				GetClusterSize(void);

	bool				HasNextFreeCluser(void);
	bool				IsFreeFat(uint32 uifat);

	void				SetNextFreeCluster(uint32 uiCluster);
	void				SetTotalFreeClusters(uint32 uiTotalFreeClusters);
	void				SetFatSharedBufferSector(uint32 uiSector);

	bool				IsFatSectorLoaded(uint32 uiSector);

	uint32				GetBytesPerCluster(void);
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
	uint16				GetSectorSize(void);
	uint16				GetNoOfSectorsPerCluster(void);
	bool				IsUseLongFilenames(void);
	EFatFileSystemType	GetFileSystemType(void);
	uint8				GetNoOfFatTables(void);
	char*				GetLabel(void);
	uint8*				GetFatSharedBuffer(void);
	uint32				GetFatSharedBufferSector(void);
	CFileDrive*			GetFileDrive(void);

public:
	uint32				FatAllocateCluster(SFatRawDirectoryEntry* parent, uint32 count, char zero, uint32 page_size, EFatCode* result);
	uint32				FatAllocateDirectoryCluster(SFatRawDirectoryEntry* parent, EFatCode* result);
	uint32				FatAllocateDataCluster(uint32 count, char zero, EFatCode* result);
	uint32				FatAllocateDataClusterEx(uint32 count, char zero, uint32 page_size, EFatCode* result);
	EFatCode			FatFreeClusterChain(uint32 uiCluster);
	EFatCode			GetNextClusterEntry(uint32 uiCluster, uint32* puiNextCluster);
	EFatCode			FatSetClusterEntry(uint32 uiCluster, fatEntry fat_entry);
	EFatCode			FatIncreaseClusterAddress(uint32 cluster, uint16 count, uint32* value);
	bool				FatIsEOFEntry(fatEntry fat);
	EFatCode			FatInitializeDirectoryCluster(SFatRawDirectoryEntry* parent, uint32 cluster);
	EFatCode			FatWriteFatSector(uint32 uiSectorAddress, uint8* puiBuffer);
	EFatCode			FatReadFatSector(uint32 uiFatInfoSector);
	EFatCode			FatReadFatSector(uint32 uiFatInfoSector, uint8* puiBuffer);
	EFatCode			FatFlushFatSector(void);

	EFatCode			FatFindFirstEntry(char* parent_path, uint8 attributes, SFatDirectoryEntry** dir_entry, SFatFileSystemQuery* q);
	EFatCode			FatFindNextEntry(SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* psQuery);
	void				FatFillDirectoryEntryFromRaw(SFatDirectoryEntry* psEntry, SFatRawDirectoryEntry* psRawEntry);
	EFatCode			FatCreateDirectory(char* szDirectory);
	EFatCode			FatGetFileEntry(char* szPath, SFatDirectoryEntry* psEntry);
	EFatCode			FatQueryFirstEntry(SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, bool bBufferLocked);
	EFatCode			FatCreateDirectoryEntry(SFatRawDirectoryEntry* psParent, char* szName, uint8 attribs, uint32 uiEntryCluster, SFatDirectoryEntry* psNewEntry);
	EFatCode			FatQueryNextEntry(SFatQueryState* query, bool bBufferLocked, bool bFirstEntry);

	void				FatGetShortNameFromEntry(uint8* dest, const uint8* src);
	uint32				CalculateFirstSectorOfCluster(uint32 uiCluster);
	char				GetLongNameForEntry(uint16* dst, uint8* src);

protected:
	void				TrimPath(char* dest, char* src, size_t max);
	void				FatParsePath(char* path, char* szPathPart, char** pszFilenamePart);
	bool				FatCompareShortName(char* szName1, char* szName2);
	char				FatCompareLongName(uint16* name1, uint16* puiName2);
	EFatCode			GetShortNameForEntry(uint8* dest, uint8* src, bool bLFNDisabled);
	uint8				FatLongEntryChecksum(uint8* filename);
	int					FatIndexOf(char chr, char* str, int index);

	uint32				FindNextPageCluster(uint32 uiPageSize, uint32 uiCluster, uint16 uiStep);
	uint16				CalculateStepSize(uint32 uiPageSize);
	fatEntry			GetEndOfClusterMarker(void);
	uint32				CalculateFatEntryOffset(uint32 cluster);
	EFatCode			FlushAndInvalidate(void);
	EFatCode			FatZeroCluster(uint32 cluster);

	EFatCode			WriteAllocatedFatEntry(uint32 uiClusterIndexInTable, uint32 uiClusterBytesRemainder, uint32 uiFirstClusterSector, fatEntry uiLastFatEntry, uint32 uiPreviousClusterSector, uint32 uiLastEntryOffset);
	EFatCode			InitialiseAllocatedFatCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 uiClusterIndexInTable, bool bZero);

	EFatCode			ReadFat12Entry(uint32 uiClusterBytesRemainder, uint32 uiCluster, uint32 uiSector, fatEntry* puiFatEntry);
	EFatCode			WriteFat12Entry(uint32 uiClusterBytesRemainder, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSecto, fatEntry uiLastFatEntry);
	EFatCode			WriteFat16Entry(uint32 uiClusterBytesRemainder, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSecto, fatEntry uiLastFatEntry);
	EFatCode			WriteFat32Entry(uint32 uiClusterBytesRemainder, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSecto, fatEntry uiLastFatEntry);

	EFatCode			CreateFakeRootEntry(SFatDirectoryEntry* psEntry);
	char*				FindNextPathItem(char* szPath, char* szCurrentLevelPath);
	EFatCode			MatchesFileName(bool* pbMatch, bool* pbUsingLFN, char* szConstructedShortFileName, uint16* puiTargetFileLongName, char* szCurrentLevelPath, SFatQueryState* psQuery);
};


#endif // !__FAT_VOLUME_H__

