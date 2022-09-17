#ifndef __FAT_CACHE_H__
#define __FAT_CACHE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "FileDrive.h"


#define	NO_CLUSTER_CACHED	0xffffffff


struct SClusterCache
{
	uint32	uiCluster;
	uint32	uiClusterFirstSector;
	uint8*	pvCache;
	uint8*	pbCachedSectors;	//1 bit per sector.
	uint8*	pbDirtySectors;		//1 bit per sector.
};


class CFatCache
{
protected:
	SClusterCache	msCluster;
	uint16			muiClusterSize;
	uint16			muiSectorSize;
	uint16			muiSectorsPerCluster;
	uint16			muiBytesForBits;
	CFileDrive*		mpcDrive;

public:
	void	Init(CFileDrive* pcDrive, uint16 uiClusterSize, uint16 uiSectorSize);
	void	Kill(void);

	bool	Read(uint8* pvDestination, uint32 uiCluster, uint32 uiClusterFirstSector, uint16 uiOffset, uint32* puiLength, uint16 uiMaximumOffset);
	bool	Write(uint8* pvSource, uint32 uiCluster, uint32 uiClusterFirstSector, uint16 uiOffset, uint32* puiLength, uint16 uiPreviousMaximumOffset);

	void	Clear(void);
	uint8*	GetCache(void);

	bool	IsSectorDirty(int iSectorIndex);
	bool	IsSectorCached(int iSectorIndex);

protected:
	bool	FlushAndInvalidate(SClusterCache* psCluster);
	bool	IsCached(uint32 uiSectorIndex);
	bool	CacheSector(SClusterCache* psCluster, uint32 uiSectorIndex);
};


#endif // __FAT_CACHE_H__

