/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "Quad2D.h"
#include "World.h"
#include "SupportLib/ColourARGB32.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::Init(CGraphicsMaterial* pcGraphicsMaterial, CGraphicsState* pcGraphicsState, CGraphicsViewport* pcGraphicsViewport, int iWidth, int iHeight, int x, int y)
{
	int		i;

	for (i = 0; i < MAX_QUAD_UV_LAYERS; i++)
	{
		SetUVCoords(i, 0.0f, 0.0f, 1.0f, 1.0f);
	}
	mpcGraphicsMaterial = pcGraphicsMaterial;
	mpcGraphicsState = pcGraphicsState;
	mpcGraphicsViewport = pcGraphicsViewport;
	msSize.x = iWidth;
	msSize.y = iHeight;
	msPosition.x = x; 
	msPosition.y = y;
	msPosition.z = 0;
	mdwColour = 0xffffffff;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::Kill(void)
{
	CUnknown::Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::SetPosition(SInt3* psPosition)
{
	msPosition.x = psPosition->x;
	msPosition.y = psPosition->y;
	msPosition.z = psPosition->z;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::SetPosition(int x, int y, int z)
{
	msPosition.x = x;
	msPosition.y = y;
	msPosition.z = z;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt3 CQuad2D::GetPosition(void)
{
	return msPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CQuad2D::Draw(CGraphicsObject* pcGraphicsObject)
{
	int							i;
	SFloat4*					psTopLeft;
	SFloat4*					psTopRight1;
	SFloat4*					psTopRight2;
	SFloat4*					psBottomLeft1;
	SFloat4*					psBottomLeft2;
	SFloat4*					psBottomRight;
	SFloat2*					psUVTL;
	SFloat2*					psUVTR1;
	SFloat2*					psUVBL1;
	SFloat2*					psUVTR2;
	SFloat2*					psUVBL2;
	SFloat2*					psUVBR;
	void*						pvFirstVert;
	int							iOffset;
	int							iMaterialIndex;
	int							iStateIndex;
	int							iViewportIndex;
	int							iD3DVertexType;
	bool						bColour;
	CVertexBufferExtended*		psVertexBuffer;
	CGraphicsPrimitive*			pcGraphicsPrimitive;
	int							iSize;
	int							iColour;

	//Make sure there is something to draw.
	if (!IsValid())
	{
		return false;
	}

	iMaterialIndex = pcGraphicsObject->AddMaterial(mpcGraphicsMaterial);
	iStateIndex = pcGraphicsObject->AddState(mpcGraphicsState);
	iViewportIndex = pcGraphicsObject->AddViewport(mpcGraphicsViewport);

	bColour = (mdwColour != 0xffffffff);

	iD3DVertexType = gcD3D.GetVertexFormatFor(false, bColour, mpcGraphicsMaterial->GetNumTextures(), 0, true);

	pvFirstVert = pcGraphicsObject->GrowPrimitiveNew(2, D3DPT_TRIANGLELIST, 6, iD3DVertexType, iMaterialIndex, iStateIndex, iViewportIndex, &pcGraphicsPrimitive);
	psVertexBuffer = pcGraphicsObject->GetVertexBufferForIndex(pcGraphicsPrimitive->miVertexBufferIndex);
	iSize = gcD3D.GetVertexFormatSize(iD3DVertexType);
	if (iSize != psVertexBuffer->iVertexSize)
	{
		gcUserError.Set("Vertex size in buffer is different to registered size.");
	}

	psBottomLeft1 =		(SFloat4*)pvFirstVert;
	psTopLeft =			(SFloat4*)RemapSinglePointer(pvFirstVert, psVertexBuffer->iVertexSize*1);
	psTopRight1 =		(SFloat4*)RemapSinglePointer(pvFirstVert, psVertexBuffer->iVertexSize*2);
	psTopRight2 =		(SFloat4*)RemapSinglePointer(pvFirstVert, psVertexBuffer->iVertexSize*3);
	psBottomRight =		(SFloat4*)RemapSinglePointer(pvFirstVert, psVertexBuffer->iVertexSize*4);
	psBottomLeft2 =		(SFloat4*)RemapSinglePointer(pvFirstVert, psVertexBuffer->iVertexSize*5);

	psTopLeft->z = psTopRight1->z = psTopRight2->z = psBottomLeft1->z = psBottomLeft2->z = psBottomRight->z = (float)msPosition.z;

	psTopLeft->x =							(float)msPosition.x;
	psTopLeft->y =							(float)msPosition.y;
	psTopRight1->x = psTopRight2->x =		(float)(msPosition.x + msSize.x);
	psTopRight1->y = psTopRight2->y =		psTopLeft->y;
	psBottomLeft1->x = psBottomLeft2->x =	psTopLeft->x;
	psBottomLeft1->y = psBottomLeft2->y =	(float)(msPosition.y + msSize.y);
	psBottomRight->x =						psTopRight1->x;
	psBottomRight->y =						psBottomLeft1->y;

	//If this is a transformed vertex then the w coord must be set also.
	if (iD3DVertexType & D3DFVF_XYZRHW)
	{
		//Can't remember what to do if the w coordinate goes to infinity?
		psTopLeft->w = psTopRight1->w = psBottomLeft1->w = psTopRight2->w = psBottomLeft2->w = psBottomRight->w = 1.0f;
	}

	if (iD3DVertexType & D3DFVF_DIFFUSE)
	{
		iOffset = gcD3D.GetVertetFormatColourOffset(iD3DVertexType);
		iColour = ConvertToBGRA(mdwColour);

		*((int*)RemapSinglePointer(psTopLeft, iOffset)) = iColour;
		*((int*)RemapSinglePointer(psTopRight1, iOffset)) = iColour;
		*((int*)RemapSinglePointer(psBottomLeft1, iOffset)) = iColour;
		*((int*)RemapSinglePointer(psTopRight2, iOffset)) = iColour;
		*((int*)RemapSinglePointer(psBottomLeft2, iOffset)) = iColour;
		*((int*)RemapSinglePointer(psBottomRight, iOffset)) = iColour;
	}

	//For every texture layer...
	for (i = 0; i < mpcGraphicsMaterial->GetNumTextures(); i++)
	{
		//Assign the UV coordinates.
		iOffset = gcD3D.GetVertexFormatUVOffset(iD3DVertexType, i);
		psUVTL = (SFloat2*)RemapSinglePointer(psTopLeft, iOffset);
		psUVTR1 = (SFloat2*)RemapSinglePointer(psTopRight1, iOffset);
		psUVBL1 = (SFloat2*)RemapSinglePointer(psBottomLeft1, iOffset);
		psUVTR2 = (SFloat2*)RemapSinglePointer(psTopRight2, iOffset);
		psUVBL2 = (SFloat2*)RemapSinglePointer(psBottomLeft2, iOffset);
		psUVBR = (SFloat2*)RemapSinglePointer(psBottomRight, iOffset);

		psUVTL->x =					masUVs[i].asUV[0].x;
		psUVTL->y =					masUVs[i].asUV[0].y;
		psUVTR1->x = psUVTR2->x =	masUVs[i].asUV[1].x;
		psUVTR1->y = psUVTR2->y =	masUVs[i].asUV[1].y;
		psUVBR->x = 				masUVs[i].asUV[2].x;
		psUVBR->y = 				masUVs[i].asUV[2].y;
		psUVBL1->x = psUVBL2->x =	masUVs[i].asUV[3].x;
		psUVBL1->y = psUVBL2->y =	masUVs[i].asUV[3].y;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::SetColour(float r, float g, float b, float a)
{
	SetColour(Set32BitColour(r, g, b, a));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::SetGraphicsMaterial(CGraphicsMaterial* psMaterial)
{
	if (psMaterial->GetNumTextures() < MAX_QUAD_UV_LAYERS)
	{
		mpcGraphicsMaterial = psMaterial;
	}
	else
	{
		mpcGraphicsMaterial = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::SetGraphicsState(CGraphicsState* psState)
{
	mpcGraphicsState = psState;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::SetUVCoordsFromRectangle(int iTextureLayer, CRectangle* psRect)
{
	SFloat2		s1;
	SFloat2		s2;

	s1 = psRect->GetUVCoordinatesTopLeft(mpcGraphicsMaterial->GetTexture(iTextureLayer)->GetWidth(), mpcGraphicsMaterial->GetTexture(iTextureLayer)->GetHeight());
	s2 = psRect->GetUVCoordinatesBottomRight(mpcGraphicsMaterial->GetTexture(iTextureLayer)->GetWidth(), mpcGraphicsMaterial->GetTexture(iTextureLayer)->GetHeight());
	SetUVCoords(iTextureLayer, s1.x, s1.y, s2.x, s2.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::SetUVCoords(int iTextureLayer, float u1, float v1, float u2, float v2)
{
	masUVs[iTextureLayer].asUV[0].x = u1;
	masUVs[iTextureLayer].asUV[0].y = v1;
	masUVs[iTextureLayer].asUV[1].x = u2;
	masUVs[iTextureLayer].asUV[1].y = v1;
	masUVs[iTextureLayer].asUV[2].x = u2;
	masUVs[iTextureLayer].asUV[2].y = v2;
	masUVs[iTextureLayer].asUV[3].x = u1;
	masUVs[iTextureLayer].asUV[3].y = v2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::SetUVCoords(int iTextureLayer, SUVCornersFull* psUVs)
{
	if ((iTextureLayer < MAX_QUAD_UV_LAYERS) && (iTextureLayer >= 0))
	{
		memcpy(&masUVs[iTextureLayer].asUV, psUVs, sizeof(SUVCornersFull));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::SetDimensions(int iWidth, int iHeight)
{
	msSize.x = iWidth;
	msSize.y = iHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::MovePosition(int x, int y)
{
	msPosition.x += x;
	msPosition.y += y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CQuad2D::IsValid(void)
{
	return (mpcGraphicsMaterial != NULL) && (mpcGraphicsState != NULL) && (mpcGraphicsViewport != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CQuad2D::IsInViewport(void)
{
	return mpcGraphicsViewport->Contains((int)msPosition.x, (int)msPosition.y, (int)msPosition.x + msSize.x, (int)msPosition.y + msSize.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SUVCornersFull* CQuad2D::GetUVs(int iLayer)
{
	if (iLayer < mpcGraphicsMaterial->GetNumTextures())
	{
		return &masUVs[iLayer];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuad2D::SetColour(unsigned int dwColour)
{
	mdwColour = dwColour;
}
