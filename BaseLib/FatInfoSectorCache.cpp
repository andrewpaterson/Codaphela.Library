#include "PointerFunctions.h"
#include "Chars.h"
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
bool CFatSectorCache::Unlock(void)
{
	if (muiLocks != 0)
	{
		muiLocks--;
		return true;
	}
	return false;
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
bool CFatInfoSectorCache::Kill(void)
{
	CFatSectorCache*	pcCachedSector;
	uint32				uiLockCount;

	uiLockCount = 0;
	pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetHead();
	while (pcCachedSector != NULL)
	{
		if (pcCachedSector->muiInfoSector != NO_SECTOR_CACHED)
		{
			uiLockCount += pcCachedSector->muiLocks;
		}
		pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetNext(pcCachedSector);
	}

	mllcCachedSectors.Kill();

	return uiLockCount == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFatCache CFatInfoSectorCache::ReadSector(uint32 uiInfoSector)
{
	CFatSectorCache*	pcCachedSector;
	SFatCache			sSectorCache;

	pcCachedSector = GetOrCacheSector(uiInfoSector);

	if (pcCachedSector != NULL)
	{
		sSectorCache.pvCachedSector = pcCachedSector->GetCache();
		sSectorCache.uiTimeStamp = (uint32)pcCachedSector->muiCreationStamp;
	}
	else
	{
		sSectorCache.pvCachedSector = NULL;
		sSectorCache.uiTimeStamp = 0xffffffff;
	}
	return sSectorCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatInfoSectorCache::ValidateTimeStamp(SFatCache sSectorCache)
{
	CFatSectorCache* pcSectorCache;

	pcSectorCache = (CFatSectorCache*)RemapSinglePointer(sSectorCache.pvCachedSector, -((ptrdiff_t)sizeof(CFatSectorCache)));
	return (uint32)pcSectorCache->muiCreationStamp == sSectorCache.uiTimeStamp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFatSectorCache* CFatInfoSectorCache::AddNewCache(void)
{
	CFatSectorCache*	pcSectorCache;
	size				uiSize;

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
	else
	{
		bSuccess = FlushCache(pcOldestUnlockedSector);
		if (!bSuccess)
		{
			return NULL;
		}
		return InitialiseCache(uiInfoSector, pcOldestUnlockedSector);
	}

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
void CFatInfoSectorCache::Lock(SFatCache sSectorCache)
{
	CFatSectorCache*	pcCachedSector;

	pcCachedSector = (CFatSectorCache*)RemapSinglePointer(sSectorCache.pvCachedSector, -((ptrdiff_t) sizeof(CFatSectorCache)));
	pcCachedSector->Lock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatInfoSectorCache::Unlock(SFatCache sSectorCache)
{
	CFatSectorCache* pcCachedSector;

	pcCachedSector = (CFatSectorCache*)RemapSinglePointer(sSectorCache.pvCachedSector, -((ptrdiff_t)sizeof(CFatSectorCache)));
	return pcCachedSector->Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatInfoSectorCache::Unlock(uint32 uiSector)
{
	SFatCache			sSectorCache;
	CFatSectorCache*	pcCachedSector;
	
	sSectorCache = FindCachedSector(uiSector);
	if (sSectorCache.IsValid())
	{
		pcCachedSector = (CFatSectorCache*)RemapSinglePointer(sSectorCache.pvCachedSector, -((ptrdiff_t)sizeof(CFatSectorCache)));
		pcCachedSector->Unlock();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFatCache CFatInfoSectorCache::FindCachedSector(uint32 uiInfoSector)
{
	CFatSectorCache*	pcCachedSector;
	SFatCache			sCachedSector;

	pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetHead();
	while (pcCachedSector != NULL)
	{
		if (pcCachedSector->muiInfoSector == uiInfoSector)
		{
			sCachedSector.pvCachedSector = pcCachedSector->GetCache();
			sCachedSector.uiTimeStamp = (uint32)pcCachedSector->muiCreationStamp;
			return sCachedSector;
		}

		pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetNext(pcCachedSector);
	}
	sCachedSector.pvCachedSector = NULL;
	sCachedSector.uiTimeStamp = 0xffffffff;
	return sCachedSector;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatInfoSectorCache::Dirty(SFatCache sSectorCache)
{
	CFatSectorCache* pcCachedSector;

	pcCachedSector = (CFatSectorCache*)RemapSinglePointer(sSectorCache.pvCachedSector, -((ptrdiff_t)sizeof(CFatSectorCache)));
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
	return (uint16)mllcCachedSectors.NumElements();
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatInfoSectorCache::Dump(void)
{
	CFatSectorCache*	pcCachedSector;
	CChars				sz;

	sz.Init();
	pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetHead();
	while (pcCachedSector != NULL)
	{
		sz.Append("Sector: ");
		sz.Append(pcCachedSector->muiInfoSector);
		sz.AppendNewLine();

		sz.Append("Time:   ");
		sz.Append(pcCachedSector->muiCreationStamp);
		sz.AppendNewLine();

		sz.Append("Dirty:  ");
		sz.AppendBool(pcCachedSector->mbDirty);
		sz.AppendNewLine();

		sz.Append("Locks:  ");
		sz.Append(pcCachedSector->muiLocks);
		sz.AppendNewLine();
		sz.AppendNewLine();

		pcCachedSector = (CFatSectorCache*)mllcCachedSectors.GetNext(pcCachedSector);
	}
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SFatCache::Clear(void)
{
	pvCachedSector = NULL;
	uiTimeStamp = 0xffffffff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* SFatCache::Get(void)
{
	return (uint8*)pvCachedSector;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SFatCache::IsValid(void)
{
	return pvCachedSector != NULL;
}

