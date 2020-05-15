/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "Log.h"
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
	CFreeList*					pcList;

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
unsigned int CDataMemory::GetSize(void* pv)
{
	SDataMemoryAllocation*	psAlloc;

	psAlloc = DATA_MEMORY_GET_ALLOCATION(pv);
	return psAlloc->uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDataMemory::Remove(CArrayVoidPtr* pav)
{
	int					i;
	void*				pv;
	SDataMemoryAllocation*	psAlloc;
	CFreeList*		pcList;
	SFNode*				psNode;
	int					iNumElements;
	int					iRemoved;

	pav->QuickSort();

	iNumElements = pav->NumElements();
	i = 0;
	while (i < iNumElements)
	{
		pv = pav->GetPtr(i);
		psAlloc = DATA_MEMORY_GET_ALLOCATION(pv);
		if (psAlloc->uiSize <= mpcFreeListParams->GetMaxFreeListElementSize())
		{
			psNode = psAlloc->psFreeListNode;
			pcList = psAlloc->psFreeListNode->pcList;

			iRemoved = RemoveNode(pav, i, psAlloc, psNode, pcList);
			if (iRemoved != 0)
			{
				i += iRemoved;
			}
			else
			{
				iRemoved = RemoveElements(pav, i, psNode, pcList);
				if (iRemoved != 0)
				{
					i += iRemoved;
				}
				else
				{
					gcLogger.Error2(__METHOD__, " Could not deallocate memory.", NULL);
					return FALSE;
				}
			}
		}
		else
		{
			DeallocateInLargeList(psAlloc);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDataMemory::RemoveNode(CArrayVoidPtr* pav, int i, SDataMemoryAllocation* psAlloc, SFNode* psNode, CFreeList* pcList)
{
	void*				pvLast;
	SDataMemoryAllocation*	psPotentialLast;
	int					iNodeElements;
	SDataMemoryAllocation*	psFirst;
	SDataMemoryAllocation*	psLast;
	int					iNumElements;

	iNumElements = pav->NumElements();
	psFirst = (SDataMemoryAllocation*)pcList->GetFirstNodeElement(psNode);
	if (psAlloc == psFirst)
	{
		psLast = (SDataMemoryAllocation*)pcList->GetLastNodeElement(psNode);

		if (psNode->bFull)
		{
			iNodeElements = psNode->uiChunkSize;
		}
		else
		{
			iNodeElements = pcList->NumNodeElements(psNode);
		}

		if (i + iNodeElements - 1 < iNumElements)
		{
			pvLast = pav->GetPtr(i + iNodeElements - 1);
			psPotentialLast = DATA_MEMORY_GET_ALLOCATION(pvLast);

			if (psPotentialLast == psLast)
			{
				pcList->RemoveNode(psNode);
				return iNodeElements;
			}
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDataMemory::RemoveElements(CArrayVoidPtr* pav, int i, SFNode* psNode, CFreeList* pcList)
{
	void*						pv;
	SDataMemoryAllocation*	psFirst;
	SDataMemoryAllocation*	psLast;
	int							iCount;
	SDataMemoryAllocation*	psAlloc;
	int							iNumElements;

	iNumElements = pav->NumElements();

	psFirst = (SDataMemoryAllocation*)pcList->GetFirstNodeElement(psNode);
	psLast = (SDataMemoryAllocation*)pcList->GetLastNodeElement(psNode);

	iCount = 0;
	pv = pav->GetPtr(i);
	psAlloc = DATA_MEMORY_GET_ALLOCATION(pv);

	while (psAlloc <= psLast)
	{
		pcList->Remove(psNode, psAlloc);

		i++;
		iCount++;
		if (i >= iNumElements)
		{
			break;
		}
		pv = pav->GetPtr(i);
		psAlloc = DATA_MEMORY_GET_ALLOCATION(pv);
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CDataMemory::Add(unsigned int uiSize)
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
void* CDataMemory::Grow(void* pvInitial, unsigned int uiSize)
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
		psNode = (SLLAlignedNode*)RemapSinglePointer(psAlloc, -((int)sizeof(SLLAlignedNode)));
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
void CDataMemory::CopyAllocation(void* pvDest, void* pvSource, unsigned int uiDestSize, unsigned int uiSourceSize)
{
	unsigned int	uiSize;

	uiSize = (uiDestSize < uiSourceSize) ? uiDestSize : uiSourceSize;
	memcpy(pvDest, pvSource, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CDataMemory::AllocateInFreeList(CFreeList* pcFreeList, unsigned int uiElementSize)
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
	int		iSize;
	void*	pvMem;

	iSize = sizeof(psAlloc) + psAlloc->uiSize - sizeof(unsigned int);
	pvMem = RemapSinglePointer(psAlloc, sizeof(unsigned int));
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
	BOOL			bResult;
	SFreeListDesc	sDesc;
	int				iIndex;
	int				iStride;

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
void* CDataMemory::AllocateInLargeList(unsigned int uiSize)
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
CFreeList* CDataMemory::GetFreeList(unsigned int iElementSize)
{
	SFreeListDesc			sDesc;
	BOOL					bResult;
	int						iIndex;
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
CFreeList* CDataMemory::GetOrAddFreeList(unsigned int iElementSize)
{
	SFreeListDesc			sDesc;
	BOOL					bResult;
	int						iIndex;
	SFreeListDesc*			psDesc;
	CFreeList*				pcList;
	SMemoryFreeListParams*	psParams;
	int						iFinalOffset;
	int						iStride;

	iFinalOffset = CalculateOffset(0 - ((char)sizeof(SDataMemoryAllocation)), DATA_MEMORY_ALIGNMENT);
	psParams = mpcFreeListParams->GetFreeListParamsForSize(iElementSize);
	if (psParams == NULL)
	{
		gcLogger.Error2(__METHOD__, " No free list parameter for element size [", IntToString(iElementSize), "].  Available parameter options [", IntToString(mpcFreeListParams->NumParams()), "] (should be more than 0)." , NULL);
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
	int					iCount;

	iCount = 0;
	pcBlock = mcFreeLists.GetHead();
	while (pcBlock)
	{
		iCount += pcBlock->NumElements();
		pcBlock = mcFreeLists.GetNext(pcBlock);
	}

	iCount += mcLargeList.NumElements();
	return iCount;
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
		psIterator->bInFreeLists = TRUE;
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
		psIterator->bInFreeLists = FALSE;
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
					psIterator->bInFreeLists = FALSE;
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
	gcDataMemoryFreeListParams.Init(sizeof(SDataMemoryAllocation), TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DataMemoryKill(void)
{
	gcDataMemoryFreeListParams.Kill();
}


