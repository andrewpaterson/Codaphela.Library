#include "ListBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Init(size iElementSize)
{
	Init(iElementSize, 4);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Init(size iElementSize, uint16 iAlignment)
{
	Init(iElementSize, iAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Init(size iElementSize, uint16 iAlignment, int16 iOffset)
{
	mcFreeList.Init(iElementSize, iAlignment, iOffset);
	mapIndices.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Kill(void)
{
	mapIndices.Kill();
	mcFreeList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CListBlock::NumElements(void)
{
	return mapIndices.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CListBlock::NumBlocks(void)
{
	return mcFreeList.GetNumAllocatedChunks();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CListBlock::NumElementsFromFreeList(void)
{
	return mcFreeList.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CListBlock::IsEmpty(void)
{
	return mapIndices.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CListBlock::IsNotEmpty(void)
{
	return mapIndices.IsNotEmpty();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CListBlock::ElementSize(void)
{
	return mcFreeList.GetElementSize();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::CListBlock::Add(void)
{
	void* pv;

	pv = mcFreeList.Add();
	mapIndices.Add(pv);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::Add(void* pvData)
{
	void* pv;

	pv = mcFreeList.Add(pvData);
	mapIndices.Add(pv);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::AddGetIndex(size* piIndex)
{
	void* pv;
	void** ppv;

	pv = mcFreeList.Add();
	if (pv != NULL)
	{
		ppv = mapIndices.AddGetIndex(piIndex);
		if (ppv != NULL)
		{
			*ppv = pv;
		}
		else
		{
			mcFreeList.Remove(pv);
			pv = NULL;
		}
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::Get(size iIndex)
{
	void** ppv;

	ppv = mapIndices.SafeGet(iIndex);
	return Dereference(ppv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void*CListBlock::SafeGet(size iIndex)
{
	return Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CListBlock::GetIndex(void* pvElement)
{
	return mapIndices.Find(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::Tail(void)
{
	void**	ppv;

	ppv = mapIndices.Tail();
	return Dereference(ppv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::InsertAt(size iIndex)
{
	void*	pv;

	pv = mcFreeList.Add();
	mapIndices.InsertAt(&pv, iIndex);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::InsertAt(void* pvData, size iIndex)
{
	void*	pv;

	pv = mcFreeList.Add();
	mapIndices.InsertAt(&pv, iIndex);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CListBlock::Pop(void* pvData)
{
	void*	pv;

	pv = mapIndices.Pop();
	if (pv != NULL)
	{
		memcpy(pvData, pv, mcFreeList.GetElementSize());
		mcFreeList.Remove(pv);
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
bool CListBlock::Pop(void)
{
	void*	pv;

	pv = mapIndices.Pop();
	if (pv != NULL)
	{
		mcFreeList.Remove(pv);
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
void* CListBlock::Push(void* pvElement)
{
	void*	pv;

	pv = mcFreeList.Add(pvElement);
	mapIndices.Push(pv);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::Push(void)
{
	void*	pv;

	pv = mcFreeList.Add();
	mapIndices.Push(pv);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Reverse(void)
{
	mapIndices.Reverse();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CListBlock::Contains(void* pvData)
{
	if (GetIndex(pvData) != -1)
	{
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
void CListBlock::RemoveAt(size iIndex, bool bPreserveOrder)
{
	void*	pv;
	bool	bResult;

	bResult = mapIndices.Get(iIndex, &pv);
	if (bResult)
	{
		mapIndices.RemoveAt(iIndex, bPreserveOrder);
		mcFreeList.Remove(pv);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::RemoveRange(size iStartIndex, size iEndIndexExclusive, bool bPreserveOrder)
{
	size	i;
	void*	pv;

	for (i = iStartIndex; i < iEndIndexExclusive; i++)
	{
		pv = mapIndices.GetPtr(i);
		mcFreeList.Remove(pv);
	}

	mapIndices.RemoveRange(iStartIndex, iEndIndexExclusive, bPreserveOrder);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::RemoveTail(void)
{
	Pop();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CListBlock::Set(size iIndex, void* pvData)
{
	void*	pv;
	bool	bResult;

	bResult = mapIndices.Get(iIndex, &pv);
	if (bResult)
	{
		memcpy(pv, pvData, mcFreeList.GetElementSize());
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
bool CListBlock::SafeSet(size iIndex, void* pvData)
{
	return Set(iIndex, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Swap(size iIndex1, size iIndex2)
{
	return mapIndices.Swap(iIndex1, iIndex2);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::Dereference(void** ppv)
{
	if (ppv != NULL)
	{
		return *ppv;
	}
	else
	{
		return NULL;
	}
}

