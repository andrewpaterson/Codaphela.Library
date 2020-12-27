#include "MemoryQueue.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryQueue::Init(void* pvMemory, size_t uiByteSize)
{
	CCircularMemoryList::Init(pvMemory, uiByteSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryQueue::Kill(void)
{
	CCircularMemoryList::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryQueue::FindOverlapping(void* pvNew, size_t uiNewSize)
{
	SMemoryCacheDescriptor* psNext;

	psNext = mpsHead;
	for (;;)
	{
		if (Overlaps(pvNew, uiNewSize, psNext))
		{
			return TRUE;
		}

		psNext = psNext->psNext;
		if (psNext == mpsHead)
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryQueue::Push(size_t uiDataSize)
{
	SMemoryCacheDescriptor*		psDescriptor;
	size_t						uiRemainingAfterLast;
	size_t						uiTotalSize;
	BOOL						bOverlaps;

	uiTotalSize = uiDataSize + miDescriptorSize;
	if (uiTotalSize > muiCacheSize)
	{
		return NULL;
	}

	uiRemainingAfterLast = RemainingAfterTail();
	if (!IsEmpty())
	{
		if (uiTotalSize <= uiRemainingAfterLast)
		{
			psDescriptor = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsTail, miDescriptorSize + mpsTail->uiSize);
		}
		else
		{
			//Cycle back to the beginning of the cache.
			psDescriptor = (SMemoryCacheDescriptor*)mpvCache;
		}
		bOverlaps = FindOverlapping(psDescriptor, uiTotalSize);
		if (bOverlaps)
		{
			return NULL;
		}
		psDescriptor = InsertNext(psDescriptor);
	}
	else
	{
		psDescriptor = OneAllocation();
	}

	psDescriptor->uiSize = uiDataSize;
	return GetData(psDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryQueue::Peek(size_t* puiDataSize)
{
	void*	pv;

	if (!IsEmpty())
	{
		pv = GetData(mpsHead);
		*puiDataSize = mpsHead->uiSize;
		return pv;
	}
	else
	{
		*puiDataSize = 0;
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryQueue::Drop(void* pvHead)
{
	SMemoryCacheDescriptor*		psDescriptor;

	psDescriptor = GetDescriptor(pvHead);
	if (psDescriptor == mpsHead)
	{
		Deallocate(psDescriptor);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

