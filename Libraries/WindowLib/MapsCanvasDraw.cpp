/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "Canvas.h"
#include "MapsCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsCanvasDraw::Init(Ptr<CMaps> pMaps)
{
	PreInit();
	CBlitCanvasDraw::Init();
	mpMaps = pMaps;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapsCanvasDraw::Class(void)
{
	CBlitCanvasDraw::Class();
	M_Pointer(mpMaps);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapsCanvasDraw::Save(CObjectWriter* pcFile)
{
	CBlitCanvasDraw::Save(pcFile);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapsCanvasDraw::Load(CObjectReader* pcFile)
{
	CBlitCanvasDraw::Load(pcFile);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapsCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	bool			bResult;
	Ptr<CImage>		pImage;
	SInt32Vec2		sViewport;
	SInt32Vec2		sOrigin;
	CInt32Rect		cClip;
	bool			bClipped;

	if (!pCanvas->IsValid())
	{
		return true;
	}
	pImage = StartBlit(pCanvas);
	if (pImage.IsNull())
	{
		return true;
	}

	//The image being drawn onto keeps the blitters that draw onto it, so a new image starts with new blitters.
	mpMaps->SetCacheAndViewport(pImage->GetBlitterCache(), pImage);
	bResult = true;
	if (!mpMaps->HasCelBlitters())
	{
		bResult = mpMaps->CreateCelBlitters();
	}
	if (bResult)
	{
		//The image holds the part of the canvas from its native origin, so the maps are seen that much further on, and a
		//part of the canvas being drawn (its paint clip) is that much nearer the image's top left.
		sViewport = mpMaps->GetViewportPosition();
		sOrigin = pCanvas->GetNativeOrigin();
		mpMaps->SetViewportPosition(sViewport.x + sOrigin.x, sViewport.y + sOrigin.y);
		bClipped = pCanvas->GetPaintClip(&cClip);
		cClip.Init(cClip.GetLeft() - sOrigin.x, cClip.GetTop() - sOrigin.y, cClip.GetRight() - sOrigin.x, cClip.GetBottom() - sOrigin.y);
		bResult = mpMaps->Blit(false, bClipped ? &cClip : NULL);
		mpMaps->SetViewportPosition(sViewport);
	}

	FinishBlit(pCanvas);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The maps are blitted cut to the canvas's paint clip, straight onto a
// canvas with an image.  Onto one without, all of the image they are
// blitted onto is copied (CBlitCanvasDraw).
//
//////////////////////////////////////////////////////////////////////////
bool CMapsCanvasDraw::CanDrawPart(Ptr<CCanvas> pCanvas)
{
	return pCanvas->GetImageOrNull().IsNotNull();
}

