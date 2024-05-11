#include "ArrayVoidPtr.h"
#include "Mallocator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMallocator::Malloc(size uiSize, char(** pacDebugName)[4])
{
	if (pacDebugName)
	{
		*pacDebugName = NULL;
	}
	return Malloc(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMallocator::FreeMultiple(CArrayVoidPtr* pav)
{
	size	i;
	void*	pv;
	bool	bFreed;

	pav->QuickSort();
	i = pav->NumElements();
	if (i != 0)
	{
		do
		{
			i--;
			pv = pav->GetPtr(i);
			bFreed = Free(pv);
			if (!bFreed)
			{
				return i;
			}
		}
		while (i != 0);
	}

	return 0;
}

