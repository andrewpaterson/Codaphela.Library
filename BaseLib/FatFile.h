#ifndef __FAT_FILE_H__
#define __FAT_FILE_H__
#include "FatClusterCache.h"
#include "FatCommon.h"


// This is the file handle structure. All the fields in this structure
// are reserved for internal use and should not be accessed directly by the
// developer.
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
	void		Init(CFatVolume* pcVolume);
	EFatCode	Open(char* szFilename, uint8 uiAccessFlags);
	EFatCode	Open(SFatDirectoryEntry* psEntry, uint8 uiAccessFlags);
	EFatCode	Close(void);
	EFatCode	Write(uint8* pvSource, uint32 uiLength);
	EFatCode	Read(uint8* puiDestination, uint32 length, uint32* puiBytesRead);
	EFatCode	Seek(uint32 uiOffset, EFatSeek eMode);
	EFatCode	Flush(void);
	EFatCode	Truncate(void);

	uint32		Tell(void);
	uint32		Size(void);
	bool		IsBusy(void);
	uint8		GetAccessFlags(void);

	EFatCode	AllocateClusters(uint32 bytes);
	uint32		CalculateFirstCluster(void);

protected:
	EFatCode	OpenFileByEntry(SFatDirectoryEntry* psEntry, uint8 uiAccessFlags);
	uint32		FatFileGetUniqueId(void);
	EFatCode	WriteIntoExistingClusters(uint32 uiBytesRemaining, uint8* puiSource);

	bool		IsFat32Volume(void);
	EFatCode	ReadFromClusters(uint32 uiBytesRemaining, uint32* puiBytesRead, uint8* puiDestination);

	EFatCode	FlushCacheUpdateAccess(void);

	char*		GetShortFileName(void);
	EFatCode	ValidateCanRead(void);
	EFatCode	ValidateCanWrite(void);

	EFatCode	PotentiallyAllocateClusters(uint32 uiLength);
	EFatCode	CounterClustersInFile(uint32* puiClustersInFile, fatEntry* puiPreviousCluster, fatEntry uiFirstCluster);

	EFatCode	CalculateSeekPosition(uint32 uiOffset, EFatSeek eMode, uint32* puiBytePositionInFile);
};


#endif // !__FAT_FILE_H__

