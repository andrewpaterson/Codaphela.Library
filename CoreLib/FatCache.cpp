#include <stdlib.h>
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/IntegerHelper.h"
#include "FatCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatCache::Init(CFileDrive* pcDrive, uint16 uiClusterSize, uint16 uiSectorSize)
{
	muiClusterSize = uiClusterSize;
	muiSectorSize = uiSectorSize;
	muiSectorsPerCluster = muiClusterSize / muiSectorSize;
	mpcDrive = pcDrive;
	muiBytesForBits = muiSectorsPerCluster / sizeof(uint8);

	msCluster.pvCache = (uint8*)malloc(uiClusterSize);
	msCluster.pbCachedSectors = (uint8*)malloc(muiBytesForBits);
	msCluster.pbDirtySectors = (uint8*)malloc(muiBytesForBits);
	Invalidate(&msCluster);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatCache::Kill(void)
{
	SafeFree(msCluster.pbDirtySectors);
	SafeFree(msCluster.pbCachedSectors);
	SafeFree(msCluster.pvCache);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatCache::Read(uint8* pvDestination, uint32 uiCluster, uint32 uiClusterFirstSector, uint16 uiOffset, uint32* puiLength, uint16 uiMaximumOffset)
{
	bool	bResult;
	uint32	uiLength;
	uint16	uiFirstSectorIndex;
	uint16	uiLastSectorIndexInclusive;
	uint16	uiMaximumSector;
	int		iStart;
	int		iEnd;
	int		i;

	if (uiCluster != msCluster.uiCluster)
	{
		bResult = FlushAndInvalidate(&msCluster);
		if (!bResult)
		{
			return false;
		}
		msCluster.uiCluster = uiCluster;
		msCluster.uiClusterFirstSector = uiClusterFirstSector;
	}

	if (uiOffset >= muiClusterSize)
	{
		return false;
	}

	uiLength = *puiLength;
	uiFirstSectorIndex = uiOffset / muiSectorSize;
	uiLastSectorIndexInclusive = (uiOffset + (uiLength - 1)) / muiSectorSize;

	uiMaximumSector = (uiMaximumOffset - 1) / muiSectorSize;

	if (uiFirstSectorIndex > uiMaximumSector)
	{
		return false;
	}

	if (uiLastSectorIndexInclusive >= muiSectorsPerCluster)
	{
		uiLastSectorIndexInclusive = muiSectorsPerCluster - 1;
		uiLength = muiClusterSize - uiOffset;
		*puiLength -= uiLength;
	}
	else
	{
		*puiLength = 0;
	}

	for (;;)
	{
		iStart = FindNextClearBit(msCluster.pbCachedSectors, muiSectorsPerCluster, uiFirstSectorIndex);
		if (iStart == -1)
		{
			break;
		}

		iEnd = FindNextSetBit(msCluster.pbCachedSectors, muiSectorsPerCluster, iStart);
		if ((iEnd == -1) || (iEnd > uiLastSectorIndexInclusive))
		{
			iEnd = uiLastSectorIndexInclusive + 1;
		}
		mpcDrive->Read(msCluster.uiClusterFirstSector + iStart, iEnd - iStart, &msCluster.pvCache[iStart * muiSectorSize]);
		for (i = iStart; i < iEnd; i++)
		{
			SetBit(i, msCluster.pbCachedSectors, true);
		}

		if (iEnd > uiLastSectorIndexInclusive)
		{
			break;
		}
	}

	memcpy(pvDestination, &msCluster.pvCache[uiOffset], uiLength);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatCache::Write(uint8* pvSource, uint32 uiCluster, uint32 uiClusterFirstSector, uint16 uiOffset, uint32* puiLength, uint16 uiPreviousMaximumOffset)
{
	bool	bResult;
	uint32	uiLength;
	uint16	uiFirstSectorIndex;
	uint16	uiLastSectorIndexInclusive;
	uint16	i;
	uint16	uiPreviousMaximumSector;
	uint16	uiSectorLength;
	uint16	uiIndex;

	if (uiOffset >= muiClusterSize)
	{
		return false;
	}
	if (uiOffset > uiPreviousMaximumOffset)
	{
		return false;
	}

	if (uiCluster != msCluster.uiCluster)
	{
		bResult = FlushAndInvalidate(&msCluster);
		if (!bResult)
		{
			return false;
		}
		msCluster.uiCluster = uiCluster;
		msCluster.uiClusterFirstSector = uiClusterFirstSector;
	}

	uiLength = *puiLength;
	uiFirstSectorIndex = uiOffset / muiSectorSize;
	uiLastSectorIndexInclusive = (uiOffset + (uiLength - 1)) / muiSectorSize;

	if (uiLastSectorIndexInclusive >= muiSectorsPerCluster)
	{
		uiLastSectorIndexInclusive = muiSectorsPerCluster - 1;
		uiLength = muiClusterSize - uiOffset;
		*puiLength -= uiLength;
	}
	else
	{
		*puiLength = 0;
	}

	uiPreviousMaximumSector = (uiPreviousMaximumOffset - 1)  / muiSectorSize;

	if (uiFirstSectorIndex <= uiPreviousMaximumSector)
	{
		if ((uiOffset % muiSectorSize != 0) && (!IsCached(uiFirstSectorIndex)))
		{
			bResult = CacheSector(&msCluster, uiFirstSectorIndex);
			if (!bResult)
			{
				return false;
			}
		}
	}

	uiSectorLength = muiSectorSize - uiOffset % muiSectorSize;
	if (uiSectorLength > uiLength)
	{
		uiSectorLength = uiLength;
	}

	uiIndex = 0;
	memcpy(&msCluster.pvCache[uiOffset], &pvSource[uiIndex], uiSectorLength);
	SetBit(uiFirstSectorIndex, msCluster.pbCachedSectors, true);
	SetBit(uiFirstSectorIndex, msCluster.pbDirtySectors, true);
	uiIndex += uiSectorLength;
	uiOffset += uiSectorLength;
	uiLength -= uiSectorLength;

	uiSectorLength = muiSectorSize;
	for (i = uiFirstSectorIndex + 1; i <= uiLastSectorIndexInclusive - 1; i++)
	{
		memcpy(&msCluster.pvCache[uiOffset], &pvSource[uiIndex], uiSectorLength);
		SetBit(i, msCluster.pbCachedSectors, true);
		SetBit(i, msCluster.pbDirtySectors, true);
		uiIndex += uiSectorLength;
		uiOffset += uiSectorLength;
		uiLength -= uiSectorLength;
	}

	if (uiFirstSectorIndex != uiLastSectorIndexInclusive)
	{
		if (uiLastSectorIndexInclusive <= uiPreviousMaximumSector)
		{
			//This is wrong.
			if (((uiOffset + uiLength) % muiSectorSize != 0) && (!IsCached(uiLastSectorIndexInclusive)))
			{
				bResult = CacheSector(&msCluster, uiLastSectorIndexInclusive);
				if (!bResult)
				{
					return false;
				}
			}
		}

		uiSectorLength = uiLength;
		memcpy(&msCluster.pvCache[uiOffset], &pvSource[uiIndex], uiSectorLength);
		SetBit(uiLastSectorIndexInclusive, msCluster.pbCachedSectors, true);
		SetBit(uiLastSectorIndexInclusive, msCluster.pbDirtySectors, true);
		uiIndex += uiSectorLength;
		uiOffset += uiSectorLength;
		uiLength -= uiSectorLength;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatCache::Flush(void)
{
	return Flush(&msCluster);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatCache::Flush(SClusterCache* psCluster)
{
	int		iFirstDirtySector;
	int		iLastDirtySector;
	bool	bDone;
	bool	bResult;

	if (psCluster->uiCluster == NO_CLUSTER_CACHED)
	{
		return true;
	}

	iFirstDirtySector = FindFirstSetBit(psCluster->pbDirtySectors, muiSectorsPerCluster);
	if (iFirstDirtySector == -1)
	{
		return true;
	}

	bDone = false;
	for (;;)
	{
		iLastDirtySector = FindNextClearBit(psCluster->pbDirtySectors, muiSectorsPerCluster, iFirstDirtySector);
		if (iLastDirtySector == -1)
		{
			iLastDirtySector = muiSectorsPerCluster;
			bDone = true;
		}

		bResult = mpcDrive->Write(psCluster->uiClusterFirstSector + iFirstDirtySector, iLastDirtySector - iFirstDirtySector, &psCluster->pvCache[iFirstDirtySector * muiSectorSize]);
		if (!bResult)
		{
			return false;
		}

		if (bDone)
		{
			break;
		}

		iFirstDirtySector = FindNextSetBit(psCluster->pbDirtySectors, muiSectorsPerCluster, iLastDirtySector);
		if (iFirstDirtySector == -1)
		{
			break;
		}
	}
	memset(psCluster->pbDirtySectors, 0, muiBytesForBits);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatCache::FlushAndInvalidate(SClusterCache* psCluster)
{
	bool bResult;

	bResult = Flush(psCluster);
	
	Invalidate(psCluster);

	return bResult;;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatCache::IsCached(uint32 uiSectorIndex)
{
	return GetBit(uiSectorIndex, msCluster.pbCachedSectors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatCache::CacheSector(SClusterCache* psCluster, uint32 uiSectorIndex)
{
	bool	bResult;

	bResult = mpcDrive->Read(psCluster->uiClusterFirstSector + uiSectorIndex, psCluster->pvCache);
	if (!bResult)
	{
		return false;
	}

	SetBit(uiSectorIndex, psCluster->pbCachedSectors, true);
	SetBit(uiSectorIndex, psCluster->pbDirtySectors, false);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatCache::Clear(void)
{
	Invalidate(&msCluster);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatCache::Invalidate(SClusterCache* psCluster)
{
	psCluster->uiCluster = NO_CLUSTER_CACHED;
	memset(psCluster->pvCache, 0, muiClusterSize);
	memset(psCluster->pbCachedSectors, 0, muiBytesForBits);
	memset(psCluster->pbDirtySectors, 0, muiBytesForBits);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CFatCache::GetCache(void)
{
	return msCluster.pvCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatCache::IsSectorDirty(int iSectorIndex)
{
	if ((iSectorIndex >= 0) && (iSectorIndex < muiSectorsPerCluster))
	{
		return GetBit(iSectorIndex, msCluster.pbDirtySectors);
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatCache::IsSectorCached(int iSectorIndex)
{
	if ((iSectorIndex >= 0) && (iSectorIndex < muiSectorsPerCluster))
	{
		return GetBit(iSectorIndex, msCluster.pbCachedSectors);
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatCache::GetSectorsPerCluster(void)
{
	return muiSectorsPerCluster;
}

