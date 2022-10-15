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
	SFatDirectoryEntry		sDirectoryEntry;
	uint32					uiFileSize;
	uint32					uiFilePosition;
	uint32					uiCursorClusterInVolume;

	bool					bBusy;
	uint8					uiMagic;
	uint8					uiAccessFlags;
};


class CFatFile
{
protected:
	SFatFile				msFile;
	CFatVolume*				mpcVolume;
	CFatClusterCache		mcCache;
	char					mszName[FAT_MAX_FILENAME];

public:
	void		Init(CFatVolume* pcVolume);
	EFatCode	Open(char* filename, uint8 uiAccessFlags);
	EFatCode	Open(SFatDirectoryEntry* entry, uint8 uiAccessFlags);
	EFatCode	Close(void);
	EFatCode	Write(uint8* pvSource, uint32 length);
	EFatCode	Read(uint8* puiDestination, uint32 length, uint32* puiBytesRead);
	EFatCode	Seek(uint32 offset, EFatSeek mode);
	EFatCode	Flush(void);

	uint32		Size(void);
	bool		IsBusy(void);
	uint8		GetAccessFlags(void);

	EFatCode	AllocateClusters(uint32 bytes);

protected:
	EFatCode	OpenFileByEntry(SFatDirectoryEntry* entry, uint8 uiAccessFlags);
	uint32		FatFileGetUniqueId(void);
	EFatCode	WriteIntoExistingClusters(uint32 uiBytesRemaining, uint8* puiSource);

	bool		IsFat32Volume(void);
	EFatCode	ReadFromClusters(uint32 uiBytesRemaining, uint32* puiBytesRead, uint8* puiDestination);
	uint32		CalculateFirstCluster(void);

	EFatCode	FlushCacheUpdateAccess(void);

	char*		GetShortFileName(void);
	EFatCode	ValidateCanRead(void);
	EFatCode	ValidateCanWrite(void);
	EFatCode	PotentiallyAllocateClusters(uint32 uiLength);
};


#endif // !__FAT_FILE_H__

