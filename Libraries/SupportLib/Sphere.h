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
#ifndef __SPHERE_H__
#define __SPHERE_H__
#include "BaseLib/Float32Sphere.h"
#include "Shape.h"


//A ball.  Its distance and gradient are exact; at its centre, where every way is out, its gradient is nothing.
class CSphere : public CShape
{
CONSTRUCTABLE(CSphere);
DESTRUCTABLE(CSphere);
public:
	SFloat32Sphere	msSphere;

	void	Init(SFloat32Vec3* psCenter, float32 fRadius);
	void	Init(SFloat32Sphere* psSphere);
	void	Class(void) override;
	void	Free(void) override;

	float32	Distance(SFloat32Vec3* psPosition) override;
	void	Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition) override;
	void	GetBounds(SFloat32Bounds* psDest) override;
	bool	AddMesh(CMeshEditor* pcMeshEditor, int iSegments) override;
};


#endif // __SPHERE_H__

