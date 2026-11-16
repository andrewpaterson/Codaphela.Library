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
#ifndef __CONVEX_HULL_H__
#define __CONVEX_HULL_H__
#include "BaseLib/Float32Plane.h"
#include "Shape.h"


//The convex hull of points, made by CQuickHull: its faces' planes and its corners.  Its distance is the largest of its planes'
//(the furthest in front), exact inside and on it, and a bound outside, where past an edge or a corner it's further.  Its
//gradient is that plane's normal.  Points CQuickHull can't make a hull of (fewer than four, or all in a plane) make it nothing,
//as far as can be from everywhere.
class CConvexHull : public CShape
{
CONSTRUCTABLE(CConvexHull);
DESTRUCTABLE(CConvexHull);
public:
	CArrayFloat32Plane	mcPlanes;
	CArrayFloat32Vec3	mcCorners;

	bool	Init(SFloat32Vec3* psPoints, size uiStride, size uiNumPoints);
	void	Class(void) override;
	void	Free(void) override;
	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;

	size	NumPlanes(void) const;
	float32	Distance(SFloat32Vec3* psPosition) override;
	void	Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition) override;
	void	GetBounds(SFloat32Bounds* psDest) override;
	bool	IsExact(void) override;

protected:
	int		FindFurthestPlane(SFloat32Vec3* psPosition, float32* pfDistance);
};


#endif // __CONVEX_HULL_H__

