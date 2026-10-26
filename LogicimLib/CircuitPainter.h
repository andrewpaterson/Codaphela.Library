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
#ifndef __CIRCUIT_PAINTER_H__
#define __CIRCUIT_PAINTER_H__
#include "BaseLib/Float64Vec2.h"
#include "BaseLib/Int32Vec2.h"
#include "SupportLib/GeometryMap.h"
#include "SupportLib/SpriteMap.h"
#include "Rotation.h"
#include "CircuitViewport.h"
#include "CircuitColours.h"
#include "CircuitFonts.h"


class CViewPath;
class CCircuitSimulation;


//Draws a circuit's shapes into a geometry map.  Shapes are given in the grid space of the component being drawn,
//facing East about its position, and are turned and moved by SetTransform before the viewport puts them on the
//screen.  Line widths are pixels at zoom one, as Logicim's zoomable strokes.  Text is drawn as glyphs into a sprite
//map, which is blitted after the geometry map; without fonts or a sprite map there is no text.
//
//InitImage paints into a whole image, as the circuit canvas does: the viewport is made the image's size and the maps
//are cleared onto the image with the background drawn.  Paint, then BlitImage puts the maps on the image.
class CCircuitPainter
{
protected:
	CCircuitViewport*	mpcViewport;
	CGeometryMap*		mpcMap;
	CSpriteMap*			mpcSprites;
	CCircuitColours*	mpcColours;
	CCircuitFonts*		mpcFonts;
	CViewPath*			mpcViewPath;
	CCircuitSimulation*	mpcCircuitSimulation;
	int64				miTime;
	SInt32Vec2			msPosition;
	ERotation			meRotation;

public:
	void				Init(CCircuitViewport* pcViewport, CGeometryMap* pcMap, CSpriteMap* pcSprites, CCircuitColours* pcColours, CCircuitFonts* pcFonts, CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation);
	void				InitImage(Ptr<CImage> pImage, CCircuitViewport* pcViewport, CGeometryMap* pcMap, CSpriteMap* pcSprites, CCircuitColours* pcColours, CCircuitFonts* pcFonts, CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation);
	void				Kill(void);

	bool				BlitImage(void);

	void				SetTransform(SInt32Vec2 sPosition, ERotation eRotation);
	void				ClearTransform(void);
	void				ToScreen(SFloat64Vec2* psScreen, SFloat64Vec2* psLocal);
	void				ToScreen(SFloat64Vec2* psScreen, float64 x, float64 y);

	void				Line(float64 x0, float64 y0, float64 x1, float64 y1, float64 fWidth, ARGB32 sColour);
	void				Polygon(SFloat64Vec2* asPoints, size uiNumPoints, bool bFill, bool bBorder, float64 fWidth, ARGB32 sFill, ARGB32 sBorder);
	void				Arc(float64 fCentreX, float64 fCentreY, float64 fRadiusX, float64 fRadiusY, float64 fStartAngle, float64 fArcAngle, bool bFill, bool bBorder, float64 fWidth, ARGB32 sFill, ARGB32 sBorder);
	void				Circle(float64 fCentreX, float64 fCentreY, float64 fRadius, bool bFill, bool bBorder, float64 fWidth, ARGB32 sFill, ARGB32 sBorder);
	void				Dot(float64 x, float64 y, float64 fPixelRadius, ARGB32 sColour);
	void				Text(float64 x, float64 y, const char* szText, ECircuitFont eFont, float64 fFontSize, ETextAlignment eAlignment, int iRelativeRightRotations, ARGB32 sColour);
	void				CentredText(float64 x, float64 y, const char* szText, ECircuitFont eFont, float64 fFontSize, ARGB32 sColour);

	CCircuitViewport*	GetViewport(void);
	CCircuitColours*	GetColours(void);
	CCircuitFonts*		GetFonts(void);
	CGeometryMap*		GetMap(void);
	int64				GetTime(void);
	CViewPath*			GetViewPath(void);
	CCircuitSimulation*	GetCircuitSimulation(void);

protected:
	size				NumArcSegments(float64 fRadiusX, float64 fRadiusY, float64 fArcAngle);
	float64				GetTextWidthAdjust(float64 fOffset, float64 fWidth, ETextAlignment eAlignment);
};


#endif // __CIRCUIT_PAINTER_H__
