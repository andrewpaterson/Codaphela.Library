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
#ifndef __ROUNDED_CORNERS_H__
#define __ROUNDED_CORNERS_H__
#include "Canvas.h"
#include "Container.h"
#include "RoundedCornerCanvasDraw.h"


//Rounds the corners of the components under it, as IntelliJ IDEA's Islands theme rounds its tool windows and editor.
//It is a canvas in each corner of the area it is laid out in, each filling what lies outside a quarter circle with
//the colour around the rounded components (see CRoundedCornerCanvasDraw).  Its container adds it last, so it is drawn
//over the others, and lays it out over the area they fill.  It is never found under the pointer: the components it
//covers take the clicks.  The radius is made smaller for an area less than twice it across.
class CRoundedCorners : public CContainer
{
CONSTRUCTABLE(CRoundedCorners);
DESTRUCTABLE(CRoundedCorners);
protected:
	CArray<CCanvas>		maCorners;  //By ERoundedCorner.
	int32				miRadius;

public:
	void				Init(Ptr<CWindow> pWindow, ARGB32 sColour, int32 iRadius);
	void				Class(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetRequiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	Ptr<CComponent>		FindComponentAt(int x, int y) override;

	int32				GetRadius(void) const;
	Ptr<CCanvas>		GetCorner(ERoundedCorner eCorner);
};


#endif // __ROUNDED_CORNERS_H__

