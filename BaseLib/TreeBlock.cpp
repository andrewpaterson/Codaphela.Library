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
void* CTreeBlock::InsertRoot(int iDataSize)
{
	void*		psNode;

	psNode = AllocateDetached(iDataSize);
	__CTreeBlock::InsertDetachedOnPath(NULL, 0, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnPath(int* aiPos, int iLevel, int iDataSize)
{
	void*		psNode;

	psNode = AllocateDetached(iDataSize);
	__CTreeBlock::InsertDetachedOnPath(aiPos, iLevel, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnPath(int* aiPos, int iLevel, int iDataSize, int* aiOldPos, int iOldLevel, void* psOldNode)
{
	void*		psNode;

	psNode = AllocateDetached(iDataSize);
	__CTreeBlock::InsertDetachedOnPath(aiPos, iLevel, psNode, aiOldPos, iOldLevel, psOldNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnRightOfChildren(void* psParent, int iDataSize)
{
	void*		psNode;

	//Allocate memory for the user data and the attached tree data
	psNode = AllocateDetached(iDataSize);
	__CTreeBlock::InsertDetachedOnRightOfChildren(psParent, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnLeftOfChildren(void* psParent, int iDataSize)
{
	void *psNode;

	psNode = AllocateDetached(iDataSize);	
	__CTreeBlock::InsertDetachedOnLeftOfChildren(psParent, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertAtChildNum(void* psParent, int iChildNum, int iDataSize)
{
	void	*psNode;

	psNode = AllocateDetached(iDataSize);
	__CTreeBlock::InsertDetachedAtChildNum(psParent, iChildNum, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnUp(void* psPos, int iDataSize)
{
	void*	psNode;

	psNode = AllocateDetached(iDataSize);
	__CTreeBlock::InsertDetachedOnUp(psPos, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CTreeBlock::InsertOnAcross(void* psPos, int iDataSize)
{
	void*	psNode;

	psNode = AllocateDetached(iDataSize);
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
void* CTreeBlock::AllocateDetached(int iDataSize)
{
	STUNode*	psNode;
	
	psNode = (STUNode*)Malloc(sizeof(STUNode) + iDataSize);
	psNode->iSize = iDataSize;
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
int CTreeBlock::ByteSize(void)
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
int	CTreeBlock::GetNodeSize(void* pvData)
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
BOOL CTreeBlock::RemoveLeaf(void* psNodeData)
{
	STUNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<STUNode, void>(psNodeData);
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

	psNodeHeader = DataGetHeader<STUNode, void>(psNodeData);
	Detach(psNodeData);
	__CTreeBlock::Free(psNodeHeader);
}




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CTreeBlock::WriteTreeUnknown(CFileWriter* pcFileWriter)
{
	void*			pvData;
	int				iPathSize;
	int				aiPath[1024];
	int				iSize;

	if (!pcFileWriter->WriteData(this, sizeof(CTreeBlock))) 
	{ 
		return FALSE; 
	}

	if (NumElements() != 0)
	{
		pvData = StartTraversal();
		while (pvData != NULL)
		{
			iPathSize = GetPathTo(aiPath, pvData);
			if (iPathSize >= 1024)
			{
				return FALSE; 
			}
			iSize = GetNodeSize(pvData);
			if (!pcFileWriter->WriteInt(iSize))
			{ 
				return FALSE; 
			}
			if (!pcFileWriter->WriteInt(iPathSize)) 
			{ 
				return FALSE; 
			}
			if (!pcFileWriter->WriteData(aiPath, sizeof(int) * iPathSize)) 
			{ 
				return FALSE; 
			}
			if (!pcFileWriter->WriteData(pvData, iSize)) 
			{ 
				return FALSE; 
			}
			pvData = TraverseFrom(pvData);
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CTreeBlock::ReadTreeUnknown(CFileReader* pcFileReader)
{
	void*			pvData;
	int				i;
	int				iPathSize;
	int				aiPath[1024];
	int				iNumElements;
	int				iSize;;

	if (!pcFileReader->ReadData(this, sizeof(CTreeBlock))) 
	{ 
		return FALSE; 
	}

	iNumElements = NumElements();
	Init();

	for (i = 0; i < iNumElements; i++)
	{
		if (!pcFileReader->ReadInt(&iSize)) 
		{ 
			return FALSE; 
		}
		if (!pcFileReader->ReadData(&iPathSize, sizeof(int))) 
		{ 
			return FALSE; 
		}
		if (!pcFileReader->ReadData(aiPath, sizeof(int) * iPathSize)) 
		{ 
			return FALSE; 
		}
		pvData = InsertOnPath(aiPath, iPathSize, iSize);
		if (!pcFileReader->ReadData(pvData, iSize)) 
		{ 
			return FALSE; 
		}
	}
	return TRUE;
}
