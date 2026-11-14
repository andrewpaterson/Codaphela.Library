/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "Instance.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstanceMesh::Init(Ptr<CMeshEditor> pEditor)
{
	PreInit();

	mpEditor = pEditor;
	mapConnections.Init();
	mapMaterials.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstanceMesh::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstanceMesh::Class(void)
{
	M_Pointer(mpEditor);
	M_Embedded(mapConnections);
	M_Embedded(mapMaterials);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether its matrices are its skin's bones.  A skin with no bones, as a
// mesh read from an OBJ has, is posed by connections given to it.
//
//////////////////////////////////////////////////////////////////////////
bool CInstanceMesh::IsSkinned(void)
{
	CMesh*	pcMesh;

	if (mpEditor.IsNull() || mpEditor->mpMesh.IsNull())
	{
		return false;
	}
	pcMesh = &mpEditor->mpMesh;
	return pcMesh->mcSkin.InUse() && (pcMesh->mcSkin.mapBones.NumElements() > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The instance's connections for its skin's bones, in the skin's order.
//
//////////////////////////////////////////////////////////////////////////
void CInstanceMesh::BindBones(CInstance* pcInstance)
{
	CArray<CBone>*	papBones;
	size			i;

	if (!IsSkinned())
	{
		return;
	}

	mapConnections.RemoveAll();
	papBones = &mpEditor->mpMesh->mcSkin.mapBones;
	for (i = 0; i < papBones->NumElements(); i++)
	{
		mapConnections.Add(pcInstance->GetBoneConnection(papBones->Get(i)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstance::Init(void)
{
	PreInit();

	mszName.Init();
	mpSkeleton = NULL;
	mapConnections.Init();
	mapMeshes.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstance::Free(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstance::Class(void)
{
	U_String(mszName);
	M_Pointer(mpSkeleton);
	M_Embedded(mapConnections);
	M_Embedded(mapMeshes);
}


//////////////////////////////////////////////////////////////////////////
//
// The bones' connections go after those it has.  As a skeleton's parents
// come first, each bone's parent's connection is made before it.
//
//////////////////////////////////////////////////////////////////////////
void CInstance::MakeBones(Ptr<CSkeleton> pSkeleton, Ptr<CConnection> pParent)
{
	Ptr<CConnection>	pConnection;
	Ptr<CBone>			pBone;
	size				uiFirst;
	size				i;

	mpSkeleton = pSkeleton;
	uiFirst = mapConnections.NumElements();
	for (i = 0; i < pSkeleton->NumBones(); i++)
	{
		pBone = pSkeleton->GetBone(i);
		pConnection = OMalloc<CConnection>();
		pConnection->mszName.Append(pBone->mszName.Text());
		pConnection->mpBone = pBone;
		pConnection->msLocalMatrix = pBone->msLocalMatrix;
		pConnection->mpParent = pBone->mpParent.IsNull() ? pParent : mapConnections.Get(uiFirst + pBone->mpParent->muiIndex);
		mapConnections.Add(pConnection);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if none of its connections plays pBone.  It's only looked for when
// something is bound, so looking through them is fine.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CConnection> CInstance::GetBoneConnection(Ptr<CBone> pBone)
{
	Ptr<CConnection>	pConnection;
	size				i;

	if (pBone.IsNull())
	{
		return NULL;
	}
	for (i = 0; i < mapConnections.NumElements(); i++)
	{
		pConnection = mapConnections.Get(i);
		if (&pConnection->mpBone == &pBone)
		{
			return pConnection;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A skinned mesh is bound to the instance's bones.  A rigid one is given its
// connection by the caller.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CInstanceMesh> CInstance::AddMesh(Ptr<CMeshEditor> pEditor)
{
	Ptr<CInstanceMesh>	pInstanceMesh;

	pInstanceMesh = OMalloc<CInstanceMesh>(pEditor);
	pInstanceMesh->BindBones(this);
	mapMeshes.Add(pInstanceMesh);
	return pInstanceMesh;
}

