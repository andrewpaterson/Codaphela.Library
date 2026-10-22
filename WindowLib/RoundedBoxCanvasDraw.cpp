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
#include "SupportLib/ImageDraw.h"
#include "SupportLib/ImageColourConverter.h"
#include "Canvas.h"
#include "RoundedBoxCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
// psInsets can be NULL for a block across the whole canvas.
//
//////////////////////////////////////////////////////////////////////////
void CRoundedBoxCanvasDraw::Init(ARGB32 sColour, int32 iRadius, SInsets* psInsets)
{
	PreInit();

	CCanvasDraw::Init();
	mpCelDraw = OMalloc<CImageCanvasDraw>(Ptr<CImageCel>(), false);
	msColour = sColour;
	miRadius = iRadius;
	if (psInsets)
	{
		memcpy(&msInsets, psInsets, sizeof(SInsets));
	}
	else
	{
		msInsets.Init();
	}
	msCelSize.Init(-1, -1);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoundedBoxCanvasDraw::Class(void)
{
	CCanvasDraw::Class();
	M_Pointer(mpCelDraw);
	U_UInt32(msColour);
	U_Int32(miRadius);
	U_Data(SInsets, msInsets);
	U_2Int32(msCelSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoundedBoxCanvasDraw::Free(void)
{
	CCanvasDraw::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoundedBoxCanvasDraw::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoundedBoxCanvasDraw::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoundedBoxCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	SInt32Vec2	sSize;

	sSize = pCanvas->GetActualSize();
	if ((sSize.x <= 0) || (sSize.y <= 0))
	{
		return true;
	}

	if ((sSize.x != msCelSize.x) || (sSize.y != msCelSize.y) || mpCelDraw->GetCel().IsNull())
	{
		MakeCel(pCanvas, sSize);
	}
	return mpCelDraw->Draw(pCanvas);
}


//////////////////////////////////////////////////////////////////////////
//
// The block's edges are on pixel edges; only its corners are blended.
//
//////////////////////////////////////////////////////////////////////////
void CRoundedBoxCanvasDraw::MakeCel(Ptr<CCanvas> pCanvas, SInt32Vec2 sSize)
{
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;
	CImageColourARGB	cColour;
	float64				fLeft;
	float64				fTop;
	float64				fRight;
	float64				fBottom;

	pImage = OMalloc<CImage>(sSize.x, sSize.y, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();

	fLeft = msInsets.maiWidths[BEI_Left];
	fTop = msInsets.maiWidths[BEI_Top];
	fRight = sSize.x - msInsets.maiWidths[BEI_Right];
	fBottom = sSize.y - msInsets.maiWidths[BEI_Bottom];

	cDraw.Init(&pImage);
	cColour.Init(Get8BitAlphaColour(msColour) / 255.0f, Get8BitRedColour(msColour) / 255.0f, Get8BitGreenColour(msColour) / 255.0f, Get8BitBlueColour(msColour) / 255.0f);
	cDraw.SetColour(&cColour);
	cColour.Kill();
	if (miRadius > 0)
	{
		cDraw.DrawRoundedRectangle(fLeft, fTop, fRight, fBottom, miRadius);
	}
	else
	{
		cDraw.DrawRectangle(fLeft, fTop, fRight, fBottom);
	}
	cDraw.Kill();

	pImage = CImageColourConverter::ConvertColour(pImage, pCanvas->GetColourOrder());
	mpCelDraw->SetCel(OMalloc<CImageCel>(pImage));
	msCelSize = sSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoundedBoxCanvasDraw::SetColour(ARGB32 sColour)
{
	if (msColour != sColour)
	{
		msColour = sColour;
		msCelSize.Init(-1, -1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CRoundedBoxCanvasDraw::GetColour(void) { return msColour; }

