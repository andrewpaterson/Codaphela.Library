/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SKELETON_H__
#define __SKELETON_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Float4x4.h"
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"


//One bone of a skeleton, shared by every instance with that skeleton, which each have a connection playing it.  It doesn't
//change once its skeleton is built.  msLocalMatrix is where an instance's connection for it starts, relative to its parent's,
//or to whatever the instance hangs its root bones from.  muiIndex is its place in its skeleton.
class CBone : public CObject
{
CONSTRUCTABLE(CBone);
DESTRUCTABLE(CBone);
public:
	CChars			mszName;
	Ptr<CBone>		mpParent;
	SFloat4x4		msLocalMatrix;
	size			muiIndex;

	void 	Init(const char* szName, Ptr<CBone> pParent, SFloat4x4* psLocalMatrix, size uiIndex);
	void 	Free(void) override;
	void	Class(void) override;
};


//Bones that move together, a character's say, shared by every instance of it.  Its bones are known by what they are, not where
//they are: a skin and a skeleton's animation hold the CBones they use.  A parent comes before its children, which AddBone keeps
//by only adding a bone whose parent is already there.  A bone is only ever added at the end, so none moves.
class CSkeleton : public CObject
{
CONSTRUCTABLE(CSkeleton);
DESTRUCTABLE(CSkeleton);
public:
	CChars			mszName;
	CArray<CBone>	mapBones;

	void 		Init(void);
	void 		Free(void) override;
	void		Class(void) override;

	Ptr<CBone>	AddBone(const char* szName, Ptr<CBone> pParent, SFloat4x4* psLocalMatrix);
	size		NumBones(void);
	Ptr<CBone>	GetBone(size uiIndex);
	bool		Contains(Ptr<CBone> pBone);
};


#endif // __SKELETON_H__

