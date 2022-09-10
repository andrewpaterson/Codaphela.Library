#ifndef __FAT_FILE_H__
#define __FAT_FILE_H__
#include "BaseLib/PrimitiveTypes.h"
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
	uint16					FatFileUpdateSequentialClusterCount(CFatFile* file);
	uint16					FatFileOpen(CFatVolume* volume, char* filename, uint8 uiAccessFlags, CFatFile* file);

	uint16					FatOpenFileByEntry(CFatVolume* volume, SFatDirectoryEntry* entry, CFatFile* file, uint8 uiAccessFlags);
	uint16					FatFileSetBuffer(CFatFile* file, uint8* uiBuffer);
	uint32					FatFileGetUniqueId(CFatFile* file);
	uint16					FatFileAllocate(CFatFile* file, uint32 bytes);
	uint16					FatFileSeek(CFatFile* file, uint32 offset, char mode);
	void					FatFileWriteCallback(CFatFile* file, uint16* async_state_in);
	uint16					FatFileWrite(CFatFile* file, uint8* buff, uint32 length);
	void					FatFileReadCallback(CFatFile* file, uint16* state);
	uint16					FatFileRead(CFatFile* file, uint8* buff, uint32 length, uint32* bytes_read);
	uint16					FatFileFlush(CFatFile* file);
	uint16					FatFileClose(CFatFile* file);

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


