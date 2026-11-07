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
#include "ScrollBarImages.h"


//////////////////////////////////////////////////////////////////////////
//
// iThickness is across the thumb.  sColour's alpha is the thumb's
// opacity, hovered or not.  pcFormat may be NULL.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarImages::Init(int32 iThickness, ARGB32 sColour, CColourFormatHelper* pcFormat)
{
	Init(iThickness, sColour, sColour, pcFormat);
}


//////////////////////////////////////////////////////////////////////////
//
// sColour is the thumb's colour, and sHoverColour its colour while the
// pointer is over the scroll bar or it is being dragged.  Their alphas are
// the thumb's opacity.  pcFormat may be NULL.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarImages::Init(int32 iThickness, ARGB32 sColour, ARGB32 sHoverColour, CColourFormatHelper* pcFormat)
{
	PreInit();

	maCels.Init();
	maHoverCels.Init();
	miThickness = iThickness;
	msColour = sColour;
	msHoverColour = sHoverColour;
	meOrder = pcFormat ? pcFormat->meOrder : CCO_RGB;

	CreateCels(&maCels, msColour);
	CreateCels(&maHoverCels, msHoverColour);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The pieces in sColour, NUM_SCROLL_BAR_PIECES for each orientation in
// turn.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarImages::CreateCels(CArray<CImageCel>* paDest, ARGB32 sColour)
{
	uint	iOrientation;

	for (iOrientation = 0; iOrientation < NUM_SCROLL_BAR_ORIENTATIONS; iOrientation++)
	{
		paDest->Add(CreateCap((EScrollBarOrientation)iOrientation, SBP_Start, sColour));
		paDest->Add(CreateMiddle((EScrollBarOrientation)iOrientation, sColour));
		paDest->Add(CreateCap((EScrollBarOrientation)iOrientation, SBP_End, sColour));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarImages::Class(void)
{
	M_Embedded(maCels);
	M_Embedded(maHoverCels);
	U_Int32(miThickness);
	U_UInt32(msColour);
	U_UInt32(msHoverColour);
	U_Enum(meOrder);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarImages::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScrollBarImages::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CScrollBarImages::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// An image with straight (not premultiplied) opacity, like one read from
// a PNG, that starts clear.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CScrollBarImages::CreateImage(int32 iWidth, int32 iHeight)
{
	Ptr<CImage>	pImage;

	pImage = OMalloc<CImage>(iWidth, iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// The start of a horizontal thumb is the left half of a circle, its end
// the right half.  A vertical thumb's are the top and bottom halves.  The
// whole circle is drawn and the cap's image clips it so when the thumb is
// an odd number of pixels thick the circle's middle row or column is all
// in the cap and meets the middle without a seam.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CScrollBarImages::CreateCap(EScrollBarOrientation eOrientation, EScrollBarPiece ePiece, ARGB32 sColour)
{
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;
	int32			iOffset;

	iOffset = (ePiece == SBP_Start) ? 0 : GetCapLength() - miThickness;
	if (eOrientation == SBO_Horizontal)
	{
		pImage = CreateImage(GetCapLength(), miThickness);
		cDraw.Init(&pImage);
		cDraw.SetColour(sColour);
		cDraw.DrawSemicircle(iOffset, 0, miThickness, 0, 360, true);
	}
	else
	{
		pImage = CreateImage(miThickness, GetCapLength());
		cDraw.Init(&pImage);
		cDraw.SetColour(sColour);
		cDraw.DrawSemicircle(0, iOffset, miThickness, 0, 360, true);
	}
	cDraw.Kill();

	return CreateCel(pImage);
}


//////////////////////////////////////////////////////////////////////////
//
// One pixel long and as thick as the thumb.  It is stretched to the
// thumb's length between its ends.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CScrollBarImages::CreateMiddle(EScrollBarOrientation eOrientation, ARGB32 sColour)
{
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;

	if (eOrientation == SBO_Horizontal)
	{
		pImage = CreateImage(1, miThickness);
	}
	else
	{
		pImage = CreateImage(miThickness, 1);
	}

	cDraw.Init(&pImage);
	cDraw.SetColour(sColour);
	cDraw.DrawBox(0, 0, pImage->GetWidth() - 1, pImage->GetHeight() - 1, true);
	cDraw.Kill();

	return CreateCel(pImage);
}


//////////////////////////////////////////////////////////////////////////
//
// The pieces are drawn in RGB order and converted to the format's order.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CScrollBarImages::CreateCel(Ptr<CImage> pImage)
{
	if (meOrder != CCO_RGB)
	{
		pImage = CImageColourConverter::ConvertColour(pImage, meOrder);
	}
	return OMalloc<CImageCel>(pImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CScrollBarImages::GetCel(EScrollBarOrientation eOrientation, EScrollBarPiece ePiece, bool bHover)
{
	return (bHover ? &maHoverCels : &maCels)->Get(eOrientation * NUM_SCROLL_BAR_PIECES + ePiece);
}


//////////////////////////////////////////////////////////////////////////
//
// How far each half circle end reaches along the thumb.
//
//////////////////////////////////////////////////////////////////////////
int32 CScrollBarImages::GetCapLength(void)
{
	return (miThickness + 1) / 2;
}


int32 CScrollBarImages::GetThickness(void) { return miThickness; }
ARGB32 CScrollBarImages::GetColour(void) { return msColour; }
ARGB32 CScrollBarImages::GetHoverColour(void) { return msHoverColour; }

