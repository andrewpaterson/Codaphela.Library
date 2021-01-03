#include "PointerFunctions.h"
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
		SafeAssign(puiDataSize,  mpsHead->uiSize);
		return pv;
	}
	else
	{
		SafeAssign(puiDataSize, 0);
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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryQueue::Pop(void)
{
	size_t	uiDataSize;
	void*	pvData;

	pvData = Peek(&uiDataSize);
	if (!pvData)
	{
		return FALSE;
	}

	return Drop(pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryQueue::GetFirst(void)
{
	SMemoryCacheDescriptor* psDesc;
	void* pvData;

	psDesc = CCircularMemoryList::GetFirst();
	pvData = GetData(psDesc);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemoryQueue::GetLast(void)
{
	SMemoryCacheDescriptor* psDesc;
	void* pvData;

	psDesc = CCircularMemoryList::GetLast();
	pvData = GetData(psDesc);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryQueue::Touch(void* pvMemory, size_t uiByteSize)
{
	if ((mpvCache != pvMemory) || (uiByteSize != muiCacheSize))
	{
		
	}
}

