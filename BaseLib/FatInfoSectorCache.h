#ifndef __FAT_INFO_SECTOR_CACHE__
#define __FAT_INFO_SECTOR_CACHE__
#include "PrimitiveTypes.h"
#include "LinkedListBlock.h"
#include "FileDrive.h"


#define	NO_SECTOR_CACHED	0xffffffff


struct SFatCache
{
	void*	pvCachedSector;
	uint32	uiTimeStamp;

	//Inline: the compilers inline them where they are used (the W65C816's FAT assembly has no copies).
	void	Clear(void) { pvCachedSector = NULL; uiTimeStamp = 0xffffffff; }
	uint8*	Get(void) { return (uint8*)pvCachedSector; }
	bool	IsValid(void) { return pvCachedSector != NULL; }
}; 


class CFatSectorCache
{
public:
	uint64	muiCreationStamp;
	uint32	muiInfoSector;
	uint16	muiLocks;
	bool	mbDirty;

	void	Init(uint32 uiInfoSector, uint64 uiCreationStamp);
	void*	GetCache(void);
	void	Lock(void);
	bool	Unlock(void);
	void	Dirty(void);
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
	bool				Kill(void);
	SFatCache			ReadSector(uint32 uiInfoSector);
	void				Lock(SFatCache sSectorCache);
	bool				Unlock(SFatCache sSectorCache);
	bool				Unlock(uint32 uiSector);
	uint16				GetNumAllocatedSectors(void);
	uint16				GetNumCachedSectors(void);
	uint16				GetLockedCachedSectors(void);
	void				Dirty(SFatCache sSectorCache);
	bool				Flush(void);
	bool				ValidateTimeStamp(SFatCache sSectorCache);
	void				Dump(void);

protected:
	CFatSectorCache*	AddNewCache(void);
	CFatSectorCache*	GetOrCacheSector(uint32 uiInfoSector);
	CFatSectorCache*	InitialiseCache(uint32 uiInfoSector, CFatSectorCache* pcCachedSector);
	bool				FlushCache(CFatSectorCache* pcCachedSector);
	SFatCache			FindCachedSector(uint32 uiInfoSector);
};


#endif // __FAT_INFO_SECTOR_CACHE__

