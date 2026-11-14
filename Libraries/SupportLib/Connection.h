/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include "BaseLib/Chars.h"
#include "BaseLib/Float4x4.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "Skeleton.h"


//Something spatial that meshes, lights, cameras or other game information can be attached to.  mszName is what it's called in a
//file, and many may have the same one, which an object's name can't.
//
//Matrices are row major with vectors on their left.  UpdateWorldMatrix makes msWorldMatrix msLocalMatrix * the parent's world matrix,
//and msLocalMatrix itself with no parent, updating its parents first.  mbParity is then whether it mirrors.  uiUpdate says which update
//this is, so that a parent shared by many is only done once, and a loop of parents stops.
//
//mpBone is the bone of its instance's skeleton that it plays, NULL if it isn't one.  Many instances' connections play the same bone.
class CConnection : public CObject
{
CONSTRUCTABLE(CConnection);
DESTRUCTABLE(CConnection);
public:
	CChars				mszName;
	Ptr<CConnection>	mpParent;
	Ptr<CBone>			mpBone;
	SFloat4x4			msWorldMatrix;
	SFloat4x4			msLocalMatrix;  //Relative to the parent, or the world with no parent.
	bool				mbSelected;
	bool				mbTopLevel;
	bool				mbParity;

protected:
	size				muiUpdate;

public:
	Ptr<CConnection> 	Init(void);
	void 				Free(void) override;
	void				Class(void) override;

	void				Copy(CConnection* pcConnection);

	void				UpdateWorldMatrix(size uiUpdate);
};


#endif //__CONNECTION_H__

