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
#ifndef __SOLID_MERGE_H__
#define __SOLID_MERGE_H__
#include "SolidOperator.h"


//What a merge makes of each channel of its two inputs.
enum ESolidMerge
{
	SM_Add,			//Their sum.
	SM_Multiply,	//Their product: the second darkens the first.
	SM_Lesser,		//The smaller.
	SM_Greater,		//The larger.
	SM_Difference,	//How far apart they are.
};


//Two inputs put together channel by channel, every channel alike: colour, alpha, height, shine, gloss and glow
//(TexturePlan.txt, Operators).
class CSolidMerge : public CSolidOperator
{
CONSTRUCTABLE(CSolidMerge);
DESTRUCTABLE(CSolidMerge);
public:
	Ptr<CSolidTexture>	mpFirst;
	Ptr<CSolidTexture>	mpSecond;
	ESolidMerge			meMerge;

	void	Init(ESolidMerge eMerge, Ptr<CSolidTexture> pFirst, Ptr<CSolidTexture> pSecond);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_MERGE_H__

