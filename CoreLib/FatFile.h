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
	uint32					uiFileSize;
	uint32					uiCachedClusterInVolume;
	uint32					uiCachedClusterIndex;				// Cluster 0 is the first cluster index in the file, cluster 1 the second etc... regardless of how they are scattered on the disk.
	uint32					uiFirstCachedSectorIndexInCluster;  // Sector 0 is the first sector in the cluster etc...
	uint32					uiLastCachedSectorIndexInCluster;	// Inclusive.

	uint32					uiNoOfClustersAfterPos;
	uint16					uiNoOfSequentialClusters;
	uint32					uiBufferSeekByteIndexInSector;
	bool					bBusy;
	uint8					uiMagic;
	uint8					uiAccessFlags;
};


// holds the state of the current write operation
struct SFatWriteOperationState
{
	uint32		uiBytesRemaining;
	uint32		uiSectorAddress;
	uint8* pvUserMemory;
};


// holds the state of the current read operation
struct SFatReadOperationState
{
	uint32		uiBytesRemaining;
	uint32		uiSectorAddress;
	uint32* puiBytesRead;
	uint8* pvUserMemory;
};


class CFatFile
{
protected:
	SFatFile		msFile;
	CFatVolume* mpcVolume;
	uint8* mpvBuffer;

public:
	void					Init(CFatVolume* pcVolume);
	EFatCode				Open(char* filename, uint8 uiAccessFlags);
	EFatCode				Open(SFatDirectoryEntry* entry, uint8 uiAccessFlags);
	EFatCode				Close(void);
	EFatCode				Write(uint8* pvSource, uint32 length);
	EFatCode				Read(uint8* pvDestination, uint32 length, uint32* puiBytesRead);
	EFatCode				Seek(uint32 offset, char mode);

	uint32					GetCurrentSize(void);
	uint32					GetCurrentClusterAddress(void);
	uint32					GetCurrentClusterIdx(void);
	uint32					GetCurrentSectorIdx(void);
	uint32					GetNoOfClustersAfterPos(void);
	uint16					GetNoOfSequentialClusters(void);
	bool					IsBusy(void);
	uint8					GetMagic(void);
	uint8					GetAccessFlags(void);
	uint8* GetBuffer(void);

protected:
	EFatCode				FatOpenFileByEntry(SFatDirectoryEntry* entry, uint8 uiAccessFlags);
	uint32					FatFileGetUniqueId(void);
	EFatCode				FatFileAllocate(uint32 bytes);
	EFatCode				FatFileWriteCallback(SFatWriteOperationState* psOperation, uint32 uiBytePosition);
	EFatCode				FatFileFlush(void);

	void					AllocateBuffer(void);
	EFatCode				FatFileUpdateSequentialClusterCount(void);
	EFatCode				FatFileRead(SFatReadOperationState* psOperation);
};


#endif // !__FAT_FILE_H__

