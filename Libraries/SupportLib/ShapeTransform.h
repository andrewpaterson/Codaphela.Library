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
#ifndef __SHAPE_TRANSFORM_H__
#define __SHAPE_TRANSFORM_H__
#include "BaseLib/Float4x4.h"
#include "Shape.h"


//A shape moved, turned and scaled by msMatrix (a point in the shape's frame times msMatrix is where it is; no shear).  A point
//is turned into the shape's frame by msInverse, and the distance there scaled back by mfScale, the smallest scale along the
//matrix's axes: as nothing is nearer than that scale allows, it's a bound, and exact for a scale the same every way.
class CShapeTransform : public CShape
{
CONSTRUCTABLE(CShapeTransform);
DESTRUCTABLE(CShapeTransform);
public:
	Ptr<CShape>	mpShape;
	SFloat4x4	msMatrix;
	SFloat4x4	msInverse;
	float32		mfScale;
	bool		mbEvenScale;

	bool	Init(Ptr<CShape> pShape, SFloat4x4* psMatrix);  //False if the matrix can't be turned back.
	void	Class(void) override;
	void	Free(void) override;

	float32	Distance(SFloat32Vec3* psPosition) override;
	void	Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition) override;
	void	GetBounds(SFloat32Bounds* psDest) override;
	bool	IsExact(void) override;
};


#endif // __SHAPE_TRANSFORM_H__

