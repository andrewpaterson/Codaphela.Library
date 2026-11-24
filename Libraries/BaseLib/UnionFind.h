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
#ifndef __UNION_FIND_H__
#define __UNION_FIND_H__
#include "ArrayBlock.h"


//Elements 0 to NumElements() - 1 in sets: each starts in one of its own (Add), Join puts two elements' sets together,
//and Find gives the root of an element's set, the same element for every element in it.  Join makes the lower root the
//other's parent, so a set's root is its lowest element; JoinToFirst keeps the first element's root instead.  Find halves the path it walks, so a long chain of joins is
//walked quickly the next time.  Counted in size (CUnionFind) or lsize (CUnionFindLarge).
template<class SIZE>
class CUnionFindBase
{
public:
	typedef typename SIZE::Count	Count;

protected:
	CArrayBlockBase<SIZE>	macParents;		//Each element's parent: itself for a root.

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);
	void	ReInit(void);

	Count	Add(void);
	Count	AddNum(Count uiNumElements);
	Count	Find(Count uiElement);
	Count	Join(Count uiElement1, Count uiElement2);
	Count	JoinToFirst(Count uiElement1, Count uiElement2);
	bool	SameSet(Count uiElement1, Count uiElement2);
	Count	NumElements(void) const;

protected:
	Count	GetParent(Count uiElement);
	void	SetParent(Count uiElement, Count uiParent);
};


extern template class CUnionFindBase<SArrayNaturalSize>;
extern template class CUnionFindBase<SArrayLargeSize>;


//Classes, not typedefs, so that they can be declared ahead.
class CUnionFind : public CUnionFindBase<SArrayNaturalSize> {};
class CUnionFindLarge : public CUnionFindBase<SArrayLargeSize> {};


#endif // __UNION_FIND_H__

