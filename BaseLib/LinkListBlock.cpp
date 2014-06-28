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

	pvData = HeaderGetData<SDNode, void>(mpsHead);  //Yes this is the correct macro.
	psNode = DataGetHeader<SDUNode, void>(pvData);
	while (psNode)
	{
		pvData = HeaderGetData<SDNode, void>(psNode->sDNode.psNext);  //Yes this is the correct macro.
		psNode2 = DataGetHeader<SDUNode, void>(pvData);
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

	psNodeHeader = DataGetHeader<SDUNode, void>(pvData);
	return psNodeHeader->sType.miType;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int	CLinkListBlock::GetNodeSize(void* pvData)
{
	SDUNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SDUNode, void>(pvData);
	return psNodeHeader->sType.miSize + sizeof(SDUNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkListBlock::GetNodeTypeAndSize(void* pvData, SUnknownType* psType)
{
	SDUNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SDUNode, void>(pvData);
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

	psNodeHeader = DataGetHeader<SDUNode, void>(pvData);
	memcpy(&psNodeHeader->sType, psType, sizeof(SUnknownType));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::InsertAfterTail(int iDataSize)
{
	void*			pvData;
	
	pvData = AllocateDetached(iDataSize);
	__CLinkListTemplate<void>::InsertDetachedAfterTail(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::InsertBeforeHead(int iDataSize)
{
	void*			pvData;
	
	pvData = AllocateDetached(iDataSize);
	__CLinkListTemplate<void>::InsertDetachedBeforeHead(pvData);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::InsertBeforeNode(int iDataSize, void* psPos)
{
	void*			pvData;
	
	pvData = AllocateDetached(iDataSize);
	__CLinkListTemplate<void>::InsertDetachedBeforeNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::InsertAfterNode(int iDataSize, void* psPos)
{
	void*			pvData;
	
	pvData = AllocateDetached(iDataSize);
	__CLinkListTemplate<void>::InsertDetachedAfterNode(pvData, psPos);
	return pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CLinkListBlock::AllocateDetached(int iDataSize)
{
	SDUNode*		psNode;
	
	psNode = (SDUNode*)Malloc(sizeof(SDUNode) + iDataSize);
	psNode->sType.miSize = iDataSize;
	return HeaderGetData<SDUNode, void>(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CLinkListBlock::FreeDetached(void* psNodeData)
{
	SDUNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<SDUNode, void>(psNodeData);
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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkListBlock::WriteLinkListBlock(CFileWriter* pcFileWriter)
{
	void*			pvData;
	SUnknownType	sType;

	if (!pcFileWriter->WriteData(this, sizeof(CLinkListBlock))) 
	{ 
		return FALSE; 
	}

	pvData = GetHead();
	while (pvData)
	{
		GetNodeTypeAndSize(pvData, &sType);
		if (!pcFileWriter->WriteData(&sType, sizeof(SUnknownType))) 
		{ 
			return FALSE; 
		}

		if (!pcFileWriter->WriteData(pvData, sType.miSize)) 
		{ 
			return FALSE; 
		}

		pvData = GetNext(pvData);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLinkListBlock::ReadLinkListBlock(CFileReader* pcFileReader)
{
	int				iNumElements;
	int				i;
	SUnknownType	sType;
	void*			pvData;

	if (!pcFileReader->ReadData(this, sizeof(CLinkListBlock))) 
	{ 
		return FALSE; 
	}

	iNumElements = NumElements();
	Init();
	for (i = 0; i < iNumElements; i++)
	{
		if (!pcFileReader->ReadData(&sType, sizeof(SUnknownType))) 
		{ 
			return FALSE; 
		}

		pvData = InsertAfterTail(sType.miSize);
		if (!pcFileReader->ReadData(pvData, sType.miSize)) 
		{ 
			return FALSE; 
		}
	}
	return TRUE;
}

