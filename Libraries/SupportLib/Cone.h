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
#ifndef __CONE_H__
#define __CONE_H__
#include "RevolvedShape.h"


//A capped cone from psStart fLength along psAxis, as CMeshConeEditor's: fBottomRadius at psStart and fTopRadius at the other
//end.  Its profile is a trapezoid; its distance is exact (Quilez's capped cone) and so is its gradient.
class CCone : public CRevolvedShape
{
CONSTRUCTABLE(CCone);
DESTRUCTABLE(CCone);
public:
	float32			mfTopRadius;
	float32			mfBottomRadius;

	void	Init(SFloat32Vec3* psAxis, SFloat32Vec3* psStart, float32 fTopRadius, float32 fBottomRadius, float32 fLength);
	void	Class(void) override;
	void	Free(void) override;

	float32	ProfileDistance(float32 fRadial, float32 fAlong, SFloat32Vec2* psGradient) override;
	void	GetBounds(SFloat32Bounds* psDest) override;
	bool	AddMesh(CMeshEditor* pcMeshEditor, int iSegments) override;
};


#endif // __CONE_H__

