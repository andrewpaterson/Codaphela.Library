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
#include "TreeBlock.h"



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertRoot(size uiDataSize)
{
	void*		psNode;

	psNode = AllocateDetached(uiDataSize);
	__CTreeBlock::InsertDetachedOnPath(NULL, 0, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnPath(size* auiPos, size uiLevel, size uiDataSize)
{
	void*		psNode;

	psNode = AllocateDetached(uiDataSize);
	__CTreeBlock::InsertDetachedOnPath(auiPos, uiLevel, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnPath(size* auiPos, size uiLevel, size uiDataSize, size* auiOldPos, size uiOldLevel, void* psOldNode)
{
	void*		psNode;

	psNode = AllocateDetached(uiDataSize);
	__CTreeBlock::InsertDetachedOnPath(auiPos, uiLevel, psNode, auiOldPos, uiOldLevel, psOldNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnRightOfChildren(void* psParent, size uiDataSize)
{
	void*		psNode;

	//Allocate memory for the user data and the attached tree data
	psNode = AllocateDetached(uiDataSize);
	__CTreeBlock::InsertDetachedOnRightOfChildren(psParent, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnLeftOfChildren(void* psParent, size uiDataSize)
{
	void *psNode;

	psNode = AllocateDetached(uiDataSize);	
	__CTreeBlock::InsertDetachedOnLeftOfChildren(psParent, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertAtChildNum(void* psParent, size iChildNum, size uiDataSize)
{
	void	*psNode;

	psNode = AllocateDetached(uiDataSize);
	__CTreeBlock::InsertDetachedAtChildNum(psParent, iChildNum, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnUp(void* psPos, size uiDataSize)
{
	void*	psNode;

	psNode = AllocateDetached(uiDataSize);
	__CTreeBlock::InsertDetachedOnUp(psPos, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnAcross(void* psPos, size uiDataSize)
{
	void*	psNode;

	psNode = AllocateDetached(uiDataSize);
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
		pvData = HeaderGetData<STNode, void>(psNode);
		psFree = DataGetHeader<STUNode, void>(pvData);
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
void* CTreeBlock::AllocateDetached(size uiDataSize)
{
	STUNode*	psNode;
	
	psNode = (STUNode*)Malloc(sizeof(STUNode) + uiDataSize);
	psNode->iSize = uiDataSize;
	return HeaderGetData<STUNode, void>(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlock::FreeDetached(void* psNodeData)
{
	STUNode*	psNodeHeader;

	psNodeHeader = DataGetHeader<STUNode, void>(psNodeData);
	if (psNodeHeader)
	{
		__CTreeBlock::Free(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CTreeBlock::ByteSize(void)
{
	void*	pvNode;
	size		iSize;

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
size	CTreeBlock::GetNodeSize(void* pvData)
{
	STUNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<STUNode, void>(pvData);
	return psNodeHeader->iSize;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTreeBlock::Detach(void* psNodeData)
{
	STUNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<STUNode, void>(psNodeData);
	__CTreeBlock::Detach(&psNodeHeader->sTNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTreeBlock::RemoveLeaf(void* psNodeData)
{
	STUNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<STUNode, void>(psNodeData);
	if ((psNodeData == NULL) || (psNodeHeader->sTNode.psUp))
	{
		//This wasn't a leaf so we can't detach it.
		return false;
	}
	Detach(psNodeData);
	__CTreeBlock::Free(psNodeHeader);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CTreeBlock::RemoveBranch(void* psNodeData)
{
	CTreeBlock	cTree;
	size				iNum;

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

	psNodeHeader = DataGetHeader<STUNode, void>(psNodeData);
	Detach(psNodeData);
	__CTreeBlock::Free(psNodeHeader);
}




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTreeBlock::WriteTreeUnknown(CFileWriter* pcFileWriter)
{
	void*			pvData;
	size				iPathSize;
	size				aiPath[1024];
	size				iSize;

	if (!pcFileWriter->WriteData(this, sizeof(CTreeBlock))) 
	{ 
		return false; 
	}

	if (NumElements() != 0)
	{
		pvData = StartTraversal();
		while (pvData != NULL)
		{
			iPathSize = GetPathTo(aiPath, pvData);
			if (iPathSize >= 1024)
			{
				return false; 
			}
			iSize = GetNodeSize(pvData);
			if (!pcFileWriter->WriteSize(iSize))
			{ 
				return false; 
			}
			if (!pcFileWriter->WriteSize(iPathSize)) 
			{ 
				return false; 
			}
			if (!pcFileWriter->WriteData(aiPath, sizeof(size) * iPathSize)) 
			{ 
				return false; 
			}
			if (!pcFileWriter->WriteData(pvData, iSize)) 
			{ 
				return false; 
			}
			pvData = TraverseFrom(pvData);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTreeBlock::ReadTreeUnknown(CFileReader* pcFileReader)
{
	void*			pvData;
	size				i;
	size				iPathSize;
	size				aiPath[1024];
	size				iNumElements;
	size				iSize;;

	if (!pcFileReader->ReadData(this, sizeof(CTreeBlock))) 
	{ 
		return false; 
	}

	iNumElements = NumElements();
	Init();

	for (i = 0; i < iNumElements; i++)
	{
		if (!pcFileReader->ReadSize(&iSize)) 
		{ 
			return false; 
		}
		if (!pcFileReader->ReadSize(&iPathSize)) 
		{ 
			return false; 
		}
		if (!pcFileReader->ReadData(aiPath, sizeof(size) * iPathSize)) 
		{ 
			return false; 
		}
		pvData = InsertOnPath(aiPath, iPathSize, iSize);
		if (!pcFileReader->ReadData(pvData, iSize)) 
		{ 
			return false; 
		}
	}
	return true;
}
