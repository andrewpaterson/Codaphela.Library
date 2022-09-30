#ifndef __FAT_INFO_SECTOR_CACHE__
#define __FAT_INFO_SECTOR_CACHE__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/LinkedListBlock.h"
#include "FileDrive.h"


#define	NO_SECTOR_CACHED	0xffffffff


class CFatSectorCache
{
public:
	uint64	muiCreationStamp;
	uint32	muiInfoSector;
	uint16	muiLocks;
	bool	mbDirty;

	void	Init(uint32 uiInfoSector, uint64 uiCreationStamp);
	void*	GetCache(void);
};


class CFatInfoSectorCache
{
protected:
	CLinkedListBlock	mllcCachedSectors;
	uint16				muiSectorSize;
	uint16				muiMinimumUnlockedCaches;
	uint16				muiCaches;

	CFileDrive*			mpcDrive;
	uint64				muiCreationStamp;

public:
	void				Init(CFileDrive* pcDrive, uint16 uiMinimumUnlockedCaches);
	void				Kill(void);

protected:
	CFatSectorCache*	AddNewCache(void);
	CFatSectorCache*	GetOrCacheSector(uint32 uiInfoSector);
	CFatSectorCache*	InitialiseCache(uint32 uiInfoSector, CFatSectorCache* pcCachedSector);
	bool				FlushCache(CFatSectorCache* pcCachedSector);
};


#endif // __FAT_INFO_SECTOR_CACHE__

