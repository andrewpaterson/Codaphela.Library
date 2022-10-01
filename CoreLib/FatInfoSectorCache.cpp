#include "FatInfoSectorCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatSectorCache::Init(uint32 uiInfoSector, uint64 uiCreationStamp)
{
	muiInfoSector = uiInfoSector;
	muiLocks = 0;
	mbDirty = false;
	muiCreationStamp = uiCreationStamp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CFatSectorCache::GetCache(void)
{
	return RemapSinglePointer(this, sizeof(CFatSectorCache));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatSectorCache::Lock(void)
{
	muiLocks++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatSectorCache::Unlock(void)
{
	if (muiLocks != 0)
	{
		muiLocks--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatSectorCache::Dirty(void)
{
	mbDirty = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatInfoSectorCache::Init(CFileDrive* pcDrive, uint16 uiMinimumUnlockedCaches)
{
	mpcDrive = pcDrive;
	muiSectorSize = pcDrive->GetSectorSize();

	mllcCachedSectors.Init();
	muiMinimumUnlockedCaches = uiMinimumUnlockedCaches;
	muiCaches = 0;
	muiCreationStamp = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatInfoSectorCache::Kill(void)
{
	mllcCachedSectors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CFatInfoSectorCache::ReadSector(uint32 uiInfoSector)
{
	CFatSectorCache*	pcCachedSector;
	void*				pvData;

	pcCachedSector = GetOrCacheSector(uiInfoSector);

	if (pcCachedSector != NULL)
	{
		pvData = pcCachedSector->GetCache();
		return pvData;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFatSectorCache* CFatInfoSectorCache::AddNewCache(void)
{
	CFatSectorCache*	pcSectorCache;
	size_t				uiSize;

	uiSize = sizeof(CFatSectorCache) + muiSectorSize;
	pcSectorCache = (CFatSectorCache*)mllcCachedSectors.Add(uiSize);

	memset(pcSectorCache, 0, uiSize);
	return pcSectorCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFatSectorCache* CFatInfoSectorCache::GetOrCacheSector(uint32 uiInfoSector)
{
	CFatSectorCache*	pcCachedSector;
	CFatSectorCache*	pcEmptySector;
	CFatSectorCache*	pcOldestUnlockedSector;
	uint64				uiOldestStamp;
	uint16				uiUnlockedCaches;
	bool				bSuccess;

	pcEmptySector = NULL;
	pcOldestUnlockedSector = NULL;
	uiOldestStamp = 0xFFFFFFFFFFFFFFFF;
	uiUnlockedCaches = 0;
	pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetHead();
	while (pcCachedSector != NULL)
	{
		if (pcCachedSector->muiInfoSector == uiInfoSector)
		{
			return pcCachedSector;
		}

		if ((pcCachedSector->muiInfoSector == NO_SECTOR_CACHED) && (pcEmptySector == NULL))
		{
			pcEmptySector = pcCachedSector;
		}

		if (pcCachedSector->muiLocks == 0)
		{
			uiUnlockedCaches++;

			if (pcCachedSector->muiCreationStamp < uiOldestStamp)
			{
				pcOldestUnlockedSector = pcCachedSector;
				uiOldestStamp = pcCachedSector->muiCreationStamp;
			}
		}

		pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetNext(pcCachedSector);
	}

	if (pcEmptySector)
	{
		return InitialiseCache(uiInfoSector, pcEmptySector);
	}

	if (uiUnlockedCaches < muiMinimumUnlockedCaches)
	{
		pcCachedSector = AddNewCache();
		return InitialiseCache(uiInfoSector, pcCachedSector);
	}

	bSuccess = FlushCache(pcCachedSector);
	if (!bSuccess)
	{
		return NULL;
	}

	return InitialiseCache(uiInfoSector, pcCachedSector);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFatSectorCache* CFatInfoSectorCache::InitialiseCache(uint32 uiInfoSector, CFatSectorCache* pcCachedSector)
{
	void*	pvData;
	bool	bSuccess;

	muiCreationStamp++;
	pcCachedSector->Init(uiInfoSector, muiCreationStamp);
	pvData = pcCachedSector->GetCache();
	bSuccess = mpcDrive->Read(uiInfoSector, pvData);
	if (!bSuccess)
	{
		return NULL;
	}
	else
	{
		return pcCachedSector;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatInfoSectorCache::FlushCache(CFatSectorCache* pcCachedSector)
{
	void*	pvData;
	bool	bSuccess;

	if (pcCachedSector->mbDirty)
	{
		pvData = pcCachedSector->GetCache();
		bSuccess = mpcDrive->Write(pcCachedSector->muiInfoSector, pvData);
		if (!bSuccess)
		{
			return false;
		}
		else
		{
			pcCachedSector->mbDirty = false;
			return true;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatInfoSectorCache::Lock(void* pvSectorCache)
{
	CFatSectorCache*	pcCachedSector;

	pcCachedSector = (CFatSectorCache*)RemapSinglePointer(pvSectorCache, -((ptrdiff_t) sizeof(CFatSectorCache)));
	pcCachedSector->Lock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatInfoSectorCache::Unlock(void* pvSectorCache)
{
	CFatSectorCache* pcCachedSector;

	pcCachedSector = (CFatSectorCache*)RemapSinglePointer(pvSectorCache, -((ptrdiff_t) sizeof(CFatSectorCache)));
	pcCachedSector->Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatInfoSectorCache::Dirty(void* pvSectorCache)
{
	CFatSectorCache* pcCachedSector;

	pcCachedSector = (CFatSectorCache*)RemapSinglePointer(pvSectorCache, -((ptrdiff_t)sizeof(CFatSectorCache)));
	pcCachedSector->Dirty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatInfoSectorCache::Flush(void)
{
	CFatSectorCache*	pcCachedSector;
	uint16				uiCount;
	bool				bSuccess;

	bSuccess = true;
	uiCount = 0;
	pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetHead();
	while (pcCachedSector != NULL)
	{
		if (pcCachedSector->muiInfoSector != NO_SECTOR_CACHED)
		{
			if (!FlushCache(pcCachedSector))
			{
				bSuccess = false;
			}
		}
		pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetNext(pcCachedSector);
	}

	return bSuccess;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatInfoSectorCache::GetNumAllocatedSectors(void)
{
	return mllcCachedSectors.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatInfoSectorCache::GetNumCachedSectors(void)
{
	CFatSectorCache*	pcCachedSector;
	uint16				uiCount;

	uiCount = 0;
	pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetHead();
	while (pcCachedSector != NULL)
	{
		if (pcCachedSector->muiInfoSector != NO_SECTOR_CACHED)
		{
			uiCount++;
		}
		pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetNext(pcCachedSector);
	}

	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatInfoSectorCache::GetLockedCachedSectors(void)
{
	CFatSectorCache*	pcCachedSector;
	uint16				uiCount;

	uiCount = 0;
	pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetHead();
	while (pcCachedSector != NULL)
	{
		if (pcCachedSector->muiLocks > 0)
		{
			uiCount++;
		}
		pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetNext(pcCachedSector);
	}

	return uiCount;
}

