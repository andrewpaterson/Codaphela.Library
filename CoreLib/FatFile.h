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
	bool					bBusy;
	uint8					uiMagic;
	uint8					uiAccessFlags;
	uint8*					pvBuffer;
};


class CFatFile
{
protected:
	SFatFile		msFile;
	CFatVolume*		mpcVolume;

public:
	void					Init(CFatVolume* pcVolume);
	EFatCode				Open(char* filename, uint8 uiAccessFlags);
	EFatCode				Open(SFatDirectoryEntry* entry, uint8 uiAccessFlags);
	EFatCode				Close(void);
	EFatCode				Write(uint8* buff, uint32 length);
	EFatCode				Read(uint8* buff, uint32 length, uint32* bytes_read);
	EFatCode				Seek(uint32 offset, char mode);

	uint32					GetCurrentSize(void);
	uint32					GetCurrentClusterAddress(void);
	uint32					GetCurrentClusterIdx(void);
	uint32					GetCurrentSectorIdx(void);
	uint32					GetNoOfClustersAfterPos(void);
	uint16					GetNoOfSequentialClusters(void);
	uint8*					GetBufferHead(void);
	bool					IsBusy(void);
	uint8					GetMagic(void);
	uint8					GetAccessFlags(void);
	uint8*					GetBuffer(void);

protected:

	EFatCode				FatOpenFileByEntry(SFatDirectoryEntry* entry, uint8 uiAccessFlags);
	uint32					FatFileGetUniqueId(void);
	EFatCode				FatFileAllocate(uint32 bytes);
	EFatCode				FatFileWriteCallback(SFatOperationState* psOperation);
	//EFatCode				FatFileReadCallback(SFatOperationState* psOperation);
	EFatCode				FatFileFlush(void);

	void					AllocateBuffer(void);
	EFatCode				FatFileUpdateSequentialClusterCount(void);
	EFatCode				FatFileRead(SFatOperationState* psOperation);
};


#endif // !__FAT_FILE_H__


