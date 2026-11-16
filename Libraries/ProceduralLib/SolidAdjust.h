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
#ifndef __SOLID_ADJUST_H__
#define __SOLID_ADJUST_H__
#include "SolidOperator.h"


//Its input's colour changed, in this order (TexturePlan.txt, Operators): inverted (1 less each of red, green and blue); its
//hue turned by mfHue of a whole turn round the colour wheel; its saturation times mfSaturation (0 is grey, 1 as it was); its
//contrast times mfContrast (each channel's distance from a half); mfBrightness added.  It starts changing nothing.  Alpha and
//the other channels are the input's.
class CSolidAdjust : public CSolidOperator
{
CONSTRUCTABLE(CSolidAdjust);
DESTRUCTABLE(CSolidAdjust);
public:
	Ptr<CSolidTexture>	mpInput;
	bool				mbInvert;
	float32				mfHue;
	float32				mfSaturation;
	float32				mfContrast;
	float32				mfBrightness;

	void	Init(Ptr<CSolidTexture> pInput);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_ADJUST_H__

