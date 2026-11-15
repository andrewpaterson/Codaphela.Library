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
#ifndef __TORUS_H__
#define __TORUS_H__
#include "RevolvedShape.h"


//A ring doughnut around psAxis through psCenter, as CMeshTorusEditor's: fRadius out to the middle of its tube, and fTubeRadius
//the tube's.  Its profile is a circle, so its distance and gradient are exact.  Its length is nothing: along is from its centre.
class CTorus : public CRevolvedShape
{
CONSTRUCTABLE(CTorus);
DESTRUCTABLE(CTorus);
public:
	float32			mfRadius;
	float32			mfTubeRadius;

	void	Init(SFloat32Vec3* psAxis, SFloat32Vec3* psCenter, float32 fRadius, float32 fTubeRadius);
	void	Class(void) override;
	void	Free(void) override;

	float32	ProfileDistance(float32 fRadial, float32 fAlong, SFloat32Vec2* psGradient) override;
	void	GetBounds(SFloat32Bounds* psDest) override;
	bool	AddMesh(CMeshEditor* pcMeshEditor, int iSegments) override;
};


#endif // __TORUS_H__

