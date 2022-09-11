#ifndef __FAT_FILE_H__
#define __FAT_FILE_H__
#include "FatCommon.h"


// This is the file handle structure. All the fields in this structure
// are reserved for internal use and should not be accessed directly by the
// developer.
class CFatVolume;
struct SFatFile
{
	SFatDirectoryEntry		sDirectoryEntry;
	uint32					uiCurrentSize;
	uint32					uiCurrentClusterAddress;
	uint32					uiCurrentClusterIdx;
	uint32					uiCurrentSectorIdx;
	uint32					uiNoOfClustersAfterPos;
	uint16					uiNoOfSequentialClusters;
	uint8*					pvBufferHead;
	bool					bBufferDirty;
	bool					bBusy;
	uint8					uiMagic;
	uint8					uiAccessFlags;
	SFatOperationState		sOperationState;
	uint8*					uiBuffer;
	uint8					auiBufferInternal[MAX_SECTOR_LENGTH];
};


class CFatFile
{
protected:
	SFatFile		msFile;
	CFatVolume*		mpcVolume;

public:
	void					Init(CFatVolume* pcVolume);

public:
	uint16					FatFileUpdateSequentialClusterCount(void);
	EFatCode				FatFileOpen(CFatVolume* volume, char* filename, uint8 uiAccessFlags);

	EFatCode				FatOpenFileByEntry(CFatVolume* volume, SFatDirectoryEntry* entry, uint8 uiAccessFlags);
	uint16					FatFileSetBuffer(uint8* uiBuffer);
	uint32					FatFileGetUniqueId(void);
	uint16					FatFileAllocate(uint32 bytes);
	EFatCode				FatFileSeek(uint32 offset, char mode);
	uint16					FatFileWriteCallback(void);
	uint16					FatFileWrite(uint8* buff, uint32 length);
	uint16					FatFileReadCallback(void);
	uint16					FatFileRead(uint8* buff, uint32 length, uint32* bytes_read);
	EFatCode				FatFileFlush(void);
	EFatCode				FatFileClose(void);

	void					SetVolume(CFatVolume* pcVolume);

	uint32					GetCurrentSize(void);
	uint32					GetCurrentClusterAddress(void);
	uint32					GetCurrentClusterIdx(void);
	uint32					GetCurrentSectorIdx(void);
	uint32					GetNoOfClustersAfterPos(void);
	uint16					GetNoOfSequentialClusters(void);
	uint8*					GetBufferHead(void);
	bool					IsBufferDirty(void);
	bool					IsBusy(void);
	uint8					GetMagic(void);
	uint8					GetAccessFlags(void);
	uint8*					GetBuffer(void);
};


#endif // !__FAT_FILE_H__


