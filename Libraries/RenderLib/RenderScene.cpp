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
	mpCooked = NULL;
	muiCookedStructure = 0;
	muiCookedShape = 0;
	muiCookedModifiers = 0;

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
	M_Pointer(mpCooked);
	U_Size(muiCookedStructure);
	U_Size(muiCookedShape);
	U_Size(muiCookedModifiers);
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
	mpCooked = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// pCooked is the editor's output as the editor is now.
//
//////////////////////////////////////////////////////////////////////////
void CRenderSceneMesh::SetCooked(Ptr<CCookedMesh> pCooked)
{
	mpCooked = pCooked;
	muiCookedStructure = mpEditor->NumStructureChanges();
	muiCookedShape = mpEditor->NumShapeChanges();
	muiCookedModifiers = mpEditor->NumModifierChanges();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCookedMesh> CRenderSceneMesh::TakeCooked(void)
{
	Ptr<CCookedMesh>	pCooked;

	if ((muiCookedStructure == mpEditor->NumStructureChanges()) && (muiCookedShape == mpEditor->NumShapeChanges()) && (muiCookedModifiers == mpEditor->NumModifierChanges()))
	{
		pCooked = mpCooked;
	}
	mpCooked = NULL;
	return pCooked;
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
	Ptr<CCookedMesh>		pCooked;

	if (pEditor.IsNull())
	{
		return;
	}

	pSceneMesh = FindOrAddMesh(pEditor);
	if (pSceneMesh->mbUsed)
	{
		return;
	}

	pSceneMesh->mbUsed = true;
	if (pSceneMesh->IsOutOfDate())
	{
		pCooked = pSceneMesh->TakeCooked();
		if (pCooked.IsNull())
		{
			pOutput = pEditor->GetOutput();
			pCooked = OMalloc<CCookedMesh>();
			pCooked->Cook(&pOutput);
			pOutput = NULL;
		}
		pEditor->ClearOutput();
		pSceneMesh->Made(mpcFactory->CreateMesh(mpRenderer, &pCooked));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pCooked is the editor's output as the editor is now, cooked elsewhere:
// the editor's next render mesh is made of it, if the editor hasn't
// changed again by then.
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::SetCooked(Ptr<CMeshEditor> pEditor, Ptr<CCookedMesh> pCooked)
{
	FindOrAddMesh(pEditor)->SetCooked(pCooked);
}


//////////////////////////////////////////////////////////////////////////
//
// pMesh is the render mesh of the editor's output as the editor is now,
// made elsewhere by this scene's factory for its renderer: it is the
// editor's until the editor changes.
//
//////////////////////////////////////////////////////////////////////////
void CRenderScene::SetMesh(Ptr<CMeshEditor> pEditor, Ptr<CRenderMesh> pMesh)
{
	FindOrAddMesh(pEditor)->Made(pMesh);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRenderSceneMesh> CRenderScene::FindOrAddMesh(Ptr<CMeshEditor> pEditor)
{
	Ptr<CRenderSceneMesh>	pSceneMesh;

	pSceneMesh = FindMesh(pEditor);
	if (pSceneMesh.IsNull())
	{
		pSceneMesh = OMalloc<CRenderSceneMesh>(pEditor);
		mapMeshes.Add(pSceneMesh);
	}
	return pSceneMesh;
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CScene> CRenderScene::GetScene(void) const
{
	return mpScene;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRenderer> CRenderScene::GetRenderer(void) const
{
	return mpRenderer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRenderFactory* CRenderScene::GetFactory(void) const
{
	return mpcFactory;
}

