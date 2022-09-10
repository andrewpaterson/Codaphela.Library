#include "PointerRemapper.h"
#include "StringHelper.h"
#include "BubbleSort.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void BubbleSort(DataCompare fCompare, void* pvArray, size_t uiStride, int iNumElements)
{
	int		iIndex;
	int		iResult;
	bool	bSwapped;
	int		iNumDone;
	void*	pvLeft;
	void*	pvRight;

	bSwapped = true;
	iNumDone = 0;
	while (bSwapped)
	{
		iIndex = 0;
		bSwapped = false;

		while ((iIndex + 1) < (iNumElements - iNumDone))
		{
			pvLeft = RemapSinglePointer(pvArray, uiStride * iIndex);
			pvRight = RemapSinglePointer(pvArray, uiStride * (iIndex + 1));
			iResult = fCompare(pvLeft, pvRight);
			if (iResult > 0)
			{
				MemSwp(pvLeft, pvRight, uiStride);
				bSwapped = true;
			}
			iIndex++;
		}
		iNumDone++;
	}
}

