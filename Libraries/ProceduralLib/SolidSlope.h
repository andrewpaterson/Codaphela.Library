/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ProceduralLib

Codaphela ProceduralLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ProceduralLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ProceduralLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SOLID_SLOPE_H__
#define __SOLID_SLOPE_H__
#include "SupportLib/SolidTexture.h"


//How nearly the surface faces a direction, as a value (TexturePlan.txt, Operators): 1 where its normal is along msDirection,
//a half where it's square to it, 0 where it's against it.  Up (the texture's z, as it starts) through a ramp is moss on what
//faces the sky and none beneath; as a mask it puts dust, snow or wear where a surface is turned to them.  It's the same at
//every point: only the normal matters.
class CSolidSlope : public CSolidTexture
{
CONSTRUCTABLE(CSolidSlope);
DESTRUCTABLE(CSolidSlope);
public:
	SFloat32Vec3	msDirection;  //A unit, in the texture's own space.

	void	Init(void);  //Up: z.
	void	Init(SFloat32Vec3* psDirection);  //Made a unit.
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_SLOPE_H__

