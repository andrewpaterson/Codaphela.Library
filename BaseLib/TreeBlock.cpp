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
#include "TreeBlock.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertRoot(int iDataSize, int iDataType)
{
	void*		psNode;

	psNode = AllocateDetached(iDataSize, iDataType);
	__CTreeBlock::InsertDetachedOnPath(NULL, 0, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnPath(int* aiPos, int iLevel, int iDataSize, int iDataType)
{
	void*		psNode;

	psNode = AllocateDetached(iDataSize, iDataType);
	__CTreeBlock::InsertDetachedOnPath(aiPos, iLevel, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnPath(int* aiPos, int iLevel, int iDataSize, int iDataType, int* aiOldPos, int iOldLevel, void* psOldNode)
{
	void*		psNode;

	psNode = AllocateDetached(iDataSize, iDataType);
	__CTreeBlock::InsertDetachedOnPath(aiPos, iLevel, psNode, aiOldPos, iOldLevel, psOldNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnRightOfChildren(void* psParent, int iDataSize, int iDataType)
{
	void*		psNode;

	//Allocate memory for the user data and the attached tree data
	psNode = AllocateDetached(iDataSize, iDataType);
	__CTreeBlock::InsertDetachedOnRightOfChildren(psParent, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnLeftOfChildren(void* psParent, int iDataSize, int iDataType)
{
	void *psNode;

	psNode = AllocateDetached(iDataSize, iDataType);	
	__CTreeBlock::InsertDetachedOnLeftOfChildren(psParent, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertAtChildNum(void* psParent, int iChildNum, int iDataSize, int iDataType)
{
	void	*psNode;

	psNode = AllocateDetached(iDataSize, iDataType);
	__CTreeBlock::InsertDetachedAtChildNum(psParent, iChildNum, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnUp(void* psPos, int iDataSize, int iDataType)
{
	void*	psNode;

	psNode = AllocateDetached(iDataSize, iDataType);
	__CTreeBlock::InsertDetachedOnUp(psPos, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnAcross(void* psPos, int iDataSize, int iDataType)
{
	void*	psNode;

	psNode = AllocateDetached(iDataSize, iDataType);
	__CTreeBlock::InsertDetachedOnAcross(psPos, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlock::RecursiveFreeNodes(STNode *psNode)
{	
	STNode*		psTemp;
	void*		pvData;
	STUNode*	psFree;

	while (psNode != NULL)
	{
		RecursiveFreeNodes(psNode->psUp);
		psTemp = psNode->psAcross;
		pvData = HeaderGetDataMacro<STNode, void>(psNode);
		psFree = CTreeUnknownDataGetHeader(pvData);
		__CTreeBlock::Free(psFree);
		miNumElements--;
		psNode = psTemp;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlock::Kill(void)
{
	RecursiveFreeNodes(mpsRoot);
	mpsRoot = NULL;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::AllocateDetached(int iDataSize, int iDataType)
{
	STUNode*	psNode;
	
	psNode = (STUNode*)MemoryAllocate(sizeof(STUNode) + iDataSize);
	psNode->sType.miSize = iDataSize;
	psNode->sType.miType = iDataType;
	return CTreeUnknownHeaderGetData(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlock::FreeDetached(void* psNodeData)
{
	STUNode*	psNodeHeader;

	psNodeHeader = CTreeUnknownDataGetHeader(psNodeData);
	if (psNodeHeader)
	{
		__CTreeBlock::Free(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CTreeBlock::MallocSize(void)
{
	void*	pvNode;
	int		iSize;

	iSize = 0;
	pvNode = StartTraversal();
	while(pvNode)
	{
		iSize += (GetNodeSize(pvNode) + sizeof(STUNode));
		pvNode = TraverseFrom(pvNode);
	}
	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CTreeBlock::GetNodeType(void* pvData)
{
	STUNode*		psNodeHeader;

	psNodeHeader = CTreeUnknownDataGetHeader(pvData);
	return psNodeHeader->sType.miType;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int	CTreeBlock::GetNodeSize(void* pvData)
{
	STUNode*		psNodeHeader;

	psNodeHeader = CTreeUnknownDataGetHeader(pvData);
	return psNodeHeader->sType.miSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlock::GetNodeTypeAndSize(void* pvData, SUnknownType* psType)
{
	STUNode*		psNodeHeader;

	psNodeHeader = CTreeUnknownDataGetHeader(pvData);
	psType->miSize = psNodeHeader->sType.miSize;
	psType->miType = psNodeHeader->sType.miType;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlock::SetNodeTypeAndSize(void* pvData, SUnknownType* psType)
{
	STUNode*		psNodeHeader;

	psNodeHeader = CTreeUnknownDataGetHeader(pvData);
	psNodeHeader->sType.miSize = psType->miSize;
	psNodeHeader->sType.miType = psType->miType;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlock::Detach(void* psNodeData)
{
	STUNode*		psNodeHeader;

	psNodeHeader = CTreeUnknownDataGetHeader(psNodeData);
	__CTreeBlock::Detach(&psNodeHeader->sTNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CTreeBlock::RemoveLeaf(void* psNodeData)
{
	STUNode*		psNodeHeader;

	psNodeHeader = CTreeUnknownDataGetHeader(psNodeData);
	if ((psNodeData == NULL) || (psNodeHeader->sTNode.psUp))
	{
		//This wasn't a leaf so we can't detach it.
		return FALSE;
	}
	Detach(psNodeData);
	__CTreeBlock::Free(psNodeHeader);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CTreeBlock::RemoveBranch(void* psNodeData)
{
	CTreeBlock	cTree;
	int				iNum;

	if (!RemoveLeaf(psNodeData))
	{
		//Whilst __CTreeBlock::DetachTree reassigns the root it is not a problem that this is and STUNode and not an STNode.
		__CTreeBlock::DetachTree(&cTree, psNodeData);
		iNum = cTree.NumElements();
		cTree.Kill();

		return iNum;
	}
	return 1;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlock::Remove(void* psNodeData)
{
	STUNode*		psNodeHeader;

	psNodeHeader = CTreeUnknownDataGetHeader(psNodeData);
	Detach(psNodeData);
	__CTreeBlock::Free(psNodeHeader);
}

