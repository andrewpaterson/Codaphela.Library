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
#ifndef __SHAPE_SUBTRACTION_H__
#define __SHAPE_SUBTRACTION_H__
#include "Shape.h"


//A shape with another cut out of it: max(shape, -cut), a bound.  Its gradient is whichever side's distance that is, the cut's
//turned round.  Its bounds are the shape's.
class CShapeSubtraction : public CShape
{
CONSTRUCTABLE(CShapeSubtraction);
DESTRUCTABLE(CShapeSubtraction);
public:
	Ptr<CShape>	mpShape;
	Ptr<CShape>	mpCut;

	void	Init(Ptr<CShape> pShape, Ptr<CShape> pCut);
	void	Class(void) override;
	void	Free(void) override;

	float32	Distance(SFloat32Vec3* psPosition) override;
	void	Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition) override;
	void	GetBounds(SFloat32Bounds* psDest) override;
	bool	IsExact(void) override;
};


#endif // __SHAPE_SUBTRACTION_H__

