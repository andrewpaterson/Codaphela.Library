/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "StandardLib/Objects.h"
#include "SupportLib/ImageColourConverter.h"
#include "Canvas.h"
#include "CelCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCelCanvasDraw::Init(void)
{
	PreInit();

	CCanvasDraw::Init();
	mpCelDraw = OMalloc<CImageCanvasDraw>(Ptr<CImageCel>(), false);
	msCelSize.Init(-1, -1);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCelCanvasDraw::Class(void)
{
	CCanvasDraw::Class();

	M_Pointer(mpCelDraw);
	U_2Int32(msCelSize);
}


//////////////////////////////////////////////////////////////////////////
//
// A canvas draw isn't saved.
//
//////////////////////////////////////////////////////////////////////////
bool CCelCanvasDraw::Save(CObjectWriter* pcFile) { return false; }
bool CCelCanvasDraw::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// The cel is painted if there's none for this size, in an image with
// opacity (which the anti-aliased shapes need) that starts see-through,
// then made the canvas's colour order.
//
//////////////////////////////////////////////////////////////////////////
bool CCelCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	SInt32Vec2		sSize;
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;

	sSize = pCanvas->GetActualSize();
	if ((sSize.x <= 0) || (sSize.y <= 0))
	{
		return true;
	}

	if ((sSize.x != msCelSize.x) || (sSize.y != msCelSize.y))
	{
		pImage = OMalloc<CImage>(sSize.x, sSize.y, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
		pImage->Black();
		cDraw.Init(&pImage);
		Paint(&cDraw, sSize);
		cDraw.Kill();

		pImage = CImageColourConverter::ConvertColour(pImage, pCanvas->GetColourOrder());
		mpCelDraw->SetCel(OMalloc<CImageCel>(pImage));
		msCelSize = sSize;
	}
	return mpCelDraw->Draw(pCanvas);
}


//////////////////////////////////////////////////////////////////////////
//
// What's painted has changed: the cel is painted again at the next draw.
// The canvas still has to be told to draw (Invalidate).
//
//////////////////////////////////////////////////////////////////////////
void CCelCanvasDraw::Repaint(void)
{
	msCelSize.Init(-1, -1);
}


//////////////////////////////////////////////////////////////////////////
//
// The cel is what a draw would show: nothing has changed since it was
// painted.
//
//////////////////////////////////////////////////////////////////////////
bool CCelCanvasDraw::IsPainted(void)
{
	return msCelSize.x >= 0;
}

