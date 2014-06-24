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
#ifndef __TREE_TEMPLATE_H__
#define __TREE_TEMPLATE_H__
#include "DataTypes.h"
#include "FreeList.h"


//NEVER use this tree, use CTreeTemplateFreeList instead.
//Fix the trees that currently DO to use this one.
struct STNode
{
	STNode*		psUp;
	STNode*		psAcross;
	STNode*		psDown;
};


template<class M>
class CTreeTemplate
{
protected:
	STNode*	mpsRoot;
	int		miNumElements;

public:
	int		miLevel;  //Level is only valid during traversals.  WTF? A traversal object should hold this.

public:
	void	Init(void);
	void	Kill(void);
	void	Copy(CTreeTemplate* pcTreeSrc);
	int		NumElements(void);
	int		ByteSize(void);

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
	void	InsertDetachedAtChildNum(M* psParent, int iChildNum, M* psChild);
	M*		InsertAtChildNum(M* psParent, int iChildNum);
	M*		InsertAtChildNum(M* psParent, int iChildNum, M* psData);

	//Insert the node at the position at the end of the path.
	void	InsertDetachedOnPath(int* aiPos, int iLevel, M* psData);
	void	InsertDetachedOnPath(int* aiPos, int iLevel, M* psData, int* aiOldPos, int iOldLevel, M* psOldData);
	M*		InsertOnPath(int* aiPos, int iLevel, int* aiOldPos, int iOldLevel, M* psOldData);
	M*		InsertOnPath(int* aiPos, int iLevel);
	M*		InsertOnPath(int* psPos, int iLevel, M* psData);

	//Traversal functions.
	M* 		StartTraversal(void);
	M* 		TraverseFrom(M* psCurrent);
	M*		TraverseSubTreeFrom(STNode* psSubRoot, M* psCurrent);
	M*		TraverseSubTreeFrom(M* psSubRoot, M* psCurrent);
	M* 		TraversePath(int* aiPos, int iLevel);

	int		GetNumChildren(M* psPos);
	M* 		GetRoot(void);
	M* 		GetDown(M* psPos);
	M* 		GetUp(M* psPos);
	M* 		GetRight(M* psPos);
	int		GetPathTo(int* aiPath, M* psNode);
	M*		GetChildNum(M* psParent, int iChildNum);
	int		GetChildNum(M* psChild);
	int		GetNodeDepth(M* psNode);

	//Tree insertion functions.
	void	InitTreeFromNode(CTreeTemplate<M>* pcTreeSource, M* psNodeSource);
void	InsertTreeOnLeftOfChildren(M* psParent, CTreeTemplate<M>* pcTree);
void	InsertTreeOnRightOfChildren(M* psParent, CTreeTemplate<M>* pcTree);
void	InsertTreeOnPath(int* aiPos, int iLevel, CTreeTemplate<M>* pcTree);

	//Comparison functions.
	int		Equals(CTreeTemplate<M>* pcTree);
	int		Equals(CTreeTemplate<M>* pcTree, int iKeyOffset, int iKeySize);

	//Insertion functions assuming a good understanding of internal tree structure.
	void	InsertDetachedOnUp(M* psPosData, M* psNewData);
	M*		InsertOnUp(M* psPos);
	M*		InsertOnUp(M* psPos, M* psNewData);

	void	InsertDetachedOnAcross(M* psPosData, M* psNewData);
	M*		InsertOnAcross(M* psPos);
	M*		InsertOnAcross(M* psPos, M* psNewData);

	void	Detach(M* psNodeData);
	void	Detach(STNode* psNodeHeader, BOOL bDetachChildren = FALSE);
	M*		AllocateDetached(void);
	void	FreeDetached(M* psNodeData);
	void	DetachTree(CTreeTemplate<M>* pcDestTree, M* psNodeData);

	BOOL	RemoveLeaf(M* psNodeData);
	int		RemoveBranch(M* psNodeData);
	void	Remove(M* psNodeData);

protected:
	void*	Malloc(size_t tSize);
	void*	Realloc(void* pv, size_t tSize);
	void	Free(void* pv);

	void	RecursiveFreeNodes(STNode* pNode);
	void	RecursiveAddNodeForCreate(CTreeTemplate<M>* pcTreeSource, M* pvData1Source, M* pvData2Dest);
	void	ReplaceLeafWithTree(CTreeTemplate<M>* pcTree, STNode* psNode);
	STNode*	PrivateFindLeftChild(STNode* psNodeHeader);
	int		CountElements();
};


#define CTreeTemplateDataGetHeader(pvData)		DataGetHeader<STNode, M>(pvData)
#define CTreeTemplateHeaderGetData(pvHeader)	HeaderGetData<STNode, M>(pvHeader)


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* CTreeTemplate<M>::Malloc(size_t tSize)
{
	return malloc(tSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::Free(void* pv)
{
	free(pv);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void* CTreeTemplate<M>::Realloc(void* pv, size_t tSize)
{
	pv = realloc(pv, tSize);
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
	InitTreeFromNode(pcTreeSrc, CTreeTemplateHeaderGetData(pcTreeSrc->mpsRoot));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::TraversePath(int* aiPos, int iLevel)
{
	STNode	*psNode;
	int		iNumAcross;
	int		i,j;

	psNode = mpsRoot;
	for (i = 0; i < iLevel; i++)
	{
		iNumAcross = aiPos[i];
		psNode = psNode->psUp;
		for (j = 0; j < iNumAcross;j++)
		{
			psNode = psNode->psAcross;
		}
	}
	return CTreeTemplateHeaderGetData(psNode);
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

	psPos = CTreeTemplateDataGetHeader(psParent);
	psNode = CTreeTemplateDataGetHeader(psChild);
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


template<class M>
M* CTreeTemplate<M>::InsertOnRightOfChildren(M* psParent)
{
	M*	psNode;

	psNode = AllocateDetached();
	InsertDetachedOnRightOfChildren(psParent, psNode);
	return psNode;
}


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

	psPos = CTreeTemplateDataGetHeader(psParent);
	psNode = CTreeTemplateDataGetHeader(psChild);
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


template<class M>
M* CTreeTemplate<M>::InsertOnLeftOfChildren(M* psParent)
{
	M *psNode;

	psNode = AllocateDetached();
	InsertDetachedOnLeftOfChildren(psParent, psNode);
	return psNode;
}


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
void CTreeTemplate<M>::InsertDetachedAtChildNum(M* psParent, int iChildNum, M* psChild)
{
	STNode* 	psPos;
	STNode*		temp;
	STNode*		psDown;
	STNode*		psNode;
	int			count;

	psPos = CTreeTemplateDataGetHeader(psParent);
	psNode = CTreeTemplateDataGetHeader(psChild);
	if (psPos->psUp!=NULL)
	{

		temp = NULL;

		if (iChildNum == 0)
		{
			InsertDetachedOnLeftOfChildren(CTreeTemplateHeaderGetData(psPos), CTreeTemplateHeaderGetData(psNode));
			return;
		}
		psDown = psPos;
		psPos = psPos->psUp;
		count = 0;
		while(iChildNum != count)
		{
			temp = psPos;
			psPos = psPos->psAcross;
			count++;
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


template<class M>
M* CTreeTemplate<M>::InsertAtChildNum(M* psParent, int iChildNum)
{
	M	*psNode;

	psNode = AllocateDetached();
	InsertDetachedAtChildNum(psParent, iChildNum, psNode);
	return psNode;
}


template<class M>
M* CTreeTemplate<M>::InsertAtChildNum(M* psParent, int iChildNum, M* psData)
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
void CTreeTemplate<M>::InsertDetachedOnPath(int* aiPos, int iLevel, M* psData)
{
	STNode*		psNode;
	M*			psThing;

	if (iLevel == 0)
	{
		//Setup the root psNode
		psNode = CTreeTemplateDataGetHeader(psData);
		mpsRoot = psNode;
		psNode->psAcross = NULL;
		psNode->psUp = NULL;
		psNode->psDown = NULL;
		miNumElements++;
	}
	else
	{
		psThing = TraversePath(aiPos, iLevel-1);
		InsertDetachedAtChildNum(psThing, aiPos[iLevel-1], psData);
	}
}

template<class M>
void CTreeTemplate<M>::InsertDetachedOnPath(int* aiPos, int iLevel, M* psData, int* aiOldPos, int iOldLevel, M* psOldData)
{
	//If we're the root node or the new path is not exactly one right of the old path.
	if ((iLevel == 0) || (iOldLevel != iLevel) || (aiPos[iLevel-1] != aiOldPos[iLevel-1]+1))
	{
		//Then just call the general method.
		InsertDetachedOnPath(aiPos, iLevel, psData);
	}
	else
	{
		InsertDetachedOnAcross(psOldData, psData);
	}
}

template<class M>
M* CTreeTemplate<M>::InsertOnPath(int* aiPos, int iLevel, int* aiOldPos, int iOldLevel, M* psOldData)
{
	M*	psNode;

	psNode = AllocateDetached();
	InsertDetachedOnPath(aiPos, iLevel, psNode, aiOldPos, iOldLevel, psOldData);
	return psNode;
}

template<class M>
M* CTreeTemplate<M>::InsertOnPath(int* aiPos, int iLevel)
{
	M*	psNode;

	psNode = AllocateDetached();
	InsertDetachedOnPath(aiPos, iLevel, psNode);
	return psNode;
}

template<class M>
M* CTreeTemplate<M>::InsertOnPath(int* psPos, int iLevel, M* psData)
{
	M*	psNode;

	psNode = InsertOnPath(psPos, iLevel);
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
	return CTreeTemplateHeaderGetData(mpsRoot);
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

	psSubrRootHeader = CTreeTemplateDataGetHeader(psSubRoot);
	return TraverseSubTreeFrom(psSubrRootHeader, psCurrent);
}


template<class M>
M* CTreeTemplate<M>::TraverseSubTreeFrom(STNode* psSubRoot, M* psCurrent)
{
	STNode*		psNode;

	psNode = CTreeTemplateDataGetHeader(psCurrent);
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
	return CTreeTemplateHeaderGetData(psNode);
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
	STNode*	psTemp;

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
int CTreeTemplate<M>::GetNumChildren(M *psParent)
{
	int		iNumChildren;
	STNode	*psTemp;
	STNode	*psPos;

	psPos = CTreeTemplateDataGetHeader(psParent);
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
	return CTreeTemplateHeaderGetData(mpsRoot);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::GetChildNum(M* psParent, int iChildNum)
{
	int			i;
	STNode*		psPos;

	psPos = CTreeTemplateDataGetHeader(psParent);
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
	return CTreeTemplateHeaderGetData(psPos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CTreeTemplate<M>::NumElements(void)
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

	psNode = CTreeTemplateDataGetHeader(psPos);
	return CTreeTemplateHeaderGetData(psNode->psDown);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::GetUp(M* psPos)
{
	STNode*	psNode;

	psNode = CTreeTemplateDataGetHeader(psPos);
	return CTreeTemplateHeaderGetData(psNode->psUp);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CTreeTemplate<M>::GetRight(M* psPos)
{
	STNode*	psNode;

	psNode = CTreeTemplateDataGetHeader(psPos);
	return CTreeTemplateHeaderGetData(psNode->psAcross);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CTreeTemplate<M>::GetNodeDepth(M* psNode)
{
	int			i;
	STNode*		psPos;

	psPos = CTreeTemplateDataGetHeader(psNode);
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
int CTreeTemplate<M>::GetChildNum(M* psChild)
{
	STNode*		psNode;
	int			iChildNum;
	STNode*		psPos;

	psPos = CTreeTemplateDataGetHeader(psChild);

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
int CTreeTemplate<M>::GetPathTo(int* aiPath, M* psData)
{
	int			iDistFromRoot;
	int			i;
	STNode*		psPos;

	iDistFromRoot = GetNodeDepth(psData);
	psPos = CTreeTemplateDataGetHeader(psData);

	for (i = iDistFromRoot-1; i >= 0; i--)
	{
		aiPath[i] = GetChildNum(CTreeTemplateHeaderGetData(psPos));
		psPos = psPos->psDown;
	}

	return iDistFromRoot;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CTreeTemplate<M>::ByteSize()
{
	return (sizeof(M) + sizeof(STNode)) * miNumElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CTreeTemplate<M>::Equals(CTreeTemplate<M>* pcTree)
{
	return Equals(pcTree, 0, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CTreeTemplate<M>::Equals(CTreeTemplate<M>* pcTree, int iKeyOffset, int iKeySize)
{
	M*			pData1;
	M*			pData2;
	int			iDepth1;
	int			iDepth2;
	int			iNum1;
	int			iNum2;

	if (pcTree->miNumElements == miNumElements)
	{
		pData1 = StartTraversal();
		pData2 = pcTree->StartTraversal();

		while ((pData1 != NULL) && (pData2 != NULL))
		{
			//Check the elements are the same.
			if (memcmp((M*)((ENGINE_SIZE_T) pData1 + (ENGINE_SIZE_T) iKeyOffset), (M*)((ENGINE_SIZE_T) pData2 + (ENGINE_SIZE_T) iKeyOffset), iKeySize) != 0)
			{
				return 0;
			}

			//Check the topology of the trees are the same.
			iDepth1 = GetNodeDepth(pData1);
			iDepth2 = pcTree->GetNodeDepth(pData2);
			iNum1 = GetChildNum(pData1);
			iNum2 = pcTree->GetChildNum(pData2);
			if ((iDepth1 != iDepth2) || (iNum1 != iNum2))
			{
				return 0;
			}

			pData1 = TraverseFrom(pData1);
			pData2 = pcTree->TraverseFrom(pData2);
		}

		if ((pData1 == NULL) && (pData2 == NULL))
		{
			return 1;
		}
	}
	return 0;
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

	pNode = CTreeTemplateDataGetHeader(psNodeSource);
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
	int			iNumNodes;
	int			i;
	M*			pvChildData1;
	M*			pvChildData2;

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

	psPos = CTreeTemplateDataGetHeader(psPosData);
	psNew = CTreeTemplateDataGetHeader(psNewData);

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

	psPos = CTreeTemplateDataGetHeader(psPosData);
	psNew = CTreeTemplateDataGetHeader(psNewData);

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

	psNodeHeader = CTreeTemplateDataGetHeader(psNodeData);
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
	return CTreeTemplateHeaderGetData(psNode);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::Detach(STNode* psNodeHeader, BOOL bDetachChildren)
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
		psNodeHeader = CTreeTemplateDataGetHeader(psNodeData);
		Detach(psNodeHeader);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CTreeTemplate<M>::RemoveLeaf(M* psNodeData)
{
	STNode*		psNodeHeader;

	psNodeHeader = CTreeTemplateDataGetHeader(psNodeData);
	if ((psNodeData == NULL) || (psNodeHeader->psUp))
	{
		//This wasn't a leaf so we can't detach it.
		return FALSE;
	}
	Detach(psNodeHeader);
	Free(psNodeHeader);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CTreeTemplate<M>::RemoveBranch(M* psNodeData)
{
	CTreeTemplate<M>	cTree;
	int					iNum;

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

	psNodeHeader = CTreeTemplateDataGetHeader(psNodeData);
	Detach(psNodeHeader, TRUE);

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
int CTreeTemplate<M>::CountElements()
{
	M*		pcNode;
	int		iNumElements;

	pcNode = StartTraversal();
	iNumElements = 0;
	while (pcNode)
	{
		iNumElements ++;
		pcNode = TraverseFrom(pcNode);
	}
	return iNumElements;
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
void CTreeTemplate<M>::ReplaceLeafWithTree(CTreeTemplate<M>* pcTree, STNode* psNode)
{
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::InsertTreeOnLeftOfChildren(M* psParent, CTreeTemplate<M>* pcTree)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::InsertTreeOnRightOfChildren(M* psParent, CTreeTemplate<M>* pcTree)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CTreeTemplate<M>::InsertTreeOnPath(int* aiPos, int iLevel, CTreeTemplate<M>* pcTree)
{
}


#endif //__TREE_TEMPLATE_H__

