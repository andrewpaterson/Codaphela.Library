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
#ifndef __SHAPE_SMOOTH_UNION_H__
#define __SHAPE_SMOOTH_UNION_H__
#include "Shape.h"


//Two shapes joined with their seam rounded off over mfRadius: Quilez's polynomial smooth min, a bound.  Where their distances
//differ by mfRadius or more it's the nearer's; between, it blends them and comes in by up to a quarter of mfRadius, filling
//the seam.  Its gradient is the same blend of theirs, exactly, and its bounds are theirs grown by that quarter.
class CShapeSmoothUnion : public CShape
{
CONSTRUCTABLE(CShapeSmoothUnion);
DESTRUCTABLE(CShapeSmoothUnion);
public:
	Ptr<CShape>	mpShape1;
	Ptr<CShape>	mpShape2;
	float32		mfRadius;

	void	Init(Ptr<CShape> pShape1, Ptr<CShape> pShape2, float32 fRadius);
	void	Class(void) override;
	void	Free(void) override;

	float32	Distance(SFloat32Vec3* psPosition) override;
	void	Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition) override;
	void	GetBounds(SFloat32Bounds* psDest) override;
	bool	IsExact(void) override;

protected:
	float32	Blend(float32 fDistance1, float32 fDistance2);
};


#endif // __SHAPE_SMOOTH_UNION_H__

