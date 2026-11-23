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
#include "SupportLib/VectorPath.h"
#include "Canvas.h"
#include "RoundedCornerCanvasDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoundedCornerCanvasDraw::Init(ARGB32 sColour, ERoundedCorner eCorner)
{
	PreInit();

	CCanvasDraw::Init();
	mpCelDraw = OMalloc<CImageCanvasDraw>(Ptr<CImageCel>(), false);
	msColour = sColour;
	meCorner = eCorner;
	msCelSize.Init(-1, -1);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoundedCornerCanvasDraw::Class(void)
{
	CCanvasDraw::Class();
	M_Pointer(mpCelDraw);
	U_UInt32(msColour);
	U_Enum(meCorner);
	U_2Int32(msCelSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoundedCornerCanvasDraw::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoundedCornerCanvasDraw::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A canvas that isn't square is drawn as if it were as big as its
// smaller side.
//
//////////////////////////////////////////////////////////////////////////
bool CRoundedCornerCanvasDraw::Draw(Ptr<CCanvas> pCanvas)
{
	SInt32Vec2	sSize;
	int32		iRadius;

	sSize = pCanvas->GetActualSize();
	iRadius = (sSize.x < sSize.y) ? sSize.x : sSize.y;
	if (iRadius <= 0)
	{
		return true;
	}

	if ((sSize.x != msCelSize.x) || (sSize.y != msCelSize.y) || mpCelDraw->GetCel().IsNull())
	{
		MakeCel(pCanvas, iRadius);
		msCelSize = sSize;
	}
	return mpCelDraw->Draw(pCanvas);
}


//////////////////////////////////////////////////////////////////////////
//
// The piece runs from the corner along one edge to where the arc starts,
// round the arc to the other edge, and back to the corner.  The arc's
// centre is the square's opposite corner.  Arcs turn the other way round
// in a mirrored corner.
//
//////////////////////////////////////////////////////////////////////////
void CRoundedCornerCanvasDraw::MakeCel(Ptr<CCanvas> pCanvas, int32 iRadius)
{
	Ptr<CImage>		pImage;
	CImageDraw		cDraw;
	CVectorPath		cPath;
	bool			bRight;
	bool			bBottom;
	SFloat64Vec2	sCorner;
	SFloat64Vec2	sCenter;
	SFloat64Vec2	sPoint;
	float64			fRadius;

	pImage = OMalloc<CImage>(iRadius, iRadius, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();

	fRadius = iRadius;
	bRight = (meCorner == RC_TopRight) || (meCorner == RC_BottomRight);
	bBottom = (meCorner == RC_BottomLeft) || (meCorner == RC_BottomRight);
	sCorner.Init(bRight ? fRadius : 0, bBottom ? fRadius : 0);
	sCenter.Init(bRight ? 0 : fRadius, bBottom ? 0 : fRadius);

	cPath.Init();
	cPath.MoveTo(&sCorner);
	sPoint.Init(sCenter.x, sCorner.y);
	cPath.LineTo(&sPoint);
	sPoint.Init(sCorner.x, sCenter.y);
	cPath.ArcTo(&sPoint, &sCenter, bRight == bBottom);
	cPath.LineTo(&sCorner);

	cDraw.Init(&pImage);
	cDraw.SetColour(msColour);
	cDraw.DrawPath(&cPath);
	cDraw.Kill();
	cPath.Kill();

	pImage = CImageColourConverter::ConvertColour(pImage, pCanvas->GetColourOrder());
	mpCelDraw->SetCel(OMalloc<CImageCel>(pImage));
}


//////////////////////////////////////////////////////////////////////////
//
// Whoever sets it invalidates the canvas.
//
//////////////////////////////////////////////////////////////////////////
void CRoundedCornerCanvasDraw::SetColour(ARGB32 sColour)
{
	if (msColour != sColour)
	{
		msColour = sColour;
		msCelSize.Init(-1, -1);
	}
}


ARGB32 CRoundedCornerCanvasDraw::GetColour(void) const { return msColour; }
ERoundedCorner CRoundedCornerCanvasDraw::GetCorner(void) const { return meCorner; }

