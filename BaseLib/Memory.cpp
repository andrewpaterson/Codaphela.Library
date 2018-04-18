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
#include "Memory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::Init(void)
{
	Init(4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::Init(int iDefaultAlignment, BOOL bDefaultFreeListParams)
{
	mcFreeLists.Init();
	mcLargeList.Init();
	miDefaultAlignment = iDefaultAlignment;
 	mcOrder.Init(8);
	muiAllocCount = 0;
	muiBreakAlloc = 0;
	mbBreakOnAlloc = FALSE;

	mcFreeListParams.Init(sizeof(SMemoryAllocation), bDefaultFreeListParams);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::Kill(void)
{
	CFreeList*	pcFreeList;

	pcFreeList = mcFreeLists.GetHead();
	while (pcFreeList)
	{
		pcFreeList->Kill();
		pcFreeList = mcFreeLists.GetNext(pcFreeList);
	}
	mcFreeLists.Kill();

	mcLargeList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemory::Add(unsigned int iSize)
{
	return Add(iSize, miDefaultAlignment);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::Remove(void* pv)
{
	SMemoryAllocation*	psAlloc;
	CFreeList*		pcList;

	psAlloc = MEMORY_GET_ALLOCATION(pv);
	if (psAlloc->uiSize <= mcFreeListParams.GetMaxFreeListElementSize())
	{
		pcList = (CFreeList*)psAlloc->psFreeListNode->pcList;

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
unsigned int CMemory::GetSize(void* pv)
{
	SMemoryAllocation*	psAlloc;

	psAlloc = MEMORY_GET_ALLOCATION(pv);
	return psAlloc->uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMemory::Remove(CArrayVoidPtr* pav)
{
	int					i;
	void*				pv;
	SMemoryAllocation*	psAlloc;
	CFreeList*		pcList;
	SFNode*				psNode;
	int					iNumElements;
	int					iChunkSize;
	int					iRemoved;
	
	pav->QuickSort();

	iNumElements = pav->NumElements();
	i = 0;
	while (i < iNumElements)
	{
		pv = pav->GetPtr(i);
		psAlloc = MEMORY_GET_ALLOCATION(pv);
		if (psAlloc->uiSize <= mcFreeListParams.GetMaxFreeListElementSize())
		{
			psNode = psAlloc->psFreeListNode;
			pcList = (CFreeList*)psAlloc->psFreeListNode->pcList;
			iChunkSize = pcList->GetChunkSize();

			iRemoved = RemoveNode(pav, i, psAlloc, iChunkSize, psNode, pcList);
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
int CMemory::RemoveNode(CArrayVoidPtr* pav, int i, SMemoryAllocation* psAlloc, int iChunkSize, SFNode* psNode, CFreeList* pcList)
{
	void*				pvLast;
	SMemoryAllocation*	psPotentialLast;
	int					iNodeElements;
	SMemoryAllocation*	psFirst;
	SMemoryAllocation*	psLast;
	int					iNumElements;

	iNumElements = pav->NumElements();
	psFirst = (SMemoryAllocation*)pcList->GetFirstNodeElement(psNode);
	if (psAlloc == psFirst)
	{
		psLast = (SMemoryAllocation*)pcList->GetLastNodeElement(psNode);

		if (psNode->bFull)
		{
			iNodeElements = iChunkSize;
		}
		else
		{
			iNodeElements = pcList->NumNodeElements(psNode);
		}

		if (i + iNodeElements - 1 < iNumElements)
		{
			pvLast = pav->GetPtr(i + iNodeElements - 1);
			psPotentialLast = MEMORY_GET_ALLOCATION(pvLast);

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
int CMemory::RemoveElements(CArrayVoidPtr* pav, int i, SFNode* psNode, CFreeList* pcList)
{
	void*				pv;
	SMemoryAllocation*	psFirst;
	SMemoryAllocation*	psLast;
	int					iCount;
	SMemoryAllocation*	psAlloc;
	int					iNumElements;

	iNumElements = pav->NumElements();

	psFirst = (SMemoryAllocation*)pcList->GetFirstNodeElement(psNode);
	psLast = (SMemoryAllocation*)pcList->GetLastNodeElement(psNode);

	iCount = 0;
	pv = pav->GetPtr(i);
	psAlloc = MEMORY_GET_ALLOCATION(pv);

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
		psAlloc = MEMORY_GET_ALLOCATION(pv);
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemory::Add(unsigned int uiSize, int iAlignment, int iOffset)
{
	CFreeList*	pcFreeList;
	void*				pv;

	if ((mbBreakOnAlloc) && (muiAllocCount == muiBreakAlloc))
	{
		CChars	sz;

		sz.Init("CMemory::Add: muiAllocCount == ");
		sz.Append(muiBreakAlloc);
		sz.AppendNewLine();
		sz.Dump();
		sz.Kill();

		Break();
	}

	if (uiSize == 0)
	{
		muiAllocCount++;
		return NULL;
	}

	if (uiSize <= mcFreeListParams.GetMaxFreeListElementSize())
	{
		pcFreeList = GetOrAddFreeList(uiSize, iAlignment, iOffset);
		pv = AllocateInFreeList(pcFreeList, uiSize);
		muiAllocCount++;
		return pv;
	}
	else
	{
		pv = AllocateInLargeList(uiSize, iAlignment, iOffset);
		muiAllocCount++;
		return pv;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemory::Grow(void* pvInitial, unsigned int uiSize)
{
	SMemoryAllocation*		psAlloc;
	CFreeList*				pcList;
	SMemoryFreeListParams*	psParams;
	void*					pvNew;
	SLLANode*				psNode;

	psAlloc = MEMORY_GET_ALLOCATION(pvInitial);
	if (psAlloc->uiSize <= mcFreeListParams.GetMaxFreeListElementSize())
	{
		psParams = mcFreeListParams.GetFreeListParamsForSize(psAlloc->uiSize);
		if ((uiSize <= psParams->iMaxElementSize) && (uiSize >= psParams->iMinElementSize))
		{
			psAlloc->uiSize = uiSize;
			return pvInitial;
		}
		else
		{
			pcList = (CFreeList*)psAlloc->psFreeListNode->pcList;
			pvNew = Add(uiSize, pcList->GetAlignment(), pcList->GetOffset());
			CopyAllocation(pvNew, pvInitial, uiSize, psAlloc->uiSize);
			DeallocateInFreeList(pcList, psAlloc);
			return pvNew;
		}
	}
	else
	{
		psNode = (SLLANode*)RemapSinglePointer(psAlloc, -((int)sizeof(SLLANode)));
		if (uiSize <= mcFreeListParams.GetMaxFreeListElementSize())
		{
			pvNew = Add(uiSize, psNode->sAligned.iAlignment, psNode->sAligned.iOffset);
			CopyAllocation(pvNew, pvInitial, uiSize, psAlloc->uiSize);
			DeallocateInLargeList(psAlloc);
			return pvNew;
		}
		else
		{
			psAlloc = (SMemoryAllocation*)mcLargeList.Grow(psAlloc, uiSize);
			psAlloc->uiSize = uiSize;
			psAlloc->psFreeListNode = NULL;
			pvNew = RemapSinglePointer(psAlloc, sizeof(SMemoryAllocation));
			return pvNew;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::CopyAllocation(void* pvDest, void* pvSource, unsigned int uiDestSize, unsigned int uiSourceSize)
{
	unsigned int	uiSize;

	uiSize = (uiDestSize < uiSourceSize) ? uiDestSize : uiSourceSize;
	memcpy(pvDest, pvSource, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemory::AllocateInFreeList(CFreeList* pcFreeList, unsigned int uiElementSize)
{
	SMemoryAllocation*	psMemoryAllocation;
	SFNode*				psNode;

	psMemoryAllocation = (SMemoryAllocation*)pcFreeList->Add(&psNode);
	psMemoryAllocation->uiSize = uiElementSize;
	psMemoryAllocation->psFreeListNode = psNode;
	psMemoryAllocation->uiAllocCount = muiAllocCount;
	psMemoryAllocation->szDebug[0] = psMemoryAllocation->szDebug[1] = psMemoryAllocation->szDebug[2]= psMemoryAllocation->szDebug[3] = -1;
	return RemapSinglePointer(psMemoryAllocation, sizeof(SMemoryAllocation));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::DeallocateInFreeList(CFreeList* pcFreeList, SMemoryAllocation* psAlloc)
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemory::AllocateInLargeList(unsigned int uiSize, int iAlignment, int iOffset)
{
	SMemoryAllocation*	psMemoryAllocation;

	psMemoryAllocation = (SMemoryAllocation*)mcLargeList.InsertAfterTail(uiSize + sizeof(SMemoryAllocation), iAlignment, iOffset);
	psMemoryAllocation->uiSize = uiSize;
	psMemoryAllocation->psFreeListNode = NULL;
	psMemoryAllocation->uiAllocCount = muiAllocCount;
	psMemoryAllocation->szDebug[0] = psMemoryAllocation->szDebug[1] = psMemoryAllocation->szDebug[2]= psMemoryAllocation->szDebug[3] = -1;
	return RemapSinglePointer(psMemoryAllocation, sizeof(SMemoryAllocation));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::DeallocateInLargeList(SMemoryAllocation* psAlloc)
{
	mcLargeList.Remove(psAlloc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFreeList* CMemory::GetFreeList(unsigned int iElementSize, int iAlignment, int iOffset)
{
	SFreeListDesc			sDesc;
	BOOL					bResult;
	int						iIndex;
	SFreeListDesc*			psDesc;
	SMemoryFreeListParams*	psParams;
	int						iStride;
	int						iFinalOffset;

	iFinalOffset = CalculateOffset(iOffset - sizeof(SMemoryAllocation), iAlignment);
	psParams = mcFreeListParams.GetFreeListParamsForSize(iElementSize);
	iStride = CalculateStride(psParams->iMaxElementSize, iAlignment);

	sDesc.Init(iStride, iAlignment, iOffset);
	bResult = mcOrder.FindInSorted(&sDesc, CompareFreeListDesc, &iIndex);
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
CFreeList* CMemory::GetFreeList(unsigned int iElementSize)
{
	return GetFreeList(iElementSize, miDefaultAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFreeList* CMemory::GetOrAddFreeList(unsigned int iElementSize, int iAlignment, int iOffset)
{
	SFreeListDesc			sDesc;
	BOOL					bResult;
	int						iIndex;
	SFreeListDesc*			psDesc;
	CFreeList*				pcList;
	SMemoryFreeListParams*	psParams;
	int						iFinalOffset;
	int						iStride;

	iFinalOffset = CalculateOffset(iOffset - sizeof(SMemoryAllocation), iAlignment);
	psParams = mcFreeListParams.GetFreeListParamsForSize(iElementSize);
	iStride = CalculateStride(psParams->iMaxElementSize, iAlignment);

	sDesc.Init(iStride, iAlignment, iOffset);
	bResult = mcOrder.FindInSorted(&sDesc, CompareFreeListDesc, &iIndex);
	if (bResult)
	{
		psDesc = mcOrder.Get(iIndex);
		return psDesc->pcFreeList;
	}
	else
	{
		pcList = mcFreeLists.InsertAfterTail();
		pcList->Init(psParams->iChunkSize, psParams->iMaxListSize, iAlignment);
		sDesc.pcFreeList = pcList;
		mcOrder.InsertAt(&sDesc, iIndex);
		return pcList;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemory::SetDebugName(void* pv, char (*pszDebug)[4])
{
	SMemoryAllocation*	psAlloc;

	psAlloc = MEMORY_GET_ALLOCATION(pv);
	psAlloc->szDebug[0] = (*pszDebug)[0];
	psAlloc->szDebug[1] = (*pszDebug)[1];
	psAlloc->szDebug[2] = (*pszDebug)[2];
	psAlloc->szDebug[3] = (*pszDebug)[3];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::BreakOnAdd(unsigned int uiAllocCount)
{
	mbBreakOnAlloc = TRUE;
	muiBreakAlloc = uiAllocCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CMemory::NumElements(void)
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
int CMemory::ByteSize(void)
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
SMemory CMemory::StartIteration(SMemoryIterator* psIterator)
{
	void*		pv;
	SMemory		sResult;

	sResult.Init();

	psIterator->pcFreeList = mcFreeLists.GetHead();
	if (psIterator->pcFreeList != NULL)
	{
		pv = psIterator->pcFreeList->StartIteration(&psIterator->sFreeListIterator);
		if (pv == NULL)
		{
			return Iterate(psIterator);
		}
		else
		{
			sResult.Set((SMemoryAllocation*)pv);
			return sResult;
		}
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
SMemory CMemory::Iterate(SMemoryIterator* psIterator)
{
	void*		pv;
	SMemory		sResult;

	sResult.Init();
	pv = psIterator->pcFreeList->Iterate(&psIterator->sFreeListIterator);
	if (pv == NULL)
	{
		psIterator->pcFreeList = mcFreeLists.GetNext(psIterator->pcFreeList);
		if (psIterator->pcFreeList == NULL)
		{
			return sResult;
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
				sResult.Set((SMemoryAllocation*)pv);
				return sResult;
			}
		}
	}
	else
	{
		sResult.Set((SMemoryAllocation*)pv);
		return sResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint64 CMemory::GetTotalAllocatedMemory(void)
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
uint64 CMemory::GetTotalAllocations(void)
{
	SMemoryIterator		sIter;
	SMemory				sMem;
	uint64				ulliTotal;

	ulliTotal = 0;
	sMem = StartIteration(&sIter);
	while (sMem.bValid)
	{
		ulliTotal ++;
		sMem = Iterate(&sIter);
	}

	return ulliTotal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFreeList* CMemory::TestGetFreeListsHead(void) { return mcFreeLists.GetHead(); }
void* CMemory::TestGetLargeListsHead(void) { return mcLargeList.GetHead(); }
CMemoryFreeListParams* CMemory::GetFreeListParams(void) { return &mcFreeListParams; }
int CMemory::GetDefaultAlignment(void) { return miDefaultAlignment; }
int	CMemory::NumFreeLists(void) { return mcFreeLists.NumElements(); }