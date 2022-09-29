#ifndef __FAT_INFO_SECTOR_CACHE__
#define __FAT_INFO_SECTOR_CACHE__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/LinkedListBlock.h"


struct SFatSectorCache
{
	uint32	uiFatSharedBufferSector;
	uint16	uiLocks;
	bool	bDirty;
};


class CFatInfoSectorCache
{
protected:
	CLinkedListBlock	mllcCachedSectors;
	uint16				muiBytesPerSector;

public:
	void				Init(uint16 uiBytesPerSector);
	void				Kill(void);

	SFatSectorCache* AddCache(uint32 uiFatSharedBufferSector);
};


#endif // __FAT_INFO_SECTOR_CACHE__
