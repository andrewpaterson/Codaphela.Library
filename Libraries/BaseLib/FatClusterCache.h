#ifndef __FAT_CLUSTER_CACHE_H__
#define __FAT_CLUSTER_CACHE_H__
#include "PrimitiveTypes.h"
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


class CFatClusterCache
{
protected:
	SClusterCache	msCluster;

	uint16			muiClusterSize;
	uint16			muiSectorSize;
	uint16			muiSectorsPerCluster;
	uint16			muiBytesForBits;
	uint16			muiSectorSizeShift;		//muiSectorSize is 1 << muiSectorSizeShift.

	CFileDrive*		mpcDrive;

public:
	void	Init(CFileDrive* pcDrive, uint16 uiClusterSize, uint16 uiSectorSize) noexcept;
	void	Kill(void) noexcept;

	bool	Read(uint8* pvDestination, uint32 uiCluster, uint32 uiClusterFirstSector, uint16 uiOffset, uint32* puiLength, uint16 uiMaximumOffset) noexcept;
	bool	Write(uint8* pvSource, uint32 uiCluster, uint32 uiClusterFirstSector, uint16 uiOffset, uint32* puiLength, uint16 uiPreviousMaximumOffset) noexcept;
	bool	Flush(void) noexcept;

	void	Clear(void) noexcept;
	uint8*	GetCache(void) const noexcept;

	bool	IsSectorDirty(uint16 iSectorIndex) noexcept;
	bool	IsSectorCached(uint16 iSectorIndex) noexcept;

	uint16	GetSectorsPerCluster(void) const noexcept;

protected:
	void	Invalidate(SClusterCache* psCluster) noexcept;
	bool	Flush(SClusterCache* psCluster) noexcept;
	bool	FlushAndInvalidate(SClusterCache* psCluster) noexcept;
	bool	IsCached(uint32 uiSectorIndex) noexcept;
	uint16	LastSectorBefore(uint16 uiEnd) noexcept;
	bool	CacheSector(SClusterCache* psCluster, uint32 uiSectorIndex) noexcept;

	bool	SetCachedSector(size uiSectorIndex, bool bCached) noexcept;
	bool	SetDirtySector(size uiSectorIndex, bool bDirty) noexcept;
};


#endif // __FAT_CLUSTER_CACHE_H__

