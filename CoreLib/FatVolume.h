#ifndef __FAT_VOLUME_H__
#define __FAT_VOLUME_H__
#include "BaseLib/Chars.h"
#include "FileDrive.h"
#include "FatInfoSectorCache.h"
#include "FatCommon.h"


class CFatVolume
{
protected:
	uint32					uiID;
	uint32					uiFatSize;
	uint32					uiRootCluster;
	uint32					uiRootSector;
	uint32					uiFirstDataSector;
	uint32					uiNoOfSectors;
	uint32					uiNoOfDataSectors;
	uint32					uiNoOfClusters;
	uint32					uiNoOfReservedSectors;
	uint32					uiNextFreeCluster;
	uint32					uiTotalFreeClusters;
	uint32					uiFileSystemInfoSector;
	uint32					uiBytesPerCluster;
	uint16					uiRootDirectorySectors;
	uint16					uiBytesPerSector;
	uint16					uiNoOfSectorsPerCluster;
	bool					bUseLongFilenames;
	EFatFileSystemType		eFileSystem;
	uint8					uiNoOfFatTables;
	CChars					mszLabel;

	CFileDrive*				mpcDevice;
	CFatInfoSectorCache		mcSectorCache;

public:
	EFatCode			Mount(CFileDrive* device);
	EFatCode			Unmount(void);
	EFatCode			Flush(void);
	bool				Erase(uint32 uiStartSector, uint32 uiStopSectorInclusive);

	EFatCode			Allocate(uint32 uiClustersNeeded, uint32 uiCursorClusterInVolume, uint32* puiNewClusterInVolume);
	EFatCode			Free(SFatDirectoryEntry* psEntry);
	EFatCode			UpdateDirectoryEntry(SFatDirectoryEntry* psEntry, uint32 uiCluster, uint32 uiNewClusterInVolume, uint32 uiClustersInFile);

	uint8*				ReadInfoSector(uint32 uiFatInfoSector, bool bLock = false);
	void				SetInfoSectorDirty(uint8* pvCachedSector);

	uint32				GetClusterSize(void);
	uint32				GetID(void);
	uint32				GetFatSize(void);
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
	CFileDrive*			GetFileDrive(void);
	uint32				GetRootCluster(void);
	uint32				GetRootSector(void);
	uint32				GetPageSize(void);

protected:
	EFatCode			QueryFirstEntry(SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, bool bBufferLocked);
	EFatCode			QueryNextEntry(SFatQueryState* psQuery, bool bBufferLocked, bool bFirstEntry);

	uint32				AllocateDirectoryCluster(SFatRawDirectoryEntry* parent, EFatCode* peResult);
	uint32				AllocateDataCluster(uint32 uiClusterCount, bool bEraseCluster, EFatCode* peResult);
	uint32				AllocateDataClusterEx(uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult);
	uint32				AllocateCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult);

	EFatCode			GetNextClusterEntry(uint32 uiCurrentCluster, uint32* puiNextCluster);

	EFatCode			FreeFat12Chain(bool* pbFat12MultiStepProgress, uint32* puiClusterIndex, uint32* puiSector, uint32* puiOffsetInSector);
	EFatCode			FreeFat16Chain(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector);
	EFatCode			FreeFat32Chain(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector);
	EFatCode			FreeChain(uint32 uiClusterIndex);

	uint32				CalculateFATEntryIndex(uint32 uiClusterIndexInFAT);
	uint32				CalculateFirstSectorOfCluster(uint32 uiCluster);
	uint8				FatLongEntryChecksum(uint8* filename);
	bool				FatIsEOFEntry(fatEntry uiFat);
	uint32				CalculateRootSector(void);

	EFatCode			SetFat12ClusterEntry(uint32 uiClusterIndex, fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector);
	EFatCode			SetFat16ClusterEntry(fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector);
	EFatCode			SetFat32ClusterEntry(fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector);
	EFatCode			SetClusterEntry(uint32 uiClusterIndex, fatEntry uiClusterInVolume);

	EFatCode			IncreaseFat12ClusterAddress(uint32* puiClusterIndex, uint32* puiSector, uint32* puiOffsetInSector, bool* pbFat12OddClusterBeingProcessed, bool* pbFat12MultiStepProgress);
	EFatCode			IncreaseFat16ClusterAddress(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector);
	EFatCode			IncreaseFat32ClusterAddress(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector);
	EFatCode			IncreaseClusterAddress(uint32 uiClusterIndex, uint16 uiClusterCount, uint32* puiNewClusterIndex);

	EFatCode			ReadFat12Entry(uint32 uiOffsetInSector, uint32 uiCluster, uint32 uiSector, fatEntry* puiFatEntry);
	EFatCode			ReadFat16Entry(uint32 uiOffsetInSector, uint32 uiSector, fatEntry* puiFatEntry);
	EFatCode			ReadFat32Entry(uint32 uiOffsetInSector, uint32 uiSector, fatEntry* puiFatEntry);
	EFatCode			ReadFatEntry(uint32 uiOffsetInSector, uint32 uiClusterIndex, uint32 uiFat12Sector, fatEntry* puiFatEntry);

	EFatCode			WriteFat12Entry(uint32 uiOffsetInSector, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiLastFatEntry);
	EFatCode			WriteFat16Entry(uint32 uiOffsetInSector, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiLastFatEntry);
	EFatCode			WriteFat32Entry(uint32 uiOffsetInSector, uint32 uiLastEntryOffset, uint32 uiClusterIndexInTable, uint32 uiSector, uint32 uiLastSector, fatEntry uiLastFatEntry);
	EFatCode			WriteAllocatedFatEntry(uint32 uiClusterIndexInTable, uint32 uiOffsetInSector, uint32 uiSector, fatEntry uiLastFatEntry, uint32 uiLastSector, uint32 uiLastEntryOffset);

	void				SetNextFreeCluster(uint32 uiCluster);
	void				SetTotalFreeClusters(uint32 uiTotalFreeClusters);
	uint32				FindNextPageCluster(uint32 uiPageSize, uint32 uiCluster, uint16 uiStep);
	fatEntry			GetEndOfClusterMarker(void);
	bool				HasNextFreeCluser(void);
	uint16				CalculateClusterStepSize(uint32 uiPageSize);
	EFatCode			WrapAllocate(uint32 uiStartCluster, uint32 uiLastFatEntry, bool* pbWrappedAround, uint32* puiClusterIndex, uint32* puiOffsetInSector, uint32* puiFirstClusterSector);
	EFatCode			InitialiseAllocatedFatCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 uiCluster, bool bZero, uint32 uiSector);
	EFatCode			InitialiseCluster(uint32 uiCluster);
	EFatCode			InitializeDirectoryCluster(SFatRawDirectoryEntry* psDirectoryParent, uint32 uiCluster, uint32 uiSector);
	bool				IsFreeFat(fatEntry uifat);

public:
	void				CalculateFATIndexAndOffset(uint32* puiOffsetInSector, uint32 uiClusterIndex, uint32* puiSector);
};


#endif __FAT_VOLUME_H__

