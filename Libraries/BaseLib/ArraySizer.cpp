#include "ArraySizer.h"


//////////////////////////////////////////////////////////////////////////
//
// How many elements an array grows by when it has iNumElements: more as it
// gets larger, so it is copied fewer times (a third of them for the
// largest).
//
//////////////////////////////////////////////////////////////////////////
lsize CalculateChunkSize(lsize iNumElements)
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
		//A third of the elements (60750 at 182250), so a large array still grows by a third or more each time: a fixed
		//chunk would copy it again and again as it grew.
		return iNumElements / 3;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
lsize CalculateChunkSize(lsize iNumElements, lsize iCurrentChunkSize)
{
	lsize	iNewChunkSize;

	iNewChunkSize = CalculateChunkSize(iNumElements);
	if (iNewChunkSize < iCurrentChunkSize)
	{
		iNewChunkSize = iCurrentChunkSize;
	}
	return iNewChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
lsize CalculateArraySize(lsize iNumElements, lsize iChunkeSize)
{
	lsize	iNumAllocations;
	lsize	iNumLeftOvers;


	iNumAllocations = iNumElements / iChunkeSize;
	iNumLeftOvers = iNumElements % iChunkeSize;

	if (iNumLeftOvers != 0)
	{
		iNumAllocations++;
	}

	return iNumAllocations * iChunkeSize;
}

