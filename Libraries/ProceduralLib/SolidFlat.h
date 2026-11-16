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
#ifndef __SOLID_FLAT_H__
#define __SOLID_FLAT_H__
#include "SupportLib/SolidTexture.h"


//One sample everywhere (TexturePlan.txt, Operators): a colour, or all a sample's channels, for the operators to work on.
class CSolidFlat : public CSolidTexture
{
CONSTRUCTABLE(CSolidFlat);
DESTRUCTABLE(CSolidFlat);
public:
	SSolidSample	msSample;

	void	Init(ARGB32 uiColour);
	void	Init(SSolidSample* psSample);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_FLAT_H__

