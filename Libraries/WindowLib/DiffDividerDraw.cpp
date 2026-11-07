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
#include <string.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ImageDraw.h"
#include "SupportLib/ImageColourConverter.h"
#include "Canvas.h"
#include "DiffDividerDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiffDividerDraw::Init(ARGB32 sBackground)
{
	PreInit();

	CCanvasDraw::Init();
	mpCelDraw = OMalloc<CImageCanvasDraw>(Ptr<CImageCel>(), false);
	msBackground = sBackground;
	masShapes.Init();
	msCelSize.Init(-1, -1);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiffDividerDraw::Class(void)
{
	CCanvasDraw::Class();
	M_Pointer(mpCelDraw);
	U_UInt32(msBackground);
	U_Data(CArrayDiffDividerShape, masShapes);
	U_2Int32(msCelSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiffDividerDraw::Free(void)
{
	masShapes.Kill();
	CCanvasDraw::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiffDividerDraw::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiffDividerDraw::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDiffDividerDraw::Draw(Ptr<CCanvas> pCanvas)
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
// Replaces the shapes.  True if they changed (the canvas should be drawn
// again); the same shapes again change nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CDiffDividerDraw::SetShapes(CArrayDiffDividerShape* pasShapes)
{
	if ((pasShapes->NumElements() == masShapes.NumElements()) && ((masShapes.NumElements() == 0) || (memcmp(pasShapes->GetData(), masShapes.GetData(), masShapes.NumElements() * sizeof(SDiffDividerShape)) == 0)))
	{
		return false;
	}
	masShapes.ReInit();
	masShapes.Copy(pasShapes);
	msCelSize.Init(-1, -1);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The background, then each shape's outline (the shape grown up and down
// by the outline's width) with the shape filled over it.
//
//////////////////////////////////////////////////////////////////////////
void CDiffDividerDraw::MakeCel(Ptr<CCanvas> pCanvas, SInt32Vec2 sSize)
{
	Ptr<CImage>			pImage;
	CImageDraw			cDraw;
	CVectorPath			cPath;
	SDiffDividerShape*	psShape;
	size				i;

	pImage = OMalloc<CImage>(sSize.x, sSize.y, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	pImage->Black();
	cDraw.Init(&pImage);

	cDraw.SetColour(msBackground);
	cDraw.DrawRectangle(0, 0, sSize.x, sSize.y);

	for (i = 0; i < masShapes.NumElements(); i++)
	{
		psShape = masShapes.Get(i);

		cDraw.SetColour(psShape->sOutline);
		cPath.Init();
		MakeShapePath(&cPath, psShape, sSize.x, DIFF_DIVIDER_OUTLINE);
		cDraw.DrawPath(&cPath);
		cPath.Kill();

		cDraw.SetColour(psShape->sFill);
		cPath.Init();
		MakeShapePath(&cPath, psShape, sSize.x, 0);
		cDraw.DrawPath(&cPath);
		cPath.Kill();
	}
	cDraw.Kill();

	pImage = CImageColourConverter::ConvertColour(pImage, pCanvas->GetColourOrder());
	mpCelDraw->SetCel(OMalloc<CImageCel>(pImage));
	msCelSize = sSize;
}


//////////////////////////////////////////////////////////////////////////
//
// The shape's outline grown fGrow pixels up and down: along the top from
// the left to the right, down the right edge, back along the bottom and
// up the left edge.  The curves leave and arrive flat, their controls
// half way across.
//
//////////////////////////////////////////////////////////////////////////
void CDiffDividerDraw::MakeShapePath(CVectorPath* pcPath, SDiffDividerShape* psShape, float64 fWidth, float64 fGrow)
{
	SFloat64Vec2	sPoint;
	SFloat64Vec2	sControl1;
	SFloat64Vec2	sControl2;
	float64			fLeftTop;
	float64			fLeftBottom;
	float64			fRightTop;
	float64			fRightBottom;

	fLeftTop = psShape->fLeftTop - fGrow;
	fLeftBottom = psShape->fLeftBottom + fGrow;
	fRightTop = psShape->fRightTop - fGrow;
	fRightBottom = psShape->fRightBottom + fGrow;

	sPoint.Init(0, fLeftTop);
	pcPath->MoveTo(&sPoint);
	sControl1.Init(fWidth / 2, fLeftTop);
	sControl2.Init(fWidth / 2, fRightTop);
	sPoint.Init(fWidth, fRightTop);
	pcPath->CubicTo(&sControl1, &sControl2, &sPoint);
	sPoint.Init(fWidth, fRightBottom);
	pcPath->LineTo(&sPoint);
	sControl1.Init(fWidth / 2, fRightBottom);
	sControl2.Init(fWidth / 2, fLeftBottom);
	sPoint.Init(0, fLeftBottom);
	pcPath->CubicTo(&sControl1, &sControl2, &sPoint);
}


size CDiffDividerDraw::NumShapes(void) { return masShapes.NumElements(); }
SDiffDividerShape* CDiffDividerDraw::GetShape(size uiIndex) { return masShapes.SafeGet(uiIndex); }

