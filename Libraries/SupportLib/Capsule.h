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
#ifndef __CAPSULE_H__
#define __CAPSULE_H__
#include "RevolvedShape.h"


//A cylinder with round ends: everything within fRadius of the line from psStart fLength along psAxis.  Its distance and
//gradient are exact.
class CCapsule : public CRevolvedShape
{
CONSTRUCTABLE(CCapsule);
DESTRUCTABLE(CCapsule);
public:
	float32			mfRadius;

	void	Init(SFloat32Vec3* psAxis, SFloat32Vec3* psStart, float32 fRadius, float32 fLength);
	void	Class(void) override;
	void	Free(void) override;

	float32	ProfileDistance(float32 fRadial, float32 fAlong, SFloat32Vec2* psGradient) override;
	void	GetBounds(SFloat32Bounds* psDest) override;
};


#endif // __CAPSULE_H__

