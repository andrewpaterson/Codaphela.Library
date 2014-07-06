/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ShapeLib

Codaphela ShapeLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ShapeLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ShapeLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __POLYGON_H__
#define __POLYGON_H__
#include "Plane.h"
#include "BaseLib/ArraySimpleInt.h"

class CTriangle;
class CPolygon : public CPlane
{
public:
	BOOL			mbConvex;
	CArrayBlock		mapsPositions;  //Array of SFloat3* (not an array of SFloat3).

	void Init(void);
	void Init(SFloat3* psNormal);
	void Kill(void);
	void Set(void);
	BOOL AddPosition(SFloat3* psPosition);
	BOOL AddPositions(SFloat3* psPos1, SFloat3* psPos2, SFloat3* psPos3);
	BOOL AddTriangle(CTriangle* pcTriangle);
	BOOL UnionPosition(SFloat3* psPosition);
	BOOL In(SFloat3* psPosition);
	void PrivateCalculateConvexity(void);
	void CalculateConvexity(void);
	BOOL Contains(SFloat3* psPosition);
	void FindIndicesOfVisibleHalfSpaces(SFloat3* psPosition, CArrayInt* pcVisibleIndices);
	BOOL HasPositionPtr(SFloat3* psPosition);
};


typedef CArrayTemplate<CPolygon>	CArrayPolygons;


#endif // __POLYGON_H__

