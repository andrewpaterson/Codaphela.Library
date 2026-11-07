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
#include "Canvas.h"
#include "BlitCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlitCanvasDraw::Init(void)
{
	PreInit();
	CCanvasDraw::Init();
	mpDestImage = NULL;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBlitCanvasDraw::Class(void)
{
	CCanvasDraw::Class();
	M_Pointer(mpDestImage);
}


//////////////////////////////////////////////////////////////////////////
//
// The image to blit onto, NULL if the canvas has no area.  The image of
// its own is made again when the canvas's size or colour format changed
// (made transparent, the canvas has an opacity channel).
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CBlitCanvasDraw::StartBlit(Ptr<CCanvas> pCanvas)
{
	SInt32Vec2	sSize;
	Ptr<CImage>	pImage;

	sSize = pCanvas->GetNativeSize();
	if ((sSize.x <= 0) || (sSize.y <= 0))
	{
		return NULL;
	}

	pImage = pCanvas->GetImageOrNull();
	if (pImage.IsNotNull())
	{
		mpDestImage = NULL;
		return pImage;
	}

	if (mpDestImage.IsNull() || (mpDestImage->GetWidth() != (uint32)sSize.x) || (mpDestImage->GetHeight() != (uint32)sSize.y) ||
		(mpDestImage->GetColourFormat() != pCanvas->GetColourFormat()) || (mpDestImage->GetAlphaBits() != pCanvas->GetAlphaBits()))
	{
		mpDestImage = OMalloc<CImage>(sSize.x, sSize.y, pCanvas->GetColourFormat(), pCanvas->GetColourOrder(), pCanvas->GetColourBits(), pCanvas->GetAlphaBits());
		mpDestImage->Black();
	}
	else if (pCanvas->IsTransparent())
	{
		//The whole image is copied onto the canvas, so what an earlier draw left on it must go.
		mpDestImage->Black();
	}
	return mpDestImage;
}


//////////////////////////////////////////////////////////////////////////
//
// The image of its own, if the blit went onto one, is copied onto the
// canvas.
//
//////////////////////////////////////////////////////////////////////////
void CBlitCanvasDraw::FinishBlit(Ptr<CCanvas> pCanvas)
{
	if (mpDestImage.IsNotNull())
	{
		pCanvas->DrawImage(pCanvas->GetNativeOrigin().x, pCanvas->GetNativeOrigin().y, mpDestImage);
	}
}

