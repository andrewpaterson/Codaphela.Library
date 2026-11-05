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
#ifndef __DIFF_DIVIDER_DRAW_H__
#define __DIFF_DIVIDER_DRAW_H__
#include "BaseLib/ArrayTemplate.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/VectorPath.h"
#include "ImageCanvasDraw.h"


//A changed block's shape on the divider: where its lines are on the left (at the divider's left edge) and on the right
//(at its right edge), in pixels down from the divider's top.  A side with no lines has its top equal to its bottom.
struct SDiffDividerShape
{
	float64	fLeftTop;
	float64	fLeftBottom;
	float64	fRightTop;
	float64	fRightBottom;
	ARGB32	sFill;
	ARGB32	sOutline;
};


typedef CArrayTemplate<SDiffDividerShape>	CArrayDiffDividerShape;


#define DIFF_DIVIDER_OUTLINE	1.0		//Pixels of outline above and below a shape.


//Draws the strip between a diff view's two sides, as IntelliJ IDEA does: on its background each changed block is a
//shape from its lines on the left to its lines on the right, the top and bottom edges curves (cubic Beziers flat at
//both ends), filled and outlined above and below.  A side with no lines pinches the shape to a point there.  Shapes are
//set whenever the sides scroll or are laid out (SetShapes); they are drawn into a cel the size of the canvas, made
//again only when the shapes or the size change.
class CDiffDividerDraw : public CCanvasDraw
{
CONSTRUCTABLE(CDiffDividerDraw);
DESTRUCTABLE(CDiffDividerDraw);
protected:
	Ptr<CImageCanvasDraw>	mpCelDraw;
	ARGB32					msBackground;
	CArrayDiffDividerShape	masShapes;
	SInt32Vec2				msCelSize;  //The canvas size the cel was made for, -1, -1 to make it again.

public:
	void				Init(ARGB32 sBackground);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	bool				Draw(Ptr<CCanvas> pCanvas) override;

	bool				SetShapes(CArrayDiffDividerShape* pasShapes);
	size				NumShapes(void);
	SDiffDividerShape*	GetShape(size uiIndex);

protected:
	void				MakeCel(Ptr<CCanvas> pCanvas, SInt32Vec2 sSize);
	void				MakeShapePath(CVectorPath* pcPath, SDiffDividerShape* psShape, float64 fWidth, float64 fGrow);
};


#endif // __DIFF_DIVIDER_DRAW_H__

