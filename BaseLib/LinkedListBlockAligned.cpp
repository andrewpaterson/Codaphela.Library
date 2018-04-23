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
	SLLANode*	psNode;
	SLLANode*	psNode2;
	void*		pvData;

	pvData = HeaderGetData<SLLNode, void>(mpsHead);  //Yes this is the correct macro.
	psNode = DataGetHeader<SLLANode, void>(pvData);
	while (psNode)
	{
		pvData = HeaderGetData<SLLNode, void>(psNode->sDNode.psNext);  //Yes this is the correct macro.
		psNode2 = DataGetHeader<SLLANode, void>(pvData);
		FreeNode(psNode);
		psNode = psNode2;
	}
	mpsHead = NULL;
	mpsTail = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkedListBlockAligned::FreeNode(SLLANode* psNode)
{
	Free(psNode->sAligned.pvAlloc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertAfterTail(unsigned int iSize, int iAlignment, int iOffset)
{
	void*			pvData;

	pvData = AllocateDetached(iSize, iAlignment, iOffset);
	InsertDetachedAfterTail(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertBeforeHead(int iSize, int iAlignment, int iOffset)
{
	void*			pvData;

	pvData = AllocateDetached(iSize, iAlignment, iOffset);
	InsertDetachedBeforeHead(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertBeforeNode(void* psPos, int iSize, int iAlignment, int iOffset)
{
	void*			pvData;

	pvData = AllocateDetached(iSize, iAlignment, iOffset);
	InsertDetachedBeforeNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::InsertAfterNode(void* psPos, int iSize, int iAlignment, int iOffset)
{
	void*			pvData;

	pvData = AllocateDetached(iSize, iAlignment, iOffset);
	InsertDetachedAfterNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkedListBlockAligned::AllocateDetached(int iDataSize, int iAlignment, int iOffset)
{
	void*			pvMem;
	int				iTotalSize;
	SLLANode*		psNode;

	iOffset = ::CalculateOffset(iOffset - sizeof(SLLANode), iAlignment);
	iTotalSize = iDataSize + sizeof(SLLANode) + iAlignment - 1;

	pvMem = Malloc(iTotalSize);
	if (pvMem != NULL)
	{
		psNode = CalculateActualStart(pvMem, iAlignment, iOffset);
		psNode->sAligned.iAlignment = iAlignment;
		psNode->sAligned.iOffset = iOffset;
		psNode->sAligned.iSize = iDataSize;
		psNode->sAligned.pvAlloc = pvMem;

		return HeaderGetData<SLLANode, void>(psNode);
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
	SLLANode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLANode, void>(pvMem);
	return psNodeHeader->sAligned.iSize + sizeof(SLLANode) + psNodeHeader->sAligned.iAlignment-1;;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SLLANode* CLinkedListBlockAligned::CalculateActualStart(void* pvMem, int iAlignment, int iOffset)
{
	size_t	iStart;
	size_t	iByteDiff;
	size_t	iActualOffset;

	iStart = (size_t)pvMem;

	iByteDiff = iStart % iAlignment;
	if (iByteDiff == 0)
	{
		return (SLLANode*)(size_t)(iStart + iOffset);
	}
	else
	{
		iActualOffset = (((size_t)iAlignment - iByteDiff) + iOffset) % (size_t)iAlignment;
		return (SLLANode*)(size_t)(iStart + iActualOffset);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SLLANode* CLinkedListBlockAligned::GetNode(void* pvMem)
{
	return (SLLANode*)(size_t) ((unsigned int)(size_t) pvMem - sizeof(SLLANode));
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
	SLLANode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SLLANode, void>(pvData);
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
	SLLANode*		psNodeHeader;
	void*			pvAllocatedEnd;
	void*			pvObjectEnd;
	void*			pvNew;
	unsigned int	uiSize;

	psNodeHeader = DataGetHeader<SLLANode, void>(pvData);

	if (uiNewSize == 0)
	{
		Remove(pvData);
		return NULL;
	}

	pvAllocatedEnd = (void*)(size_t) ((int)(size_t) psNodeHeader->sAligned.pvAlloc + sizeof(SLLANode) + psNodeHeader->sAligned.iSize + psNodeHeader->sAligned.iAlignment-1);
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

