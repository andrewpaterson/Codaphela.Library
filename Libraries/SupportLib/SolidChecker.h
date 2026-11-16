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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __SOLID_CHECKER_H__
#define __SOLID_CHECKER_H__
#include "SolidTexture.h"


//Cubes of mfSize world units in two colours, alternating every way: the test of an unwrap, as its squares stay square and the
//same size on a surface only where the unwrap doesn't stretch it.  It's box filtered over the texel's width (Quilez's filtered
//checkerboard, in three dimensions): each axis's square wave averaged over the width, the three multiplied, which is exact for a
//box, so where the width is a cube or more it fades to the colours' average instead of flickering.
class CSolidChecker : public CSolidTexture
{
CONSTRUCTABLE(CSolidChecker);
DESTRUCTABLE(CSolidChecker);
public:
	float32	mfSize;
	ARGB32	muiColour1;  //Of the cube at the origin's positive corner.
	ARGB32	muiColour2;

	void	Init(float32 fSize, ARGB32 uiColour1, ARGB32 uiColour2);
	void	Class(void) override;

	float32	Mix(SFloat32Vec3* psPosition, float32 fWidth);  //How much of muiColour2, 0 to 1, at a point in its own space.

protected:
	ARGB32	TextureColour(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth) override;
};


#endif // __SOLID_CHECKER_H__

