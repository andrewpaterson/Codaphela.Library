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
#include "LinkedListBlockAligned.h"
#include "IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlockAligned::Kill(void)
{
	SLLAlignedNode*		psNode;
	SLLAlignedNode*		psNode2;
	void*				pvData;

	pvData = HeaderGetData<SLLBlockNode, void>((SLLBlockNode*)mcList.GetHead());  //Yes this is the correct macro.
	psNode = DataGetHeader<SLLAlignedNode, void>(pvData);
	while (psNode)
	{
		pvData = HeaderGetData<SLLBlockNode, void>((SLLBlockNode*)psNode->sDNode.psNext);  //Yes this is the correct macro.
		psNode2 = DataGetHeader<SLLAlignedNode, void>(pvData);
		FreeNode(psNode);
		psNode = psNode2;
	}
	mcList.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlockAligned::FreeNode(SLLAlignedNode* psNode)
{
	Free(psNode->sAligned.pvAlloc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertAfterTail(unsigned int iSize, int iAlignment, int iOffset)
{
	SLLAlignedNode*	psNode;

	psNode = AllocateDetached(iSize, iAlignment, iOffset);
	return CBaseLinkedListBlock::InsertDetachedAfterTail(&psNode->sDNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertBeforeHead(int iSize, int iAlignment, int iOffset)
{
	SLLAlignedNode* psNode;

	psNode = AllocateDetached(iSize, iAlignment, iOffset);
	return CBaseLinkedListBlock::InsertDetachedBeforeHead(&psNode->sDNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertBeforeNode(void* psPos, int iSize, int iAlignment, int iOffset)
{
	SLLAlignedNode* psNode;
	SLLAlignedNode* psNodePos;

	psNode = AllocateDetached(iSize, iAlignment, iOffset);
	psNodePos = DataGetHeader<SLLAlignedNode, void>(psPos);
	return CBaseLinkedListBlock::InsertDetachedBeforeNode(&psNode->sDNode, &psNodePos->sDNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertAfterNode(void* psPos, int iSize, int iAlignment, int iOffset)
{
	SLLAlignedNode* psNode;
	SLLAlignedNode* psNodePos;

	psNode = AllocateDetached(iSize, iAlignment, iOffset);
	psNodePos = DataGetHeader<SLLAlignedNode, void>(psPos);
	return CBaseLinkedListBlock::InsertDetachedAfterNode(&psNode->sDNode, &psNodePos->sDNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLLAlignedNode* CLinkedListBlockAligned::AllocateDetached(int iDataSize, int iAlignment, int iOffset)
{
	void*				pvMem;
	int					iTotalSize;
	SLLAlignedNode*		psNode;

	iOffset = ::CalculateOffset(iOffset - sizeof(SLLAlignedNode), iAlignment);
	iTotalSize = iDataSize + sizeof(SLLAlignedNode) + iAlignment - 1;

	pvMem = Malloc(iTotalSize);
	if (pvMem != NULL)
	{
		psNode = CalculateActualStart(pvMem, iAlignment, iOffset);
		psNode->sAligned.iAlignment = iAlignment;
		psNode->sAligned.iOffset = iOffset;
		psNode->sAligned.iSize = iDataSize;
		psNode->sAligned.pvAlloc = pvMem;

		return psNode;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::Add(int iDataSize, int iAlignment)
{
	return InsertAfterTail(iDataSize, iAlignment, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkedListBlockAligned::ByteSize(void)
{
	int		iSize;
	void*	pvNode;

	iSize = 0;

	pvNode = GetHead();
	while(pvNode)
	{
		iSize += GetNodeSize(pvNode);
		pvNode = GetNext(pvNode);
	}

	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkedListBlockAligned::GetNodeSize(void* pvMem)
{
	SLLAlignedNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLAlignedNode, void>(pvMem);
	return psNodeHeader->sAligned.iSize + sizeof(SLLAlignedNode) + psNodeHeader->sAligned.iAlignment-1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLAlignedNode* CLinkedListBlockAligned::CalculateActualStart(void* pvMem, int iAlignment, int iOffset)
{
	size_t	iStart;
	size_t	iByteDiff;
	size_t	iActualOffset;

	iStart = (size_t)pvMem;

	iByteDiff = iStart % iAlignment;
	if (iByteDiff == 0)
	{
		return (SLLAlignedNode*)(size_t)(iStart + iOffset);
	}
	else
	{
		iActualOffset = (((size_t)iAlignment - iByteDiff) + iOffset) % (size_t)iAlignment;
		return (SLLAlignedNode*)(size_t)(iStart + iActualOffset);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLLAlignedNode* CLinkedListBlockAligned::GetNode(void* pvMem)
{
	return (SLLAlignedNode*)(size_t) ((unsigned int)(size_t) pvMem - sizeof(SLLAlignedNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlockAligned::Remove(void* pvData)
{
	Detach(pvData);
	FreeDetached(pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkedListBlockAligned::SafeRemove(void* pvData)
{
	if (IsInList(pvData))
	{
		Remove(pvData);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlockAligned::FreeDetached(void* pvData)
{
	SLLAlignedNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLAlignedNode, void>(pvData);
	if (psNodeHeader)
	{
		FreeNode(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::Grow(void* pvData, unsigned int uiNewSize)
{
	SLLAlignedNode*		psNodeHeader;
	void*				pvAllocatedEnd;
	void*				pvObjectEnd;
	void*				pvNew;
	unsigned int		uiSize;

	psNodeHeader = DataGetHeader<SLLAlignedNode, void>(pvData);

	if (uiNewSize == 0)
	{
		Remove(pvData);
		return NULL;
	}

	pvAllocatedEnd = (void*)(size_t) ((int)(size_t) psNodeHeader->sAligned.pvAlloc + sizeof(SLLAlignedNode) + psNodeHeader->sAligned.iSize + psNodeHeader->sAligned.iAlignment-1);
	pvObjectEnd = (void*)(size_t) ((int)(size_t) pvData + uiNewSize);

	if (pvAllocatedEnd >= pvObjectEnd)
	{
		psNodeHeader->sAligned.iSize = uiNewSize;
		return pvData;
	}
	else
	{
		pvNew = InsertAfterNode(pvData, uiNewSize, psNodeHeader->sAligned.iAlignment, psNodeHeader->sAligned.iOffset);
		uiSize = (uiNewSize < psNodeHeader->sAligned.iSize) ? uiNewSize : psNodeHeader->sAligned.iSize;
		memcpy(pvNew, pvData, uiSize);
		Remove(pvData);
		return pvNew;
	}
}

