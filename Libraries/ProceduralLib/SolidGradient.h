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
#ifndef __SOLID_GRADIENT_H__
#define __SOLID_GRADIENT_H__
#include "SupportLib/SolidTexture.h"


//What a gradient's number is the distance from.
enum ESolidGradient
{
	SG_Plane,	//The plane x = 0: it rises along x.
	SG_Axis,	//The z axis: rings round it, as a tree's.
	SG_Point,	//The origin: shells round it.
};


//What becomes of the distance past 0 to 1.
enum ESolidRepeat
{
	SR_Once,	//It stays 0 before 0 and 1 after 1.
	SR_Saw,		//It starts again from 0 at each whole number.
	SR_Wave,	//It rises to 1 half way between whole numbers and falls back to 0: bands with no hard edge.
};


//A value that rises with distance from a plane, an axis or a point, in the texture's own units, once or again and again
//(TexturePlan.txt, Operators).  Placed, scaled and through a CSolidRamp it's stripes, rings and fades.  It's the average over
//the texel's width along the distance (exactly, as CSolidChecker's is), so a saw's edge is as soft as the texel is wide and
//bands narrower than a texel are their mean.
class CSolidGradient : public CSolidTexture
{
CONSTRUCTABLE(CSolidGradient);
DESTRUCTABLE(CSolidGradient);
public:
	ESolidGradient	meFrom;
	ESolidRepeat	meRepeat;

	void	Init(ESolidGradient eFrom, ESolidRepeat eRepeat);
	void	Class(void) override;

	float32	Value(SFloat32Vec3* psPosition, float32 fWidth);  //At a point in its own space.

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_GRADIENT_H__

