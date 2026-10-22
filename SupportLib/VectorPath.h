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
#ifndef __VECTOR_PATH_H__
#define __VECTOR_PATH_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/Float64Vec2.h"


enum EVectorSegmentType
{
	VS_Part,	//Starts a part; a set of contours drawn with an exposure.
	VS_Move,	//Starts a contour at the point.
	VS_Line,	//A straight edge to the point.
	VS_Arc,		//A circular edge to the point around the center.  The end equal to the start is a full circle.
};


struct SVectorSegment
{
	EVectorSegmentType	eType;
	bool				bExposure;		//Part: on draws, off erases the earlier parts of the path.
	bool				bClockwise;		//Arc.
	SFloat64Vec2		sPoint;
	SFloat64Vec2		sCenter;		//Arc.
};


typedef CArrayTemplate<SVectorSegment>	CArrayVectorSegment;


//A 2D shape made of parts.  Each part is a set of closed contours of straight and circular edges filled with the
//non-zero winding rule; holes wind the other way.  Parts with exposure off erase the earlier parts of the same path
//only.  Arcs are counterclockwise or clockwise with y up; drawn y down they look the other way round.
class CVectorPath
{
protected:
	CArrayVectorSegment		masSegments;
	SFloat64Vec2			msMinimum;
	SFloat64Vec2			msMaximum;
	bool					mbBounds;

public:
	void				Init(void);
	void				Init(CVectorPath* pcSource);
	void				Kill(void);

	void				BeginPart(bool bExposure);
	void				MoveTo(SFloat64Vec2* psPoint);
	void				LineTo(SFloat64Vec2* psPoint);
	void				ArcTo(SFloat64Vec2* psPoint, SFloat64Vec2* psCenter, bool bClockwise);

	void				AddCircle(SFloat64Vec2* psCenter, float64 fRadius, bool bClockwise);
	void				AddPolygon(SFloat64Vec2* asPoints, size uiNumPoints, bool bReverse);

	void				Transform(bool bMirrorX, bool bMirrorY, float64 fRotation, float64 fScale, SFloat64Vec2* psTranslation);
	void				CalculateBounds(void);

	size				NumSegments(void);
	SVectorSegment*		GetSegment(size uiIndex);
	bool				HasBounds(void);
	SFloat64Vec2*		GetMinimum(void);
	SFloat64Vec2*		GetMaximum(void);
	bool				HasExposureOff(void);
	float64				Area(void);
};


#endif // __VECTOR_PATH_H__

