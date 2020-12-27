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
void* CMemoryQueue::InsertAfterTail(size_t uiDataSize)
{
	SMemoryCacheDescriptor*		psDescriptor;
	size_t						iRemainingAfterLast;

	if (uiDataSize > muiCacheSize)
	{
		return NULL;
	}

	iRemainingAfterLast = RemainingAfterTail();
	if (!IsEmpty())
	{
		if (uiDataSize <= iRemainingAfterLast)
		{
			psDescriptor = (SMemoryCacheDescriptor*)RemapSinglePointer(mpsTail, miDescriptorSize + mpsTail->uiSize);
		}
		else
		{
			//Cycle back to the beginning of the cache.
			psDescriptor = (SMemoryCacheDescriptor*)mpvCache;
		}
		FindOverlapping(psDescriptor, uiDataSize);
	}
	else
	{
		psDescriptor = (SMemoryCacheDescriptor*)mpvCache;
	}

	CCircularMemoryList::InsertNext(psDescriptor);
	psDescriptor->uiSize = uiDataSize;
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryQueue::GetHead(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryQueue::RemoveHead(void)
{
}

