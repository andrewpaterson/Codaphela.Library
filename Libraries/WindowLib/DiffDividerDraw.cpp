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
#include "SupportLib/VectorShapes.h"
#include "Canvas.h"
#include "DiffDividerDraw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiffDividerDraw::Init(ARGB32 sBackground)
{
	PreInit();

	CCelCanvasDraw::Init();
	msBackground = sBackground;
	masShapes.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiffDividerDraw::Class(void)
{
	CCelCanvasDraw::Class();
	U_UInt32(msBackground);
	U_Unknown(CArrayDiffDividerShape, masShapes);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDiffDividerDraw::Free(void)
{
	masShapes.Kill();
	CCelCanvasDraw::Free();
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
	Repaint();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The background, then each shape's outline (the shape grown up and down
// by the outline's width) with the shape filled over it.
//
//////////////////////////////////////////////////////////////////////////
void CDiffDividerDraw::Paint(CImageDraw* pcDraw, SInt32Vec2 sSize)
{
	CVectorPath			cPath;
	SDiffDividerShape*	psShape;
	size				i;

	pcDraw->SetColour(msBackground);
	pcDraw->DrawRectangle(0, 0, sSize.x, sSize.y);

	for (i = 0; i < masShapes.NumElements(); i++)
	{
		psShape = masShapes.Get(i);

		pcDraw->SetColour(psShape->sOutline);
		cPath.Init();
		MakeShapePath(&cPath, psShape, sSize.x, DIFF_DIVIDER_OUTLINE);
		pcDraw->DrawPath(&cPath);
		cPath.Kill();

		pcDraw->SetColour(psShape->sFill);
		cPath.Init();
		MakeShapePath(&cPath, psShape, sSize.x, 0);
		pcDraw->DrawPath(&cPath);
		cPath.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The shape's outline grown fGrow pixels up and down: along the top from
// the left to the right, down the right edge, back along the bottom and
// up the left edge.  The curves are level cubics (VectorLevelCubic),
// their controls half way across.
//
//////////////////////////////////////////////////////////////////////////
void CDiffDividerDraw::MakeShapePath(CVectorPath* pcPath, SDiffDividerShape* psShape, float64 fWidth, float64 fGrow)
{
	SFloat64Vec2	sStart;
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

	sStart.Init(0, fLeftTop);
	pcPath->MoveTo(&sStart);
	sPoint.Init(fWidth, fRightTop);
	VectorLevelCubic(&sStart, &sPoint, fWidth / 2, &sControl1, &sControl2);
	pcPath->CubicTo(&sControl1, &sControl2, &sPoint);
	sStart.Init(fWidth, fRightBottom);
	pcPath->LineTo(&sStart);
	sPoint.Init(0, fLeftBottom);
	VectorLevelCubic(&sStart, &sPoint, -fWidth / 2, &sControl1, &sControl2);
	pcPath->CubicTo(&sControl1, &sControl2, &sPoint);
}


size CDiffDividerDraw::NumShapes(void) const { return masShapes.NumElements(); }
SDiffDividerShape* CDiffDividerDraw::GetShape(size uiIndex) { return masShapes.SafeGet(uiIndex); }

