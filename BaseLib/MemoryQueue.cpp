#include "PointerFunctions.h"
#include "MemoryQueue.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryQueue::Init(void* pvMemory, size uiByteSize)
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
bool CMemoryQueue::FindOverlapping(SMemoryCacheDescriptor* pvCacheBasedNew, size uiNewSize)
{
	SMemoryCacheDescriptor* psNext;

	psNext = CCircularMemoryList::GetFirst();
	for (;;)
	{
		if (Overlaps(pvCacheBasedNew, uiNewSize, psNext))
		{
			return true;
		}

		psNext = GetNext(psNext);
		if (IsFirst(psNext))
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryQueue::Push(size uiDataSize)
{
	SMemoryCacheDescriptor*		psCacheBasedDescriptor;
	size						uiRemainingAfterLast;
	size						uiTotalSize;
	bool						bOverlaps;
	SMemoryCacheDescriptor*		psCacheBasedTail;

	uiTotalSize = uiDataSize + miDescriptorSize;
	if ((mpsDetail == NULL) || (uiTotalSize > mpsDetail->muiCacheSize))
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
void* CMemoryQueue::Peek(size* puiDataSize)
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
bool CMemoryQueue::Drop(void* pvCacheBasedHead)
{
	SMemoryCacheDescriptor*		psDescriptor;

	psDescriptor = GetDescriptorNoRemap(pvCacheBasedHead);
	if (IsFirst(psDescriptor))
	{
		Deallocate(psDescriptor);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMemoryQueue::Pop(void)
{
	size	uiDataSize;
	void*	pvData;

	pvData = Peek(&uiDataSize);
	if (!pvData)
	{
		return false;
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

