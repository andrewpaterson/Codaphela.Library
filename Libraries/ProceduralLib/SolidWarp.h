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
#ifndef __SOLID_WARP_H__
#define __SOLID_WARP_H__
#include "SolidOperator.h"


//Its input sampled somewhere else: at the point moved, along each axis, by mfAmount (in the warp's own units) times how far a
//second texture's value is from a half, doubled, so a value of 0 moves it mfAmount back, a half not at all and 1 mfAmount on
//(TexturePlan.txt, Operators).  The three axes' values are the second texture's at the point and at two points far from it, so
//one noise bends a pattern every way: bands into marble's veins, rings into wood's.  With no second texture nothing moves.
class CSolidWarp : public CSolidOperator
{
CONSTRUCTABLE(CSolidWarp);
DESTRUCTABLE(CSolidWarp);
public:
	Ptr<CSolidTexture>	mpInput;
	Ptr<CSolidTexture>	mpWarp;
	float32				mfAmount;

	void	Init(Ptr<CSolidTexture> pInput, Ptr<CSolidTexture> pWarp, float32 fAmount);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_WARP_H__

