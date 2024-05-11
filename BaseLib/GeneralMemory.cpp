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
#include "Chars.h"
#include "GeneralMemory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeneralMemory::Init(void)
{
	Init(4);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeneralMemory::Init(uint16 iDefaultAlignment, bool bDefaultFreeListParams)
{
	mcFreeLists.Init();
	mcLargeList.Init();
	miDefaultAlignment = (uint8)iDefaultAlignment;
 	mcOrder.Init();
	muiAllocCount = 0;
	muiBreakAlloc = 0;
	mbBreakOnAlloc = false;

	mcFreeListParams.Init(sizeof(SGeneralMemoryAllocation), bDefaultFreeListParams);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeneralMemory::Kill(void)
{
	CFreeList*	pcFreeList;

	pcFreeList = mcFreeLists.GetHead();
	while (pcFreeList)
	{
		pcFreeList->Kill();
		pcFreeList = mcFreeLists.GetNext(pcFreeList);
	}
	mcOrder.Kill();
	mcFreeLists.Kill();

	mcLargeList.Kill();
	 
	mcFreeListParams.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGeneralMemory::Add(size iSize)
{
	return Add(iSize, miDefaultAlignment);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGeneralMemory::Remove(void* pv)
{
	SGeneralMemoryAllocation*	psAlloc;
	CFreeList*					pcList;

	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv);
	if (psAlloc->uiSize <= mcFreeListParams.GetMaxFreeListElementSize())
	{
		pcList = psAlloc->psFreeListNode->pcList;

		return DeallocateInFreeList(pcList, psAlloc);
	}
	else
	{
		return DeallocateInLargeList(psAlloc);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGeneralMemory::GetSize(void* pv)
{
	SGeneralMemoryAllocation*	psAlloc;

	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv);
	return psAlloc->uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGeneralMemory::RemoveMultiple(CArrayVoidPtr* pav)
{
	size							i;
	void*						pv;
	SGeneralMemoryAllocation*	psAlloc;
	CFreeList*					pcList;
	SFNode*						psNode;
	size							iNumElements;
	size							iRemoved;
	bool						bResult;
	
	pav->QuickSort();

	iNumElements = pav->NumElements();
	i = 0;
	while (i < iNumElements)
	{
		pv = pav->GetPtr(i);
		psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv);

#ifdef _DEBUG
		memset(pv, 0xcc, psAlloc->uiSize);
#endif // _DEBUG


		if (psAlloc->uiSize <= mcFreeListParams.GetMaxFreeListElementSize())
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
					return i;
				}
			}
		}
		else
		{
			bResult = DeallocateInLargeList(psAlloc);
			if (bResult)
			{
				i++;
			}
			else
			{
				gcLogger.Error2(__METHOD__, " Could not deallocate memory.", NULL);
				return i;
			}
		}
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGeneralMemory::RemoveNode(CArrayVoidPtr* pav, size i, SGeneralMemoryAllocation* psAlloc, SFNode* psNode, CFreeList* pcList)
{
	void*						pvLast;
	SGeneralMemoryAllocation*	psPotentialLast;
	size							iNodeElements;
	SGeneralMemoryAllocation*	psFirst;
	SGeneralMemoryAllocation*	psLast;
	size							iNumElements;

	iNumElements = pav->NumElements();
	psFirst = (SGeneralMemoryAllocation*)pcList->GetFirstNodeElement(psNode);
	if (psAlloc == psFirst)
	{
		psLast = (SGeneralMemoryAllocation*)pcList->GetLastNodeElement(psNode);

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
			psPotentialLast = GENERAL_MEMORY_GET_ALLOCATION(pvLast);

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
size CGeneralMemory::RemoveElements(CArrayVoidPtr* pav, size i, SFNode* psNode, CFreeList* pcList)
{
	void*						pv;
	SGeneralMemoryAllocation*	psFirst;
	SGeneralMemoryAllocation*	psLast;
	size						iCount;
	SGeneralMemoryAllocation*	psAlloc;
	size						iNumElements;

	iNumElements = pav->NumElements();

	psFirst = (SGeneralMemoryAllocation*)pcList->GetFirstNodeElement(psNode);
	psLast = (SGeneralMemoryAllocation*)pcList->GetLastNodeElement(psNode);

	iCount = 0;
	pv = pav->GetPtr(i);
	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv);

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
		psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv);
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGeneralMemory::Add(size uiSize, uint16 iAlignment, int16 iOffset)
{
	CFreeList*	pcFreeList;
	void*		pv;

	if ((mbBreakOnAlloc) && (muiAllocCount == muiBreakAlloc))
	{
		CChars	sz;

		sz.Init("CGeneralMemory::Add: muiAllocCount == ");
		sz.Append(muiBreakAlloc);
		sz.AppendNewLine();
		sz.Dump();
		sz.Kill();

		BREAK();
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
void* CGeneralMemory::Grow(void* pvInitial, size uiSize)
{
	SGeneralMemoryAllocation*	psAlloc;
	CFreeList*					pcList;
	SMemoryFreeListParams*		psParams;
	void*						pvNew;
	SLLAlignedNode*				psNode;

	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pvInitial);
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
			pcList = psAlloc->psFreeListNode->pcList;
			pvNew = Add(uiSize, pcList->GetAlignment(), pcList->GetOffset());
			CopyAllocation(pvNew, pvInitial, uiSize, psAlloc->uiSize);
			DeallocateInFreeList(pcList, psAlloc);
			return pvNew;
		}
	}
	else
	{
		psNode = (SLLAlignedNode*)RemapSinglePointer(psAlloc, -(ptrdiff_t)sizeof(SLLAlignedNode));
		if (uiSize <= mcFreeListParams.GetMaxFreeListElementSize())
		{
			pvNew = Add(uiSize, psNode->sAligned.iAlignment, psNode->sAligned.iOffset);
			CopyAllocation(pvNew, pvInitial, uiSize, psAlloc->uiSize);
			DeallocateInLargeList(psAlloc);
			return pvNew;
		}
		else
		{
			psAlloc = (SGeneralMemoryAllocation*)mcLargeList.Grow(psAlloc, uiSize);
			psAlloc->uiSize = uiSize;
			psAlloc->psFreeListNode = NULL;
			pvNew = RemapSinglePointer(psAlloc, sizeof(SGeneralMemoryAllocation));
			return pvNew;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeneralMemory::CopyAllocation(void* pvDest, void* pvSource, size uiDestSize, size uiSourceSize)
{
	size	uiSize;

	uiSize = (uiDestSize < uiSourceSize) ? uiDestSize : uiSourceSize;
	memcpy(pvDest, pvSource, uiSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CGeneralMemory::AllocateInFreeList(CFreeList* pcFreeList, size uiElementSize)
{
	SGeneralMemoryAllocation*	psMemoryAllocation;
	SFNode*						psNode;

	psMemoryAllocation = (SGeneralMemoryAllocation*)pcFreeList->Add(&psNode);
	psMemoryAllocation->uiSize = uiElementSize;
	psMemoryAllocation->psFreeListNode = psNode;
	psMemoryAllocation->uiAllocCount = muiAllocCount;
	psMemoryAllocation->szDebug[0] = psMemoryAllocation->szDebug[1] = psMemoryAllocation->szDebug[2]= psMemoryAllocation->szDebug[3] = -1;
	return RemapSinglePointer(psMemoryAllocation, sizeof(SGeneralMemoryAllocation));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGeneralMemory::DeallocateInFreeList(CFreeList* pcFreeList, SGeneralMemoryAllocation* psAlloc)
{
	SFNode*		psFreeListNode;
	bool		bFreed;

	if (!psAlloc)
	{
		return false;
	}

	psFreeListNode = psAlloc->psFreeListNode;

#ifdef _DEBUG
	size		iSize;
	void*	pvMem;

	iSize = sizeof(psAlloc) + psAlloc->uiSize - sizeof(size);
	pvMem = RemapSinglePointer(psAlloc, sizeof(size));
	memset(pvMem, 0xef, iSize);
#endif

	bFreed = pcFreeList->Remove(psFreeListNode, psAlloc);
	if (!pcFreeList->HasElements())
	{
		FreeFreeList(pcFreeList);
	}
	return bFreed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeneralMemory::FreeFreeList(CFreeList* pcFreeList)
{
	bool					bResult;
	SAlignedFreeListDesc	sDesc;
	size					iIndex;
	size					iStride;

	iStride = pcFreeList->GetElementStride() - sizeof(SGeneralMemoryAllocation);
	sDesc.Init(iStride, pcFreeList->GetAlignment(), pcFreeList->GetOffset());
	bResult = mcOrder.FindInSorted(&sDesc, CompareAlignedFreeListDesc, &iIndex);
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
void* CGeneralMemory::AllocateInLargeList(size uiSize, uint16 iAlignment, int16 iOffset)
{
	SGeneralMemoryAllocation*	psMemoryAllocation;

	psMemoryAllocation = (SGeneralMemoryAllocation*)mcLargeList.InsertAfterTail(uiSize + sizeof(SGeneralMemoryAllocation), iAlignment, iOffset);
	psMemoryAllocation->uiSize = uiSize;
	psMemoryAllocation->psFreeListNode = NULL;
	psMemoryAllocation->uiAllocCount = muiAllocCount;
	psMemoryAllocation->szDebug[0] = psMemoryAllocation->szDebug[1] = psMemoryAllocation->szDebug[2]= psMemoryAllocation->szDebug[3] = -1;
	return RemapSinglePointer(psMemoryAllocation, sizeof(SGeneralMemoryAllocation));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGeneralMemory::DeallocateInLargeList(SGeneralMemoryAllocation* psAlloc)
{
	if (psAlloc)
	{
		mcLargeList.Remove(psAlloc);
		return true;
	}
	else
	{
		return false;
	}

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFreeList* CGeneralMemory::GetFreeList(size iElementSize, uint16 iAlignment, int16 iOffset)
{
	SAlignedFreeListDesc	sDesc;
	bool					bResult;
	size					iIndex;
	SAlignedFreeListDesc*	psDesc;
	SMemoryFreeListParams*	psParams;
	size					iStride;

	psParams = mcFreeListParams.GetFreeListParamsForSize(iElementSize);
	iStride = CalculateStride(psParams->iMaxElementSize, iAlignment);

	sDesc.Init(iStride, iAlignment, iOffset);
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
CFreeList* CGeneralMemory::GetFreeList(size iElementSize)
{
	return GetFreeList(iElementSize, miDefaultAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFreeList* CGeneralMemory::GetOrAddFreeList(size iElementSize, uint16 iAlignment, int16 iOffset)
{
	SAlignedFreeListDesc	sDesc;
	bool					bResult;
	size					iIndex;
	SAlignedFreeListDesc*	psDesc;
	CFreeList*				pcList;
	SMemoryFreeListParams*	psParams;
	size					iFinalOffset;
	size					iStride;

	if ((iOffset < -128) || (iOffset > 127))
	{
		return NULL;
	}
	if (iAlignment > 255)
	{
		return NULL;
	}

	iFinalOffset = CalculateOffset(iOffset - sizeof(SGeneralMemoryAllocation), iAlignment);
	psParams = mcFreeListParams.GetFreeListParamsForSize(iElementSize);
	iStride = CalculateStride(psParams->iMaxElementSize, iAlignment);

	sDesc.Init(iStride, iAlignment, iOffset);
	bResult = mcOrder.FindInSorted(&sDesc, CompareAlignedFreeListDesc, &iIndex);
	if (bResult)
	{
		psDesc = mcOrder.Get(iIndex);
		return psDesc->pcFreeList;
	}
	else
	{
		pcList = mcFreeLists.InsertAfterTail();
		pcList->Init(psParams->iMaxListSize, (uint8)iAlignment);
		sDesc.pcFreeList = pcList;
		mcOrder.InsertAt(&sDesc, iIndex);
		return pcList;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CGeneralMemory::SetDebugName(void* pv, char (*pszDebug)[4])
{
	SGeneralMemoryAllocation*	psAlloc;

	psAlloc = GENERAL_MEMORY_GET_ALLOCATION(pv);
	psAlloc->szDebug[0] = (*pszDebug)[0];
	psAlloc->szDebug[1] = (*pszDebug)[1];
	psAlloc->szDebug[2] = (*pszDebug)[2];
	psAlloc->szDebug[3] = (*pszDebug)[3];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeneralMemory::BreakOnAdd(size uiAllocCount)
{
	mbBreakOnAlloc = true;
	muiBreakAlloc = uiAllocCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CGeneralMemory::NumElements(void)
{
	CFreeList*	pcBlock;
	size			iCount;

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
size CGeneralMemory::ByteSize(void)
{
	size			iSize;
	CFreeList*	pcFreeList;

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
SMemory CGeneralMemory::StartIteration(SMemoryIterator* psIterator)
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
			sResult.Set((SGeneralMemoryAllocation*)pv);
			return sResult;
		}
	}
	else if (pvLargeHead != NULL)
	{
		psIterator->bInFreeLists = false;
		psIterator->pcFreeList = NULL;

		psIterator->pvLarge = pvLargeHead;
		sResult.Set((SGeneralMemoryAllocation*)pvLargeHead);
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
SMemory CGeneralMemory::Iterate(SMemoryIterator* psIterator)
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
					sResult.Set((SGeneralMemoryAllocation*)pv);
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
					sResult.Set((SGeneralMemoryAllocation*)pv);
					return sResult;
				}
			}
		}
		else
		{
			sResult.Set((SGeneralMemoryAllocation*)pv);
			return sResult;
		}
	}
	else
	{
		pv = mcLargeList.GetNext(psIterator->pvLarge);
		if (pv)
		{
			psIterator->pvLarge = pv;
			sResult.Set((SGeneralMemoryAllocation*)pv);
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
uint64 CGeneralMemory::GetTotalAllocatedMemory(void)
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
uint64 CGeneralMemory::GetTotalAllocations(void)
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
CFreeList* CGeneralMemory::TestGetFreeListsHead(void) { return mcFreeLists.GetHead(); }
void* CGeneralMemory::TestGetLargeListsHead(void) { return mcLargeList.GetHead(); }
CMemoryFreeListParams* CGeneralMemory::GetFreeListParams(void) { return &mcFreeListParams; }
size CGeneralMemory::GetDefaultAlignment(void) { return miDefaultAlignment; }
size CGeneralMemory::NumFreeLists(void) { return mcFreeLists.NumElements(); }

