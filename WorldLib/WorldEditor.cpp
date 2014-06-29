/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela EditorLib

Codaphela EditorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela EditorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela EditorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "SceneConverter.h"
#include "WorldEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorldEditor::Init(CSceneConverter* pcSceneConverter)
{
	mpcSceneConverter = pcSceneConverter;
	mcMeshSelectionConverter.Init(mpcSceneConverter->GetScene(), mpcSceneConverter->GetWorld());
	mcMatrixSelectionConverter.Init(mpcSceneConverter->GetScene(), mpcSceneConverter->GetWorld());
	macMeshAdjusters.Init();
	SetupSelectionColours();
	SetupMaterials();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorldEditor::Kill(void)
{
	mcWhiteZeroColours.Kill();
	macMeshAdjusters.Kill();
	mcMatrixSelectionConverter.Kill();
	mcMeshSelectionConverter.Kill();
	mpcSceneConverter = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorldEditor::SetupMaterials(void)
{
	mfPointSize = 3.5f;

	mpcZBufferlessMaterial = mpcSceneConverter->GetWorld()->CreateGraphicsMaterial();
	mpcZBufferlessMaterial->SetWhiteDefaults(0);
	mpcZBufferlessMaterial->SetRenderState(D3DRS_ZENABLE, FALSE);

	mpCLightInstancelessMaterial = mpcSceneConverter->GetWorld()->CreateGraphicsMaterial();
	mpCLightInstancelessMaterial->SetWhiteDefaults(0);
	mpCLightInstancelessMaterial->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&mfPointSize));
	mpCLightInstancelessMaterial->SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);

	mpcBlendMaterial = mpcSceneConverter->GetWorld()->CreateGraphicsMaterial();
	mpcBlendMaterial->SetWhiteDefaults(0);
	mpcBlendMaterial->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	mpcBlendMaterial->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	mpcBlendMaterial->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_LESS);
	mpcBlendMaterial->SetRenderState(D3DRS_ALPHAREF, 255);
	mpcBlendMaterial->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	mpcBlendMaterial->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	mpcZBufferlessMaterial->SetRenderState(D3DRS_ZENABLE, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorldEditor::SetPointSize(float fPointSize)
{
	mfPointSize = fPointSize;
	mpCLightInstancelessMaterial->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&mfPointSize));
	
	CChars	sz;
	sz.Init();
	sz.Append(mfPointSize);
	sz.Append("\n");
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorldEditor::Draw(void)
{
	CMeshAdjuster*	pcMeshAdjuster;
	SSetIterator	sIter;

	//The mesh adjusters must be drawn before the graphics instances as they will mark the graphics
	//instances as visible or not (Depending on wether or not textured faces are selected).
	pcMeshAdjuster = macMeshAdjusters.StartIteration(&sIter);
	while (pcMeshAdjuster)
	{
		pcMeshAdjuster->Draw();
		pcMeshAdjuster = macMeshAdjusters.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CWorldEditor::AddScene(void)
{
	int												iNumSceneInstances;
	int												i;
	CMesh*											pcMesh;
	SSpecimenIDToGraphicsInstanceAndMeshInstance*	pcInstanceInstance;
	CInstance*										pcInstance;
	int												iMeshIndex;
	BOOL											bResult;

	bResult = mpcSceneConverter->ConvertScene();
	if (!bResult)
	{
		return FALSE;
	}

	iNumSceneInstances = mpcSceneConverter->GetScene()->mcInstanceTracker.Num();
	for (i = 0; i < iNumSceneInstances; i++)
	{
		pcInstance = (CInstance*)mpcSceneConverter->GetScene()->mcInstanceTracker.Get(i);
		if (pcInstance->meType == TT_Mesh)
		{
			iMeshIndex = pcInstance->miObjectIndex;
			pcInstanceInstance = mpcSceneConverter->GetMapper()->macInstances.SafeGet((int)pcInstance->GetOI());
			if (pcInstanceInstance)
			{
				pcMesh = (CMesh*)mpcSceneConverter->GetScene()->mcMeshTracker.GetWithID(iMeshIndex);
//				AddMeshAdjuster(pcMesh, pcInstanceInstance->pcGraphicsInstance, pcInstanceInstance->pcMeshInstance);
			}
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMeshAdjuster* CWorldEditor::AddMeshAdjuster(CMeshEditor* pcMeshEditor, CGraphicsInstance* pcGraphicsInstance, CMeshInstance* pcMeshInstance)
{
	CMeshAdjuster*		pcMeshAdjuster;

	pcMeshAdjuster = macMeshAdjusters.Add();
	pcMeshAdjuster->Init(pcMeshEditor, pcGraphicsInstance, pcMeshInstance, this);
	return pcMeshAdjuster;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorldEditor::SetupSelectionColours(void)
{
	SSelectionColour*	psSelectionColour;

	mcWhiteZeroColours.Init(1);
	psSelectionColour = mcWhiteZeroColours.Add();
	psSelectionColour->iSelection = 0;
	psSelectionColour->iColour = 0xffffffff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWorldEditor::ClearAllSelections(void)
{
	CMeshAdjuster*	pcMeshAdjuster;
	SSetIterator	sIter;

	pcMeshAdjuster = macMeshAdjusters.StartIteration(&sIter);
	while (pcMeshAdjuster)
	{
		pcMeshAdjuster->ClearVertSelection();
		pcMeshAdjuster->ClearEdgeSelection();
		pcMeshAdjuster->ClearFaceSelection();
		pcMeshAdjuster = macMeshAdjusters.Iterate(&sIter);
	}	
}

