#include "ListBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Init(int iChunkSize, int iElementSize)
{
	Init(iChunkSize, iElementSize, 4);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Init(int iChunkSize, int iElementSize, int iAlignment)
{
	Init(iChunkSize, iElementSize, iAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Init(int iChunkSize, int iElementSize, int iAlignment, int iOffset)
{
	mcFreeList.Init(iChunkSize, iElementSize, iAlignment, iOffset);
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
int CListBlock::NumElements(void)
{
	return mapIndices.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CListBlock::NumBlocks(void)
{
	return mcFreeList.GetNumAllocatedChunks();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CListBlock::NumElementsFromFreeList(void)
{
	return mcFreeList.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CListBlock::IsEmpty(void)
{
	return mapIndices.IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CListBlock::IsNotEmpty(void)
{
	return mapIndices.IsNotEmpty();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CListBlock::ElementSize(void)
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
void* CListBlock::AddGetIndex(int* piIndex)
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
void* CListBlock::Get(int iIndex)
{
	void** ppv;

	ppv = mapIndices.SafeGet(iIndex);
	return Dereference(ppv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void*CListBlock::SafeGet(int iIndex)
{
	return Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CListBlock::GetIndex(void* pvElement)
{
	return mapIndices.Find(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::Tail(void)
{
	void** ppv;

	ppv = mapIndices.Tail();
	return Dereference(ppv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::InsertAt(int iIndex)
{
	void* pv;

	pv = mcFreeList.Add();
	mapIndices.InsertAt(&pv, iIndex);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::InsertAt(void* pvData, int iIndex)
{
	void* pv;

	pv = mcFreeList.Add();
	mapIndices.InsertAt(&pv, iIndex);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Pop(void* pvData)
{
	void* pv;

	mapIndices.Pop(&pv);
	memcpy(pvData, pv, mcFreeList.GetElementSize());
	mcFreeList.Remove(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Pop(void)
{
	void* pv;

	mapIndices.Pop(&pv);
	mcFreeList.Remove(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::Push(void* pvElement)
{
	void* pv;

	pv = mcFreeList.Add(pvElement);
	mapIndices.Push(&pv);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CListBlock::Push(void)
{
	void* pv;

	pv = mcFreeList.Add();
	mapIndices.Push(&pv);
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
BOOL CListBlock::Contains(void* pData)
{
	if (GetIndex(pData) != -1)
	{
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
void CListBlock::RemoveAt(int iIndex, int bPreserveOrder)
{
	void* pv;
	BOOL bResult;

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
void CListBlock::RemoveRange(int iStartIndex, int iEndIndexExclusive, BOOL bPreserveOrder)
{
	int		i;
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
BOOL CListBlock::Set(int iIndex, void* pvData)
{
	void*	pv;
	BOOL	bResult;

	bResult = mapIndices.Get(iIndex, &pv);
	if (bResult)
	{
		memcpy(pv, pvData, mcFreeList.GetElementSize());
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
BOOL CListBlock::SafeSet(int iIndex, void* pvData)
{
	return Set(iIndex, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CListBlock::Swap(int iIndex1, int iIndex2)
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

