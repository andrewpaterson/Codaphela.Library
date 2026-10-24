/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __GEOMETRY_MAP_H__
#define __GEOMETRY_MAP_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Float64Vec2.h"
#include "ColourARGB32.h"
#include "ImageAccessor.h"
#include "VectorRasteriser.h"
#include "BaseMap.h"


enum EGeometryCommand
{
	GC_Rectangle,
	GC_Line,
	GC_Circle,
	GC_FilledCircle,
	GC_Polygon,
	GC_Polyline,
};


//How the ends of a line are drawn.  Square extends the line by half its thickness (Java's BasicStroke default), round
//adds a half circle and butt stops at the end points.
enum EGeometryLineCap
{
	GLC_Square,
	GLC_Round,
	GLC_Butt,
};


//A shape to draw.  The bounds are in map pixels, right and bottom exclusive, and hold every pixel the shape touches.
//A polygon is filled with the non-zero rule.  A polyline is stroked with mitred joins (bevelled past a mitre limit of
//ten, as Java's BasicStroke) and square caps when it isn't closed.
//Lines and circles are anti-aliased and placed by points where a whole number is the centre of a pixel; a square
//capped line from (1, 2) to (5, 2) one pixel thick covers pixels 1 to 5 of row 2 exactly, a butt capped one covers
//half of pixels 1 and 5.
struct SGeometryCommand
{
	EGeometryCommand	eType;
	CInt32Rect			cBounds;
	ARGB32				sColour;
	SFloat64Vec2		sStart;			//Line start or circle centre.
	SFloat64Vec2		sEnd;			//Line end.
	float64				fRadius;		//Circle.
	float64				fThickness;		//Line or circle outline, centred on the line or the radius.
	EGeometryLineCap	eCap;			//Line.
	size				uiFirstPoint;	//Polygon or polyline points in the map's point array.
	size				uiNumPoints;
	bool				bClosed;		//Polyline.
};


typedef CArrayTemplate<SGeometryCommand>	CArrayGeometryCommand;
typedef CArrayTemplate<SFloat64Vec2>		CArrayGeometryPoint;


//A map of shapes drawn in colours, such as the lines under and through text and the traces and junctions of a circuit.  The commands are plain data (not
//objects) so a map can hold many.  A shape is blended onto the viewport the way a translucent cel is: its colour is
//blended with what is there by its alpha, and where the viewport has opacity the shape is added to it.  On a
//transparent canvas, cleared to nothing, that leaves premultiplied pixels like a cel's.
class CGeometryMap : public CBaseMap
{
CONSTRUCTABLE(CGeometryMap);
DESTRUCTABLE(CGeometryMap);
protected:
	CArrayGeometryCommand	masCommands;
	CArrayGeometryPoint		masPoints;
	Ptr<CImage>				mpViewport;

public:
	void				Init(void);
	void 				Free(void) override;
	void				Class(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				Clear(void);
	void				AddRectangle(int32 iLeft, int32 iTop, int32 iWidth, int32 iHeight, ARGB32 sColour);
	void				AddLine(float64 x0, float64 y0, float64 x1, float64 y1, float64 fThickness, ARGB32 sColour, EGeometryLineCap eCap = GLC_Square);
	void				AddCircle(float64 x, float64 y, float64 fRadius, float64 fThickness, ARGB32 sColour);
	void				AddFilledCircle(float64 x, float64 y, float64 fRadius, ARGB32 sColour);
	void				AddPolygon(SFloat64Vec2* asPoints, size uiNumPoints, ARGB32 sColour);
	void				AddPolyline(SFloat64Vec2* asPoints, size uiNumPoints, bool bClosed, float64 fThickness, ARGB32 sColour);
	SFloat64Vec2*		GetPoint(size uiIndex);
	size				NumCommands(void);
	SGeometryCommand*	GetCommand(size uiIndex);

	bool				Blit(CInt32Rect* pcViewportRect, bool bEnsureBlitters) override;
	void				SetViewport(Ptr<CImage> pViewport) override;
	Ptr<CImage>			GetViewport(void);

protected:
	SGeometryCommand*	AddShape(EGeometryCommand eType, float64 fLeft, float64 fTop, float64 fRight, float64 fBottom, ARGB32 sColour);
	void				FillRectangle(CImageAccessor* pcAccessor, bool bOpacity, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, ARGB32 sColour);
	bool				FillShape(CImageAccessor* pcAccessor, CVectorRasteriser* pcRasteriser, bool bOpacity, SGeometryCommand* psCommand, CInt32Rect* pcViewportRect, int32 iClipWidth, int32 iClipHeight);
	void				AddShapePath(CVectorPath* pcPath, SGeometryCommand* psCommand);
	void				AddPolylinePath(CVectorPath* pcPath, SGeometryCommand* psCommand);
	SGeometryCommand*	AddPointsShape(EGeometryCommand eType, SFloat64Vec2* asPoints, size uiNumPoints, ARGB32 sColour);
	void				BlendPixel(CImageAccessor* pcAccessor, bool bOpacity, int32 x, int32 y, ARGB32 sColour, uint32 uiAlpha);
};


#endif // __GEOMETRY_MAP_H__

