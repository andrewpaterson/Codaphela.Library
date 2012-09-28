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
	mcStats.Init();
 	mcOrder.Init(8);
	muiAdds = 0;
	muiBreakAlloc = 0xffffffff;

	if (bDefaultFreeListParams)
	{
		muiFreeListSizeLimit = MEMORY_FREE_LIST_SIZE_LIMIT;
		InitFreeListParams();
	}
	else
	{
		mcParams.Init(1);
		muiFreeListSizeLimit = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::Kill(void)
{
	CFreeListBlock*	pcFreeList;

	pcFreeList = mcFreeLists.GetHead();
	while (pcFreeList)
	{
		pcFreeList->Kill();
		pcFreeList = mcFreeLists.GetNext(pcFreeList);
	}
	mcFreeLists.Kill();

	mcLargeList.Kill();
	mcStats.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::InitFreeListParams(void)
{
	SFreeListParams		sParam;

	mcParams.Init(1);
	mcParams.Add(sParam.Init(16  , 8   , 32*32));
	mcParams.Add(sParam.Init(24  , 16  , 32*32));
	mcParams.Add(sParam.Init(32  , 24  , 28*32));
	mcParams.Add(sParam.Init(40  , 32  , 24*32));
	mcParams.Add(sParam.Init(48  , 40  , 22*32));
	mcParams.Add(sParam.Init(64  , 48  , 20*32));
	mcParams.Add(sParam.Init(80  , 64  , 18*32));
	mcParams.Add(sParam.Init(96  , 80  , 17*32));
	mcParams.Add(sParam.Init(112 , 96  , 16*32));
	mcParams.Add(sParam.Init(160 , 112 , 12*32));
	mcParams.Add(sParam.Init(192 , 160 , 10*32));
	mcParams.Add(sParam.Init(224 , 192 ,  9*32));
	mcParams.Add(sParam.Init(256 , 224 ,  8*32));
	mcParams.Add(sParam.Init(288 , 256 ,  7*32));
	mcParams.Add(sParam.Init(320 , 288 ,  6*32));
	mcParams.Add(sParam.Init(384 , 320 ,  5*32));
	mcParams.Add(sParam.Init(448 , 384 ,  4*32));
	mcParams.Add(sParam.Init(512 , 448 ,  4*32));
	mcParams.Add(sParam.Init(576 , 512 ,  3*32));
	mcParams.Add(sParam.Init(640 , 576 ,  3*32));
	mcParams.Add(sParam.Init(704 , 640 ,  2*32));
	mcParams.Add(sParam.Init(768 , 704 ,  2*32));
	mcParams.Add(sParam.Init(896 , 768 ,  2*32));
	mcParams.Add(sParam.Init(1024, 896 ,  2*32));
	mcParams.Add(sParam.Init(1152, 1024,    32));
	mcParams.Add(sParam.Init(1280, 1152,    32));
	mcParams.Add(sParam.Init(1408, 1280,    32));
	mcParams.Add(sParam.Init(1536, 1408,    32));
	mcParams.Add(sParam.Init(1664, 1536,    32));
	mcParams.Add(sParam.Init(1792, 1664,    16));
	mcParams.Add(sParam.Init(1920, 1792,    16));
	mcParams.Add(sParam.Init(muiFreeListSizeLimit, 1920,    16));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::AddParamBlock(unsigned int iFreeListSize, int iPrevSize, int iChunkSize)
{
	SFreeListParams		sParam;

	iFreeListSize += + sizeof(SMemoryAllocation);
	iPrevSize += + sizeof(SMemoryAllocation);

	mcParams.Add(sParam.Init(iFreeListSize, iPrevSize,    iChunkSize));
	if (muiFreeListSizeLimit < iFreeListSize)
	{
		muiFreeListSizeLimit = iFreeListSize;
	}
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
	CFreeListBlock*		pcList;

	psAlloc = MEMORY_GET_ALLOCATION(pv);
	if (psAlloc->uiSize <= (muiFreeListSizeLimit - sizeof(SMemoryAllocation)))
	{
		pcList = (CFreeListBlock*)psAlloc->psFreeListNode->pcList;

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
void CMemory::Remove(CArrayVoidPtr* pav)
{
	int					i;
	void*				pv;
	SMemoryAllocation*	psAlloc;
	CFreeListBlock*		pcList;
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
		if (psAlloc->uiSize <= (muiFreeListSizeLimit - sizeof(SMemoryAllocation)))
		{
			psNode = psAlloc->psFreeListNode;
			pcList = (CFreeListBlock*)psAlloc->psFreeListNode->pcList;
			iChunkSize = pcList->GetChunkSize();

			iRemoved = RemoveNode(pav, i, psAlloc, iChunkSize, psNode, pcList);
			if (iRemoved != 0)
			{
				i += iRemoved;
			}
			else
			{
				iRemoved = RemoveElements(pav, i, iChunkSize, psNode, pcList);
				if (iRemoved != 0)
				{
					i += iRemoved;
				}
				else
				{
					//WierdSpace.
					int xxx = 0;
				}
			}
		}
		else
		{
			DeallocateInLargeList(psAlloc);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMemory::RemoveNode(CArrayVoidPtr* pav, int i, SMemoryAllocation* psAlloc, int iChunkSize, SFNode* psNode, CFreeListBlock* pcList)
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
int CMemory::RemoveElements(CArrayVoidPtr* pav, int i, int iChunkSize, SFNode* psNode, CFreeListBlock* pcList)
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
	CFreeListBlock*	pcFreeList;
	void*				pv;

	if (muiAdds == muiBreakAlloc)
	{
		CChars	sz;

		sz.Init("CMemory::Add: muiAdds == ");
		sz.Append(muiBreakAlloc);
		sz.AppendNewLine();
		sz.Dump();
		sz.Kill();
	}

	if (uiSize == 0)
	{
		muiAdds++;
		return NULL;
	}

	if (uiSize <= (muiFreeListSizeLimit - sizeof(SMemoryAllocation)))
	{
		pcFreeList = GetOrAddFreeList(uiSize, iAlignment, iOffset);
		pv = AllocateInFreeList(pcFreeList, uiSize);
		muiAdds++;
		return pv;
	}
	else
	{
		pv = AllocateInLargeList(uiSize, iAlignment, iOffset);
		muiAdds++;
		return pv;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemory::Grow(void* pvInitial, unsigned int uiSize)
{
	SMemoryAllocation*	psAlloc;
	CFreeListBlock*	pcList;
	SFreeListParams*	psParams;
	void*				pvNew;
	SDANode*			psNode;

	psAlloc = MEMORY_GET_ALLOCATION(pvInitial);
	if (psAlloc->uiSize <= (muiFreeListSizeLimit - sizeof(SMemoryAllocation)))
	{
		psParams = GetParamsForSize(psAlloc->uiSize);
		if ((uiSize <= psParams->iMaxElementSize) && (uiSize >= psParams->iMinElementSize))
		{
			psAlloc->uiSize = uiSize;
			return pvInitial;
		}
		else
		{
			pcList = (CFreeListBlock*)psAlloc->psFreeListNode->pcList;
			pvNew = Add(uiSize, pcList->GetAlignment(), pcList->GetOffset());
			CopyAllocation(pvNew, pvInitial, uiSize, psAlloc->uiSize);
			DeallocateInFreeList(pcList, psAlloc);
			return pvNew;
		}
	}
	else
	{
		psNode = (SDANode*)RemapSinglePointer(psAlloc, -((int)sizeof(SDANode)));
		if (uiSize <= (muiFreeListSizeLimit - sizeof(SMemoryAllocation)))
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
	return NULL;
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
void* CMemory::AllocateInFreeList(CFreeListBlock* pcFreeList, unsigned int uiElementSize)
{
	SMemoryAllocation*	psMemoryAllocation;
	SFNode*				psNode;

	psMemoryAllocation = (SMemoryAllocation*)pcFreeList->Add(&psNode);
	psMemoryAllocation->uiSize = uiElementSize;
	psMemoryAllocation->psFreeListNode = psNode;
	psMemoryAllocation->uiAllocCount = muiAdds;
	psMemoryAllocation->szDebug[0] = psMemoryAllocation->szDebug[1] = psMemoryAllocation->szDebug[2]= psMemoryAllocation->szDebug[3] = -1;
	return RemapSinglePointer(psMemoryAllocation, sizeof(SMemoryAllocation));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemory::DeallocateInFreeList(CFreeListBlock* pcFreeList, SMemoryAllocation* psAlloc)
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
	psMemoryAllocation->uiAllocCount = muiAdds;
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
CFreeListBlock* CMemory::GetFreeList(unsigned int iElementSize, int iAlignment, int iOffset)
{
	SFreeListDesc		sDesc;
	BOOL				bResult;
	int					iIndex;
	SFreeListDesc*		psDesc;
	SFreeListParams*	psParams;
	int					iStride;
	int					iFinalOffset;

	iFinalOffset = CalculateOffset(iOffset - sizeof(SMemoryAllocation), iAlignment);
	psParams = GetParamsForSize(iElementSize);
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
CFreeListBlock* CMemory::GetFreeList(unsigned int iElementSize)
{
	return GetFreeList(iElementSize, miDefaultAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFreeListBlock* CMemory::GetOrAddFreeList(unsigned int iElementSize, int iAlignment, int iOffset)
{
	SFreeListDesc		sDesc;
	BOOL				bResult;
	int					iIndex;
	SFreeListDesc*		psDesc;
	CFreeListBlock*		pcList;
	SFreeListParams*	psParams;
	int					iFinalOffset;
	int					iStride;

	iFinalOffset = CalculateOffset(iOffset - sizeof(SMemoryAllocation), iAlignment);
	psParams = GetParamsForSize(iElementSize);
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
//
//
//////////////////////////////////////////////////////////////////////////
SFreeListParams* CMemory::GetParamsForSize(unsigned int iElementSize)
{
	int					iIndex;
	SFreeListParams*	psParams;

	if ((iElementSize > 0) && (iElementSize <= muiFreeListSizeLimit))
	{
		mcParams.FindInSorted((SFreeListParams*)&iElementSize, CompareFreeListParam, &iIndex);
		psParams = mcParams.Get(iIndex);
		return psParams;
	}
	return NULL;
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
	muiBreakAlloc = uiAllocCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CMemory::NumElements(void)
{
	CFreeListBlock*		pcBlock;
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
	CFreeListBlock*		pcFreeList;

	iSize = 0;
	pcFreeList = mcFreeLists.GetHead();
	while (pcFreeList)
	{
		iSize += pcFreeList->ByteSize();
		iSize += sizeof(CFreeListBlock);
		pcFreeList = mcFreeLists.GetNext(pcFreeList);
	}

	iSize += mcLargeList.ByteSize();
	iSize += sizeof(CLinkListAligned);

	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemory::StartIteration(SMemoryIterator* psIterator)
{
	void*	pv;

	psIterator->pcFreeList = mcFreeLists.GetHead();
	pv = psIterator->pcFreeList->StartIteration(&psIterator->sFreeListIterator);

	if (pv == NULL)
	{
		return Iterate(psIterator);
	}
	else
	{
		return RemapSinglePointer(pv, sizeof(SMemoryAllocation));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CMemory::Iterate(SMemoryIterator* psIterator)
{
	void*	pv;

	pv = psIterator->pcFreeList->Iterate(&psIterator->sFreeListIterator);
	if (pv == NULL)
	{
		psIterator->pcFreeList = mcFreeLists.GetNext(psIterator->pcFreeList);
		if (psIterator->pcFreeList == NULL)
		{
			return NULL;
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
				return pv;
			}
		}
	}
	else
	{
		return RemapSinglePointer(pv, sizeof(SMemoryAllocation));
	}
}




