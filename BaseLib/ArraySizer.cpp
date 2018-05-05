#include "ArraySizer.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CalculateChunkSize(int iNumElements)
{
	if (iNumElements < 192)
	{
		if (iNumElements < 2)
		{
			return 1;
		}
		if (iNumElements < 4)
		{
			return 2;
		}
		if (iNumElements < 8)
		{
			return 4;
		}
		if (iNumElements < 16)
		{
			return 8;
		}
		if (iNumElements < 64)
		{
			return 16;
		}
		return 32;
	}
	else if (iNumElements < 2916)
	{
		if (iNumElements < 288)
		{
			return 48;
		}
		if (iNumElements < 432)
		{
			return 72;
		}
		if (iNumElements < 648)
		{
			return 108;
		}
		if (iNumElements < 972)
		{
			return 162;
		}
		if (iNumElements < 1458)
		{
			return 243;
		}
		return 486;
	}
	else if (iNumElements < 182250)
	{
		if (iNumElements < 4374)
		{
			return 729;
		}
		if (iNumElements < 8748)
		{
			return 1458;
		}
		if (iNumElements < 13122)
		{
			return 2187;
		}
		if (iNumElements < 26244)
		{
			return 4374;
		}
		if (iNumElements < 39366)
		{
			return 6561;
		}
		if (iNumElements < 65610)
		{
			return 13122;
		}
		if (iNumElements < 109350)
		{
			return 21870;
		}
		return 36450;

	}
	else
	{
		return 60750;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CalculateArraySize(int iNumElements, int iChunkSize)
{
	int	iNewChunkSize;
	int	iNumAllocations;
	int iNumLeftOvers;
	int	iOldUsedElements;

	iNewChunkSize = CalculateChunkSize(iNumElements);
	if (iNewChunkSize < iChunkSize)
	{
		iNewChunkSize = iChunkSize;
	}

	iOldUsedElements = iNumElements;

	iNumAllocations = iNumElements / iNewChunkSize;
	iNumLeftOvers = iNumElements % iNewChunkSize;

	if (iNumLeftOvers > 0)
	{
		iNumAllocations++;
	}

	return iNumAllocations * iNewChunkSize;
}

