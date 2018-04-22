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
#ifndef __ARRAY_DENSE_TEMPLATE_H__
#define __ARRAY_DENSE_TEMPLATE_H__
#include <stdlib.h>
#include "Define.h"
#include "FastMemset.h"
#include "FreeList.h"
#include "FreeList.h"
#include "PointerFunctions.h"


#define DENSE_NODE_LEFT_COUNT_MASK	0x3fffffff
#define DENSE_NODE_LEFT_ELEMENT		0x40000000
#define DENSE_NODE_RIGHT_ELEMENT	0x80000000


struct SDenseNode
{
	unsigned int	iFlags;
	SDenseNode*		psParent;
	SDenseNode*		psLeftNode;  //NULL must always be marked as DENSE_NODE_LEFT_ELEMENT
	SDenseNode*		psRightNode; //NULL must always be marked as DENSE_NODE_RIGHT_ELEMENT

	void 			Set(int iLeftCount, BOOL bLeftElement, BOOL bRightElement);
	int  			GetLeftCount(void);
	void  			SetLeftCount(int iLeftCount);
	SDenseNode*		GetRight(void);
	SDenseNode*		GetLeft(void);
	SDenseNode**	GetParentPointer(void);
	BOOL			IsLeftElement(void);
	BOOL			IsRightElement(void);
	void 			SetRightElement(BOOL bRightElement);
	void 			SetLeftElement(BOOL bLeftElement);
};


template<class M>
class __CArrayDenseTemplate
{
private:
	CFreeList		mcDenseNodes;
	CFreeList		mcElementNodes;
	SDenseNode*		mpsRoot;
	int				miUsedElements;
	int				miUsedNodes;

public:
	void			Init(unsigned short iElementSize);
	void			Kill(void);
	M*				Add(void);
	M*				Add(M* pvData);
	M* 				AddGetIndex(int* piPos);
	void			Set(int iElementPos, M* pvData);
	BOOL			SafeSet(int iElementPos, M* pvData);
	M* 				InsertAt(int iElementPos);
	M*				InsertAt(M* pvData, int iElementPos);
	void 			RemoveAt(int iElementPos);
	void			RemoveRange(int iStartPos, int iEndPos);
	void 			RemoveTail(void);
	M*				Get(int iElementPos);
	M*				SafeGet(int iElementPos);
	int				NumUsedElements(void);;
	int				NumUsedNodes(void);
	SDenseNode*		RotateLeft(SDenseNode* psNode);
	SDenseNode*		RotateRight(SDenseNode* psNode);
	void			Rebalance(void);
	void			RecurseRebalance(SDenseNode* psNode, int iNumElements);
	void			RecurseRebalanceChildren(SDenseNode* psNode, int iNumElements);
	void			IncreaseLeftCounts(SDenseNode* psNode);
	void			DecreaseLeftCounts(SDenseNode* psNode);
	SDenseNode*		TestGetRoot(void);
	BOOL			TestStructure(void);

protected:
	void			Search(int iElementPos, SDenseNode** ppsNode, BOOL* pbLeft, BOOL* pbInsertionLeft);
	SDenseNode*		PrivateAddNode(void);
	M*				PrivateAddElement(void);
	void			PrivateRemoveElement(M* psElement);
	void			PrivateRemoveNode(SDenseNode* psNode);
	void			Dump(void);
	void			RecurseDump(int iDepth, SDenseNode* psNode, CChars* psz, BOOL bLeft);
	void			RecurseDumpElement(int iDepth, M* psElement, CChars* psz, BOOL bLeft);
	BOOL			RecurseTestStructure1(SDenseNode* psNode);
	BOOL			RecurseTestStructure2(SDenseNode* psNode);
};


template<class M>
class CArrayDenseTemplate : public __CArrayDenseTemplate<M>
{
public:
	void Init(void);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDenseNode::Set(int iLeftCount, BOOL bLeftElement, BOOL bRightElement)
{
	iFlags = iLeftCount;
	if (bLeftElement)
	{
		iFlags |= DENSE_NODE_LEFT_ELEMENT;
	}
	if (bRightElement)
	{
		iFlags |= DENSE_NODE_RIGHT_ELEMENT;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SDenseNode::GetLeftCount(void)
{
	return iFlags & DENSE_NODE_LEFT_COUNT_MASK;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDenseNode::SetLeftCount(int iLeftCount)
{
	iFlags &= (DENSE_NODE_LEFT_ELEMENT | DENSE_NODE_RIGHT_ELEMENT);
	iFlags |= iLeftCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SDenseNode::IsLeftElement(void)
{
	return iFlags & DENSE_NODE_LEFT_ELEMENT;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL SDenseNode::IsRightElement(void)
{
	return iFlags & DENSE_NODE_RIGHT_ELEMENT;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDenseNode::SetRightElement(BOOL bRightElement)
{
	SetFlag(&iFlags, DENSE_NODE_RIGHT_ELEMENT, bRightElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDenseNode::SetLeftElement(BOOL bLeftElement)
{
	SetFlag(&iFlags, DENSE_NODE_LEFT_ELEMENT, bLeftElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDenseNode* SDenseNode::GetRight(void)
{
	return psRightNode;  //Or psNode... it's all the same, ask the node what you got back.
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDenseNode** SDenseNode::GetParentPointer(void)
{
	if (psParent)
	{
		if (psParent->psLeftNode == this)
		{
			return &psParent->psLeftNode;
		}
		else if (psParent->psRightNode == this)
		{
			return &psParent->psRightNode;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SDenseNode* SDenseNode::GetLeft(void)
{
	return psLeftNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::Init(unsigned short iElementSize)
{
	mcDenseNodes.Init(sizeof(SDenseNode));
	mcElementNodes.Init(iElementSize);
	mpsRoot = NULL;
	miUsedElements = 0;
	miUsedNodes = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::Kill(void)
{
	mcElementNodes.Kill();
	mcDenseNodes.Kill();
	miUsedElements = 0;
	miUsedNodes = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::Add(void)
{
	return InsertAt(miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::Add(M* pvData)
{
	M*	psElement;

	psElement = Add();
	memcpy(psElement, pvData, mcElementNodes.miElementSize);
	return psElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::AddGetIndex(int* piPos)
{
	M*	psElement;

	psElement = Add();
	*piPos = miUsedElements-1;
	return psElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::Set(int iElementPos, M* pvData)
{
	SafeSet(iElementPos, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayDenseTemplate<M>::SafeSet(int iElementPos, M* pvData)
{
	if ((iElementPos >= 0) && (iElementPos < miUsedElements))
	{
		M*	psElement;

		psElement = Get(iElementPos);
		memcpy(psElement, pvData, mcElementNodes.miElementSize);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SDenseNode* __CArrayDenseTemplate<M>::PrivateAddNode(void)
{
	SDenseNode*		psNode;

	psNode = (SDenseNode*)mcDenseNodes.Add();
	miUsedNodes++;
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::PrivateAddElement(void)
{
	M*				psElement;

	psElement = (M*)mcElementNodes.Add();
	miUsedElements++;
	return psElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::PrivateRemoveElement(M* psElement)
{
	if (mcElementNodes.Remove(psElement))
	{
		miUsedElements--;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::PrivateRemoveNode(SDenseNode* psNode)
{
	if (mcDenseNodes.Remove(psNode))
	{
		miUsedNodes--;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::Search(int iElementPos, SDenseNode** ppsNode, BOOL* pbLeft, BOOL* pbInsertionLeft)
{
	SDenseNode*		psNode;
	int				iCount;
	int				iLefts;

	if (miUsedElements > 0)
	{
		psNode = mpsRoot;
		iCount = iElementPos;
		for (;;)
		{
			iLefts = psNode->GetLeftCount();
			if (iCount < iLefts)
			{
				if (psNode->IsLeftElement())
				{
					*ppsNode = psNode;
					*pbLeft = TRUE;
					return;
				}
				psNode = psNode->GetLeft();

			}
			else
			{
				iCount -= iLefts;
				if (psNode->IsRightElement())
				{
					*ppsNode = psNode;
					*pbLeft = FALSE;
					if (iCount == 0)
					{
						*pbInsertionLeft = TRUE;
					}
					else
					{
						*pbInsertionLeft = FALSE;
					}
					return;
				}
				psNode = psNode->GetRight();
			}
		}
	}
	else if (miUsedElements == 0)
	{
		*ppsNode = NULL;
		*pbLeft = TRUE;
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::IncreaseLeftCounts(SDenseNode* psNode)
{
	for (;;)
	{
		if (!psNode->psParent)
		{
			return;
		}
		if (psNode->psParent->psLeftNode == psNode)
		{
			psNode->psParent->iFlags++;  //This is safe although confusing.
		}
		psNode = psNode->psParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::DecreaseLeftCounts(SDenseNode* psNode)
{
	for (;;)
	{
		if (!psNode->psParent)
		{
			return;
		}
		if (psNode->psParent->psLeftNode == psNode)
		{
			psNode->psParent->iFlags--;  //Yup, this is safe although a bit confusing.
		}
		psNode = psNode->psParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SDenseNode* __CArrayDenseTemplate<M>::TestGetRoot(void)
{
	return mpsRoot;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayDenseTemplate<M>::NumUsedElements(void)
{
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int __CArrayDenseTemplate<M>::NumUsedNodes(void)
{
	return miUsedNodes;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::InsertAt(int iElementPos)
{
	SDenseNode*		psNode;
	SDenseNode*		psNewNode;
	M*				psElement;
	BOOL			bLeft;
	BOOL			bInsertionLeft;

	if ((iElementPos < 0) || (iElementPos > miUsedElements))
	{
		return NULL;
	}

	Search(iElementPos, &psNode, &bLeft, &bInsertionLeft);
	psElement = PrivateAddElement();

	if (miUsedElements > 2)
	{
		psNewNode = PrivateAddNode();
		psNewNode->Set(1, TRUE, TRUE);

		if (bLeft)
		{
			psNewNode->psLeftNode = (SDenseNode*)psElement;
			psNewNode->psRightNode = psNode->psLeftNode;
			psNewNode->psParent = psNode;
			psNode->psLeftNode = psNewNode;
			psNode->SetLeftElement(FALSE);
		}
		else
		{
			if (bInsertionLeft)
			{
				psNewNode->psLeftNode = (SDenseNode*)psElement;
				psNewNode->psRightNode = psNode->psRightNode;
				psNewNode->psParent = psNode;
				psNode->psRightNode = psNewNode;
				psNode->SetRightElement(FALSE);
			}
			else
			{
				psNewNode->psLeftNode = psNode->psRightNode;
				psNewNode->psRightNode = (SDenseNode*)psElement;
				psNewNode->psParent = psNode;
				psNode->psRightNode = psNewNode;
				psNode->SetRightElement(FALSE);
			}
		}
		IncreaseLeftCounts(psNewNode);
	}
	else if (miUsedElements == 1)
	{
		mpsRoot = PrivateAddNode();
		mpsRoot->psParent = NULL;
		mpsRoot->psLeftNode = (SDenseNode*)psElement;
		mpsRoot->psRightNode = NULL;
		mpsRoot->Set(1, TRUE, TRUE);
	}
	else if (miUsedElements == 2)
	{
		if (iElementPos == 1)
		{
			mpsRoot->psRightNode = (SDenseNode*)psElement;
		}
		else if (iElementPos == 0)
		{
			mpsRoot->psRightNode = mpsRoot->psLeftNode;
			mpsRoot->psLeftNode = (SDenseNode*)psElement;
		}
	}
	Rebalance();
	return psElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::InsertAt(M* pvData, int iElementPos)
{
	M*	psElement;

	psElement = InsertAt(iElementPos);
	memcpy(psElement, pvData, mcElementNodes.miElementSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::RemoveAt(int iElementPos)
{
	SDenseNode*		psRemovedNode;
	M*				psElement;
	BOOL			bLeft;
	BOOL			bRemovalLeft;

	if ((iElementPos < 0) || (iElementPos >= miUsedElements))
	{
		return;
	}

	Search(iElementPos, &psRemovedNode, &bLeft, &bRemovalLeft);
	if (bLeft)
	{
		psElement = (M*)psRemovedNode->GetLeft();
	}
	else
	{
		psElement = (M*)psRemovedNode->GetRight();
	}
	PrivateRemoveElement(psElement);

	if (miUsedElements > 1)
	{
		DecreaseLeftCounts(psRemovedNode);

		if (psRemovedNode != mpsRoot)
		{
			if (bLeft)
			{
				if (psRemovedNode->psParent->psLeftNode == psRemovedNode)
				{
					psRemovedNode->psParent->psLeftNode = psRemovedNode->psRightNode;
					psRemovedNode->psParent->SetLeftElement(psRemovedNode->IsRightElement());
				}
				else
				{
					psRemovedNode->psParent->psRightNode = psRemovedNode->psRightNode;
					psRemovedNode->psParent->SetRightElement(psRemovedNode->IsRightElement());
				}
			}
			else
			{
				if (psRemovedNode->psParent->psLeftNode == psRemovedNode)
				{

					psRemovedNode->psParent->psLeftNode = psRemovedNode->psLeftNode;
					psRemovedNode->psParent->SetLeftElement(psRemovedNode->IsLeftElement());
				}
				else
				{
					psRemovedNode->psParent->psRightNode = psRemovedNode->psLeftNode;
					psRemovedNode->psParent->SetRightElement(psRemovedNode->IsLeftElement());
				}
			}
		}
		else
		{
			if (bLeft)
			{
				mpsRoot = mpsRoot->GetRight();
			}
			else
			{
				mpsRoot = mpsRoot->GetLeft();
			}
		}

		PrivateRemoveNode(psRemovedNode);
	}
	else if (miUsedElements == 0)
	{
		PrivateRemoveNode(mpsRoot);
		mpsRoot = NULL;
		return;
	}
	else if (miUsedElements == 1)
	{
		if (iElementPos == 1)
		{
			mpsRoot->psRightNode = NULL;
		}
		else if (iElementPos == 0)
		{
			mpsRoot->psLeftNode = mpsRoot->psRightNode;
			mpsRoot->psRightNode = NULL;
		}
	}
	Rebalance();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::RemoveRange(int iStartPos, int iEndPos)
{
	int	i;

	//There's a more optimal way of doing this...
	for (i = iEndPos-1; i >= iStartPos; i--)
	{
		RemoveAt(i);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::RemoveTail(void)
{
	RemoveAt(miUsedElements-1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::Get(int iElementPos)
{
	return SafeGet(iElementPos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::SafeGet(int iElementPos)
{
	SDenseNode*		psNode;
	BOOL			bLeft;
	BOOL			bIgnored;

	if ((iElementPos < 0) || (iElementPos >= miUsedElements))
	{
		return NULL;
	}

	Search(iElementPos, &psNode, &bLeft, &bIgnored);
	if (bLeft)
	{
		return (M*)psNode->GetLeft();
	}
	else
	{
		return (M*)psNode->GetRight();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SDenseNode* __CArrayDenseTemplate<M>::RotateLeft(SDenseNode* psNode)
{
	SDenseNode*		psRight;
	SDenseNode*		psRightsLeft;
	SDenseNode**	ppsNodePointer;
	BOOL			bRightsLeftElement;

	if (psNode->IsRightElement())
	{
		//Can't Rotate left if the right not has no children.
		return NULL;
	}

	psRight = psNode->GetRight();
	if (psRight)
	{
		ppsNodePointer = psNode->GetParentPointer();
		psRightsLeft = psRight->GetLeft();
		bRightsLeftElement = psRight->IsLeftElement();

		//Set the parent node to point to right instead of node.
		if (ppsNodePointer)
		{
			*ppsNodePointer = psRight;
		}
		else
		{
			mpsRoot = psRight;
		}

		psRight->psParent = psNode->psParent;
		psNode->psParent = psRight;
		psRight->psLeftNode = psNode;
		psNode->psRightNode = psRightsLeft;
		if (!bRightsLeftElement)
		{
			psRightsLeft->psParent = psNode;
		}

		psNode->SetRightElement(bRightsLeftElement);
		psRight->SetLeftElement(FALSE);
		psRight->SetLeftCount(psRight->GetLeftCount() + psNode->GetLeftCount());
		return psRight;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
SDenseNode* __CArrayDenseTemplate<M>::RotateRight(SDenseNode* psNode)
{
	SDenseNode*		psLeft;
	SDenseNode*		psLeftsRight;
	SDenseNode**	ppsNodePointer;
	BOOL			bLeftsRightElement;

	if (psNode->IsLeftElement())
	{
		//Can't Rotate right if the left not has no children.
		return NULL;
	}

	psLeft = psNode->GetLeft();
	if (psLeft)
	{
		ppsNodePointer = psNode->GetParentPointer();
		psLeftsRight = psLeft->GetRight();
		bLeftsRightElement = psLeft->IsRightElement();

		//Set the parent node to point to right instead of node.
		if (ppsNodePointer)
		{
			*ppsNodePointer = psLeft;
		}
		else
		{
			mpsRoot = psLeft;
		}

		psLeft->psParent = psNode->psParent;
		psNode->psParent = psLeft;
		psLeft->psRightNode = psNode;
		psNode->psLeftNode = psLeftsRight;
		if (!bLeftsRightElement)
		{
			psLeftsRight->psParent = psNode;
		}

		psNode->SetLeftElement(bLeftsRightElement);
		psLeft->SetRightElement(FALSE);
		psNode->SetLeftCount(psNode->GetLeftCount() - psLeft->GetLeftCount());
		return psLeft;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::Rebalance(void)
{
	RecurseRebalance(mpsRoot, miUsedElements);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::RecurseRebalance(SDenseNode* psNode, int iNumElements)
{
	if (psNode->GetLeftCount() < (iNumElements / 2))
	{
		psNode = RotateLeft(psNode);
		RecurseRebalanceChildren(psNode, iNumElements);
	}
	else if (psNode->GetLeftCount() > (iNumElements / 4))
	{
		psNode = RotateRight(psNode);
		RecurseRebalanceChildren(psNode, iNumElements);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::RecurseRebalanceChildren(SDenseNode* psNode, int iNumElements)
{
	if (psNode)
	{
		if (!psNode->IsLeftElement())
		{
			RecurseRebalance(psNode->GetLeft(), psNode->GetLeftCount());
		}
		if (!psNode->IsRightElement())
		{
			RecurseRebalance(psNode->GetRight(), iNumElements - psNode->GetLeftCount());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayDenseTemplate<M>::TestStructure(void)
{
	BOOL	bResult;

	bResult =  RecurseTestStructure1(mpsRoot);
	if (!bResult)
	{
		return FALSE;
	}
	return RecurseTestStructure2(mpsRoot);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayDenseTemplate<M>::RecurseTestStructure1(SDenseNode* psNode)
{
	BOOL	bResult;

	if (!psNode->IsLeftElement())
	{
		if (psNode->GetLeft()->psParent != psNode)
		{
			return FALSE;
		}
		else
		{
			bResult = RecurseTestStructure1(psNode->GetLeft());
			if (!bResult)
			{
				return FALSE;
			}
		}
	}
	else
	{
		return TRUE;
	}

	if (!psNode->IsRightElement())
	{
		if (psNode->GetRight()->psParent != psNode)
		{
			return FALSE;
		}
		else
		{
			bResult =  RecurseTestStructure1(psNode->GetRight());
			if (!bResult)
			{
				return FALSE;
			}
		}
	}
	else
	{
		return TRUE;
	}
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL __CArrayDenseTemplate<M>::RecurseTestStructure2(SDenseNode* psNode)
{
	BOOL	bResult;

	if (psNode->psParent)
	{
		if (!((psNode->psParent->GetLeft() == psNode) || (psNode->psParent->GetRight() == psNode)))
		{
			return FALSE;
		}
	}

	if (!psNode->IsLeftElement())
	{
		bResult = RecurseTestStructure2(psNode->GetLeft());
		if (!bResult)
		{
			return FALSE;
		}
	}
	if (!psNode->IsRightElement())
	{
		bResult = RecurseTestStructure2(psNode->GetRight());
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::Dump(void)
{
	CChars	sz;

	sz.Init();
	sz.Append(">>>>>>>>>\n");

	RecurseDump(0, mpsRoot, &sz, TRUE);

	sz.Append("<<<<<<<<<\n");
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::RecurseDump(int iDepth, SDenseNode* psNode, CChars* psz, BOOL bLeft)
{
	if (psNode)
	{
		if (bLeft)
		{
			psz->Append("L ");
		}
		else
		{
			psz->Append("R ");
		}
		psz->Append(' ', iDepth*2);
		psz->Append("(");
		psz->Append(psNode->GetLeftCount());
		psz->Append(")\n");
		if (!psNode->IsLeftElement())
		{
			RecurseDump(iDepth+1, psNode->GetLeft(), psz, TRUE);
		}
		else
		{
			RecurseDumpElement(iDepth+1, (M*)psNode->GetLeft(), psz, TRUE);
		}
		if (!psNode->IsRightElement())
		{
			RecurseDump(iDepth+1, psNode->GetRight(), psz, FALSE);
		}
		else
		{
			RecurseDumpElement(iDepth+1, (M*)psNode->GetRight(), psz, FALSE);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::RecurseDumpElement(int iDepth, M* psElement, CChars* psz, BOOL bLeft)
{
	if (bLeft)
	{
		psz->Append("L ");
	}
	else
	{
		psz->Append("R ");
	}
	psz->Append(' ', iDepth*2);
	if (psElement)
	{
		psz->Append("*");
		psz->Append(*((int*)psElement));
		psz->Append("*\n");
	}
	else
	{
		psz->Append("*void*\n");
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayDenseTemplate<M>::Init(void)
{
	__CArrayDenseTemplate<M>::Init(sizeof(M));
}


#endif // __ARRAY_DENSE_TEMPLATE_H__

