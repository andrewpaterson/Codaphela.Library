#ifndef __FAT_FILE_H__
#define __FAT_FILE_H__
#include "FatCache.h"
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
	SFatFile		msFile;
	CFatVolume*		mpcVolume;
	CFatCache		mcCache;
	char			mszName[FAT_MAX_FILENAME];

public:
	void		Init(CFatVolume* pcVolume);
	EFatCode	Open(char* filename, uint8 uiAccessFlags);
	EFatCode	Open(SFatDirectoryEntry* entry, uint8 uiAccessFlags);
	EFatCode	Close(void);
	EFatCode	Write(uint8* pvSource, uint32 length);
	EFatCode	Read(uint8* puiDestination, uint32 length, uint32* puiBytesRead);
	EFatCode	Seek(uint32 offset, EFatSeek mode);

	uint32		GetCurrentSize(void);
	uint32		GetCurrentClusterAddress(void);
	bool		IsBusy(void);
	uint8		GetMagic(void);
	uint8		GetAccessFlags(void);

protected:
	EFatCode	FatOpenFileByEntry(SFatDirectoryEntry* entry, uint8 uiAccessFlags);
	uint32		FatFileGetUniqueId(void);
	EFatCode	FatFileAllocate(uint32 bytes);
	EFatCode	FatFileWrite(uint32 uiBytesRemaining, uint8* puiSource);
	EFatCode	FatFileFlush(void);

	bool		IsFat32Volume(void);
	EFatCode	FatFileUpdateSequentialClusterCount(uint32 uiCursorClusterInVolume);
	EFatCode	FatFileRead(uint32 uiBytesRemaining, uint32* puiBytesRead, uint8* puiDestination);
	uint32		CalculateFirstCluster(void);

	char*		GetShortFileName(void);
	EFatCode	FatIncreaseClusterAddress(char* szMethod, uint32 uiClusterIndex, uint16 uiCount, uint32* puiNewClusterIndex);
	EFatCode	GetNextClusterEntry(char* szMethod, uint32 uiCurrentCluster, uint32* puiNextCluster);
	EFatCode	FatGetFileEntry(char* szMethod, char* path, SFatDirectoryEntry* sEntry);
	EFatCode	FatSetClusterEntry(char* szMethod, uint32 uiClusterIndex, fatEntry uiClusterInVolume);
	EFatCode	ReadFatSector(char* szMethod, uint32 uiFatInfoSector);
	EFatCode	FlushFatSector(char* szMethod);
	EFatCode	FatCreateDirectoryEntry(char* szMethod, SFatRawDirectoryEntry* psParentEntry, char* szName, SFatDirectoryEntry* psDirectoryEntry);
};


#endif // !__FAT_FILE_H__

