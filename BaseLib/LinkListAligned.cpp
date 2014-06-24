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
#include "LinkListAligned.h"
#include "IntegerHelper.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkListAligned::Init(void)
{
	mpsHead = NULL;
	mpsTail = NULL;
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkListAligned::Kill(void)
{
	SDANode*	psNode;
	SDANode*	psNode2;
	void*		pvData;

	pvData = HeaderGetDataMacro<SDNode, void>(mpsHead);  //Yes this is the correct macro.
	psNode = CLinkListAlignedDataGetHeader(pvData);
	while (psNode)
	{
		pvData = HeaderGetDataMacro<SDNode, void>(psNode->sDNode.psNext);  //Yes this is the correct macro.
		psNode2 = CLinkListAlignedDataGetHeader(pvData);
		FreeNode(psNode);
		psNode = psNode2;
	}
	mpsHead = NULL;
	mpsTail = NULL;
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkListAligned::FreeNode(SDANode* psNode)
{
	Free(psNode->sAligned.pvAlloc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkListAligned::InsertAfterTail(unsigned int iSize, int iAlignment, int iOffset)
{
	void*			pvData;

	pvData = AllocateDetached(iSize, iAlignment, iOffset);
	__CLinkListTemplate<void>::InsertDetachedAfterTail(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkListAligned::InsertBeforeHead(int iSize, int iAlignment, int iOffset)
{
	void*			pvData;

	pvData = AllocateDetached(iSize, iAlignment, iOffset);
	__CLinkListTemplate<void>::InsertDetachedBeforeHead(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkListAligned::InsertBeforeNode(void* psPos, int iSize, int iAlignment, int iOffset)
{
	void*			pvData;

	pvData = AllocateDetached(iSize, iAlignment, iOffset);
	__CLinkListTemplate<void>::InsertDetachedBeforeNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkListAligned::InsertAfterNode(void* psPos, int iSize, int iAlignment, int iOffset)
{
	void*			pvData;

	pvData = AllocateDetached(iSize, iAlignment, iOffset);
	__CLinkListTemplate<void>::InsertDetachedAfterNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkListAligned::AllocateDetached(int iDataSize, int iAlignment, int iOffset)
{
	void*			pvMem;
	int				iTotalSize;
	SDANode*		psNode;

	iOffset = ::CalculateOffset(iOffset - sizeof(SDANode), iAlignment);
	iTotalSize = iDataSize + sizeof(SDANode) + iAlignment-1;

	pvMem = Malloc(iTotalSize);
	psNode = CalculateActualStart(pvMem, iAlignment, iOffset);
	psNode->sAligned.iAlignment = iAlignment;
	psNode->sAligned.iOffset = iOffset;
	psNode->sAligned.iSize = iDataSize;
	psNode->sAligned.pvAlloc = pvMem;

	return CLinkListAlignedHeaderGetData(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkListAligned::ByteSize(void)
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
int CLinkListAligned::GetNodeSize(void* pvMem)
{
	SDANode*		psNodeHeader;

	psNodeHeader = CLinkListAlignedDataGetHeader(pvMem);
	return psNodeHeader->sAligned.iSize + sizeof(SDANode) + psNodeHeader->sAligned.iAlignment-1;;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDANode* CLinkListAligned::CalculateActualStart(void* pvMem, int iAlignment, int iOffset)
{
	ENGINE_SIZE_T	iStart;
	ENGINE_SIZE_T	iByteDiff;
	ENGINE_SIZE_T	iActualOffset;

	iStart = (ENGINE_SIZE_T)pvMem;

	iByteDiff = iStart % iAlignment;
	if (iByteDiff == 0)
	{
		return (SDANode*)(ENGINE_SIZE_T)(iStart + iOffset);
	}
	else
	{
		iActualOffset = (((ENGINE_SIZE_T)iAlignment - iByteDiff) + iOffset) % (ENGINE_SIZE_T)iAlignment;
		return (SDANode*)(ENGINE_SIZE_T)(iStart + iActualOffset);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SDANode* CLinkListAligned::GetNode(void* pvMem)
{
	return (SDANode*)(ENGINE_SIZE_T) ((unsigned int)(ENGINE_SIZE_T) pvMem - sizeof(SDANode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkListAligned::Remove(void* pvData)
{
	Detach(pvData);
	FreeDetached(pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkListAligned::SafeRemove(void* pvData)
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
void CLinkListAligned::FreeDetached(void* pvData)
{
	SDANode*		psNodeHeader;

	psNodeHeader = CLinkListAlignedDataGetHeader(pvData);
	if (psNodeHeader)
	{
		FreeNode(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkListAligned::Grow(void* pvData, unsigned int uiNewSize)
{
	SDANode*		psNodeHeader;
	void*			pvAllocatedEnd;
	void*			pvObjectEnd;
	void*			pvNew;
	unsigned int	uiSize;

	psNodeHeader = CLinkListAlignedDataGetHeader(pvData);

	if (uiNewSize == 0)
	{
		Remove(pvData);
		return NULL;
	}

	pvAllocatedEnd = (void*)(ENGINE_SIZE_T) ((int)(ENGINE_SIZE_T) psNodeHeader->sAligned.pvAlloc + sizeof(SDANode) + psNodeHeader->sAligned.iSize + psNodeHeader->sAligned.iAlignment-1);
	pvObjectEnd = (void*)(ENGINE_SIZE_T) ((int)(ENGINE_SIZE_T) pvData + uiNewSize);

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

