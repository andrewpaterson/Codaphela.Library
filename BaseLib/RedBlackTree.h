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
#ifndef __RED_BLACK_TREE_H__
#define __RED_BLACK_TREE_H__
#include "Define.h"
#include "DataCompare.h"
#include "DataMacro.h"
#include "FreeList.h"


#define RED_BLACK_RED		0
#define RED_BLACK_BLACK		1


struct SRedBlackNode
{
	SRedBlackNode*	psParent;
	SRedBlackNode*	psLeft;
	SRedBlackNode*	psRight;
	int32			iColour;

	//Element data follows node.

	SRedBlackNode*	GetNext(void);
	SRedBlackNode*	GetPrevious(void);
	SRedBlackNode*	GetLeftmost(void);
	SRedBlackNode*	GetRightmost(void);
};


struct SRedBlackTreeIterator
{
	SRedBlackNode*	psCurrent;
	SRedBlackNode*	psStop;  //The last node to iterate, NULL when there is nothing left.
};


//A red-black tree of fixed size elements, smallest key first, like Logicim's RedBlackTree.  Each element holds its own
//key (Logicim's TreeItem.getTreeKey) at miKeyOffset and fCompare compares two keys.  Keys are unique.
//
//Elements are copied into the tree and stay at the same address until they are removed; rebalancing relinks nodes
//rather than moving element data between them.  Removing an element other than the one being iterated doesn't
//disturb an iteration.
class CRedBlackTree
{
protected:
	CFreeList		mcNodes;
	SRedBlackNode*	mpsRoot;
	DataCompare		mfCompare;
	size			miKeyOffset;
	size			miElementSize;
	size			miNumElements;

public:
	void			Init(size iElementSize, size iKeyOffset, DataCompare fCompare);
	void			Kill(void);

	void*			Add(void* pvElement);
	void*			Put(void* pvElement);

	void*			Get(void* pvKey);
	void*			GetOnOrBefore(void* pvKey);
	void*			GetOnOrAfter(void* pvKey);
	void*			GetFirst(void);
	void*			GetLast(void);
	void*			GetNext(void* pvElement);
	void*			GetPrevious(void* pvElement);
	bool			Contains(void* pvKey);

	bool			Remove(void* pvKey);
	void			RemoveElement(void* pvElement);
	void			RemoveAll(void);

	size			NumElements(void);
	bool			IsEmpty(void);

	void*			StartIteration(SRedBlackTreeIterator* psIter);
	void*			StartIteration(SRedBlackTreeIterator* psIter, void* pvStartKey, void* pvEndKey);
	void*			Iterate(SRedBlackTreeIterator* psIter);

	bool			IsValid(void);
	SRedBlackNode*	TestGetRoot(void);

protected:
	SRedBlackNode*	FindNode(void* pvKey);
	SRedBlackNode*	FindNodeOnOrBefore(void* pvKey);
	SRedBlackNode*	FindNodeOnOrAfter(void* pvKey);
	void*			GetKey(SRedBlackNode* psNode);
	void*			GetData(SRedBlackNode* psNode);
	SRedBlackNode*	GetNode(void* pvElement);
	void			FixAfterAdd(SRedBlackNode* psNode);
	void			FixDoubleRed(SRedBlackNode* psChild);
	void			RemoveNode(SRedBlackNode* psNode);
	void			FixAfterRemove(SRedBlackNode* psNode, SRedBlackNode* psParent);
	void			Replace(SRedBlackNode* psNode, SRedBlackNode* psReplacement);
	void			RotateLeft(SRedBlackNode* psNode);
	void			RotateRight(SRedBlackNode* psNode);
	int				RecurseValidate(SRedBlackNode* psNode, size* piCount);
};


template<class M>
class CRedBlackTreeTemplate : public CRedBlackTree
{
public:
	void	Init(size iKeyOffset, DataCompare fCompare);

	M*		Add(M* pvElement);
	M*		Put(M* pvElement);

	M*		Get(void* pvKey);
	M*		GetOnOrBefore(void* pvKey);
	M*		GetOnOrAfter(void* pvKey);
	M*		GetFirst(void);
	M*		GetLast(void);
	M*		GetNext(M* pvElement);
	M*		GetPrevious(M* pvElement);

	M*		StartIteration(SRedBlackTreeIterator* psIter);
	M*		StartIteration(SRedBlackTreeIterator* psIter, void* pvStartKey, void* pvEndKey);
	M*		Iterate(SRedBlackTreeIterator* psIter);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CRedBlackTreeTemplate<M>::Init(size iKeyOffset, DataCompare fCompare)
{
	CRedBlackTree::Init(sizeof(M), iKeyOffset, fCompare);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::Add(M* pvElement)
{
	return (M*)CRedBlackTree::Add(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::Put(M* pvElement)
{
	return (M*)CRedBlackTree::Put(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::Get(void* pvKey)
{
	return (M*)CRedBlackTree::Get(pvKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::GetOnOrBefore(void* pvKey)
{
	return (M*)CRedBlackTree::GetOnOrBefore(pvKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::GetOnOrAfter(void* pvKey)
{
	return (M*)CRedBlackTree::GetOnOrAfter(pvKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::GetFirst(void)
{
	return (M*)CRedBlackTree::GetFirst();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::GetLast(void)
{
	return (M*)CRedBlackTree::GetLast();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::GetNext(M* pvElement)
{
	return (M*)CRedBlackTree::GetNext(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::GetPrevious(M* pvElement)
{
	return (M*)CRedBlackTree::GetPrevious(pvElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::StartIteration(SRedBlackTreeIterator* psIter)
{
	return (M*)CRedBlackTree::StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::StartIteration(SRedBlackTreeIterator* psIter, void* pvStartKey, void* pvEndKey)
{
	return (M*)CRedBlackTree::StartIteration(psIter, pvStartKey, pvEndKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CRedBlackTreeTemplate<M>::Iterate(SRedBlackTreeIterator* psIter)
{
	return (M*)CRedBlackTree::Iterate(psIter);
}


#endif // __RED_BLACK_TREE_H__

