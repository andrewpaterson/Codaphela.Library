#ifndef __CLUSTER_CACHE_H__
#define __CLUSTER_CACHE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "FileDrive.h"


#define	NO_CLUSTER_CACHED	0xffffffff


struct SClusterCache
{
	uint32	muiCluster;
	uint8*	mpvCache;
	bool*	mpbCachedSectors;
	bool*	mpbDirtySectors;

};


class CClusterCache
{
protected:
	uint32	muiCluster;
	uint8*	mpvCache;
	bool	mbDirty;
	uint16	muiClusterSize;
	uint16	muiSectorSize;
	bool*	mpbCachedSectors;

public:
	void	Init(CFileDrive* pcDrive, uint16 uiClusterSize);
	void	Kill(void);

	bool	Read(uint8* pvDestination, uint32 uiCluster, uint16 uiOffset, uint32* puiLength);
};


#endif // !__CLUSTER_CACHE_H__

