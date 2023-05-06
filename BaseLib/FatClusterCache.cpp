#include <stdlib.h>
#include "PointerFunctions.h"
#include "IntegerHelper.h"
#include "FatClusterCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatClusterCache::Init(CFileDrive* pcDrive, uint16 uiClusterSize, uint16 uiSectorSize)
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
void CFatClusterCache::Kill(void)
{
	SafeFree(msCluster.pbDirtySectors);
	SafeFree(msCluster.pbCachedSectors);
	SafeFree(msCluster.pvCache);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatClusterCache::Read(uint8* pvDestination, uint32 uiCluster, uint32 uiClusterFirstSector, uint16 uiOffset, uint32* puiLength, uint16 uiMaximumOffset)
{
	bool	bResult;
	uint32	uiLength;
	uint16	uiFirstSectorIndex;
	uint16	uiLastSectorIndexInclusive;
	uint16	uiMaximumSector;
	int		iStart;
	int		iEnd;
	uint16	uiNumSectors;

	if (uiOffset >= muiClusterSize)
	{
		return false;
	}
	if (uiOffset > uiMaximumOffset)
	{
		return false;
	}
	if (uiMaximumOffset > muiClusterSize)
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
	if (uiOffset + uiLength > uiMaximumOffset)
	{
		uiLength = uiMaximumOffset - uiOffset;
	}

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
		*puiLength -= uiLength;
	}

	for (;;)
	{
		iStart = FindNextClearBit(msCluster.pbCachedSectors, muiSectorsPerCluster, uiFirstSectorIndex);
		if ((iStart == -1) || (iStart > uiLastSectorIndexInclusive))
		{
			break;
		}

		iEnd = FindNextSetBit(msCluster.pbCachedSectors, muiSectorsPerCluster, iStart);
		if ((iEnd == -1) || (iEnd > uiLastSectorIndexInclusive))
		{
			iEnd = uiLastSectorIndexInclusive + 1;
		}

		uiNumSectors = iEnd - iStart;
		mpcDrive->Read(msCluster.uiClusterFirstSector + iStart, uiNumSectors, &msCluster.pvCache[iStart * muiSectorSize]);
		SetBits(iStart, msCluster.pbCachedSectors, true, uiNumSectors);

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
bool CFatClusterCache::Write(uint8* pvSource, uint32 uiCluster, uint32 uiClusterFirstSector, uint16 uiOffset, uint32* puiLength, uint16 uiPreviousMaximumOffset)
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
	if (uiPreviousMaximumOffset > muiClusterSize)
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
bool CFatClusterCache::Flush(void)
{
	return Flush(&msCluster);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatClusterCache::Flush(SClusterCache* psCluster)
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
bool CFatClusterCache::FlushAndInvalidate(SClusterCache* psCluster)
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
bool CFatClusterCache::IsCached(uint32 uiSectorIndex)
{
	return GetBit(uiSectorIndex, msCluster.pbCachedSectors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatClusterCache::CacheSector(SClusterCache* psCluster, uint32 uiSectorIndex)
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
void CFatClusterCache::Clear(void)
{
	Invalidate(&msCluster);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatClusterCache::Invalidate(SClusterCache* psCluster)
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
uint8* CFatClusterCache::GetCache(void)
{
	return msCluster.pvCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatClusterCache::IsSectorDirty(int iSectorIndex)
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
bool CFatClusterCache::IsSectorCached(int iSectorIndex)
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
uint16 CFatClusterCache::GetSectorsPerCluster(void)
{
	return muiSectorsPerCluster;
}

