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
	uint32			iFlags;
	SDenseNode*		psParent;
	SDenseNode*		psLeftNode;  //NULL must always be marked as DENSE_NODE_LEFT_ELEMENT
	SDenseNode*		psRightNode; //NULL must always be marked as DENSE_NODE_RIGHT_ELEMENT

	void 			Set(size iLeftCount, bool bLeftElement, bool bRightElement);
	size  			GetLeftCount(void);
	void  			SetLeftCount(size iLeftCount);
	SDenseNode*		GetRight(void);
	SDenseNode*		GetLeft(void);
	SDenseNode**	GetParentPointer(void);
	bool			IsLeftElement(void);
	bool			IsRightElement(void);
	void 			SetRightElement(bool bRightElement);
	void 			SetLeftElement(bool bLeftElement);
};


template<class M>
class __CArrayDenseTemplate
{
private:
	CFreeList		mcDenseNodes;
	CFreeList		mcElementNodes;
	SDenseNode*		mpsRoot;
	size			miUsedElements;
	size			miUsedNodes;

public:
	void			Init(size iElementSize);
	void			Kill(void);
	M*				Add(void);
	M*				Add(M* pvData);
	M* 				AddGetIndex(size* piPos);
	void			Set(size iElementPos, M* pvData);
	bool			SafeSet(size iElementPos, M* pvData);
	M* 				InsertAt(size iElementPos);
	M*				InsertAt(M* pvData, size iElementPos);
	void 			RemoveAt(size iElementPos);
	void			RemoveRange(size iStartPos, size iEndPos);
	void 			RemoveTail(void);
	M*				Get(size iElementPos);
	M*				SafeGet(size iElementPos);
	size			NumUsedElements(void);;
	size			NumUsedNodes(void);
	SDenseNode*		RotateLeft(SDenseNode* psNode);
	SDenseNode*		RotateRight(SDenseNode* psNode);
	void			Rebalance(void);
	void			RecurseRebalance(SDenseNode* psNode, size iNumElements);
	void			RecurseRebalanceChildren(SDenseNode* psNode, size iNumElements);
	void			IncreaseLeftCounts(SDenseNode* psNode);
	void			DecreaseLeftCounts(SDenseNode* psNode);
	SDenseNode*		TestGetRoot(void);
	bool			TestStructure(void);

protected:
	void			Search(size iElementPos, SDenseNode** ppsNode, bool* pbLeft, bool* pbInsertionLeft);
	SDenseNode*		PrivateAddNode(void);
	M*				PrivateAddElement(void);
	void			PrivateRemoveElement(M* psElement);
	void			PrivateRemoveNode(SDenseNode* psNode);
	void			Dump(void);
	void			RecurseDump(size iDepth, SDenseNode* psNode, CChars* psz, bool bLeft);
	void			RecurseDumpElement(size iDepth, M* psElement, CChars* psz, bool bLeft);
	bool			RecurseTestStructure1(SDenseNode* psNode);
	bool			RecurseTestStructure2(SDenseNode* psNode);
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
void SDenseNode::Set(size iLeftCount, bool bLeftElement, bool bRightElement)
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
size SDenseNode::GetLeftCount(void)
{
	return iFlags & DENSE_NODE_LEFT_COUNT_MASK;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDenseNode::SetLeftCount(size iLeftCount)
{
	iFlags &= (DENSE_NODE_LEFT_ELEMENT | DENSE_NODE_RIGHT_ELEMENT);
	iFlags |= iLeftCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SDenseNode::IsLeftElement(void)
{
	return iFlags & DENSE_NODE_LEFT_ELEMENT;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SDenseNode::IsRightElement(void)
{
	return iFlags & DENSE_NODE_RIGHT_ELEMENT;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDenseNode::SetRightElement(bool bRightElement)
{
	SetFlagInt(&iFlags, DENSE_NODE_RIGHT_ELEMENT, bRightElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SDenseNode::SetLeftElement(bool bLeftElement)
{
	SetFlagInt(&iFlags, DENSE_NODE_LEFT_ELEMENT, bLeftElement);
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
void __CArrayDenseTemplate<M>::Init(size iElementSize)
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
M* __CArrayDenseTemplate<M>::AddGetIndex(size* piPos)
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
void __CArrayDenseTemplate<M>::Set(size iElementPos, M* pvData)
{
	SafeSet(iElementPos, pvData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool __CArrayDenseTemplate<M>::SafeSet(size iElementPos, M* pvData)
{
	if (iElementPos < miUsedElements)
	{
		M*	psElement;

		psElement = Get(iElementPos);
		memcpy(psElement, pvData, mcElementNodes.miElementSize);
		return true;
	}
	return false;
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
void __CArrayDenseTemplate<M>::Search(size iElementPos, SDenseNode** ppsNode, bool* pbLeft, bool* pbInsertionLeft)
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
					*pbLeft = true;
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
					*pbLeft = false;
					if (iCount == 0)
					{
						*pbInsertionLeft = true;
					}
					else
					{
						*pbInsertionLeft = false;
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
		*pbLeft = true;
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
size __CArrayDenseTemplate<M>::NumUsedElements(void)
{
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size __CArrayDenseTemplate<M>::NumUsedNodes(void)
{
	return miUsedNodes;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::InsertAt(size iElementPos)
{
	SDenseNode*		psNode;
	SDenseNode*		psNewNode;
	M*				psElement;
	bool			bLeft;
	bool			bInsertionLeft;

	if (iElementPos > miUsedElements)
	{
		return NULL;
	}

	Search(iElementPos, &psNode, &bLeft, &bInsertionLeft);
	psElement = PrivateAddElement();

	if (miUsedElements > 2)
	{
		psNewNode = PrivateAddNode();
		psNewNode->Set(1, true, true);

		if (bLeft)
		{
			psNewNode->psLeftNode = (SDenseNode*)psElement;
			psNewNode->psRightNode = psNode->psLeftNode;
			psNewNode->psParent = psNode;
			psNode->psLeftNode = psNewNode;
			psNode->SetLeftElement(false);
		}
		else
		{
			if (bInsertionLeft)
			{
				psNewNode->psLeftNode = (SDenseNode*)psElement;
				psNewNode->psRightNode = psNode->psRightNode;
				psNewNode->psParent = psNode;
				psNode->psRightNode = psNewNode;
				psNode->SetRightElement(false);
			}
			else
			{
				psNewNode->psLeftNode = psNode->psRightNode;
				psNewNode->psRightNode = (SDenseNode*)psElement;
				psNewNode->psParent = psNode;
				psNode->psRightNode = psNewNode;
				psNode->SetRightElement(false);
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
		mpsRoot->Set(1, true, true);
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
M* __CArrayDenseTemplate<M>::InsertAt(M* pvData, size iElementPos)
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
void __CArrayDenseTemplate<M>::RemoveAt(size iElementPos)
{
	SDenseNode*		psRemovedNode;
	M*				psElement;
	bool			bLeft;
	bool			bRemovalLeft;

	if (iElementPos >= miUsedElements)
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
void __CArrayDenseTemplate<M>::RemoveRange(size iStartPos, size iEndPos)
{
	size	i;

	//There's a more optimal way of doing this...
	i = iEndPos;
	if (i != 0)
	{
		do
		{
			i--;
			RemoveAt(i);
		}
		while (i != iStartPos);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::RemoveTail(void)
{
	RemoveAt(miUsedElements - 1);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::Get(size iElementPos)
{
	return SafeGet(iElementPos);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* __CArrayDenseTemplate<M>::SafeGet(size iElementPos)
{
	SDenseNode*		psNode;
	bool			bLeft;
	bool			bIgnored;

	if (iElementPos >= miUsedElements)
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
	bool			bRightsLeftElement;

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
		psRight->SetLeftElement(false);
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
	bool			bLeftsRightElement;

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
		psLeft->SetRightElement(false);
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
void __CArrayDenseTemplate<M>::RecurseRebalance(SDenseNode* psNode, size iNumElements)
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
void __CArrayDenseTemplate<M>::RecurseRebalanceChildren(SDenseNode* psNode, size iNumElements)
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
bool __CArrayDenseTemplate<M>::TestStructure(void)
{
	bool	bResult;

	bResult =  RecurseTestStructure1(mpsRoot);
	if (!bResult)
	{
		return false;
	}
	return RecurseTestStructure2(mpsRoot);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool __CArrayDenseTemplate<M>::RecurseTestStructure1(SDenseNode* psNode)
{
	bool	bResult;

	if (!psNode->IsLeftElement())
	{
		if (psNode->GetLeft()->psParent != psNode)
		{
			return false;
		}
		else
		{
			bResult = RecurseTestStructure1(psNode->GetLeft());
			if (!bResult)
			{
				return false;
			}
		}
	}
	else
	{
		return true;
	}

	if (!psNode->IsRightElement())
	{
		if (psNode->GetRight()->psParent != psNode)
		{
			return false;
		}
		else
		{
			bResult =  RecurseTestStructure1(psNode->GetRight());
			if (!bResult)
			{
				return false;
			}
		}
	}
	else
	{
		return true;
	}
	return true;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool __CArrayDenseTemplate<M>::RecurseTestStructure2(SDenseNode* psNode)
{
	bool	bResult;

	if (psNode->psParent)
	{
		if (!((psNode->psParent->GetLeft() == psNode) || (psNode->psParent->GetRight() == psNode)))
		{
			return false;
		}
	}

	if (!psNode->IsLeftElement())
	{
		bResult = RecurseTestStructure2(psNode->GetLeft());
		if (!bResult)
		{
			return false;
		}
	}
	if (!psNode->IsRightElement())
	{
		bResult = RecurseTestStructure2(psNode->GetRight());
		if (!bResult)
		{
			return false;
		}
	}
	return true;
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

	RecurseDump(0, mpsRoot, &sz, true);

	sz.Append("<<<<<<<<<\n");
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::RecurseDump(size iDepth, SDenseNode* psNode, CChars* psz, bool bLeft)
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
			RecurseDump(iDepth+1, psNode->GetLeft(), psz, true);
		}
		else
		{
			RecurseDumpElement(iDepth+1, (M*)psNode->GetLeft(), psz, true);
		}
		if (!psNode->IsRightElement())
		{
			RecurseDump(iDepth+1, psNode->GetRight(), psz, false);
		}
		else
		{
			RecurseDumpElement(iDepth+1, (M*)psNode->GetRight(), psz, false);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void __CArrayDenseTemplate<M>::RecurseDumpElement(size iDepth, M* psElement, CChars* psz, bool bLeft)
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
		psz->Append(*((size*)psElement));
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

