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
#include "LinkListBlock.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkListBlock::Init(void)
{
	mpsHead = NULL;
	mpsTail = NULL;
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkListBlock::Kill(void)
{
	SDUNode*	psNode;
	SDUNode*	psNode2;
	void*		pvData;

	pvData = HeaderGetDataMacro<SDNode, void>(mpsHead);  //Yes this is the correct macro.
	psNode = CLinkListBlockDataGetHeader(pvData);
	while (psNode)
	{
		pvData = HeaderGetDataMacro<SDNode, void>(psNode->sDNode.psNext);  //Yes this is the correct macro.
		psNode2 = CLinkListBlockDataGetHeader(pvData);
		Free(psNode);
		psNode = psNode2;
	}
	mpsHead = NULL;
	mpsTail = NULL;
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkListBlock::GetNodeType(void* pvData)
{
	SDUNode*		psNodeHeader;

	psNodeHeader = CLinkListBlockDataGetHeader(pvData);
	return psNodeHeader->sType.miType;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int	CLinkListBlock::GetNodeSize(void* pvData)
{
	SDUNode*		psNodeHeader;

	psNodeHeader = CLinkListBlockDataGetHeader(pvData);
	return psNodeHeader->sType.miSize + sizeof(SDUNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkListBlock::GetNodeTypeAndSize(void* pvData, SUnknownType* psType)
{
	SDUNode*		psNodeHeader;

	psNodeHeader = CLinkListBlockDataGetHeader(pvData);
	psType->miSize = psNodeHeader->sType.miSize;
	psType->miType = psNodeHeader->sType.miType;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkListBlock::SetNodeTypeAndSize(void* pvData, SUnknownType* psType)
{
	SDUNode*		psNodeHeader;

	psNodeHeader = CLinkListBlockDataGetHeader(pvData);
	memcpy(&psNodeHeader->sType, psType, sizeof(SUnknownType));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::InsertAfterTail(int iDataSize, int iDataType)
{
	void*			pvData;
	
	pvData = AllocateDetached(iDataSize, iDataType);
	__CLinkListTemplate<void>::InsertDetachedAfterTail(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::InsertBeforeHead(int iDataSize, int iDataType)
{
	void*			pvData;
	
	pvData = AllocateDetached(iDataSize, iDataType);
	__CLinkListTemplate<void>::InsertDetachedBeforeHead(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::InsertBeforeNode(int iDataSize, int iDataType, void* psPos)
{
	void*			pvData;
	
	pvData = AllocateDetached(iDataSize, iDataType);
	__CLinkListTemplate<void>::InsertDetachedBeforeNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::InsertAfterNode(int iDataSize, int iDataType, void* psPos)
{
	void*			pvData;
	
	pvData = AllocateDetached(iDataSize, iDataType);
	__CLinkListTemplate<void>::InsertDetachedAfterNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::AllocateDetached(int iDataSize, int iDataType)
{
	SDUNode*		psNode;
	
	psNode = (SDUNode*)MemoryAllocate(sizeof(SDUNode) + iDataSize);
	psNode->sType.miSize = iDataSize;
	psNode->sType.miType = iDataType;
	return CLinkListBlockHeaderGetData(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkListBlock::FreeDetached(void* psNodeData)
{
	SDUNode*		psNodeHeader;

	psNodeHeader = CLinkListBlockDataGetHeader(psNodeData);
	if (psNodeHeader)
	{
		Free(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CLinkListBlock::ByteSize(void)
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
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::GetHeadAndType(int* piType)
{
	SDUNode*		psNodeHeader;
	void*			pvData;

	pvData = GetHead();
	psNodeHeader = CLinkListBlockDataGetHeader(pvData);
	*piType = psNodeHeader->sType.miType;
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::GetNextAndType(void* pvData, int* piType)
{
	SDUNode*		psNodeHeader;

	pvData = GetNext(pvData);
	if (pvData)
	{
		psNodeHeader = CLinkListBlockDataGetHeader(pvData);
		*piType = psNodeHeader->sType.miType;
	}
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinkListBlock::Remove(void* pvData)
{
	Detach(pvData);
	FreeDetached(pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkListBlock::SafeRemove(void* pvData)
{
	if (IsInList(pvData))
	{
		Remove(pvData);
		return TRUE;
	}
	return FALSE;
}

