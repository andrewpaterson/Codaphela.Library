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
#include "ScrollBarImages.h"


//////////////////////////////////////////////////////////////////////////
//
// iThickness is across the thumb.  sColour's alpha is the thumb's
// opacity.
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarImages::Init(int32 iThickness, ARGB32 sColour)
{
	int		iOrientation;

	PreInit();

	maCels.Init();
	miThickness = iThickness;
	msColour = sColour;

	for (iOrientation = 0; iOrientation < NUM_SCROLL_BAR_ORIENTATIONS; iOrientation++)
	{
		maCels.Add(CreateCap((EScrollBarOrientation)iOrientation, SBP_Start));
		maCels.Add(CreateMiddle((EScrollBarOrientation)iOrientation));
		maCels.Add(CreateCap((EScrollBarOrientation)iOrientation, SBP_End));
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarImages::Class(void)
{
	M_Embedded(maCels);
	U_Int32(miThickness);
	U_UInt32(msColour);
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
//
//////////////////////////////////////////////////////////////////////////
void CScrollBarImages::SetColour(CImageDraw* pcDraw)
{
	CImageColourARGB	cColour;

	cColour.Init(Get8BitAlphaColour(msColour) / 255.0f, Get8BitRedColour(msColour) / 255.0f, Get8BitGreenColour(msColour) / 255.0f, Get8BitBlueColour(msColour) / 255.0f);
	pcDraw->SetColour(&cColour);
	cColour.Kill();
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
Ptr<CImageCel> CScrollBarImages::CreateCap(EScrollBarOrientation eOrientation, EScrollBarPiece ePiece)
{
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;
	int32			iOffset;

	iOffset = (ePiece == SBP_Start) ? 0 : GetCapLength() - miThickness;
	if (eOrientation == SBO_Horizontal)
	{
		pImage = CreateImage(GetCapLength(), miThickness);
		cDraw.Init(&pImage);
		SetColour(&cDraw);
		cDraw.DrawSemicircle(iOffset, 0, miThickness, 0, 360, true);
	}
	else
	{
		pImage = CreateImage(miThickness, GetCapLength());
		cDraw.Init(&pImage);
		SetColour(&cDraw);
		cDraw.DrawSemicircle(0, iOffset, miThickness, 0, 360, true);
	}
	cDraw.Kill();

	return OMalloc<CImageCel>(pImage);
}


//////////////////////////////////////////////////////////////////////////
//
// One pixel long and as thick as the thumb.  It is stretched to the
// thumb's length between its ends.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CScrollBarImages::CreateMiddle(EScrollBarOrientation eOrientation)
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
	SetColour(&cDraw);
	cDraw.DrawBox(0, 0, pImage->GetWidth() - 1, pImage->GetHeight() - 1, true);
	cDraw.Kill();

	return OMalloc<CImageCel>(pImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CScrollBarImages::GetCel(EScrollBarOrientation eOrientation, EScrollBarPiece ePiece)
{
	return maCels.Get(eOrientation * NUM_SCROLL_BAR_PIECES + ePiece);
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

