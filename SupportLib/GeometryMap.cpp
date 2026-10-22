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
#include "StandardLib/ClassDefines.h"
#include "ImageAccessorCreator.h"
#include "ColourBlend.h"
#include "GeometryMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::Init(void)
{
	PreInit();
	CBaseMap::Init();
	masCommands.Init();
	mpViewport = NULL;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::Free(void)
{
	masCommands.Kill();
	CBaseMap::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::Class(void)
{
	CBaseMap::Class();
	U_Data(CArrayGeometryCommand, masCommands);
	M_Pointer(mpViewport);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGeometryMap::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGeometryMap::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::Clear(void)
{
	masCommands.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// A filled rectangle in map pixels.  An empty rectangle isn't kept.
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::AddRectangle(int32 iLeft, int32 iTop, int32 iWidth, int32 iHeight, ARGB32 sColour)
{
	SGeometryCommand*	psCommand;

	if ((iWidth <= 0) || (iHeight <= 0))
	{
		return;
	}
	psCommand = masCommands.Add();
	psCommand->eType = GC_Rectangle;
	psCommand->cBounds.Init(iLeft, iTop, iLeft + iWidth, iTop + iHeight);
	psCommand->sColour = sColour;
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the part of each shape inside pcViewportRect (in map pixels, right
// and bottom exclusive) onto the viewport, the viewport rectangle's top
// left at the viewport's top left.
//
//////////////////////////////////////////////////////////////////////////
bool CGeometryMap::Blit(CInt32Rect* pcViewportRect, bool bEnsureBlitters)
{
	CImageAccessorCreator	cCreator;
	CImageAccessor*			pcAccessor;
	bool					bOpacity;
	size					i;
	SGeometryCommand*		psCommand;
	int32					iLeft;
	int32					iTop;
	int32					iRight;
	int32					iBottom;
	int32					iViewportRight;
	int32					iViewportBottom;

	if (mpViewport.IsNull() || (masCommands.NumElements() == 0))
	{
		return true;
	}

	//The viewport image clips too.
	iViewportRight = pcViewportRect->msTopLeft.x + (int32)mpViewport->GetWidth();
	if (pcViewportRect->msBottomRight.x < iViewportRight)
	{
		iViewportRight = pcViewportRect->msBottomRight.x;
	}
	iViewportBottom = pcViewportRect->msTopLeft.y + (int32)mpViewport->GetHeight();
	if (pcViewportRect->msBottomRight.y < iViewportBottom)
	{
		iViewportBottom = pcViewportRect->msBottomRight.y;
	}

	bOpacity = mpViewport->HasChannel(IMAGE_OPACITY);
	cCreator.Init(&mpViewport);
	if (bOpacity)
	{
		cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, PT_uint8);
	}
	else
	{
		cCreator.AddAccess(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, PT_uint8);
	}
	pcAccessor = cCreator.CreateAndKill();

	for (i = 0; i < masCommands.NumElements(); i++)
	{
		psCommand = masCommands.Get(i);
		iLeft = (psCommand->cBounds.msTopLeft.x > pcViewportRect->msTopLeft.x) ? psCommand->cBounds.msTopLeft.x : pcViewportRect->msTopLeft.x;
		iTop = (psCommand->cBounds.msTopLeft.y > pcViewportRect->msTopLeft.y) ? psCommand->cBounds.msTopLeft.y : pcViewportRect->msTopLeft.y;
		iRight = (psCommand->cBounds.msBottomRight.x < iViewportRight) ? psCommand->cBounds.msBottomRight.x : iViewportRight;
		iBottom = (psCommand->cBounds.msBottomRight.y < iViewportBottom) ? psCommand->cBounds.msBottomRight.y : iViewportBottom;
		if ((iLeft < iRight) && (iTop < iBottom))
		{
			FillRectangle(pcAccessor, bOpacity,
						  iLeft - pcViewportRect->msTopLeft.x, iTop - pcViewportRect->msTopLeft.y,
						  iRight - pcViewportRect->msTopLeft.x, iBottom - pcViewportRect->msTopLeft.y,
						  psCommand->sColour);
		}
	}

	pcAccessor->Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Blends the colour over each pixel as a translucent cel's pixel is
// blended (see BlendRGBByteAlphaByte and BlendAlphaByte): the colour by
// lerping with the alpha and the opacity, if there is one, by over.
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::FillRectangle(CImageAccessor* pcAccessor, bool bOpacity, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, ARGB32 sColour)
{
	uint8	auiPixel[4];
	uint8	uiRed;
	uint8	uiGreen;
	uint8	uiBlue;
	uint8	uiAlpha;
	int32	x;
	int32	y;

	uiRed = Get8BitRedColour(sColour);
	uiGreen = Get8BitGreenColour(sColour);
	uiBlue = Get8BitBlueColour(sColour);
	uiAlpha = Get8BitAlphaColour(sColour);

	for (y = iTop; y < iBottom; y++)
	{
		for (x = iLeft; x < iRight; x++)
		{
			pcAccessor->Get(x, y, auiPixel);
			auiPixel[0] = BlendLerpByte(auiPixel[0], uiRed, uiAlpha);
			auiPixel[1] = BlendLerpByte(auiPixel[1], uiGreen, uiAlpha);
			auiPixel[2] = BlendLerpByte(auiPixel[2], uiBlue, uiAlpha);
			if (bOpacity)
			{
				auiPixel[3] = BlendOverByte(auiPixel[3], uiAlpha);
			}
			pcAccessor->Set(x, y, auiPixel);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGeometryMap::SetViewport(Ptr<CImage> pViewport)
{
	mpViewport = pViewport;
}


Ptr<CImage> CGeometryMap::GetViewport(void) { return mpViewport; }
size CGeometryMap::NumCommands(void) { return masCommands.NumElements(); }
SGeometryCommand* CGeometryMap::GetCommand(size uiIndex) { return masCommands.Get(uiIndex); }

