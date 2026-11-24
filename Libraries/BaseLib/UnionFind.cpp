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

** ------------------------------------------------------------------------ **/
#include "SystemAllocator.h"
#include "UnionFind.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CUnionFindBase<SIZE>::Init(void)
{
	Init(&gcSystemAllocator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CUnionFindBase<SIZE>::Init(CMallocator* pcMalloc)
{
	macParents.Init(pcMalloc, sizeof(Count));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CUnionFindBase<SIZE>::Kill(void)
{
	macParents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// No elements.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CUnionFindBase<SIZE>::ReInit(void)
{
	macParents.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// A new element, in a set of its own: its index, or SIZE::NotFound if
// there is no memory for it.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CUnionFindBase<SIZE>::Add(void)
{
	return AddNum(1);
}


//////////////////////////////////////////////////////////////////////////
//
// uiNumElements new elements, each in a set of its own: the first's index,
// or SIZE::NotFound if there is no memory for them.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CUnionFindBase<SIZE>::AddNum(Count uiNumElements)
{
	Count	uiFirst;
	Count	ui;

	uiFirst = macParents.AddNum(uiNumElements);
	if (uiFirst == SIZE::NotFound)
	{
		return SIZE::NotFound;
	}
	for (ui = 0; ui < uiNumElements; ui++)
	{
		SetParent(uiFirst + ui, uiFirst + ui);
	}
	return uiFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// The root of uiElement's set, halving the path to it: each element walked
// is given its grandparent as its parent.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CUnionFindBase<SIZE>::Find(Count uiElement)
{
	Count	uiParent;

	uiParent = GetParent(uiElement);
	while (uiParent != uiElement)
	{
		SetParent(uiElement, GetParent(uiParent));
		uiElement = uiParent;
		uiParent = GetParent(uiElement);
	}
	return uiElement;
}


//////////////////////////////////////////////////////////////////////////
//
// uiElement1's and uiElement2's sets made one: the root of it, the lower
// of their two roots.
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CUnionFindBase<SIZE>::Join(Count uiElement1, Count uiElement2)
{
	Count	uiRoot1;
	Count	uiRoot2;

	uiRoot1 = Find(uiElement1);
	uiRoot2 = Find(uiElement2);
	if (uiRoot1 < uiRoot2)
	{
		SetParent(uiRoot2, uiRoot1);
		return uiRoot1;
	}
	if (uiRoot2 < uiRoot1)
	{
		SetParent(uiRoot1, uiRoot2);
	}
	return uiRoot2;
}


//////////////////////////////////////////////////////////////////////////
//
// uiElement1's and uiElement2's sets made one: the root of it, uiElement1's
// root (where which is the root matters to the caller, not only which set).
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CUnionFindBase<SIZE>::JoinToFirst(Count uiElement1, Count uiElement2)
{
	Count	uiRoot1;
	Count	uiRoot2;

	uiRoot1 = Find(uiElement1);
	uiRoot2 = Find(uiElement2);
	if (uiRoot1 != uiRoot2)
	{
		SetParent(uiRoot2, uiRoot1);
	}
	return uiRoot1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
bool CUnionFindBase<SIZE>::SameSet(Count uiElement1, Count uiElement2)
{
	return Find(uiElement1) == Find(uiElement2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CUnionFindBase<SIZE>::NumElements(void) const
{
	return macParents.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
typename SIZE::Count CUnionFindBase<SIZE>::GetParent(Count uiElement)
{
	return *(Count*)macParents.Get(uiElement);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class SIZE>
void CUnionFindBase<SIZE>::SetParent(Count uiElement, Count uiParent)
{
	*(Count*)macParents.Get(uiElement) = uiParent;
}


template class CUnionFindBase<SArrayNaturalSize>;
template class CUnionFindBase<SArrayLargeSize>;

