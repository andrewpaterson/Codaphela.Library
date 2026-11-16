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
#ifndef __SOLID_OPERATOR_H__
#define __SOLID_OPERATOR_H__
#include "StandardLib/Pointer.h"
#include "SupportLib/SolidTexture.h"


//A solid texture made from other solid textures (TexturePlan.txt, Operators): its inputs are pointers it holds, and a texture
//is a graph of them, saved as it is.  A texture can be the input of several operators; it can't be its own, however far up.
//
//An input is sampled at the operator's own point (Input), so its placement is within the operator's: a graph is put in the
//world once, at its top, and each texture in it can still be moved, turned and scaled against the rest.
class CSolidOperator : public CSolidTexture
{
CONSTRUCTABLE(CSolidOperator);
DESTRUCTABLE(CSolidOperator);
protected:
	void	Input(Ptr<CSolidTexture>& pInput, SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample);  //Opaque black from an input that isn't there.
};


#endif // __SOLID_OPERATOR_H__

