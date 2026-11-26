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
	bool	IsValid(void) const { return pvCachedSector != NULL; }
};


class CFatSectorCache
{
public:
	uint64	muiCreationStamp;
	uint32	muiInfoSector;
	uint16	muiLocks;
	bool	mbDirty;

	void	Init(uint32 uiInfoSector, uint64 uiCreationStamp) noexcept;
	void*	GetCache(void) const noexcept;
	void	Lock(void) noexcept;
	bool	Unlock(void) noexcept;
	void	Dirty(void) noexcept;
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
	void				Init(CFileDrive* pcDrive, uint16 uiMinimumUnlockedCaches) noexcept;
	bool				Kill(void) noexcept;
	SFatCache			ReadSector(uint32 uiInfoSector) noexcept;
	void				Lock(SFatCache sSectorCache) noexcept;
	bool				Unlock(SFatCache sSectorCache) noexcept;
	bool				Unlock(uint32 uiSector) noexcept;
	uint16				GetNumAllocatedSectors(void) noexcept;
	uint16				GetNumCachedSectors(void) noexcept;
	uint16				GetLockedCachedSectors(void) noexcept;
	void				Dirty(SFatCache sSectorCache) noexcept;
	bool				Flush(void) noexcept;
	bool				ValidateTimeStamp(SFatCache sSectorCache) noexcept;
	void				Dump(void) noexcept;

protected:
	CFatSectorCache*	AddNewCache(void) noexcept;
	CFatSectorCache*	GetOrCacheSector(uint32 uiInfoSector) noexcept;
	CFatSectorCache*	InitialiseCache(uint32 uiInfoSector, CFatSectorCache* pcCachedSector) noexcept;
	bool				FlushCache(CFatSectorCache* pcCachedSector) noexcept;
	SFatCache			FindCachedSector(uint32 uiInfoSector) noexcept;
};


#endif // __FAT_INFO_SECTOR_CACHE__

