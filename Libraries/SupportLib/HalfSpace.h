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
#ifndef __HALF_SPACE_H__
#define __HALF_SPACE_H__
#include "BaseLib/Float32Plane.h"
#include "Shape.h"


//Everything behind a plane.  Its distance is exact, its gradient the plane's normal, and its bounds are as large as a float32
//goes.
class CHalfSpace : public CShape
{
CONSTRUCTABLE(CHalfSpace);
DESTRUCTABLE(CHalfSpace);
public:
	SFloat32Plane	msPlane;

	void	Init(SFloat32Plane* psPlane);
	void	Class(void) override;
	void	Free(void) override;

	float32	Distance(SFloat32Vec3* psPosition) override;
	void	Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition) override;
	void	GetBounds(SFloat32Bounds* psDest) override;
};


#endif // __HALF_SPACE_H__

