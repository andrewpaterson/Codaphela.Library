#ifndef __FAT_VOLUME_OLD_H__
#define __FAT_VOLUME_OLD_H__
#include "FileDrive.h"
#include "FatInfoSectorCache.h"
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
	uint16				uiBytesPerSector;
	uint16				uiNoOfSectorsPerCluster;
	bool				bUseLongFilenames;
	EFatFileSystemType	eFileSystem;
	uint8				uiNoOfFatTables;
	char				szLabel[12];
};


class CFatVolumeOld
{
public:
	SFatVolume				msVolume;
	CFileDrive*				mpcDevice;

		uint8					mauiFatSharedBuffer[MAX_SECTOR_LENGTH];
		uint32					muiFatSharedBufferSector;
		bool					mbEntriesUpdated;

	uint32					muiSharedReadCount;
	CFatInfoSectorCache		mcSectorCache;

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
	uint32				GetRootCluster(void);
	uint32				GetRootSector(void);

	uint32				GetBytesPerCluster(void);
	uint32				GetID(void);
	uint32				GetFatSize(void);
	uint32				GetFat32RootCluster(void);
	uint32				GetFirstDataSector(void);
	uint32				GetNoOfSectors(void);
	uint32				GetNoOfDataSectors(void);
	uint32				GetNumClusters(void);
	uint32				GetNoOfReservedSectors(void);
	uint32				GetNextFreeCluster(void);
	uint32				GetTotalFreeClusters(void);
	uint32				GetFileSystemInfoSector(void);
	uint16				GetRootDirectorySectors(void);
	uint16				GetSectorSize(void);
	uint16				NumSectorsPerCluster(void);
	bool				IsUseLongFilenames(void);
	EFatFileSystemType	GetFileSystemType(void);
	uint8				GetNoOfFatTables(void);
	char*				GetLabel(void);
	uint8*				GetFatSharedBuffer(void);
	uint32				GetFatSharedBufferSector(void);
	CFileDrive*			GetFileDrive(void);

public:
	uint32				FatAllocateCluster(SFatRawDirectoryEntry* parent, uint32 uiClusterCount, bool bEraseCluster, uint32 page_size, EFatCode* result);
	uint32				FatAllocateDirectoryCluster(SFatRawDirectoryEntry* parent, EFatCode* result);
	uint32				FatAllocateDataCluster(uint32 uiClusterCount, bool bEraseCluster, EFatCode* peResult);
	uint32				FatAllocateDataClusterEx(uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult);
	EFatCode			FatFreeClusterChain(uint32 uiClusterIndex);
	EFatCode			GetNextClusterEntry(uint32 uiCluster, uint32* puiNextCluster);
	EFatCode			FatSetClusterEntry(uint32 uiClusterIndex, fatEntry uiClusterInVolume);
	EFatCode			FatIncreaseClusterAddress(uint32 uiClusterIndex, uint16 uiClusterCount, uint32* puiNewClusterIndex);
	bool				FatIsEOFEntry(fatEntry fat);
	EFatCode			FatInitializeDirectoryCluster(SFatRawDirectoryEntry* parent, uint32 cluster);
	EFatCode			FatWriteFatSector(uint32 uiSectorAddress, uint8* puiBuffer);
	EFatCode			FatReadFatSector(uint32 uiFatInfoSector);
	EFatCode			FatReadFatSector(uint32 uiFatInfoSector, uint8* puiBuffer);
	EFatCode			FatFlushFatSector(void);
	uint8*				ReadInfoSector(uint32 uiFatInfoSector);
	void				SetInfoSectorDirty(uint8* pvCachedSector);

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
	uint32				CalculateFATEntryIndex(uint32 cluster);

	EFatCode			ReadFatEntry(uint32 uiOffsetInSector, uint32 uiClusterIndex, uint32 uiFat12Sector, fatEntry* puiFatEntry);
	void				CalculateFATIndexAndOffset(uint32* puiOffsetInSector, uint32 uiClusterIndex, uint32* puiSector);

protected:
	void				TrimPath(char* dest, char* src, size_t max);
	void				FatParsePath(char* path, char* szPathPart, char** pszFilenamePart);
	bool				FatCompareShortName(char* szName1, char* szName2);
	char				FatCompareLongName(uint16* name1, uint16* puiName2);
	EFatCode			GetShortNameForEntry(uint8* dest, uint8* src, bool bLFNDisabled);
	uint8				FatLongEntryChecksum(uint8* filename);
	int					FatIndexOf(char chr, char* str, int index);

	uint32				FindNextPageCluster(uint32 uiPageSize, uint32 uiCluster, uint16 uiStep);
	EFatCode			WrapClusterSearch(uint32 uiStartCluster, uint32 uiLastFatEntry, bool* pbWrappedAround, uint32* puiClusterIndexInTable, uint32* puiOffsetInSector, uint32* puiFirstClusterSector);
	uint16				CalculateClusterStepSize(uint32 uiPageSize);
	fatEntry			GetEndOfClusterMarker(void);
	EFatCode			FlushAndInvalidate(void);
	EFatCode			FatZeroCluster(uint32 cluster);

	EFatCode			InitialiseAllocatedFatCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 uiClusterIndexInTable, bool bZero);

	EFatCode			ReadFat12Entry(uint32 uiClusterBytesRemainder, uint32 uiCluster, uint32 uiSector, fatEntry* puiFatEntry);

	EFatCode			WriteAllocatedFatEntry(uint32 uiClusterIndexInTable, uint32 uiClusterBytesRemainder, uint32 uiFirstClusterSector, fatEntry uiLastFatEntry, uint32 uiPreviousClusterSector, uint32 uiLastEntryOffset);
	EFatCode			WriteFat12Entry(uint32 uiClusterBytesRemainder, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSecto, fatEntry uiLastFatEntry);
	EFatCode			WriteFat16Entry(uint32 uiClusterBytesRemainder, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSecto, fatEntry uiLastFatEntry);
	EFatCode			WriteFat32Entry(uint32 uiClusterBytesRemainder, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSecto, fatEntry uiLastFatEntry);

	EFatCode			Fat12IncreaseClusterAddress(uint32 uiClusterIndex, uint32* puiFirstClusterSector, uint32* puiOffsetInSector, bool* pbFat12OddClusterBeingProcessed, bool* pbFat12MultiStepProgress);

	EFatCode			CreateFakeRootEntry(SFatDirectoryEntry* psEntry);
	char*				FindNextPathItem(char* szPath, char* szCurrentLevelPath);
	EFatCode			MatchesFileName(bool* pbMatch, bool* pbUsingLFN, char* szConstructedShortFileName, uint16* puiTargetFileLongName, char* szCurrentLevelPath, SFatQueryState* psQuery);
};


#endif // !__FAT_VOLUME_H__

