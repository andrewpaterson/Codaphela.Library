#include <stdlib.h>
#include <string.h>
#include "PointerFunctions.h"
#include "ArrayElementNotFound.h"
#include "IntegerHelper.h"
#include "FatClusterCache.h"


//The W65C816 has these in FatClusterCache816.asm.
#ifndef __W65C816__


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFatClusterCache::Init(CFileDrive* pcDrive, uint16 uiClusterSize, uint16 uiSectorSize)
{
	muiClusterSize = uiClusterSize;
	muiSectorSize = uiSectorSize;
	muiSectorsPerCluster = muiClusterSize / muiSectorSize;

	//Sector sizes are powers of two (the volume checks): offsets are shifted and masked, not divided.
	muiSectorSizeShift = 0;
	while (((uint32)1 << muiSectorSizeShift) < muiSectorSize)
	{
		muiSectorSizeShift++;
	}
	mpcDrive = pcDrive;
	muiBytesForBits = muiSectorsPerCluster / sizeof(uint8);

	msCluster.pvCache = (uint8*)malloc(uiClusterSize);
	msCluster.pbCachedSectors = (uint8*)malloc(muiBytesForBits);
	msCluster.pbDirtySectors = (uint8*)malloc(muiBytesForBits);
	memset(msCluster.pvCache, 0, muiClusterSize);
	memset(msCluster.pbCachedSectors, 0, muiBytesForBits);
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
// The sector of the byte before uiEnd (0 when uiEnd is 0).
//
//////////////////////////////////////////////////////////////////////////
uint16 CFatClusterCache::LastSectorBefore(uint16 uiEnd)
{
	return (uiEnd == 0) ? 0 : (uint16)((uiEnd - 1) >> muiSectorSizeShift);
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
	size	iStart;
	size	iEnd;
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

	uiFirstSectorIndex = uiOffset >> muiSectorSizeShift;
	uiLastSectorIndexInclusive = (uiOffset + (uiLength - 1)) >> muiSectorSizeShift;

	uiMaximumSector = LastSectorBefore(uiMaximumOffset);

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
		if ((iStart == ARRAY_ELEMENT_NOT_FOUND) || (iStart > uiLastSectorIndexInclusive))
		{
			break;
		}

		iEnd = FindNextSetBit(msCluster.pbCachedSectors, muiSectorsPerCluster, iStart);
		if ((iEnd == ARRAY_ELEMENT_NOT_FOUND) || (iEnd > uiLastSectorIndexInclusive))
		{
			iEnd = uiLastSectorIndexInclusive + 1;
		}

		uiNumSectors = (uint16)(iEnd - iStart);
		mpcDrive->Read(msCluster.uiClusterFirstSector + iStart, uiNumSectors, &msCluster.pvCache[iStart << muiSectorSizeShift]);
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
	uiFirstSectorIndex = uiOffset >> muiSectorSizeShift;
	uiLastSectorIndexInclusive = (uiOffset + (uiLength - 1)) >> muiSectorSizeShift;

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

	uiPreviousMaximumSector = LastSectorBefore(uiPreviousMaximumOffset);

	if (uiFirstSectorIndex <= uiPreviousMaximumSector)
	{
		//The first sector keeps what the write does not cover: before it, and after it when it ends in the same sector.
		if ((((uiOffset & (muiSectorSize - 1)) != 0) || ((uiFirstSectorIndex == uiLastSectorIndexInclusive) && (((uiOffset + uiLength) & (muiSectorSize - 1)) != 0))) && (!IsCached(uiFirstSectorIndex)))
		{
			bResult = CacheSector(&msCluster, uiFirstSectorIndex);
			if (!bResult)
			{
				return false;
			}
		}
	}

	uiSectorLength = (uint16)(muiSectorSize - (uiOffset & (muiSectorSize - 1)));
	if (uiSectorLength > uiLength)
	{
		uiSectorLength = uiLength;
	}

	uiIndex = 0;
	memcpy(&msCluster.pvCache[uiOffset], &pvSource[uiIndex], uiSectorLength);
	SetCachedSector(uiFirstSectorIndex, true);
	SetDirtySector(uiFirstSectorIndex, true);

	uiIndex += uiSectorLength;
	uiOffset += uiSectorLength;
	uiLength -= uiSectorLength;

	uiSectorLength = muiSectorSize;
	for (i = uiFirstSectorIndex + 1; i < uiLastSectorIndexInclusive; i++)
	{
		memcpy(&msCluster.pvCache[uiOffset], &pvSource[uiIndex], uiSectorLength);
		SetCachedSector(i, true);
		SetDirtySector(i, true);

		uiIndex += uiSectorLength;
		uiOffset += uiSectorLength;
		uiLength -= uiSectorLength;
	}

	if (uiFirstSectorIndex != uiLastSectorIndexInclusive)
	{
		if (uiLastSectorIndexInclusive <= uiPreviousMaximumSector)
		{
			if ((((uiOffset + uiLength) & (muiSectorSize - 1)) != 0) && (!IsCached(uiLastSectorIndexInclusive)))
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
		SetCachedSector(uiLastSectorIndexInclusive, true);
		SetDirtySector(uiLastSectorIndexInclusive, true);

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
	size	iFirstDirtySector;
	size	iLastDirtySector;
	bool	bDone;
	bool	bResult;

	if (psCluster->uiCluster == NO_CLUSTER_CACHED)
	{
		return true;
	}

	iFirstDirtySector = FindFirstSetBit(psCluster->pbDirtySectors, muiSectorsPerCluster);
	if (iFirstDirtySector == ARRAY_ELEMENT_NOT_FOUND)
	{
		return true;
	}

	bDone = false;
	for (;;)
	{
		iLastDirtySector = FindNextClearBit(psCluster->pbDirtySectors, muiSectorsPerCluster, iFirstDirtySector);
		if (iLastDirtySector == ARRAY_ELEMENT_NOT_FOUND)
		{
			iLastDirtySector = muiSectorsPerCluster;
			bDone = true;
		}

		bResult = mpcDrive->Write(psCluster->uiClusterFirstSector + (uint32)iFirstDirtySector, (uint32)(iLastDirtySector - iFirstDirtySector), &psCluster->pvCache[iFirstDirtySector << muiSectorSizeShift]);
		if (!bResult)
		{
			return false;
		}

		if (bDone)
		{
			break;
		}

		iFirstDirtySector = FindNextSetBit(psCluster->pbDirtySectors, muiSectorsPerCluster, iLastDirtySector);
		if (iFirstDirtySector == ARRAY_ELEMENT_NOT_FOUND)
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

	bResult = mpcDrive->Read(psCluster->uiClusterFirstSector + uiSectorIndex, &psCluster->pvCache[uiSectorIndex << muiSectorSizeShift]);
	if (!bResult)
	{
		return false;
	}

	SetCachedSector(uiSectorIndex, true);
	SetDirtySector(uiSectorIndex, false);

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
	uint16	i;

	psCluster->uiCluster = NO_CLUSTER_CACHED;

	//A sector that isn't cached is all 0 (Init clears them all), so only the cached ones are cleared.
	for (i = 0; i < muiSectorsPerCluster; i++)
	{
		if (GetBit(i, psCluster->pbCachedSectors))
		{
			memset(&psCluster->pvCache[i << muiSectorSizeShift], 0, muiSectorSize);
		}
	}
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
bool CFatClusterCache::IsSectorDirty(uint16 iSectorIndex)
{
	if (iSectorIndex < muiSectorsPerCluster)
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
bool CFatClusterCache::IsSectorCached(uint16 iSectorIndex)
{
	if (iSectorIndex < muiSectorsPerCluster)
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFatClusterCache::SetCachedSector(size uiSectorIndex, bool bCached)
{
	if (uiSectorIndex < muiBytesForBits)
	{
		SetBit(uiSectorIndex, msCluster.pbCachedSectors, bCached);
		return true;
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
bool CFatClusterCache::SetDirtySector(size uiSectorIndex, bool bDirty)
{
	if (uiSectorIndex < muiBytesForBits)
	{
		SetBit(uiSectorIndex, msCluster.pbDirtySectors, bDirty);
		return true;
	}
	else
	{
		return false;
	}
}
#endif // __W65C816__

