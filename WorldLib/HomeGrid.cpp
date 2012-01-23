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
#include "BaseLib/PointerRemapper.h"
#include "SceneConverter.h"
#include "HomeGrid.h"
#include "WorldEditor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHomeGrid::Init(CWorldEditor* pcWorldEditor)
{
	mpcWorldEditor = pcWorldEditor;

	mpcWorld = mpcWorldEditor->mpcSceneConverter->GetWorld();
	mpsMatrix = mpcWorld->CreateMatrix();
	msSpacing.x = 10.0f;
	msSpacing.y = 10.0f;
	msMajorLineGap.x = 10;
	msMajorLineGap.y = 10;
	msSize.x = 400.0f;
	msSize.y = 400.0f;
	miCenterColour = 0x00000000;
	miMajorColour = 0xffe0e0e0;
	miMinorColour = 0xffa0a0a0;

	mpcGraphicsObject = mpcWorld->CreateGraphicsObject(TRUE);
	mpcGridMaterial = mpcWorld->GetWhiteGraphicsMaterial();
	mpcGridState = mpcWorld->GetWhiteGraphicsState();
	mpcGraphicsObject->AddVertexBuffer(D3DFVF_CVERTEX, 0);
	mpcGraphicsObject->AddMatrix(mpsMatrix);

	mfPictureTranslucency = 1.0f;
	mpcPictureMaterial = NULL;
	mpcPictureState = NULL;
	Float2Assign(&msPictureUVs.sUVTopLeft, 0.0f, 0.0f);
	Float2Assign(&msPictureUVs.sUVBottomRight, 1.0f, 1.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHomeGrid::Kill(void)
{
	mpcWorld->ReleaseMatrix(mpsMatrix);
	mpcWorld->ReleaseGraphicsObject(mpcGraphicsObject);
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHomeGrid::SetGrid(float fSizeX, float fSizeY, float fSpaceX, float fSpaceY)
{
	msSpacing.x = fSpaceX;
	msSpacing.y = fSpaceY;
	msSize.x = fSizeX;
	msSize.y = fSizeY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHomeGrid::Draw(void)
{
	int						iXSize;
	int						iZSize;
	D3DXVECTOR2				sTemp;
	CGraphicsPrimitive*		pcGraphicsPrimitive;
	int						iNumPrimitives;
	D3DCVERTEX*				psVertex;
	int						iZCenterLine;
	int						iXCenterLine;
	SFloat3				sMinOffset;
	SFloat3				sMaxOffset;
	SFloat3*			psCenter;
	SFloat3*			psXAxis;
	SFloat3*			psZAxis;

	if (!mpcGraphicsObject->BeginDynamic())
	{
		return FALSE;
	}

	pcGraphicsPrimitive = mpcGraphicsObject->StartPrimitive(D3DPT_LINELIST, D3DFVF_CVERTEX, mpcGridMaterial, mpcGridState, mpcWorld->mpcFullViewport);
	if (!pcGraphicsPrimitive)
	{
		return FALSE;
	}

	pcGraphicsPrimitive->miFlags = GRAPH_PRIM_FLAGS_DRAW;

	sTemp.x = (msSize.x / msSpacing.x) + 0.333f;
	sTemp.y = (msSize.y / msSpacing.y) + 0.333f;

	iXSize = (int)sTemp.x;
	iZSize = (int)sTemp.y;
	iXCenterLine = iXSize / 2;
	iZCenterLine = iZSize / 2;

	psCenter = mpsMatrix->sD3DMatrix.Pos();
	psXAxis = mpsMatrix->sD3DMatrix.Across();
	psZAxis = mpsMatrix->sD3DMatrix.At();

	iNumPrimitives = iXSize + 1 +iZSize + 1;
	psVertex = (D3DCVERTEX*)mpcGraphicsObject->AddDynamicVerticies(pcGraphicsPrimitive, D3DFVF_CVERTEX, iNumPrimitives, iNumPrimitives * 2);

	psVertex = DrawHalfGrid(psVertex, psCenter, psXAxis, psZAxis, -iZCenterLine, iZSize - iZCenterLine, msSpacing.y, msSize.x);
	psVertex = DrawHalfGrid(psVertex, psCenter, psZAxis, psXAxis, -iXCenterLine, iXSize - iXCenterLine, msSpacing.x, msSize.y);

	if (!mpcGraphicsObject->EndDynamic())
	{
		return FALSE;
	}

	return mpcGraphicsObject->Draw();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
D3DCVERTEX*	CHomeGrid::DrawHalfGrid(D3DCVERTEX* psVertex, SFloat3* psCenter, SFloat3* psPerp, SFloat3* psDir, int iMin, int iMax, float fStep, float fWidth)
{
	int				i;
	SFloat3		sPos;
	SFloat3		sWidth;
	SFloat3		sVec;
	unsigned int				iColour;

	Float3Scale(&sWidth, psPerp, fWidth / 2.0f);
	for (i = iMin; i <= iMax; i++)
	{
		iColour = miMinorColour;
		if (i % msMajorLineGap.y == 0)
		{
			iColour = miMajorColour;
		}
		if (i == 0)
		{
			iColour = miCenterColour;
		}
		
		Float3Scale(&sPos, psDir, i * fStep);
		Float3Add(&sVec, &sPos, psCenter);

		Float3Subtract(&sPos, &sVec, &sWidth);
		Float3Assign((SFloat3*)&psVertex->x, &sPos);
		psVertex->color = iColour;
		psVertex = (D3DCVERTEX*)RemapSinglePointer(psVertex, sizeof(D3DCVERTEX));

		Float3Add(&sPos, &sVec, &sWidth);
		Float3Assign((SFloat3*)&psVertex->x, &sPos);
		psVertex->color = iColour;
		psVertex = (D3DCVERTEX*)RemapSinglePointer(psVertex, sizeof(D3DCVERTEX));
	}
	return psVertex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHomeGrid::SetColours(int iCenterColour, int iMajorColour, int iMinorColour)
{
	miCenterColour = iCenterColour;
	miMajorColour = iMajorColour;
	miMinorColour = iMinorColour;
}

