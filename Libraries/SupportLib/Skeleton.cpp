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
#include "BaseLib/Logger.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "Skeleton.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBone::Init(const char* szName, Ptr<CBone> pParent, SFloat4x4* psLocalMatrix, size uiIndex)
{
	PreInit();

	mszName.Init(szName);
	mpParent = pParent;
	msLocalMatrix = *psLocalMatrix;
	muiIndex = uiIndex;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBone::Free(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBone::Class(void)
{
	U_String(mszName);
	M_Pointer(mpParent);
	U_4x4Float32(msLocalMatrix);
	U_Size(muiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSkeleton::Init(void)
{
	PreInit();

	mszName.Init();
	mapBones.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSkeleton::Free(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSkeleton::Class(void)
{
	U_String(mszName);
	M_Embedded(mapBones);
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if pParent isn't a bone of this skeleton.  A NULL parent makes a
// root bone.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CBone> CSkeleton::AddBone(const char* szName, Ptr<CBone> pParent, SFloat4x4* psLocalMatrix)
{
	Ptr<CBone>	pBone;

	if (pParent.IsNotNull() && !Contains(pParent))
	{
		gcLogger.Error2(__METHOD__, " Bone [", szName, "] has a parent that isn't in skeleton [", mszName.Text(), "].", NULL);
		return NULL;
	}

	pBone = OMalloc<CBone>(szName, pParent, psLocalMatrix, mapBones.NumElements());
	mapBones.Add(pBone);
	return pBone;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CSkeleton::NumBones(void)
{
	return mapBones.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CBone> CSkeleton::GetBone(size uiIndex)
{
	return mapBones.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// A bone knows its place, so this needn't look through them all.
//
//////////////////////////////////////////////////////////////////////////
bool CSkeleton::Contains(Ptr<CBone> pBone)
{
	if (pBone.IsNull() || (pBone->muiIndex >= mapBones.NumElements()))
	{
		return false;
	}
	return &mapBones.Get(pBone->muiIndex) == &pBone;
}

