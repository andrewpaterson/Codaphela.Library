/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "Logger.h"
#include "Validation.h"
#include "DataMemory.h"


CMemoryFreeListParams gcDataMemoryFreeListParams;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::Init(CMemoryFreeListParams* pcFreeListParams)
{
	mcFreeLists.Init();
	mcLargeList.Init();
	mcOrder.Init();

	mpcFreeListParams = pcFreeListParams;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::Kill(void)
{
	CFreeList*	pcFreeList;

	mcOrder.Kill();

	pcFreeList = mcFreeLists.GetHead();
	while (pcFreeList)
	{
		pcFreeList->Kill();
		pcFreeList = mcFreeLists.GetNext(pcFreeList);
	}
	mcFreeLists.Kill();

	mcLargeList.Kill();

	mpcFreeListParams = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::ReInit(void)
{
	CMemoryFreeListParams* pcFreeListParams;

	pcFreeListParams = mpcFreeListParams;

	Kill();
	Init(pcFreeListParams);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::Remove(void* pv)
{
	SDataMemoryAllocation*	psAlloc;
	CFreeList*				pcList;

	psAlloc = DATA_MEMORY_GET_ALLOCATION(pv);
	if (psAlloc->uiSize <= mpcFreeListParams->GetMaxFreeListElementSize())
	{
		pcList = psAlloc->psFreeListNode->pcList;

		DeallocateInFreeList(pcList, psAlloc);
	}
	else
	{
		DeallocateInLargeList(psAlloc);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CDataMemory::GetSize(void* pv)
{
	SDataMemoryAllocation*	psAlloc;

	psAlloc = DATA_MEMORY_GET_ALLOCATION(pv);
	return psAlloc->uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDataMemory::Remove(CArrayVoidPtr* pav)
{
	size					i;
	void*					pv;
	SDataMemoryAllocation*	psAlloc;
	CFreeList*				pcList;
	SFNode*					psNode;
	size					uiNumElements;
	size					uiRemoved;

	pav->QuickSort();

	uiNumElements = pav->NumElements();
	i = 0;
	while (i < uiNumElements)
	{
		pv = pav->GetPtr(i);
		psAlloc = DATA_MEMORY_GET_ALLOCATION(pv);
		if (psAlloc->uiSize <= mpcFreeListParams->GetMaxFreeListElementSize())
		{
			psNode = psAlloc->psFreeListNode;
			pcList = psAlloc->psFreeListNode->pcList;

			uiRemoved = RemoveNode(pav, i, psAlloc, psNode, pcList);
			if (uiRemoved != 0)
			{
				i += uiRemoved;
			}
			else
			{
				uiRemoved = RemoveElements(pav, i, psNode, pcList);
				if (uiRemoved != 0)
				{
					i += uiRemoved;
				}
				else
				{
					gcLogger.Error2(__METHOD__, " Could not deallocate memory.", NULL);
					return false;
				}
			}
		}
		else
		{
			DeallocateInLargeList(psAlloc);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDataMemory::RemoveNode(CArrayVoidPtr* pav, size i, SDataMemoryAllocation* psAlloc, SFNode* psNode, CFreeList* pcList)
{
	void*					pvLast;
	SDataMemoryAllocation*	psPotentialLast;
	size					uiNodeElements;
	SDataMemoryAllocation*	psFirst;
	SDataMemoryAllocation*	psLast;
	size					uiNumElements;

	uiNumElements = pav->NumElements();
	psFirst = (SDataMemoryAllocation*)pcList->GetFirstNodeElement(psNode);
	if (psAlloc == psFirst)
	{
		psLast = (SDataMemoryAllocation*)pcList->GetLastNodeElement(psNode);

		if (psNode->bFull)
		{
			uiNodeElements = psNode->uiChunkSize;
		}
		else
		{
			uiNodeElements = pcList->NumNodeElements(psNode);
		}

		if ((i != 0) && (uiNodeElements != 0))
		{
			if (i + uiNodeElements - 1 < uiNumElements)
			{
				pvLast = pav->GetPtr(i + uiNodeElements - 1);
				psPotentialLast = DATA_MEMORY_GET_ALLOCATION(pvLast);

				if (psPotentialLast == psLast)
				{
					pcList->RemoveNode(psNode);
					return uiNodeElements;
				}
			}
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDataMemory::RemoveElements(CArrayVoidPtr* pav, size i, SFNode* psNode, CFreeList* pcList)
{
	void*						pv;
	SDataMemoryAllocation*		psFirst;
	SDataMemoryAllocation*		psLast;
	size						uiCount;
	SDataMemoryAllocation*		psAlloc;
	size						uiNumElements;

	uiNumElements = pav->NumElements();

	psFirst = (SDataMemoryAllocation*)pcList->GetFirstNodeElement(psNode);
	psLast = (SDataMemoryAllocation*)pcList->GetLastNodeElement(psNode);

	uiCount = 0;
	pv = pav->GetPtr(i);
	psAlloc = DATA_MEMORY_GET_ALLOCATION(pv);

	while (psAlloc <= psLast)
	{
		pcList->Remove(psNode, psAlloc);

		i++;
		uiCount++;
		if (i >= uiNumElements)
		{
			break;
		}
		pv = pav->GetPtr(i);
		psAlloc = DATA_MEMORY_GET_ALLOCATION(pv);
	}

	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CDataMemory::Add(size uiSize)
{
	CFreeList*	pcFreeList;
	void*				pv;

	if (uiSize <= mpcFreeListParams->GetMaxFreeListElementSize())
	{
		pcFreeList = GetOrAddFreeList(uiSize);
		pv = AllocateInFreeList(pcFreeList, uiSize);
		return pv;
	}
	else
	{
		pv = AllocateInLargeList(uiSize);
		return pv;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CDataMemory::Grow(void* pvInitial, size uiSize)
{
	SDataMemoryAllocation*	psAlloc;
	CFreeList*				pcList;
	SMemoryFreeListParams*	psParams;
	void*					pvNew;
	SLLAlignedNode*			psNode;

	psAlloc = DATA_MEMORY_GET_ALLOCATION(pvInitial);
	if (psAlloc->uiSize <= mpcFreeListParams->GetMaxFreeListElementSize())
	{
		psParams = mpcFreeListParams->GetFreeListParamsForSize(psAlloc->uiSize);
		if ((uiSize <= psParams->iMaxElementSize) && (uiSize >= psParams->iMinElementSize))
		{
			psAlloc->uiSize = uiSize;
			return pvInitial;
		}
		else
		{
			pcList = psAlloc->psFreeListNode->pcList;
			pvNew = Add(uiSize);
			CopyAllocation(pvNew, pvInitial, uiSize, psAlloc->uiSize);
			DeallocateInFreeList(pcList, psAlloc);
			return pvNew;
		}
	}
	else
	{
		psNode = (SLLAlignedNode*)RemapSinglePointer(psAlloc, -(ptrdiff_t)sizeof(SLLAlignedNode));
		if (uiSize <= mpcFreeListParams->GetMaxFreeListElementSize())
		{
			pvNew = Add(uiSize);
			CopyAllocation(pvNew, pvInitial, uiSize, psAlloc->uiSize);
			DeallocateInLargeList(psAlloc);
			return pvNew;
		}
		else
		{
			psAlloc = (SDataMemoryAllocation*)mcLargeList.Grow(psAlloc, uiSize);
			psAlloc->uiSize = uiSize;
			psAlloc->psFreeListNode = NULL;
			pvNew = RemapSinglePointer(psAlloc, sizeof(SDataMemoryAllocation));
			return pvNew;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::CopyAllocation(void* pvDest, void* pvSource, size uiDestSize, size uiSourceSize)
{
	size	uiSize;

	uiSize = (uiDestSize < uiSourceSize) ? uiDestSize : uiSourceSize;
	memcpy(pvDest, pvSource, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CDataMemory::AllocateInFreeList(CFreeList* pcFreeList, size uiElementSize)
{
	SDataMemoryAllocation*	psMemoryAllocation;
	SFNode*						psNode;

	psMemoryAllocation = (SDataMemoryAllocation*)pcFreeList->Add(&psNode);
	psMemoryAllocation->uiSize = uiElementSize;
	psMemoryAllocation->psFreeListNode = psNode;
	return RemapSinglePointer(psMemoryAllocation, sizeof(SDataMemoryAllocation));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::DeallocateInFreeList(CFreeList* pcFreeList, SDataMemoryAllocation* psAlloc)
{
	SFNode*			psFreeListNode;

	psFreeListNode = psAlloc->psFreeListNode;

#ifdef _DEBUG
	size	iSize;
	void*	pvMem;

	iSize = sizeof(psAlloc) + psAlloc->uiSize - sizeof(size);
	pvMem = RemapSinglePointer(psAlloc, sizeof(size));
	memset(pvMem, 0xef, iSize);
#endif

	pcFreeList->Remove(psFreeListNode, psAlloc);

	if (!pcFreeList->HasElements())
	{
		FreeFreeList(pcFreeList);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::FreeFreeList(CFreeList* pcFreeList)
{
	bool			bResult;
	SFreeListDesc	sDesc;
	size			iIndex;
	size			iStride;

	iStride = pcFreeList->GetElementStride() - sizeof(SDataMemoryAllocation);
	sDesc.Init(iStride);
	bResult = mcOrder.FindInSorted(&sDesc, CompareFreeListDesc, &iIndex);
	if (bResult)
	{
		mcOrder.RemoveAt(iIndex);
	}
	pcFreeList->Kill();
	mcFreeLists.Remove(pcFreeList);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CDataMemory::AllocateInLargeList(size uiSize)
{
	SDataMemoryAllocation*	psMemoryAllocation;

	psMemoryAllocation = (SDataMemoryAllocation*)mcLargeList.InsertAfterTail(uiSize + sizeof(SDataMemoryAllocation), DATA_MEMORY_ALIGNMENT, 0);
	psMemoryAllocation->uiSize = uiSize;
	psMemoryAllocation->psFreeListNode = NULL;
	return RemapSinglePointer(psMemoryAllocation, sizeof(SDataMemoryAllocation));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDataMemory::DeallocateInLargeList(SDataMemoryAllocation* psAlloc)
{
	mcLargeList.Remove(psAlloc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFreeList* CDataMemory::GetFreeList(size iElementSize)
{
	SFreeListDesc			sDesc;
	bool					bResult;
	size					iIndex;
	SFreeListDesc*			psDesc;
	SMemoryFreeListParams*	psParams;
	int						iStride;

	psParams = mpcFreeListParams->GetFreeListParamsForSize(iElementSize);
	iStride = CalculateStride(psParams->iMaxElementSize, DATA_MEMORY_ALIGNMENT);

	sDesc.Init(iStride);
	bResult = mcOrder.FindInSorted(&sDesc, CompareAlignedFreeListDesc, &iIndex);
	if (bResult)
	{
		psDesc = mcOrder.Get(iIndex);
		return psDesc->pcFreeList;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFreeList* CDataMemory::GetOrAddFreeList(size iElementSize)
{
	SFreeListDesc			sDesc;
	bool					bResult;
	size					iIndex;
	SFreeListDesc*			psDesc;
	CFreeList*				pcList;
	SMemoryFreeListParams*	psParams;
	uint8					iFinalOffset;
	size					iStride;

	iFinalOffset = CalculateOffset((int16)(0 - sizeof(SDataMemoryAllocation)), DATA_MEMORY_ALIGNMENT);
	psParams = mpcFreeListParams->GetFreeListParamsForSize(iElementSize);
	if (psParams == NULL)
	{
		gcLogger.Error2(__METHOD__, " No free list parameter for element size [", SizeToString(iElementSize), "].  Available parameter options [", IntToString(mpcFreeListParams->NumParams()), "] (should be more than 0, call DataMemoryInit())." , NULL);
	}
	iStride = CalculateStride(psParams->iMaxElementSize, DATA_MEMORY_ALIGNMENT);

	sDesc.Init(iStride);
	bResult = mcOrder.FindInSorted(&sDesc, CompareFreeListDesc, &iIndex);
	if (bResult)
	{
		psDesc = mcOrder.Get(iIndex);
		return psDesc->pcFreeList;
	}
	else
	{
		pcList = mcFreeLists.InsertAfterTail();
		pcList->Init(psParams->iMaxListSize, DATA_MEMORY_ALIGNMENT);
		sDesc.pcFreeList = pcList;
		mcOrder.InsertAt(&sDesc, iIndex);
		return pcList;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CDataMemory::NumElements(void)
{
	CFreeList*		pcBlock;
	int					uiCount;

	uiCount = 0;
	pcBlock = mcFreeLists.GetHead();
	while (pcBlock)
	{
		uiCount += pcBlock->NumElements();
		pcBlock = mcFreeLists.GetNext(pcBlock);
	}

	uiCount += mcLargeList.NumElements();
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CDataMemory::ByteSize(void)
{
	int					iSize;
	CFreeList*		pcFreeList;

	iSize = 0;
	pcFreeList = mcFreeLists.GetHead();
	while (pcFreeList)
	{
		iSize += pcFreeList->ByteSize();
		iSize += sizeof(CFreeList);
		pcFreeList = mcFreeLists.GetNext(pcFreeList);
	}

	iSize += mcLargeList.ByteSize();
	iSize += sizeof(CLinkedListBlockAligned);

	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemory CDataMemory::StartIteration(SMemoryIterator* psIterator)
{
	void*		pv;
	SMemory		sResult;
	void*		pvLargeHead;

	sResult.Init();

	psIterator->pcFreeList = mcFreeLists.GetHead();
	pvLargeHead = mcLargeList.GetHead();

	if (psIterator->pcFreeList != NULL)
	{
		psIterator->bInFreeLists = true;
		psIterator->pvLarge = NULL;

		pv = psIterator->pcFreeList->StartIteration(&psIterator->sFreeListIterator);
		if (pv == NULL)
		{
			return Iterate(psIterator);
		}
		else
		{
			sResult.Set((SDataMemoryAllocation*)pv);
			return sResult;
		}
	}
	else if (pvLargeHead != NULL)
	{
		psIterator->bInFreeLists = false;
		psIterator->pcFreeList = NULL;

		psIterator->pvLarge = pvLargeHead;
		sResult.Set((SDataMemoryAllocation*)pvLargeHead);
		return sResult;
	}
	else
	{
		return sResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SMemory CDataMemory::Iterate(SMemoryIterator* psIterator)
{
	void*		pv;
	SMemory		sResult;

	sResult.Init();
	if (psIterator->bInFreeLists)
	{
		pv = psIterator->pcFreeList->Iterate(&psIterator->sFreeListIterator);
		if (pv == NULL)
		{
			psIterator->pcFreeList = mcFreeLists.GetNext(psIterator->pcFreeList);
			if (psIterator->pcFreeList == NULL)
			{
				pv = mcLargeList.GetHead();
				if (pv != NULL)
				{
					psIterator->bInFreeLists = false;
					psIterator->pcFreeList = NULL;

					psIterator->pvLarge = pv;
					sResult.Set((SDataMemoryAllocation*)pv);
					return sResult;
				}
				else
				{
					return sResult;
				}
			}
			else
			{
				pv = psIterator->pcFreeList->StartIteration(&psIterator->sFreeListIterator);

				if (pv == NULL)
				{
					return Iterate(psIterator);
				}
				else
				{
					sResult.Set((SDataMemoryAllocation*)pv);
					return sResult;
				}
			}
		}
		else
		{
			sResult.Set((SDataMemoryAllocation*)pv);
			return sResult;
		}
	}
	else
	{
		pv = mcLargeList.GetNext(psIterator->pvLarge);
		if (pv)
		{
			psIterator->pvLarge = pv;
			sResult.Set((SDataMemoryAllocation*)pv);
			return sResult;
		}
		else
		{
			return sResult;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CDataMemory::GetTotalAllocatedMemory(void)
{
	SMemoryIterator		sIter;
	SMemory				sMem;
	uint64				ulliTotal;

	ulliTotal = 0;
	sMem = StartIteration(&sIter);
	while (sMem.bValid)
	{
		ulliTotal += sMem.uiSize;
		sMem = Iterate(&sIter);
	}

	return ulliTotal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CDataMemory::GetTotalAllocations(void)
{
	SMemoryIterator		sIter;
	SMemory				sMem;
	uint64				ulliTotal;

	ulliTotal = 0;
	sMem = StartIteration(&sIter);
	while (sMem.bValid)
	{
		ulliTotal++;
		sMem = Iterate(&sIter);
	}

	return ulliTotal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFreeList* CDataMemory::TestGetFreeListsHead(void) { return mcFreeLists.GetHead(); }
void* CDataMemory::TestGetLargeListsHead(void) { return mcLargeList.GetHead(); }
CMemoryFreeListParams* CDataMemory::GetFreeListParams(void) { return mpcFreeListParams; }
int	CDataMemory::NumFreeLists(void) { return mcFreeLists.NumElements(); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataMemoryInit(void)
{
	gcDataMemoryFreeListParams.Init(sizeof(SDataMemoryAllocation), true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataMemoryKill(void)
{
	gcDataMemoryFreeListParams.Kill();
}


