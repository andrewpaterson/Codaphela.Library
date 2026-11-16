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
#ifndef __SOLID_MASK_H__
#define __SOLID_MASK_H__
#include "SolidOperator.h"


//Two inputs blended, a third saying how much of each (TexturePlan.txt, Operators): the first where the mask's value (its
//brightness) is 0 or less, the second where it's 1 or more, and between, that much of the way from the first to the second,
//in every channel.  Where it's all one input the other isn't worked out.
class CSolidMask : public CSolidOperator
{
CONSTRUCTABLE(CSolidMask);
DESTRUCTABLE(CSolidMask);
public:
	Ptr<CSolidTexture>	mpFirst;
	Ptr<CSolidTexture>	mpSecond;
	Ptr<CSolidTexture>	mpMask;

	void	Init(Ptr<CSolidTexture> pFirst, Ptr<CSolidTexture> pSecond, Ptr<CSolidTexture> pMask);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_MASK_H__

