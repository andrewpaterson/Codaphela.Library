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
BOOL CMemoryQueue::FindOverlapping(SMemoryCacheDescriptor* pvCacheBasedNew, size_t uiNewSize)
{
	SMemoryCacheDescriptor* psNext;

	psNext = CCircularMemoryList::GetFirst();
	for (;;)
	{
		if (Overlaps(pvCacheBasedNew, uiNewSize, psNext))
		{
			return TRUE;
		}

		psNext = GetNext(psNext);
		if (IsFirst(psNext))
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
	SMemoryCacheDescriptor*		psCacheBasedDescriptor;
	size_t						uiRemainingAfterLast;
	size_t						uiTotalSize;
	BOOL						bOverlaps;
	SMemoryCacheDescriptor*		psCacheBasedTail;

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
			psCacheBasedTail = CCircularMemoryList::GetLast();
			psCacheBasedDescriptor = (SMemoryCacheDescriptor*)RemapSinglePointer(psCacheBasedTail, miDescriptorSize + psCacheBasedTail->uiSize);
		}
		else
		{
			//Cycle back to the beginning of the cache.
			psCacheBasedDescriptor = (SMemoryCacheDescriptor*)mpvCache;
		}
		bOverlaps = FindOverlapping(psCacheBasedDescriptor, uiTotalSize);
		if (bOverlaps)
		{
			return NULL;
		}
		psCacheBasedDescriptor = InsertNext(psCacheBasedDescriptor);
	}
	else
	{
		psCacheBasedDescriptor = OneAllocation();
	}

	psCacheBasedDescriptor->uiSize = uiDataSize;
	return GetData(psCacheBasedDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryQueue::Peek(size_t* puiDataSize)
{
	void*						pv;
	SMemoryCacheDescriptor*		psCacheBasedHead;

	if (!IsEmpty())
	{
		psCacheBasedHead = CCircularMemoryList::GetFirst();
		pv = GetData(psCacheBasedHead);
		SafeAssign(puiDataSize, psCacheBasedHead->uiSize);
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
BOOL CMemoryQueue::Drop(void* pvCacheBasedHead)
{
	SMemoryCacheDescriptor*		psDescriptor;

	psDescriptor = GetDescriptorNoRemap(pvCacheBasedHead);
	if (IsFirst(psDescriptor))
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
	void*					pvData;

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
	void*					pvData;

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
	mpvCache = (SMemoryCacheDescriptor*)pvMemory;
	muiCacheSize = uiByteSize;
}

