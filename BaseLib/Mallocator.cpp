#include "ArrayVoidPtr.h"
#include "Mallocator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMallocator::Malloc(size_t tSize, char(** pacDebugName)[4])
{
	if (pacDebugName)
	{
		*pacDebugName = NULL;
	}
	return Malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMallocator::FreeMultiple(CArrayVoidPtr* pav)
{
	int		iNumElements;
	int		i;
	void*	pv;
	bool	bFreed;

	pav->QuickSort();
	iNumElements = pav->NumElements();
	for (i = iNumElements - 1; i >=0; i--)
	{
		pv = pav->GetPtr(i);
		bFreed = Free(pv);
		if (!bFreed)
		{
			return i;
		}
	}
	return i;
}

