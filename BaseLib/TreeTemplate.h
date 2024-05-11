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
#ifndef __TREE_TEMPLATE_H__
#define __TREE_TEMPLATE_H__
#include "PointerFunctions.h"
#include "DataTypes.h"
#include "FreeList.h"
#include "ConstructorCall.h"


#define MAX_TREE_DEPTH	1024


//NEVER use this tree.
//Fix the trees that currently DO to use this one.
struct STNode
{
	STNode*		psUp;
	STNode*		psAcross;
	STNode*		psDown;
};


template<class M>
class CTreeTemplate : protected CPostMalloc<M>
{
protected:
	STNode*	mpsRoot;
	size	miNumElements;

public:
	size		miLevel;  //Level is only valid during traversals.  WTF? A traversal object should hold this.

public:
	void	Init(void);
	void	Kill(void);
	void	Copy(CTreeTemplate* pcTreeSrc);
	size	NumElements(void);
	size	ByteSize(void);

	//Create the root node
	M*		InsertRoot();
	void	InsertRoot(M* psData);

	//Insert on the right of the parents children nodes.
	void	InsertDetachedOnRightOfChildren(M* psParent, M* psChild);
	M*		InsertOnRightOfChildren(M* psParent);
	M*		InsertOnRightOfChildren(M* psParent, M* psData);

	//Insert on the left of the parents children nodes.
	void	InsertDetachedOnLeftOfChildren(M* psParent, M* psChild);
	M*		InsertOnLeftOfChildren(M* psParent);
	M*		InsertOnLeftOfChildren(M* psParent, M* psData);

	//Insert the node at specific position.
	void	InsertDetachedAtChildNum(M* psParent, size iChildNum, M* psChild);
	M*		InsertAtChildNum(M* psParent, size iChildNum);
	M*		InsertAtChildNum(M* psParent, size iChildNum, M* psData);

	//Insert the node at the position at the end of the path.
	void	InsertDetachedOnPath(size* auiPos, size uiLevel, M* psData);
	void	InsertDetachedOnPath(size* auiPos, size uiLevel, M* psData, size* aiOldPos, size uiOldLevel, M* psOldData);
	M*		InsertOnPath(size* auiPos, size uiLevel, size* aiOldPos, size uiOldLevel, M* psOldData);
	M*		InsertOnPath(size* auiPos, size uiLevel);
	M*		InsertOnPath(size* psPos, size uiLevel, M* psData);

	//Traversal functions.
	M* 		StartTraversal(void);
	M* 		TraverseFrom(M* psCurrent);
	M*		TraverseSubTreeFrom(STNode* psSubRoot, M* psCurrent);
	M*		TraverseSubTreeFrom(M* psSubRoot, M* psCurrent);
	M* 		TraversePath(size* auiPos, size uiLevel);

	size	GetNumChildren(M* psPos);
	M* 		GetRoot(void);
	M* 		GetDown(M* psPos);
	M* 		GetUp(M* psPos);
	M* 		GetRight(M* psPos);
	size	GetPathTo(size* auiPath, M* psNode);
	M*		GetChildNum(M* psParent, size iChildNum);
	size	GetChildNum(M* psChild);
	size	GetNodeDepth(M* psNode);

	//Tree insertion functions.
	void	InitTreeFromNode(CTreeTemplate<M>* pcTreeSource, M* psNodeSource);

	//Comparison functions.
	bool	Equals(CTreeTemplate<M>* pcTree);
	bool	Equals(CTreeTemplate<M>* pcTree, size iKeyOffset, size iKeySize);

	//Insertion functions assuming a good understanding of internal tree structure.
	void	InsertDetachedOnUp(M* psPosData, M* psNewData);
	M*		InsertOnUp(M* psPos);
	M*		InsertOnUp(M* psPos, M* psNewData);

	void	InsertDetachedOnAcross(M* psPosData, M* psNewData);
	M*		InsertOnAcross(M* psPos);
	M*		InsertOnAcross(M* psPos, M* psNewData);

	void	Detach(M* psNodeData);
	void	Detach(STNode* psNodeHeader, bool bDetachChildren = false);
	M*		AllocateDetached(void);
	void	FreeDetached(M* psNodeData);
	void	DetachTree(CTreeTemplate<M>* pcDestTree, M* psNodeData);

	bool	RemoveLeaf(M* psNodeData);
	size	RemoveBranch(M* psNodeData);
	void	Remove(M* psNodeData);

	bool	WriteTreeTemplate(CFileWriter* pcFileWriter);
	bool	ReadTreeTemplate(CFileReader* pcFileReader);

protected:
	void*	Malloc(size uiSize);
	void*	Realloc(void* pv, size uiSize);
	void	Free(void* pv);

	void	RecursiveFreeNodes(STNode* pNode);
	void	RecursiveAddNodeForCreate(CTreeTemplate<M>* pcTreeSource, M* pvData1Source, M* pvData2Dest);
	STNode*	PrivateFindLeftChild(STNode* psNodeHeader);

public:
	size		CountElements(void);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* CTreeTemplate<M>::Malloc(size uiSize)
{
	return malloc(uiSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::Free(void* pv)
{
	SafeFree(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* CTreeTemplate<M>::Realloc(void* pv, size uiSize)
{
	pv = realloc(pv, uiSize);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::Init(void)
{
	mpsRoot = NULL;
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::Copy(CTreeTemplate<M>* pcTreeSrc)
{
	InitTreeFromNode(pcTreeSrc, HeaderGetData<STNode, M>(pcTreeSrc->mpsRoot));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::TraversePath(size* auiPos, size uiLevel)
{
	STNode	*psNode;
	size	iNumAcross;
	size	i;
	size	j;

	psNode = mpsRoot;
	for (i = 0; i < uiLevel; i++)
	{
		iNumAcross = auiPos[i];
		psNode = psNode->psUp;
		for (j = 0; j < iNumAcross;j++)
		{
			psNode = psNode->psAcross;
		}
	}
	return HeaderGetData<STNode, M>(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::InsertRoot()
{
	M*	psNode;

	psNode = InsertOnPath(NULL, 0);
	return psNode;
}


template<class M>
void CTreeTemplate<M>::InsertRoot(M* psData)
{
	M*	psNode;

	psNode = InsertRoot();
	memcpy(psNode, psData, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::InsertDetachedOnRightOfChildren(M* psParent, M* psChild)
{
	STNode* 	temp;
	STNode* 	psPos;
	STNode* 	psNode;

	psPos = DataGetHeader<STNode, M>(psParent);
	psNode = DataGetHeader<STNode, M>(psChild);
	temp = psPos;
	if (psPos->psUp != NULL)
	{
		psPos = psPos->psUp;
		while(psPos->psAcross != NULL)
		{
			psPos = psPos->psAcross;
		}
		psPos->psAcross = psNode;
	}
	else
	{
		psPos->psUp = psNode;
	}
	psNode->psUp = NULL;
	psNode->psAcross = NULL;
	psNode->psDown = temp;
	miNumElements++;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::InsertOnRightOfChildren(M* psParent)
{
	M*	psNode;

	psNode = AllocateDetached();
	InsertDetachedOnRightOfChildren(psParent, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::InsertOnRightOfChildren(M* psParent, M* psData)
{
	M*	psNode;

	psNode = InsertOnRightOfChildren(psParent);
	memcpy(psNode, psData, sizeof(M));
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::InsertDetachedOnLeftOfChildren(M* psParent, M* psChild)
{
	STNode*		temp;
	STNode*		psPos;
	STNode*		psNode;

	psPos = DataGetHeader<STNode, M>(psParent);
	psNode = DataGetHeader<STNode, M>(psChild);
	if (psPos->psUp != NULL)
	{
		temp = psPos;
		psPos = psPos->psUp;
		temp->psUp = psNode;
		psNode->psUp = NULL;
		psNode->psAcross = psPos;
		psNode->psDown = temp;
	}
	else
	{
		psPos->psUp = psNode;
		psNode->psUp = NULL;
		psNode->psAcross = NULL;
		psNode->psDown = psPos;
	}
	miNumElements++;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::InsertOnLeftOfChildren(M* psParent)
{
	M *psNode;

	psNode = AllocateDetached();
	InsertDetachedOnLeftOfChildren(psParent, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::InsertOnLeftOfChildren(M* psParent, M* psData)
{
	M*		psNode;

	psNode = InsertOnLeftOfChildren(psParent);
	memcpy(psNode, psData, sizeof(M));
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::InsertDetachedAtChildNum(M* psParent, size iChildNum, M* psChild)
{
	STNode* 	psPos;
	STNode*		temp;
	STNode*		psDown;
	STNode*		psNode;
	size		uiCount;

	psPos = DataGetHeader<STNode, M>(psParent);
	psNode = DataGetHeader<STNode, M>(psChild);
	if (psPos->psUp!=NULL)
	{

		temp = NULL;

		if (iChildNum == 0)
		{
			InsertDetachedOnLeftOfChildren(HeaderGetData<STNode, M>(psPos), HeaderGetData<STNode, M>(psNode));
			return;
		}
		psDown = psPos;
		psPos = psPos->psUp;
		uiCount = 0;
		while(iChildNum != uiCount)
		{
			temp = psPos;
			psPos = psPos->psAcross;
			uiCount++;
		}
		temp->psAcross = psNode;
		psNode->psUp = NULL;
		psNode->psAcross = psPos;
		psNode->psDown = psDown;
	}
	else
	{
		psPos->psUp=psNode;
		psNode->psUp=NULL;
		psNode->psAcross=NULL;
		psNode->psDown=psPos;
	}
	miNumElements++;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::InsertAtChildNum(M* psParent, size iChildNum)
{
	M	*psNode;

	psNode = AllocateDetached();
	InsertDetachedAtChildNum(psParent, iChildNum, psNode);
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::InsertAtChildNum(M* psParent, size iChildNum, M* psData)
{
	M*	psNode;

	psNode = InsertAtChildNum(psParent, iChildNum);
	memcpy(psNode, psData, sizeof(M));
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::InsertDetachedOnPath(size* auiPos, size uiLevel, M* psData)
{
	STNode*		psNode;
	M*			psThing;

	if (uiLevel == 0)
	{
		//Setup the root psNode
		psNode = DataGetHeader<STNode, M>(psData);
		mpsRoot = psNode;
		psNode->psAcross = NULL;
		psNode->psUp = NULL;
		psNode->psDown = NULL;
		miNumElements++;
	}
	else
	{
		psThing = TraversePath(auiPos, uiLevel-1);
		InsertDetachedAtChildNum(psThing, auiPos[uiLevel-1], psData);
	}
}

template<class M>
void CTreeTemplate<M>::InsertDetachedOnPath(size* auiPos, size uiLevel, M* psData, size* aiOldPos, size uiOldLevel, M* psOldData)
{
	//If we're the root node or the new path is not exactly one right of the old path.
	if ((uiLevel == 0) || (uiOldLevel != uiLevel) || (auiPos[uiLevel-1] != aiOldPos[uiLevel-1]+1))
	{
		//Then just call the general method.
		InsertDetachedOnPath(auiPos, uiLevel, psData);
	}
	else
	{
		InsertDetachedOnAcross(psOldData, psData);
	}
}


template<class M>
M* CTreeTemplate<M>::InsertOnPath(size* auiPos, size uiLevel, size* aiOldPos, size uiOldLevel, M* psOldData)
{
	M*	psNode;

	psNode = AllocateDetached();
	InsertDetachedOnPath(auiPos, uiLevel, psNode, aiOldPos, uiOldLevel, psOldData);
	return psNode;
}


template<class M>
M* CTreeTemplate<M>::InsertOnPath(size* auiPos, size uiLevel)
{
	M*	psNode;

	psNode = AllocateDetached();
	InsertDetachedOnPath(auiPos, uiLevel, psNode);
	return psNode;
}


template<class M>
M* CTreeTemplate<M>::InsertOnPath(size* psPos, size uiLevel, M* psData)
{
	M*	psNode;

	psNode = InsertOnPath(psPos, uiLevel);
	memcpy(psNode, psData, sizeof(M));
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::StartTraversal(void)
{
	miLevel = 0;
	return HeaderGetData<STNode, M>(mpsRoot);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::TraverseFrom(M* psCurrent)
{
	return TraverseSubTreeFrom(mpsRoot, psCurrent);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::TraverseSubTreeFrom(M* psSubRoot, M* psCurrent)
{
	STNode*	psSubrRootHeader;

	psSubrRootHeader = DataGetHeader<STNode, M>(psSubRoot);
	return TraverseSubTreeFrom(psSubrRootHeader, psCurrent);
}


template<class M>
M* CTreeTemplate<M>::TraverseSubTreeFrom(STNode* psSubRoot, M* psCurrent)
{
	STNode*		psNode;

	psNode = DataGetHeader<STNode, M>(psCurrent);
	if (psNode->psUp != NULL)
	{
		psNode = psNode->psUp;
		miLevel++;
	}
	else
	{
		while (psNode->psAcross == NULL)
		{
			psNode=psNode->psDown;
			miLevel--;
			if ((psNode == psSubRoot) || (psNode == NULL))
			{
				return NULL;
			}
		}
		psNode = psNode->psAcross;
	}
	return HeaderGetData<STNode, M>(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::Kill()
{
	RecursiveFreeNodes(mpsRoot);
	mpsRoot = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::RecursiveFreeNodes(STNode *psNode)
{
	STNode*		psTemp;

	while (psNode != NULL)
	{
		RecursiveFreeNodes(psNode->psUp);
		psTemp = psNode->psAcross;
		Free(psNode);
		miNumElements--;
		psNode = psTemp;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CTreeTemplate<M>::GetNumChildren(M *psParent)
{
	size	iNumChildren;
	STNode	*psTemp;
	STNode	*psPos;

	psPos = DataGetHeader<STNode, M>(psParent);
	iNumChildren = 0;
	psTemp = psPos;
	if (psPos->psUp != NULL)
	{
		iNumChildren++;
		psPos = psPos->psUp;
		while(psPos->psAcross!=NULL)
		{
			iNumChildren++;
			psPos = psPos->psAcross;
		}
	}
	return iNumChildren;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::GetRoot(void)
{
	return HeaderGetData<STNode, M>(mpsRoot);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::GetChildNum(M* psParent, size iChildNum)
{
	size		i;
	STNode*		psPos;

	psPos = DataGetHeader<STNode, M>(psParent);
	psPos = psPos->psUp;  //First child
	if (psPos == NULL)
	{
		return NULL;
	}

	for	(i = 0; i < iChildNum; i++)
	{
		psPos = psPos->psAcross;
		if (psPos == NULL)
		{
			return NULL;
		}
	}
	return HeaderGetData<STNode, M>(psPos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CTreeTemplate<M>::NumElements(void)
{
	return miNumElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::GetDown(M* psPos)
{
	STNode*	psNode;

	psNode = DataGetHeader<STNode, M>(psPos);
	return HeaderGetData<STNode, M>(psNode->psDown);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::GetUp(M* psPos)
{
	STNode*	psNode;

	psNode = DataGetHeader<STNode, M>(psPos);
	return HeaderGetData<STNode, M>(psNode->psUp);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::GetRight(M* psPos)
{
	STNode*	psNode;

	psNode = DataGetHeader<STNode, M>(psPos);
	return HeaderGetData<STNode, M>(psNode->psAcross);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CTreeTemplate<M>::GetNodeDepth(M* psNode)
{
	size		i;
	STNode*		psPos;

	psPos = DataGetHeader<STNode, M>(psNode);
	i = 0;
	while (psPos != mpsRoot)
	{
		i++;
		psPos = psPos->psDown;
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CTreeTemplate<M>::GetChildNum(M* psChild)
{
	STNode*		psNode;
	size		iChildNum;
	STNode*		psPos;

	psPos = DataGetHeader<STNode, M>(psChild);

	//Copy the psPosition of psPos.
	psNode = psPos;

	//Make sure we're not on the root psNode.
	if (psNode != mpsRoot)
	{
		iChildNum = 0;

		//Get the first child in this set of children.
		psNode = psNode->psDown;
		psNode = psNode->psUp;

		while (psNode != psPos)
		{
			iChildNum++;

			psNode = psNode->psAcross;
		}
		return iChildNum;
	}
	else
	{
		//The can only ever be one root 'child'.
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CTreeTemplate<M>::GetPathTo(size* auiPath, M* psData)
{
	size		iDistFromRoot;
	size		i;
	STNode*		psPos;

	iDistFromRoot = GetNodeDepth(psData);
	psPos = DataGetHeader<STNode, M>(psData);

	if (iDistFromRoot != 0)
	{
		i = iDistFromRoot;
		do
		{
			i--;
			auiPath[i] = GetChildNum(HeaderGetData<STNode, M>(psPos));
			psPos = psPos->psDown;
		}
		while (i != 0);
	}

	return iDistFromRoot;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CTreeTemplate<M>::ByteSize()
{
	return (sizeof(M) + sizeof(STNode)) * miNumElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CTreeTemplate<M>::Equals(CTreeTemplate<M>* pcTree)
{
	return Equals(pcTree, 0, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CTreeTemplate<M>::Equals(CTreeTemplate<M>* pcTree, size iKeyOffset, size iKeySize)
{
	M*		pvData1;
	M*		pvData2;
	size	iDepth1;
	size	iDepth2;
	size	iNum1;
	size	iNum2;

	if (pcTree->miNumElements == miNumElements)
	{
		pvData1 = StartTraversal();
		pvData2 = pcTree->StartTraversal();

		while ((pvData1 != NULL) && (pvData2 != NULL))
		{
			//Check the elements are the same.
			if (memcmp((M*)((ptr) pvData1 + (ptr) iKeyOffset), (M*)((ptr) pvData2 + (ptr) iKeyOffset), iKeySize) != 0)
			{
				return false;
			}

			//Check the topology of the trees are the same.
			iDepth1 = GetNodeDepth(pvData1);
			iDepth2 = pcTree->GetNodeDepth(pvData2);
			iNum1 = GetChildNum(pvData1);
			iNum2 = pcTree->GetChildNum(pvData2);
			if ((iDepth1 != iDepth2) || (iNum1 != iNum2))
			{
				return false;
			}

			pvData1 = TraverseFrom(pvData1);
			pvData2 = pcTree->TraverseFrom(pvData2);
		}

		if ((pvData1 == NULL) && (pvData2 == NULL))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::InitTreeFromNode(CTreeTemplate<M>* pcTreeSource, M* psNodeSource)
{
	M*			pvData1;
	M*			pvData2;
	STNode*		pNode;

	pNode = DataGetHeader<STNode, M>(psNodeSource);
	if (pNode != NULL)
	{
		Init();

		pvData1 = psNodeSource;
		pvData2 = InsertOnPath(NULL, 0);

		RecursiveAddNodeForCreate(pcTreeSource, pvData1, pvData2);
	}
	else
	{
		Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::RecursiveAddNodeForCreate(CTreeTemplate* pcTreeSource, M* pvData1Source, M* pvData2Dest)
{
	size	iNumNodes;
	size	i;
	M*		pvChildData1;
	M*		pvChildData2;

	memcpy(pvData2Dest, pvData1Source, sizeof(M));

	iNumNodes = GetNumChildren(pvData1Source);

	for (i = 0; i < iNumNodes; i++)
	{
		pvChildData1 = pcTreeSource->GetChildNum(pvData1Source, i);
		pvChildData2 = InsertOnRightOfChildren(pvData2Dest);
		RecursiveAddNodeForCreate(pcTreeSource, pvChildData1, pvChildData2);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::InsertDetachedOnUp(M* psPosData, M* psNewData)
{
	STNode*		psPos;
	STNode*		psNew;
	STNode*		psOld;

	psPos = DataGetHeader<STNode, M>(psPosData);
	psNew = DataGetHeader<STNode, M>(psNewData);

	psOld = psPos->psUp;

	psPos->psUp = psNew;
	psNew->psDown = psPos;
	psNew->psUp = psOld;
	while (psOld)
	{
		psOld->psDown = psNew;
		psOld = psOld->psAcross;
	}
	psNew->psAcross = NULL;
	miNumElements++;
}


template<class M>
M* CTreeTemplate<M>::InsertOnUp(M* psPos)
{
	M*	psNode;

	psNode = AllocateDetached();
	InsertDetachedOnUp(psPos, psNode);
	return psNode;
}


template<class M>
M* CTreeTemplate<M>::InsertOnUp(M* psPos, M* psNewData)
{
	M*	psNode;

	psNode = InsertOnUp(psPos);
	memcpy(psNode, psNewData, sizeof(M));
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::InsertDetachedOnAcross(M* psPosData, M* psNewData)
{
	STNode*		psPos;
	STNode*		psNew;
	STNode*		psOld;

	psPos = DataGetHeader<STNode, M>(psPosData);
	psNew = DataGetHeader<STNode, M>(psNewData);

	psOld = psPos->psAcross;

	psPos->psAcross = psNew;
	psNew->psDown = psPos->psDown;
	psNew->psUp = NULL;
	psNew->psAcross = psOld;
	miNumElements++;
}


template<class M>
M* CTreeTemplate<M>::InsertOnAcross(M* psPos)
{
	M*	psNode;

	psNode = AllocateDetached();
	InsertDetachedOnAcross(psPos, psNode);
	return psNode;
}


template<class M>
M* CTreeTemplate<M>::InsertOnAcross(M* psPos, M* psNewData)
{
	M*	psNode;

	psNode = InsertOnAcross(psPos);
	memcpy(psNode, psNewData, sizeof(M));
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::FreeDetached(M* psNodeData)
{
	STNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<STNode, M>(psNodeData);
	if (psNodeHeader)
	{
		Free(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::AllocateDetached(void)
{
	STNode*		psNode;

	psNode = (STNode*)Malloc(sizeof(STNode) + sizeof(M));
	return PostMalloc((M*)HeaderGetData<STNode, M>(psNode));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::Detach(STNode* psNodeHeader, bool bDetachChildren)
{
	STNode*		psNodeLeft;
	STNode*		psNodeUp;
	STNode*		psNodeRight;
	STNode*		psNodeDown;
	STNode*		psNode;

	if (psNodeHeader == mpsRoot)
	{
		//It is not possible to detach the root.
		return;
	}

	psNodeRight = psNodeHeader->psAcross;
	psNodeDown = psNodeHeader->psDown;
	psNodeLeft = PrivateFindLeftChild(psNodeHeader);
	psNodeUp = psNodeHeader->psUp;

	if (!bDetachChildren)
	{
		psNodeHeader->psUp = NULL;
	}
	psNodeHeader->psAcross = NULL;
	psNodeHeader->psDown = NULL;

	//If there were nodes upwards then they must be brought down and inserted where the detached node was.
	//If detach children is true then the upwards structure on the detached node must be maintained
	//(This means all children become detached from the tree).
	if ((psNodeUp) && (!bDetachChildren))
	{
		if (psNodeLeft)
		{
			psNodeLeft->psAcross = psNodeUp;
		}
		else
		{
			psNodeDown->psUp = psNodeUp;
		}
		psNode = psNodeUp;
		for (;;)
		{
			psNode->psDown = psNodeDown;
			if (psNode->psAcross)
			{
				psNode = psNode->psAcross;
			}
			else
			{
				break;
			}
		}
		psNode->psAcross = psNodeRight;
	}
	else
	{
		if (psNodeLeft)
		{
			psNodeLeft->psAcross = psNodeRight;
		}
		else
		{
			psNodeDown->psUp = psNodeRight;
		}
	}
	miNumElements--;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::Detach(M* psNodeData)
{
	STNode*		psNodeHeader;

	if (psNodeData)
	{
		psNodeHeader = DataGetHeader<STNode, M>(psNodeData);
		Detach(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CTreeTemplate<M>::RemoveLeaf(M* psNodeData)
{
	STNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<STNode, M>(psNodeData);
	if ((psNodeData == NULL) || (psNodeHeader->psUp))
	{
		//This wasn't a leaf so we can't detach it.
		return false;
	}
	Detach(psNodeHeader);
	Free(psNodeHeader);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CTreeTemplate<M>::RemoveBranch(M* psNodeData)
{
	CTreeTemplate<M>	cTree;
	size				iNum;

	if (!RemoveLeaf(psNodeData))
	{
		DetachTree(&cTree, psNodeData);
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
template<class M>
void CTreeTemplate<M>::Remove(M* psNodeData)
{
	Detach(psNodeData);
	FreeDetached(psNodeData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::DetachTree(CTreeTemplate<M>* pcDestTree, M* psNodeData)
{
	STNode*		psNodeHeader;

	psNodeHeader = DataGetHeader<STNode, M>(psNodeData);
	Detach(psNodeHeader, true);

	//Initialise the destination tree.
	pcDestTree->mpsRoot = psNodeHeader;
	pcDestTree->miNumElements = pcDestTree->CountElements();
	pcDestTree->miLevel = 0;

	//Setup the number of elemts after detachment (+1 is for original detached psNode).
	miNumElements = miNumElements - pcDestTree->miNumElements + 1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CTreeTemplate<M>::CountElements()
{
	M*		pcNode;
	size	uiNumElements;

	pcNode = StartTraversal();
	uiNumElements = 0;
	while (pcNode)
	{
		uiNumElements ++;
		pcNode = TraverseFrom(pcNode);
	}
	return uiNumElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
STNode* CTreeTemplate<M>::PrivateFindLeftChild(STNode* psNodeHeader)
{
	STNode*		psNodeLeft;

	if (psNodeHeader == mpsRoot)
	{
		return NULL;
	}
	psNodeLeft = psNodeHeader->psDown->psUp;
	if (psNodeHeader == psNodeLeft)
	{
		return NULL;
	}
	while (psNodeLeft->psAcross != psNodeHeader)
	{
		psNodeLeft = psNodeLeft->psAcross;
	}
	return psNodeLeft;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CTreeTemplate<M>::WriteTreeTemplate(CFileWriter* pcFileWriter)
{
	M*		pvData;
	size	iElementSize;
	size	uiPathSize;
	size	auiPath[MAX_TREE_DEPTH];

	iElementSize = sizeof(M);
	if (!pcFileWriter->WriteData(&iElementSize, sizeof(size))) 
	{ 
		return false; 
	}

	if (!pcFileWriter->WriteData(this, sizeof(CTreeTemplate<M>))) 
	{ 
		return false; 
	}

	if (NumElements() != 0)
	{
		pvData = StartTraversal();
		while (pvData != NULL)
		{
			uiPathSize = GetPathTo(auiPath, pvData);
			if (uiPathSize >= MAX_TREE_DEPTH)
			{
				return false; 
			}
			if (!pcFileWriter->WriteData(&uiPathSize, sizeof(size))) 
			{ 
				return false; 
			}

			if (uiPathSize != 0)
			{
				if (!pcFileWriter->WriteData(auiPath, sizeof(size) * uiPathSize)) 
				{ 
					return false; 
				}
			}
			if (!pcFileWriter->WriteData(pvData, sizeof(M)))
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
template<class M>
bool CTreeTemplate<M>::ReadTreeTemplate(CFileReader* pcFileReader)
{
	M*		pvData[2];
	size	iElementSize;
	size	i;
	size	uiPathSize[2];
	size	auiPath[2][MAX_TREE_DEPTH];
	size	uiNumElements;
	size	iPathNum;
	size	iOldPath;

	if (!pcFileReader->ReadData(&iElementSize, sizeof(size))) 
	{ 
		return false; 
	}

	if (iElementSize != sizeof(M))
	{
		return false;
	}

	if (!pcFileReader->ReadData(this, sizeof(CTreeTemplate<M>))) 
	{ 
		return false; 
	}

	uiNumElements = NumElements();
	iPathNum = 0;
	iOldPath = 0;
	Init();

	for (i = 0; i < uiNumElements; i++)
	{
		if (!pcFileReader->ReadData(&uiPathSize[iPathNum], sizeof(size))) 
		{ 
			return false; 
		}

		if (uiPathSize[iPathNum] != 0)
		{
			if (!pcFileReader->ReadData(auiPath[iPathNum], sizeof(size) * uiPathSize[iPathNum])) 
			{ 
				return false; 
			}
		}
		pvData[iPathNum] = InsertOnPath(auiPath[iPathNum], uiPathSize[iPathNum], auiPath[iOldPath], uiPathSize[iOldPath], pvData[iOldPath]);
		if (!pcFileReader->ReadData(pvData[iPathNum], sizeof(M))) 
		{ 
			return false; 
		}

		iOldPath = iPathNum;
		iPathNum++;
		iPathNum = iPathNum % 2;
	}
	return true;
}


#endif // __TREE_TEMPLATE_H__

