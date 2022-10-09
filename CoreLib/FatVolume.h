#ifndef __FAT_VOLUME_H__
#define __FAT_VOLUME_H__
#include "BaseLib/Chars.h"
#include "FileDrive.h"
#include "FatInfoSectorCache.h"
#include "FatCommon.h"


class CFatVolume
{
protected:
	uint32					muiID;
	uint32					muiFatSize;
	uint32					muiRootCluster;
	uint32					muiRootSector;
	uint32					muiFirstDataSector;
	uint32					muiNoOfSectors;
	uint32					muiNoOfDataSectors;
	uint32					muiNoOfClusters;
	uint32					muiNoOfReservedSectors;
	uint32					muiNextFreeCluster;
	uint32					muiTotalFreeClusters;
	uint32					muiFileSystemInfoSector;
	uint32					muiBytesPerCluster;
	uint16					muiRootDirectorySectors;
	uint16					muiBytesPerSector;
	uint16					muiNoOfSectorsPerCluster;
	bool					mbUseLongFilenames;
	EFatFileSystemType		meFileSystem;
	uint8					muiNoOfFatTables;
	CChars					mszLabel;

	CFileDrive*				mpcDevice;
	CFatInfoSectorCache		mcSectorCache;

	SFatBIOSParameterBlock	msBPB;

public:
	EFatCode				Mount(CFileDrive* device);
	EFatCode				Unmount(void);
	EFatCode				Flush(void);
	bool					Erase(uint32 uiStartSector, uint32 uiStopSectorInclusive);
	EFatCode				CreateDirectory(char* szDirectory);
	EFatCode				RenameFile(char* szOriginalFilename, char* szNewFilename);
	EFatCode				DeleteFile(char* szFilename);

	//Suspect
	EFatCode				Allocate(uint32 uiClustersNeeded, uint32 uiCursorClusterInVolume, uint32* puiNewClusterInVolume, uint32 uiPreviousCluster);
	EFatCode				Free(SFatDirectoryEntry* psEntry);
	SFatCache				ReadSector(uint32 uiFatInfoSector, bool bLock = false);
	void					DirtySector(SFatCache sCachedSector);
	void					CalculateFATIndexAndOffset(uint32* puiOffsetInSector, uint32 uiClusterIndex, uint32* puiSector);
	EFatCode				GetFileEntry(char* szPath, SFatDirectoryEntry* psEntry);
	uint32					CalculateFirstSectorOfCluster(uint32 uiCluster);
	EFatCode				GetNextClusterEntry(uint32 uiCurrentCluster, fatEntry* puiNextCluster);
	EFatCode				IncreaseClusterAddress(uint32 uiClusterIndex, uint16 uiClusterCount, uint32* puiNewClusterIndex);
	bool					FatIsEOFEntry(fatEntry uiFat);
	EFatCode				FreeChain(uint32 uiClusterIndex);
	EFatCode				SetClusterEntry(uint32 uiClusterIndex, fatEntry uiClusterInVolume);
	EFatCode				ReadFatEntry(uint32 uiOffsetInSector, uint32 uiClusterIndex, uint32 uiFat12Sector, fatEntry* puiFatEntry);
	EFatCode				UpdateDirectoryEntry(SFatDirectoryEntry* psEntry, uint32 uiCluster, uint32 uiNewClusterInVolume, uint32 uiClustersInFile);
	EFatCode				FindFirstFATEntry(char* szParentPath, uint8 uiAttributes, SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* psQuery);
	EFatCode				FindNextFATEntry(SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* psQuery);
	EFatCode				CreateFATEntry(SFatRawDirectoryEntry* psParentDirectory, char* szName, uint8 uiAttributes, uint32 uiEntryCluster, SFatDirectoryEntry* psNewEntry);
	fatEntry				GetEndOfClusterMarker(void);
	fatEntry				GetBadClusterMarker(void);
	fatEntry				GetClusterEntryMask(void);
	uint32					GetEntryCluster(SFatDirectoryEntry* psEntry);

	EFatCode				ValidateFatCache(SFatCache sCache);

	//Broken
	EFatCode				CountSequentialClusters(uint32 uiCluster, uint32* puiSequentialClustersAfter);

	//Getters
	uint32					GetClusterSize(void);
	uint32					GetID(void);
	uint32					GetFatSize(void);
	uint32					GetFirstDataSector(void);
	uint32					GetNoOfSectors(void);
	uint32					GetNoOfDataSectors(void);
	uint32					GetNumClusters(void);
	uint32					GetNoOfReservedSectors(void);
	uint32					GetNextFreeCluster(void);
	uint32					GetTotalFreeClusters(void);
	uint32					GetFileSystemInfoSector(void);
	uint16					GetRootDirectorySectors(void);
	uint16					GetSectorSize(void);
	uint16					NumSectorsPerCluster(void);
	bool					IsUseLongFilenames(void);
	EFatFileSystemType		GetFileSystemType(void);
	uint8					GetNoOfFatTables(void);
	char*					GetLabel(void);
	CFileDrive*				GetFileDrive(void);
	uint32					GetRootCluster(void);
	uint32					GetRootSector(void);
	uint32					GetPageSize(void);
	CFatInfoSectorCache*	GetSectorCache(void);

protected:
	EFatCode				QueryFirstEntry(SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, bool bBufferLocked);
	EFatCode				QueryNextEntry(SFatQueryState* psQuery, bool bBufferLocked, bool bFirstEntry);

	uint32					AllocateDirectoryCluster(SFatRawDirectoryEntry* parent, EFatCode* peResult);
	uint32					AllocateDataCluster(uint32 uiClusterCount, bool bEraseCluster, EFatCode* peResult, uint32 uiPreviousCluster);
	uint32					AllocateDataClusterEx(uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult, uint32 uiPreviousCluster);
	uint32					AllocateCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult, uint32 uiPreviousCluster);

	EFatCode				FreeFat12Chain(bool* pbFat12MultiStepProgress, uint32* puiClusterIndex, uint32* puiSector, uint32* puiOffsetInSector);
	EFatCode				FreeFat16Chain(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector);
	EFatCode				FreeFat32Chain(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector);

	uint32					CalculateFATEntryIndex(uint32 uiClusterIndexInFAT);
	uint8					FatLongEntryChecksum(uint8* filename);
	uint32					CalculateRootSector(void);

	EFatCode				SetFat12ClusterEntry(uint32 uiClusterIndex, fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector);
	EFatCode				SetFat16ClusterEntry(fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector);
	EFatCode				SetFat32ClusterEntry(fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector);

	EFatCode				IncreaseFat12ClusterAddress(uint32* puiClusterIndex, uint32* puiSector, uint32* puiOffsetInSector, bool* pbFat12OddClusterBeingProcessed, bool* pbFat12MultiStepProgress);
	EFatCode				IncreaseFat16ClusterAddress(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector);
	EFatCode				IncreaseFat32ClusterAddress(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector);

	EFatCode				ReadFat12Entry(uint32 uiOffsetInSector, uint32 uiCluster, uint32 uiSector, fatEntry* puiFatEntry);
	EFatCode				ReadFat16Entry(uint32 uiOffsetInSector, uint32 uiSector, fatEntry* puiFatEntry);
	EFatCode				ReadFat32Entry(uint32 uiOffsetInSector, uint32 uiSector, fatEntry* puiFatEntry);

	EFatCode				WriteFat12Entry(uint32 uiOffsetInSector, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiPreviousFatEntry);
	EFatCode				WriteFat16Entry(uint32 uiOffsetInSector, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiPreviousFatEntry);
	EFatCode				WriteFat32Entry(uint32 uiOffsetInSector, uint32 uiPreviousEntryOffset, uint32 uiClusterIndexInTable, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiPreviousFatEntry);
	EFatCode				WriteAllocatedFatEntry(uint32 uiClusterIndexInTable, uint32 uiOffsetInSector, uint32 uiSector, fatEntry uiPreviousFatEntry, uint32 uiPreviousSector, uint32 uiPreviousEntryOffset);

	void					SetNextFreeCluster(uint32 uiCluster);
	void					SetTotalFreeClusters(uint32 uiTotalFreeClusters);
	uint32					FindNextPageCluster(uint32 uiPageSize, uint32 uiCluster, uint16 uiStep);
	bool					HasNextFreeCluser(void);
	uint16					CalculateClusterStepSize(uint32 uiPageSize);
	EFatCode				WrapAllocate(uint32 uiStartCluster, uint32 uiLastFatEntry, bool* pbWrappedAround, uint32* puiClusterIndex, uint32* puiOffsetInSector, uint32* puiFirstClusterSector);
	EFatCode				InitialiseAllocatedFatCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 uiCluster, bool bZero);
	EFatCode				InitialiseCluster(uint32 uiCluster);
	EFatCode				InitializeDirectoryCluster(SFatRawDirectoryEntry* psDirectoryParent, uint32 uiCluster);
	bool					IsFreeFat(fatEntry uifat);

	EFatCode				CreateFakeRootEntry(SFatDirectoryEntry* psEntry);
	EFatCode				MatchesFileName(bool* pbMatch, bool* pbUsingLFN, char* szConstructedShortFileName, uint16* puiTargetFileLongName, char* szCurrentLevelPath, SFatQueryState* psQuery);
	EFatCode				GetShortNameForEntry(uint8* dest, uint8* src, bool bLFNDisabled);
	void					FillDirectoryEntryFromRawEntry(SFatDirectoryEntry* psEntry, SFatRawDirectoryEntry* psRawEntry);

	EFatCode				FindBiosParameterBlock(SFatCache sMBRSector);
	bool					CheckSectorsPerClusterIsPowerOfTwo(uint8 uiSectorsPerCluster);
	bool					CheckFileAllocationTableLargeEnough(EFatFileSystemType eFileSystem, uint32 uiFatSize, uint32 uiNoOfClusters, uint16 uiBytesPerSector);

	bool					IsIllegalFilenameCharacter(char c);
	bool					IsIllegalFilename(char* szName, uint16 uiLength);
	void					GenerateShortNameWithSuffix(uint16 uiNameSuffix, uint8* szShortName);
	EFatCode				GenerateUniqueShortNameWithSuffix(SFatRawDirectoryEntry* psParentDirectory, uint8* szShortName);
	void					InitialiseNewEntry(SFatDirectoryEntry* psNewEntry, char* szName, uint8 uiAttributes, uint32 uiEntryCluster);
	void					InitialiseParentEntry(SFatRawDirectoryEntry* psParentEntry, char* szName, char uiChecksum, int iLFNEntriesNeeded, int iLFNEntriesFound);
	void					SetQueryLongFilenamePart(SFatQueryState* psQuery);
	EFatCode				FindNextRawDirectoryEntry(SFatQueryState* psQuery);

	void					FatParsePath(char* path, char* szPathPart, char** filename_part);
};


void ConvertFATShortInternalNameInto8Dot3Format(uint8* puiDest, const uint8* puiSource);


#endif __FAT_VOLUME_H__

