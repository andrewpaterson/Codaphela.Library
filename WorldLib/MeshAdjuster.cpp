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
#include "SupportLib/ColourARGB.h"
#include "SupportLib/MeshEditor.h"
#include "SceneConverter.h"
#include "MeshAdjuster.h"
#include "WorldEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshAdjuster::Init(CMeshEditor* pcMeshEditor, CGraphicsInstance* pcGraphicsInstance, CMeshInstance* pcMeshInstance, CWorldEditor* pcWorldEditor)
{
	mpcMeshEditor = pcMeshEditor;
	mpcWorldEditor = pcWorldEditor;
	mpcMainGraphicsInstance = pcGraphicsInstance;
	mpcMainLinkInstance = pcMeshInstance;
	mpcSelectionObject = NULL;
	miMeshAdjusterDisplay = MAD_Edges | MAD_Vertices | MAD_Bounding | MAD_Faces;
	miSelected = 1;
	SetupSelectionColours();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshAdjuster::Kill(void)
{
	ReleaseObject();
	mcEdgeSelectionColours.Kill();
	mcFaceSelectionColours.Kill();
	mcVertSelectionColours.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshAdjuster::SetupSelectionColours(void)
{
	int					iRed, iGreen, iBlue;

	iRed = rand()%100+80;
	iGreen = rand()%100+80;
	iBlue = rand()%100+80;
	miObjectColour = SetM32BitColour(iRed, iGreen, iBlue, 255);
	miEdgeColour = SetM32BitColour(iRed+30, iGreen+30, iBlue+30, 255);
	miColourPartSelected = SetM32BitColour(255, 40, 40, 255);
	miColourObjectEdgeNotSelected = SetM32BitColour(255, 255, 255, 255);
	miColourObjectVertNotSelected = SetM32BitColour(100, 100, 255, 255);
	miColourBlendPartSelected = SetM32BitColour(255, 40, 40, 127);

	mcEdgeSelectionColours.Init(1);
	mcVertSelectionColours.Init(1);
	mcFaceSelectionColours.Init(1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshAdjuster::SetSelectionColours(void)
{
	SSelectionColour*	psSelectionColour;

	mcEdgeSelectionColours.Unuse();
	mcVertSelectionColours.Unuse();
	if (miSelected)
	{
		//Edge
		psSelectionColour = mcEdgeSelectionColours.Add();
		psSelectionColour->iSelection = 1;
		psSelectionColour->iColour = miColourPartSelected;

		psSelectionColour = mcEdgeSelectionColours.Add();
		psSelectionColour->iSelection = 2;
		psSelectionColour->iColour = miColourObjectEdgeNotSelected;

		psSelectionColour = mcEdgeSelectionColours.Add();
		psSelectionColour->iSelection = 3;
		psSelectionColour->iColour = miColourPartSelected;

		//Vert
		psSelectionColour = mcVertSelectionColours.Add();
		psSelectionColour->iSelection = 1;
		psSelectionColour->iColour = miColourObjectVertNotSelected;

		psSelectionColour = mcVertSelectionColours.Add();
		psSelectionColour->iSelection = 2;
		psSelectionColour->iColour = miColourPartSelected;

		psSelectionColour = mcVertSelectionColours.Add();
		psSelectionColour->iSelection = 3;
		psSelectionColour->iColour = miColourPartSelected;

		//Face
		psSelectionColour = mcFaceSelectionColours.Add();
		psSelectionColour->iSelection = 2;
		psSelectionColour->iColour = miColourBlendPartSelected;
	}
	else
	{
		//Edge
		psSelectionColour = mcEdgeSelectionColours.Add();
		psSelectionColour->iSelection = 2;
		psSelectionColour->iColour = miEdgeColour;

		psSelectionColour = mcEdgeSelectionColours.Add();
		psSelectionColour->iSelection = 3;
		psSelectionColour->iColour = miEdgeColour;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshAdjuster::Draw(void)
{
	BOOL	bDrawFaces;

	SetSelectionColours();

	bDrawFaces = FixBool(miMeshAdjusterDisplay & MAD_Faces);
	mpcMainGraphicsInstance->SetFlag(GRAPH_INST_FLAGS_DRAW, bDrawFaces);

	CreateObjectAsNecessary();
	mpcSelectionObject->BeginDynamic();

	mpcWorldEditor->mcMeshSelectionConverter.ConvertBoundingbox(mpcSelectionObject, mpcMainGraphicsInstance, miZBufferlessMaterialIndex, mpcMeshEditor, &mpcWorldEditor->mcWhiteZeroColours);
	mpcWorldEditor->mcMeshSelectionConverter.ConvertEdgeSelection(mpcSelectionObject, mpcMainGraphicsInstance, miLightlessMaterialIndex, mpcMeshEditor, &mcEdgeSelectionColours);
	mpcWorldEditor->mcMeshSelectionConverter.ConvertNormalSelection(mpcSelectionObject, mpcMainGraphicsInstance, miLightlessMaterialIndex, mpcMeshEditor, &mcEdgeSelectionColours);
	mpcWorldEditor->mcMeshSelectionConverter.ConvertFaceSelection(mpcSelectionObject, mpcMainGraphicsInstance, miBlendMaterialIndex, mpcMeshEditor, &mcFaceSelectionColours);
	mpcWorldEditor->mcMeshSelectionConverter.ConvertVertSelection(mpcSelectionObject, mpcMainGraphicsInstance, miLightlessMaterialIndex, mpcMeshEditor, &mcVertSelectionColours);
	mpcWorldEditor->mcMatrixSelectionConverter.ConvertMatrices(mpcSelectionObject, mpcMainLinkInstance, miLightlessMaterialIndex);

	mpcWorldEditor->mcMeshSelectionConverter.FinalisePrimitives();
	mpcWorldEditor->mcMatrixSelectionConverter.FinalisePrimitives();

	mpcSelectionObject->EndDynamic();
	mpcSelectionObject->Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshAdjuster::ReleaseObject(void)
{
	if (mpcSelectionObject)
	{
		mpcWorldEditor->mpcSceneConverter->GetWorld()->ReleaseGraphicsObject(mpcSelectionObject);
		mpcSelectionObject = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshAdjuster::CreateObjectAsNecessary(void)
{
	//Create the graphics object for the conversion destination.
	if (!mpcSelectionObject)
	{
		mpcSelectionObject = mpcWorldEditor->mpcSceneConverter->GetWorld()->CreateGraphicsObject(TRUE);
		miLightlessMaterialIndex= mpcSelectionObject->AddMaterial(mpcWorldEditor->mpCLightInstancelessMaterial);
		miZBufferlessMaterialIndex = mpcSelectionObject->AddMaterial(mpcWorldEditor->mpcZBufferlessMaterial);
		miBlendMaterialIndex = mpcSelectionObject->AddMaterial(mpcWorldEditor->mpcBlendMaterial);
		miStateIndex = mpcSelectionObject->AddState(mpcWorldEditor->mpcSceneConverter->GetWorld()->GetWhiteGraphicsState());
		mpcSelectionObject->AddVertexBuffer(D3DFVF_CVERTEX, 0);
		mpcSelectionObject->AddMatrix(mpcWorldEditor->mpcSceneConverter->GetWorld()->GetIdentityMatrix());							 
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshAdjuster::ClearVertSelection(void)
{
	mpcMeshEditor->mcSelections.ClearCornerSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshAdjuster::ClearEdgeSelection(void)
{
	mpcMeshEditor->mcSelections.ClearEdgeSelection();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshAdjuster::ClearFaceSelection(void)
{
	mpcMeshEditor->mcSelections.ClearFaceSelection();
}

