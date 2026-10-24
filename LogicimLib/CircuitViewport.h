/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CIRCUIT_VIEWPORT_H__
#define __CIRCUIT_VIEWPORT_H__
#include "BaseLib/Float64Vec2.h"
#include "BaseLib/Int32Vec2.h"


#define CIRCUIT_GRID_SCALE		10.0
#define CIRCUIT_MINIMUM_ZOOM	0.2
#define CIRCUIT_MAXIMUM_ZOOM	6.0


//Where the circuit's grid is on the screen, as Logicim's Viewport.  A grid unit is ten pixels at zoom one and grid
//0, 0 is at the centre of the screen moved by the position.  Screen points are pixel centres (whole numbers are the
//middle of a pixel, as CGeometryMap places shapes) and aren't rounded to whole pixels as Logicim's are.
class CCircuitViewport
{
protected:
	SInt32Vec2		msSize;
	SFloat64Vec2	msPosition;
	float64			mfZoom;

public:
	void			Init(int32 iWidth, int32 iHeight);
	void			Kill(void);

	void			SetSize(int32 iWidth, int32 iHeight);
	SInt32Vec2		GetSize(void);
	void			SetPosition(float64 x, float64 y);
	SFloat64Vec2	GetPosition(void);
	void			Scroll(float64 fRight, float64 fDown);
	void			SetZoom(float64 fZoom);
	float64			GetZoom(void);
	void			ZoomAround(float64 fScreenX, float64 fScreenY, float64 fZoom);

	void			GridToScreen(SFloat64Vec2* psScreen, SFloat64Vec2* psGrid);
	void			ScreenToGrid(SFloat64Vec2* psGrid, SFloat64Vec2* psScreen);
	float64			GridToScreenX(float64 x);
	float64			GridToScreenY(float64 y);
	float64			GetPixelsPerGrid(void);

	float64			GetLineWidth(float64 fWidth);
	float64			GetDefaultLineWidth(void);
	float64			GetCircleRadius(void);
	float64			GetConnectionRadius(void);
};


#endif // __CIRCUIT_VIEWPORT_H__
