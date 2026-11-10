/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RenderLib

Codaphela RenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "RenderScene.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderSceneMesh::Init(Ptr<CMeshEditor> pEditor)
{
	PreInit();

	mpEditor = pEditor;
	mpMesh = NULL;
	muiStructureChanges = 0;
	muiShapeChanges = 0;
	muiModifierChanges = 0;
	mbUsed = false;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderSceneMesh::Class(void)
{
	M_Pointer(mpEditor);
	M_Pointer(mpMesh);
	U_Size(muiStructureChanges);
	U_Size(muiShapeChanges);
	U_Size(muiModifierChanges);
	U_Bool(mbUsed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderSceneMesh::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRenderSceneMesh::IsOutOfDate(void)
{
	return mpMesh.IsNull() ||
		(muiStructureChanges != mpEditor->NumStructureChanges()) ||
		(muiShapeChanges != mpEditor->NumShapeChanges()) ||
		(muiModifierChanges != mpEditor->NumModifierChanges());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderSceneMesh::Made(Ptr<CRenderMesh> pMesh)
{
	mpMesh = pMesh;
	muiStructureChanges = mpEditor->NumStructureChanges();
	muiShapeChanges = mpEditor->NumShapeChanges();
	muiModifierChanges = mpEditor->NumModifierChanges();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::Init(Ptr<CScene> pScene, CRenderFactory* pcFactory, Ptr<CRenderer> pRenderer)
{
	PreInit();

	mpScene = pScene;
	mpRenderer = pRenderer;
	mpcFactory = pcFactory;
	mapMeshes.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::Class(void)
{
	M_Pointer(mpScene);
	M_Pointer(mpRenderer);
	U_Pointer(mpcFactory);
	M_Embedded(mapMeshes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::Update(void)
{
	Ptr<CInstance>	pInstance;
	size			i;
	size			j;

	for (i = 0; i < mapMeshes.NumElements(); i++)
	{
		mapMeshes.Get(i)->mbUsed = false;
	}

	for (i = 0; i < mpScene->mapInstances.NumElements(); i++)
	{
		pInstance = mpScene->mapInstances.Get(i);
		for (j = 0; j < pInstance->mapMeshes.NumElements(); j++)
		{
			UpdateEditor(pInstance->mapMeshes.Get(j)->mpEditor);
		}
	}

	RemoveUnused();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::UpdateEditor(Ptr<CMeshEditor> pEditor)
{
	Ptr<CRenderSceneMesh>	pSceneMesh;
	Ptr<CMesh>				pOutput;

	if (pEditor.IsNull())
	{
		return;
	}

	pSceneMesh = FindMesh(pEditor);
	if (pSceneMesh.IsNull())
	{
		pSceneMesh = OMalloc<CRenderSceneMesh>(pEditor);
		mapMeshes.Add(pSceneMesh);
	}
	else if (pSceneMesh->mbUsed)
	{
		return;
	}

	pSceneMesh->mbUsed = true;
	if (pSceneMesh->IsOutOfDate())
	{
		pOutput = pEditor->GetOutput();
		pSceneMesh->Made(mpcFactory->CreateMesh(mpRenderer, &pOutput));
		pOutput = NULL;
		pEditor->ClearOutput();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRenderSceneMesh> CRenderScene::FindMesh(Ptr<CMeshEditor> pEditor)
{
	Ptr<CRenderSceneMesh>	pSceneMesh;
	size					i;

	for (i = 0; i < mapMeshes.NumElements(); i++)
	{
		pSceneMesh = mapMeshes.Get(i);
		if (pSceneMesh->mpEditor == pEditor)
		{
			return pSceneMesh;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::RemoveUnused(void)
{
	size	i;

	for (i = mapMeshes.NumElements(); i > 0; i--)
	{
		if (!mapMeshes.Get(i - 1)->mbUsed)
		{
			mapMeshes.RemoveAt(i - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRenderMesh> CRenderScene::GetMesh(Ptr<CMeshEditor> pEditor)
{
	Ptr<CRenderSceneMesh>	pSceneMesh;

	pSceneMesh = FindMesh(pEditor);
	if (pSceneMesh.IsNull())
	{
		return NULL;
	}
	return pSceneMesh->mpMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CRenderScene::NumMeshes(void)
{
	return mapMeshes.NumElements();
}

