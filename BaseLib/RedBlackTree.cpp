/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include <string.h>
#include "PointerRemapper.h"
#include "RedBlackTree.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SRedBlackNode* SRedBlackNode::GetNext(void)
{
	SRedBlackNode*	psNode;

	if (psRight)
	{
		return psRight->GetLeftmost();
	}

	psNode = this;
	while (psNode->psParent && (psNode == psNode->psParent->psRight))
	{
		psNode = psNode->psParent;
	}
	return psNode->psParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SRedBlackNode* SRedBlackNode::GetPrevious(void)
{
	SRedBlackNode*	psNode;

	if (psLeft)
	{
		return psLeft->GetRightmost();
	}

	psNode = this;
	while (psNode->psParent && (psNode == psNode->psParent->psLeft))
	{
		psNode = psNode->psParent;
	}
	return psNode->psParent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SRedBlackNode* SRedBlackNode::GetLeftmost(void)
{
	SRedBlackNode*	psNode;

	psNode = this;
	while (psNode->psLeft)
	{
		psNode = psNode->psLeft;
	}
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SRedBlackNode* SRedBlackNode::GetRightmost(void)
{
	SRedBlackNode*	psNode;

	psNode = this;
	while (psNode->psRight)
	{
		psNode = psNode->psRight;
	}
	return psNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsRed(SRedBlackNode* psNode)
{
	return psNode && (psNode->iColour == RED_BLACK_RED);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsBlack(SRedBlackNode* psNode)
{
	return !IsRed(psNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::Init(size iElementSize, size iKeyOffset, DataCompare fCompare)
{
	mcNodes.Init(sizeof(SRedBlackNode) + iElementSize, 8);
	mpsRoot = NULL;
	mfCompare = fCompare;
	miKeyOffset = iKeyOffset;
	miElementSize = iElementSize;
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::Kill(void)
{
	mcNodes.Kill();
	mpsRoot = NULL;
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::Add(void* pvElement)
{
	SRedBlackNode*	psNode;
	SRedBlackNode*	psParent;
	void*			pvKey;
	int				iCompare;

	pvKey = RemapSinglePointer(pvElement, miKeyOffset);
	psParent = NULL;
	iCompare = 0;
	psNode = mpsRoot;
	while (psNode)
	{
		iCompare = mfCompare(pvKey, GetKey(psNode));
		if (iCompare == 0)
		{
			return NULL;
		}

		psParent = psNode;
		if (iCompare < 0)
		{
			psNode = psNode->psLeft;
		}
		else
		{
			psNode = psNode->psRight;
		}
	}

	psNode = (SRedBlackNode*)mcNodes.Add();
	psNode->psParent = psParent;
	psNode->psLeft = NULL;
	psNode->psRight = NULL;
	memcpy(GetData(psNode), pvElement, miElementSize);

	if (psParent == NULL)
	{
		mpsRoot = psNode;
	}
	else if (iCompare < 0)
	{
		psParent->psLeft = psNode;
	}
	else
	{
		psParent->psRight = psNode;
	}
	miNumElements++;

	FixAfterAdd(psNode);
	return GetData(psNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::Put(void* pvElement)
{
	SRedBlackNode*	psNode;

	psNode = FindNode(RemapSinglePointer(pvElement, miKeyOffset));
	if (psNode)
	{
		memcpy(GetData(psNode), pvElement, miElementSize);
		return GetData(psNode);
	}
	return Add(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::Get(void* pvKey)
{
	return GetData(FindNode(pvKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::GetOnOrBefore(void* pvKey)
{
	return GetData(FindNodeOnOrBefore(pvKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::GetOnOrAfter(void* pvKey)
{
	return GetData(FindNodeOnOrAfter(pvKey));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::GetFirst(void)
{
	if (mpsRoot)
	{
		return GetData(mpsRoot->GetLeftmost());
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::GetLast(void)
{
	if (mpsRoot)
	{
		return GetData(mpsRoot->GetRightmost());
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::GetNext(void* pvElement)
{
	return GetData(GetNode(pvElement)->GetNext());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::GetPrevious(void* pvElement)
{
	return GetData(GetNode(pvElement)->GetPrevious());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRedBlackTree::Contains(void* pvKey)
{
	return FindNode(pvKey) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRedBlackTree::Remove(void* pvKey)
{
	SRedBlackNode*	psNode;

	psNode = FindNode(pvKey);
	if (psNode)
	{
		RemoveNode(psNode);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::RemoveElement(void* pvElement)
{
	RemoveNode(GetNode(pvElement));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::RemoveAll(void)
{
	mcNodes.Kill();
	mcNodes.Init(sizeof(SRedBlackNode) + miElementSize, 8);
	mpsRoot = NULL;
	miNumElements = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CRedBlackTree::NumElements(void)
{
	return miNumElements;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRedBlackTree::IsEmpty(void)
{
	return miNumElements == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::StartIteration(SRedBlackTreeIterator* psIter)
{
	if (mpsRoot)
	{
		psIter->psCurrent = mpsRoot->GetLeftmost();
		psIter->psStop = mpsRoot->GetRightmost();
	}
	else
	{
		psIter->psCurrent = NULL;
		psIter->psStop = NULL;
	}
	return GetData(psIter->psCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
// Iterates the elements with keys from pvStartKey to pvEndKey inclusive.  A NULL key leaves that end open, like
// Logicim's iteratorFrom and iteratorTo.
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::StartIteration(SRedBlackTreeIterator* psIter, void* pvStartKey, void* pvEndKey)
{
	SRedBlackNode*	psFirst;
	SRedBlackNode*	psLast;

	psIter->psCurrent = NULL;
	psIter->psStop = NULL;
	if (mpsRoot == NULL)
	{
		return NULL;
	}

	if (pvStartKey)
	{
		psFirst = FindNodeOnOrAfter(pvStartKey);
	}
	else
	{
		psFirst = mpsRoot->GetLeftmost();
	}

	if (pvEndKey)
	{
		psLast = FindNodeOnOrBefore(pvEndKey);
	}
	else
	{
		psLast = mpsRoot->GetRightmost();
	}

	if (psFirst && psLast && (mfCompare(GetKey(psFirst), GetKey(psLast)) <= 0))
	{
		psIter->psCurrent = psFirst;
		psIter->psStop = psLast;
	}
	return GetData(psIter->psCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::Iterate(SRedBlackTreeIterator* psIter)
{
	if ((psIter->psCurrent == NULL) || (psIter->psCurrent == psIter->psStop))
	{
		psIter->psCurrent = NULL;
		psIter->psStop = NULL;
		return NULL;
	}

	psIter->psCurrent = psIter->psCurrent->GetNext();
	return GetData(psIter->psCurrent);
}


//////////////////////////////////////////////////////////////////////////
//
// Checks the parent links, the key order, that no red node has a red child, that the root is black, that every path
// has the same number of black nodes and the element count.
//
//////////////////////////////////////////////////////////////////////////
bool CRedBlackTree::IsValid(void)
{
	SRedBlackNode*	psNode;
	SRedBlackNode*	psNext;
	size			iCount;

	if (mpsRoot)
	{
		if ((mpsRoot->psParent != NULL) || IsRed(mpsRoot))
		{
			return false;
		}

		psNode = mpsRoot->GetLeftmost();
		psNext = psNode->GetNext();
		while (psNext)
		{
			if (mfCompare(GetKey(psNode), GetKey(psNext)) >= 0)
			{
				return false;
			}
			psNode = psNext;
			psNext = psNode->GetNext();
		}
	}

	iCount = 0;
	if (RecurseValidate(mpsRoot, &iCount) < 0)
	{
		return false;
	}
	return iCount == miNumElements;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SRedBlackNode* CRedBlackTree::TestGetRoot(void)
{
	return mpsRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SRedBlackNode* CRedBlackTree::FindNode(void* pvKey)
{
	SRedBlackNode*	psNode;
	int				iCompare;

	psNode = mpsRoot;
	while (psNode)
	{
		iCompare = mfCompare(pvKey, GetKey(psNode));
		if (iCompare == 0)
		{
			return psNode;
		}
		else if (iCompare < 0)
		{
			psNode = psNode->psLeft;
		}
		else
		{
			psNode = psNode->psRight;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SRedBlackNode* CRedBlackTree::FindNodeOnOrBefore(void* pvKey)
{
	SRedBlackNode*	psNode;
	SRedBlackNode*	psSmaller;
	int				iCompare;

	psNode = mpsRoot;
	psSmaller = NULL;
	while (psNode)
	{
		iCompare = mfCompare(pvKey, GetKey(psNode));
		if (iCompare == 0)
		{
			return psNode;
		}
		else if (iCompare < 0)
		{
			psNode = psNode->psLeft;
		}
		else
		{
			psSmaller = psNode;
			psNode = psNode->psRight;
		}
	}
	return psSmaller;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SRedBlackNode* CRedBlackTree::FindNodeOnOrAfter(void* pvKey)
{
	SRedBlackNode*	psNode;
	SRedBlackNode*	psLarger;
	int				iCompare;

	psNode = mpsRoot;
	psLarger = NULL;
	while (psNode)
	{
		iCompare = mfCompare(pvKey, GetKey(psNode));
		if (iCompare == 0)
		{
			return psNode;
		}
		else if (iCompare < 0)
		{
			psLarger = psNode;
			psNode = psNode->psLeft;
		}
		else
		{
			psNode = psNode->psRight;
		}
	}
	return psLarger;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::GetKey(SRedBlackNode* psNode)
{
	return RemapSinglePointer(psNode, sizeof(SRedBlackNode) + miKeyOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CRedBlackTree::GetData(SRedBlackNode* psNode)
{
	return HeaderGetData<SRedBlackNode, void>(psNode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SRedBlackNode* CRedBlackTree::GetNode(void* pvElement)
{
	return DataGetHeader<SRedBlackNode, void>(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::FixAfterAdd(SRedBlackNode* psNode)
{
	if (psNode->psParent == NULL)
	{
		psNode->iColour = RED_BLACK_BLACK;
	}
	else
	{
		psNode->iColour = RED_BLACK_RED;
		if (IsRed(psNode->psParent))
		{
			FixDoubleRed(psNode);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A red child under a red parent.  Restructures the child, parent and grandparent into n1 < n2 < n3 with n2 on top,
// red, and n1 and n3 black, keeping their subtrees t1 to t4 in order.  If n2 is now under a red node then that is
// fixed the same way.
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::FixDoubleRed(SRedBlackNode* psChild)
{
	SRedBlackNode*	psParent;
	SRedBlackNode*	psGrandParent;
	SRedBlackNode*	n1;
	SRedBlackNode*	n2;
	SRedBlackNode*	n3;
	SRedBlackNode*	t1;
	SRedBlackNode*	t2;
	SRedBlackNode*	t3;
	SRedBlackNode*	t4;

	psParent = psChild->psParent;
	psGrandParent = psParent->psParent;
	if (psGrandParent == NULL)
	{
		psParent->iColour = RED_BLACK_BLACK;
		return;
	}

	if (psParent == psGrandParent->psLeft)
	{
		n3 = psGrandParent;
		t4 = psGrandParent->psRight;
		if (psChild == psParent->psLeft)
		{
			n1 = psChild;
			n2 = psParent;
			t1 = psChild->psLeft;
			t2 = psChild->psRight;
			t3 = psParent->psRight;
		}
		else
		{
			n1 = psParent;
			n2 = psChild;
			t1 = psParent->psLeft;
			t2 = psChild->psLeft;
			t3 = psChild->psRight;
		}
	}
	else
	{
		n1 = psGrandParent;
		t1 = psGrandParent->psLeft;
		if (psChild == psParent->psLeft)
		{
			n2 = psChild;
			n3 = psParent;
			t2 = psChild->psLeft;
			t3 = psChild->psRight;
			t4 = psParent->psRight;
		}
		else
		{
			n2 = psParent;
			n3 = psChild;
			t2 = psParent->psLeft;
			t3 = psChild->psLeft;
			t4 = psChild->psRight;
		}
	}

	Replace(psGrandParent, n2);

	n1->psLeft = t1;
	n1->psRight = t2;
	n2->psLeft = n1;
	n2->psRight = n3;
	n3->psLeft = t3;
	n3->psRight = t4;
	n1->psParent = n2;
	n3->psParent = n2;
	if (t1) t1->psParent = n1;
	if (t2) t2->psParent = n1;
	if (t3) t3->psParent = n3;
	if (t4) t4->psParent = n3;

	n2->iColour = RED_BLACK_RED;
	n1->iColour = RED_BLACK_BLACK;
	n3->iColour = RED_BLACK_BLACK;

	if (n2 == mpsRoot)
	{
		n2->iColour = RED_BLACK_BLACK;
	}
	else if (IsRed(n2->psParent))
	{
		FixDoubleRed(n2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim removes a node with two children by copying its successor's object into it and swaps objects while
// rebalancing.  Here the successor node is moved into the removed node's place instead so that no element moves.
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::RemoveNode(SRedBlackNode* psNode)
{
	SRedBlackNode*	psSuccessor;
	SRedBlackNode*	psChild;
	SRedBlackNode*	psChildParent;
	int32			iRemovedColour;

	iRemovedColour = psNode->iColour;
	if (psNode->psLeft == NULL)
	{
		psChild = psNode->psRight;
		psChildParent = psNode->psParent;
		Replace(psNode, psChild);
	}
	else if (psNode->psRight == NULL)
	{
		psChild = psNode->psLeft;
		psChildParent = psNode->psParent;
		Replace(psNode, psChild);
	}
	else
	{
		psSuccessor = psNode->psRight->GetLeftmost();
		iRemovedColour = psSuccessor->iColour;
		psChild = psSuccessor->psRight;
		if (psSuccessor->psParent == psNode)
		{
			psChildParent = psSuccessor;
		}
		else
		{
			psChildParent = psSuccessor->psParent;
			Replace(psSuccessor, psChild);
			psSuccessor->psRight = psNode->psRight;
			psSuccessor->psRight->psParent = psSuccessor;
		}
		Replace(psNode, psSuccessor);
		psSuccessor->psLeft = psNode->psLeft;
		psSuccessor->psLeft->psParent = psSuccessor;
		psSuccessor->iColour = psNode->iColour;
	}

	if (iRemovedColour == RED_BLACK_BLACK)
	{
		FixAfterRemove(psChild, psChildParent);
	}

	mcNodes.Remove(psNode);
	miNumElements--;
}


//////////////////////////////////////////////////////////////////////////
//
// psNode (possibly NULL, so its parent is passed too) is short one black node on its paths.
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::FixAfterRemove(SRedBlackNode* psNode, SRedBlackNode* psParent)
{
	SRedBlackNode*	psSibling;

	while ((psNode != mpsRoot) && IsBlack(psNode))
	{
		if (psNode == psParent->psLeft)
		{
			psSibling = psParent->psRight;
			if (IsRed(psSibling))
			{
				psSibling->iColour = RED_BLACK_BLACK;
				psParent->iColour = RED_BLACK_RED;
				RotateLeft(psParent);
				psSibling = psParent->psRight;
			}

			if (IsBlack(psSibling->psLeft) && IsBlack(psSibling->psRight))
			{
				psSibling->iColour = RED_BLACK_RED;
				psNode = psParent;
				psParent = psNode->psParent;
			}
			else
			{
				if (IsBlack(psSibling->psRight))
				{
					psSibling->psLeft->iColour = RED_BLACK_BLACK;
					psSibling->iColour = RED_BLACK_RED;
					RotateRight(psSibling);
					psSibling = psParent->psRight;
				}
				psSibling->iColour = psParent->iColour;
				psParent->iColour = RED_BLACK_BLACK;
				psSibling->psRight->iColour = RED_BLACK_BLACK;
				RotateLeft(psParent);
				psNode = mpsRoot;
			}
		}
		else
		{
			psSibling = psParent->psLeft;
			if (IsRed(psSibling))
			{
				psSibling->iColour = RED_BLACK_BLACK;
				psParent->iColour = RED_BLACK_RED;
				RotateRight(psParent);
				psSibling = psParent->psLeft;
			}

			if (IsBlack(psSibling->psLeft) && IsBlack(psSibling->psRight))
			{
				psSibling->iColour = RED_BLACK_RED;
				psNode = psParent;
				psParent = psNode->psParent;
			}
			else
			{
				if (IsBlack(psSibling->psLeft))
				{
					psSibling->psRight->iColour = RED_BLACK_BLACK;
					psSibling->iColour = RED_BLACK_RED;
					RotateLeft(psSibling);
					psSibling = psParent->psLeft;
				}
				psSibling->iColour = psParent->iColour;
				psParent->iColour = RED_BLACK_BLACK;
				psSibling->psLeft->iColour = RED_BLACK_BLACK;
				RotateRight(psParent);
				psNode = mpsRoot;
			}
		}
	}

	if (psNode)
	{
		psNode->iColour = RED_BLACK_BLACK;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Puts psReplacement (possibly NULL) where psNode is under psNode's parent.  psNode's own links are left alone.
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::Replace(SRedBlackNode* psNode, SRedBlackNode* psReplacement)
{
	if (psNode->psParent == NULL)
	{
		mpsRoot = psReplacement;
	}
	else if (psNode == psNode->psParent->psLeft)
	{
		psNode->psParent->psLeft = psReplacement;
	}
	else
	{
		psNode->psParent->psRight = psReplacement;
	}

	if (psReplacement)
	{
		psReplacement->psParent = psNode->psParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::RotateLeft(SRedBlackNode* psNode)
{
	SRedBlackNode*	psRight;

	psRight = psNode->psRight;
	psNode->psRight = psRight->psLeft;
	if (psRight->psLeft)
	{
		psRight->psLeft->psParent = psNode;
	}
	Replace(psNode, psRight);
	psRight->psLeft = psNode;
	psNode->psParent = psRight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRedBlackTree::RotateRight(SRedBlackNode* psNode)
{
	SRedBlackNode*	psLeft;

	psLeft = psNode->psLeft;
	psNode->psLeft = psLeft->psRight;
	if (psLeft->psRight)
	{
		psLeft->psRight->psParent = psNode;
	}
	Replace(psNode, psLeft);
	psLeft->psRight = psNode;
	psNode->psParent = psLeft;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the number of black nodes on every path below psNode, counting the NULL leaves, or -1 if the tree is broken.
//
//////////////////////////////////////////////////////////////////////////
int CRedBlackTree::RecurseValidate(SRedBlackNode* psNode, size* piCount)
{
	int		iLeftHeight;
	int		iRightHeight;

	if (psNode == NULL)
	{
		return 1;
	}

	(*piCount)++;
	if ((psNode->iColour != RED_BLACK_RED) && (psNode->iColour != RED_BLACK_BLACK))
	{
		return -1;
	}
	if (psNode->psLeft && (psNode->psLeft->psParent != psNode))
	{
		return -1;
	}
	if (psNode->psRight && (psNode->psRight->psParent != psNode))
	{
		return -1;
	}
	if (IsRed(psNode) && (IsRed(psNode->psLeft) || IsRed(psNode->psRight)))
	{
		return -1;
	}

	iLeftHeight = RecurseValidate(psNode->psLeft, piCount);
	iRightHeight = RecurseValidate(psNode->psRight, piCount);
	if ((iLeftHeight < 0) || (iLeftHeight != iRightHeight))
	{
		return -1;
	}

	if (psNode->iColour == RED_BLACK_BLACK)
	{
		return iLeftHeight + 1;
	}
	return iLeftHeight;
}

