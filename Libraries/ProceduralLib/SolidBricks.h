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
#ifndef __SOLID_BRICKS_H__
#define __SOLID_BRICKS_H__
#include "SupportLib/SolidTexture.h"


//Which of a brick's numbers a CSolidBricks gives.
enum ESolidBricks
{
	SB_Brick,	//The brick's own number, 0 to 1, the same all through it (and its share of the mortar): a colour for each.
	SB_Inside,	//How far the point is inside its brick from the brick's nearest face, in the texture's units; 0 in the mortar.
};


//Space filled with bricks (TexturePlan.txt, Operators): boxes msSize apart along x (a brick's length), y (its depth) and z
//(its height), mfMortar of mortar between them.  Each row is moved mfOffset of a length along x from the row below, and each
//layer in depth from the layer in front: a half is a wall's running bond, on its face and on its top; none is tiles, in a
//grid.  Through ramps and a mask its two numbers are bricks of their own colours standing proud of their mortar.
//
//It's right on a surface square to its axes, where a brick is cut along its faces.  A slanted or curved surface cuts through
//the boxes as it would through a real wall's.  It doesn't average itself over a texel: the bake's samples do.
class CSolidBricks : public CSolidTexture
{
CONSTRUCTABLE(CSolidBricks);
DESTRUCTABLE(CSolidBricks);
public:
	ESolidBricks	meKind;
	SFloat32Vec3	msSize;
	float32			mfMortar;
	float32			mfOffset;
	uint32			muiSeed;

	void	Init(ESolidBricks eKind, SFloat32Vec3* psSize, float32 fMortar, float32 fOffset, uint32 uiSeed);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_BRICKS_H__

