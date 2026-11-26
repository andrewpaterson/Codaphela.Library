#ifndef __FAT_FILE_H__
#define __FAT_FILE_H__
#include "FatClusterCache.h"
#include "FatCommon.h"


class CFatVolume;
struct SFatFile
{
	SFatDirectoryEntry	sDirectoryEntry;
	uint32				uiFileSize;
	uint32				uiFilePosition;
	fatEntry			uiFilePositionCluster;

	bool				bBusy;
	uint8				uiMagic;
	uint8				uiAccessFlags;
};


class CFatFile
{
protected:
	SFatFile			msFile;
	CFatVolume*			mpcVolume;
	CFatClusterCache	mcCache;
	char				mszName[FAT_MAX_FILENAME];

public:
	void		Init(CFatVolume* pcVolume) noexcept;
	EFatCode	Open(const char* szFilename, uint8 uiAccessFlags) noexcept;
	EFatCode	Open(SFatDirectoryEntry* psEntry, uint8 uiAccessFlags) noexcept;
	EFatCode	Close(void) noexcept;
	EFatCode	Write(uint8* pvSource, lsize uiLength) noexcept;
	EFatCode	Read(uint8* puiDestination, lsize length, lsize* puiBytesRead) noexcept;
	EFatCode	Seek(uint32 uiOffset, EFatSeek eMode) noexcept;
	EFatCode	Flush(void) noexcept;
	EFatCode	Truncate(void) noexcept;

	uint32		Tell(void) noexcept;
	uint32		Size(void) noexcept;
	bool		IsBusy(void) const noexcept;
	uint8		GetAccessFlags(void) const noexcept;

	EFatCode	AllocateClusters(uint32 bytes) noexcept;
	uint32		CalculateFirstCluster(void) noexcept;

protected:
	EFatCode	OpenFileByEntry(SFatDirectoryEntry* psEntry, uint8 uiAccessFlags) noexcept;
	uint32		FatFileGetUniqueId(void) noexcept;
	EFatCode	WriteIntoExistingClusters(uint32 uiBytesRemaining, uint8* puiSource) noexcept;

	bool		IsFat32Volume(void) const noexcept;
	EFatCode	ReadFromClusters(uint32 uiBytesRemaining, lsize* puiBytesRead, uint8* puiDestination) noexcept;
	EFatCode	StepToClusterOfPosition(void) noexcept;

	EFatCode	FlushCacheUpdateAccess(void) noexcept;

	const char*		GetShortFileName(void) const noexcept;
	EFatCode		ValidateCanRead(void) noexcept;
	EFatCode		ValidateCanWrite(void) noexcept;

	EFatCode	PotentiallyAllocateClusters(uint32 uiLength) noexcept;
	EFatCode	CounterClustersInFile(uint32* puiClustersInFile, fatEntry* puiPreviousCluster, fatEntry uiFirstCluster) noexcept;

	EFatCode	CalculateSeekPosition(uint32 uiOffset, EFatSeek eMode, uint32* puiBytePositionInFile) noexcept;
};


#endif // __FAT_FILE_H__

