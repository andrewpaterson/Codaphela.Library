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
#ifndef __SOLID_NOISE_H__
#define __SOLID_NOISE_H__
#include "BaseLib/Noise.h"
#include "SupportLib/SolidTexture.h"


//A fractal sum of gradient noise as a value, 0 to 1 (TexturePlan.txt, Operators; BaseLib's Noise.h): a sum of octaves about a
//half (clouds, stone), turbulence (creased: marble, fire) or ridged.  Its first octave's lumps are about one of the texture's
//units across, so its scale is how big they are in the world.  Octaves finer than the texel are their mean.
class CSolidNoise : public CSolidTexture
{
CONSTRUCTABLE(CSolidNoise);
DESTRUCTABLE(CSolidNoise);
public:
	EFractalNoise	meKind;
	size			muiOctaves;
	uint32			muiSeed;

	void	Init(EFractalNoise eKind, size uiOctaves, uint32 uiSeed);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_NOISE_H__

