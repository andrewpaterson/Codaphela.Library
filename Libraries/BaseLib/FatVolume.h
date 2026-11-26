#ifndef __FAT_VOLUME_H__
#define __FAT_VOLUME_H__
#include "Chars.h"
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
	uint32					muiHiddenSectors;
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
	uint8					muiBytesPerSectorShift;		//Sector, cluster and sectors per cluster sizes are powers of two (Mount
	uint8					muiBytesPerClusterShift;	//checks): bytes and sectors are shifted and masked to clusters and
	uint8					muiSectorsPerClusterShift;	//sectors, not divided (the 65C64s divide in software).
	bool					mbUseLongFilenames;
	EFatFileSystemType		meFileSystem;
	uint8					muiNoOfFatTables;
	CChars					mszLabel;

	CFileDrive*				mpcDevice;
	CFatInfoSectorCache		mcSectorCache;

	SFatBIOSParameterBlock	msBPB;

public:
	EFatCode				Mount(CFileDrive* pcDevice) noexcept;
	EFatCode				Unmount(void) noexcept;
	EFatCode				Flush(void) noexcept;
	bool					Erase(uint32 uiStartSector, uint32 uiStopSectorInclusive) noexcept;
	EFatCode				CreateDirectory(const char* szDirectory) noexcept;
	EFatCode				Rename(const char* szOriginalFilename, const char* szNewFilename) noexcept;
	EFatCode				Delete(const char* szFilename) noexcept;

	EFatCode				AllocateClusters(uint32 uiClustersNeeded, uint32 uiCursorClusterInVolume, uint32* puiNewClusterInVolume, uint32 uiPreviousCluster) noexcept;
	EFatCode				FreeDirectoryEntry(SFatDirectoryEntry* psEntry) noexcept;
	SFatCache				ReadSector(uint32 uiFatInfoSector, bool bLock = false) noexcept;
	void					DirtySector(SFatCache sCachedSector) noexcept;
	void					CalculateFATIndexAndOffset(uint32* puiOffsetInSector, uint32 uiClusterIndex, uint32* puiSector) noexcept;
	EFatCode				GetFileEntry(const char* szPath, SFatDirectoryEntry* psEntry) noexcept;
	uint32					CalculateFirstSectorOfCluster(uint32 uiCluster) noexcept;
	EFatCode				GetNextClusterEntry(uint32 uiCurrentCluster, fatEntry* puiNextCluster) noexcept;
	EFatCode				SeekByClusterCount(uint32 uiClusterIndex, uint16 uiClusterCount, uint32* puiNewClusterIndex) noexcept;
	bool					FatIsEOFEntry(fatEntry uiFat) noexcept;
	EFatCode				FreeChain(uint32 uiClusterIndex) noexcept;
	EFatCode				SetClusterEntry(uint32 uiClusterIndex, fatEntry uiClusterInVolume) noexcept;
	EFatCode				ReadFatEntry(uint32 uiOffsetInSector, uint32 uiClusterIndex, uint32 uiSector, fatEntry* puiFatEntry) noexcept;
	EFatCode				UpdateDirectoryEntry(SFatDirectoryEntry* psEntry, uint32 uiFirstCluster, uint32 uiNewClusterInVolume, uint32 uiClustersInFile) noexcept;
	EFatCode				FindFirstFATEntry(const char* szParentPath, uint8 uiAttributes, SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* psQuery) noexcept;
	EFatCode				FindNextFATEntry(SFatDirectoryEntry** ppsDirectoryEntry, SFatFileSystemQuery* psQuery) noexcept;
	EFatCode				CreateFATEntry(SFatRawDirectoryEntry* psParentDirectory, const char* szName, uint8 uiAttributes, uint32 uiEntryCluster, SFatDirectoryEntry* psNewEntry) noexcept;
	fatEntry				GetEndOfClusterMarker(void) const noexcept;
	fatEntry				GetBadClusterMarker(void) const noexcept;
	fatEntry				GetClusterEntryMask(void) const noexcept;
	uint32					GetEntryCluster(SFatRawDirectoryEntry* psEntry) noexcept;

	EFatCode				ValidateFatCache(SFatCache sCache) noexcept;

	void					FillDirectoryEntryFromRawEntry(SFatDirectoryEntry* psEntry, SFatRawDirectoryEntry* psRawEntry) noexcept;

	//Getters
	uint32					GetClusterSize(void) const { return muiBytesPerCluster; }
	uint32					GetID(void) const noexcept;
	uint32					GetFatSize(void) const noexcept;
	uint32					GetFirstDataSector(void) const noexcept;
	uint32					GetNoOfSectors(void) const noexcept;
	uint32					GetNoOfDataSectors(void) const noexcept;
	uint32					GetNumClusters(void) const noexcept;
	uint32					GetNoOfReservedSectors(void) const { return muiNoOfReservedSectors; }
	uint32					GetNextFreeCluster(void) const noexcept;
	uint32					GetTotalFreeClusters(void) const noexcept;
	uint32					GetFileSystemInfoSector(void) const noexcept;
	uint16					GetRootDirectorySectors(void) const noexcept;
	uint16					GetSectorSize(void) const { return muiBytesPerSector; }
	uint16					NumSectorsPerCluster(void) const { return muiNoOfSectorsPerCluster; }
	uint32					BytesToSectors(uint32 uiBytes) { return uiBytes >> muiBytesPerSectorShift; }
	uint32					OffsetInSector(uint32 uiBytes) { return uiBytes & ((uint32)muiBytesPerSector - 1); }
	uint32					BytesToClusters(uint32 uiBytes) { return uiBytes >> muiBytesPerClusterShift; }
	uint32					OffsetInCluster(uint32 uiBytes) { return uiBytes & (muiBytesPerCluster - 1); }
	uint32					SectorsToClusters(uint32 uiSectors) { return uiSectors >> muiSectorsPerClusterShift; }
	bool					IsUseLongFilenames(void) const noexcept;
	EFatFileSystemType		GetFileSystemType(void) const noexcept;
	uint8					GetNoOfFatTables(void) const noexcept;
	const char*				GetLabel(void) const noexcept;
	CFileDrive*				GetFileDrive(void) const noexcept;
	uint32					GetRootCluster(void) const noexcept;
	uint32					GetRootSector(void) const noexcept;
	uint32					GetPageSize(void) const noexcept;
	CFatInfoSectorCache*	GetSectorCache(void) noexcept;

protected:
	EFatCode				InitialiseFat32FileSystemInfo(void) noexcept;
	uint8					PowerOfTwoShift(uint32 uiSize) noexcept;
	bool					InitialiseVolumeParametersFromBPB(uint8* puiPartitionsTried, SFatBIOSParameterBlock* psBPB, uint32 uiHiddenSectors) noexcept;

	EFatCode				QueryFirstEntry(SFatRawDirectoryEntry* directory, uint8 attributes, SFatQueryState* query, bool bBufferLocked) noexcept;
	EFatCode				QueryNextEntry(SFatQueryState* psQuery, bool bBufferLocked, bool bFirstEntry) noexcept;

	uint32					AllocateDirectoryCluster(SFatRawDirectoryEntry* parent, EFatCode* peResult) noexcept;
	uint32					AllocateDataCluster(uint32 uiClusterCount, bool bEraseCluster, EFatCode* peResult, uint32 uiPreviousCluster) noexcept;
	uint32					AllocateDataClusterEx(uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult, uint32 uiPreviousCluster) noexcept;
	uint32					AllocateClusters(SFatRawDirectoryEntry* psParentDirectory, uint32 uiClusterCount, bool bEraseCluster, uint32 uiPageSize, EFatCode* peResult, uint32 uiPreviousCluster) noexcept;

	EFatCode				FreeFat16Chain(uint32* puiFreedClusterIndex, uint32 uiSector, uint32 uiOffsetInSector) noexcept;
	EFatCode				FreeFat32Chain(uint32* puiFreedClusterIndex, uint32 uiSector, uint32 uiOffsetInSector) noexcept;

	uint32					CalculateFATEntryIndex(uint32 uiClusterIndexInFAT) noexcept;
	uint8					FatLongEntryChecksum(char* szFilename) noexcept;
	uint32					CalculateRootSector(void) noexcept;

	EFatCode				SetFat16ClusterEntry(fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector) noexcept;
	EFatCode				SetFat32ClusterEntry(fatEntry uiClusterInVolume, uint32 uiFirstClusterSector, uint16 uiOffsetInSector) noexcept;

	EFatCode				IncreaseFat16ClusterAddress(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector) noexcept;
	EFatCode				IncreaseFat32ClusterAddress(uint32* puiClusterIndex, uint32 uiSector, uint32 uiOffsetInSector) noexcept;
	EFatCode				SeekFat32ByClusterCount(uint32 uiClusterIndex, uint16 uiClusterCount, uint32* puiNewClusterIndex) noexcept;

	EFatCode				ReadFat16Entry(uint32 uiOffsetInSector, uint32 uiSector, fatEntry* puiFatEntry) noexcept;
	EFatCode				ReadFat32Entry(uint32 uiOffsetInSector, uint32 uiSector, fatEntry* puiFatEntry) noexcept;

	EFatCode				WriteFat16Entry(uint32 uiOffsetInSector, uint32 uiPreviousOffset, uint32 uiCluster, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiPreviousFatEntry) noexcept;
	EFatCode				WriteFat32Entry(uint32 uiOffsetInSector, uint32 uiPreviousEntryOffset, uint32 uiClusterIndexInTable, uint32 uiSector, uint32 uiPreviousSector, fatEntry uiPreviousFatEntry) noexcept;
	EFatCode				WriteAllocatedFatEntry(uint32 uiClusterIndexInTable, uint32 uiOffsetInSector, uint32 uiSector, fatEntry uiPreviousFatEntry, uint32 uiPreviousSector, uint32 uiPreviousEntryOffset) noexcept;

	void					SetNextFreeCluster(uint32 uiCluster) noexcept;
	void					SetTotalFreeClusters(uint32 uiTotalFreeClusters) noexcept;
	uint32					FindNextPageCluster(uint32 uiPageSize, uint32 uiCluster, uint16 uiStep) noexcept;
	bool					HasNextFreeCluser(void) const noexcept;
	uint16					CalculateClusterStepSize(uint32 uiPageSize) noexcept;
	EFatCode				WrapAllocate(uint32 uiStartCluster, uint32 uiLastFatEntry, bool* pbWrappedAround, uint32* puiClusterIndex, uint32* puiOffsetInSector, uint32* puiFirstClusterSector) noexcept;
	EFatCode				InitialiseAllocatedFatCluster(SFatRawDirectoryEntry* psParentDirectory, uint32 uiCluster, bool bZero) noexcept;
	EFatCode				EraseClusterContents(uint32 uiCluster) noexcept;
	EFatCode				InitializeDirectoryCluster(SFatRawDirectoryEntry* psDirectoryParent, uint32 uiCluster) noexcept;
	void					InitialiseDottyDirectoryEntry(SFatRawDirectoryEntry* psEntries, const char* sz, uint32 uiCluster) noexcept;
	bool					IsFreeFat(fatEntry uifat) noexcept;

	EFatCode				CreateFakeRootEntry(SFatDirectoryEntry* psEntry) noexcept;
	fatEntry				GetDirectoryCluster(SFatRawDirectoryEntry* psParentDirectory) noexcept;
	uint32					GetDirectorySector(SFatRawDirectoryEntry* psParentDirectory) noexcept;
	EFatCode				FindEnoughEntries(fatEntry* puiLastDirectoryCluster, fatEntry* puiDirectoryCluster, uint32* puiFirstSectorOfCluster, uint32* puiSector, int* piLFNEntriesFound, SFatRawDirectoryEntry** ppsParentEntry, SFatRawDirectoryEntry* psParentDirectory, int iLFNEntriesNeeded) noexcept;
	EFatCode				RewindFoundEntries(SFatRawDirectoryEntry** ppsParentEntry, uint32* puiSector, uint32* puiFirstSectorOfCluster, fatEntry* puiDirectoryCluster, int iLFNEntriesFound, fatEntry uiFirstDirectoryCluster) noexcept;
	EFatCode				FindPreviousCluster(fatEntry uiFirstCluster, fatEntry uiCluster, fatEntry* puiPreviousCluster) noexcept;

	EFatCode				FindBiosParameterBlock(SFatCache sMBRSector) noexcept;
	bool					CheckSectorsPerClusterIsPowerOfTwo(uint8 uiSectorsPerCluster) noexcept;
	bool					CheckFileAllocationTableLargeEnough(EFatFileSystemType eFileSystem, uint32 uiFatSize, uint32 uiNoOfClusters, uint16 uiBytesPerSector) noexcept;

	EFatCode				GenerateUniqueShortNameWithSuffix(SFatRawDirectoryEntry* psParentDirectory, uint8* szShortName) noexcept;
	void					InitialiseNewEntry(SFatDirectoryEntry* psNewEntry, const char* szName, uint8 uiAttributes, uint32 uiEntryCluster) noexcept;
	void					InitialiseParentEntry(SFatRawDirectoryEntry* psParentEntry, const char* szName, char uiChecksum, int iLFNEntriesNeeded, int iLFNEntriesFound) noexcept;
	void					SetQueryLongFilenamePart(SFatQueryState* psQuery) noexcept;
	EFatCode				FindNextRawDirectoryEntry(SFatQueryState* psQuery) noexcept;
	EFatCode				EndQuery(SFatQueryState* psQuery) noexcept;

	uint16					LongNameCharToWideChar(uint16 uiLFNIndex, uint16 uiNameLength, const char* szName, uint16 uiNameIndex) noexcept;
	void					GenerateDirectoryEntryNameParts(uint8* aui, uint16 uiLFNIndex, uint16 uiNameLength, const char* szName, uint16 uiNameIndex) noexcept;
	EFatCode				InitialiseVolumeLabel(void) noexcept;

	EFatCode				FindNextRawDirectoryEntrySectorInCluster(uint32* puiSector, SFatQueryState* psQuery) noexcept;
	EFatCode				FindNextRawDirectoryEntryCluster(uint32* puiSector, SFatQueryState* psQuery) noexcept;
	bool					IsFreeDirectoryEntry(SFatRawDirectoryEntry* psEntry) noexcept;
	bool					IsLastDirectoryEntry(SFatRawDirectoryEntry* psEntry) noexcept;
	void					ProcessQueryLongFileNameEntry(SFatQueryState* psQuery) noexcept;
	void					ConstructQueryLongFileNameFromShortName(SFatQueryState* psQuery) noexcept;

	EFatCode				EraseClusterChainContents(fatEntry uiCluster) noexcept;
	uint32					GetRootDirectorySector(void) const noexcept;
	bool					HasNoMoreEntries(SFatFileSystemQuery* psQuery) noexcept;
	EFatCode				TerminalQueryEntry(SFatFileSystemQuery* psQuery) noexcept;
	uint32					CalculateQuerySector(SFatFileSystemQuery* psQuery) noexcept;
	uint32					CalculateQuerySectorOffset(SFatFileSystemQuery* psQuery) noexcept;

	EFatCode				CheckEntryName(const char* szName) noexcept;
	EFatCode				WriteFATEntry(SFatDirectoryEntry* psNewEntry, const char* szName, SFatRawDirectoryEntry* psParentDirectory, int iLFNEntriesNeeded) noexcept;
	bool					IsRootDirectoryCluster(uint32 uiDirectoryCluster) noexcept;
	EFatCode				IncrementDirectorySector(uint32* puiSector, uint32 uiFirstSectorOfCluster, fatEntry uiDirectoryCluster) noexcept;
	EFatCode				IncrementOrAllocateDirectoryCluster(uint32* puiSector, fatEntry* puiDirectoryCluster, fatEntry uiLastDirectoryCluster) noexcept;
	EFatCode				CreateDirectory(const char* szDirectory, SFatDirectoryEntry* psEntry) noexcept;

	EFatCode				DeleteLongNameEntries(const char* szParentPath, uint32 uiSectorAddress, uint16 uiSectorOffset, uint8 uiChecksum) noexcept;
	void					CopyRawEntry(SFatDirectoryEntry* psOriginalEntry, SFatDirectoryEntry* psNewEntry) noexcept;
};


#endif // __FAT_VOLUME_H__

