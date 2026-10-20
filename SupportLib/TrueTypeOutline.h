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
#ifndef __TRUE_TYPE_OUTLINE_H__
#define __TRUE_TYPE_OUTLINE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayInt.h"


//A point on a glyph outline in font units, y up.  Points that aren't on the curve are quadratic control points.
struct STrueTypePoint
{
	float32	x;
	float32	y;
	bool	bOnCurve;
};


typedef CArrayTemplate<STrueTypePoint>	CArrayTrueTypePoint;


//The contours of a glyph.  Each contour ends at the point index in maiContourEnds (inclusive) and is closed.
class CTrueTypeOutline
{
public:
	CArrayTrueTypePoint		macPoints;
	CArrayInt				maiContourEnds;

	void			Init(void);
	void			Kill(void);
	void			Clear(void);

	size			NumContours(void);
	size			NumPoints(void);
	STrueTypePoint*	GetPoint(size iIndex);
	size			GetContourStart(size iContour);
	size			GetContourEnd(size iContour);
};


#endif // __TRUE_TYPE_OUTLINE_H__
