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
#ifndef __SOLID_COMBINE_H__
#define __SOLID_COMBINE_H__
#include "SolidOperator.h"


//A sample put together from several textures (TexturePlan.txt, step 39): its colour and alpha are its colour input's,
//and its height, shine, gloss and glow are each the brightness of an input of its own, so each can be a noise, a ramp
//or a number.  An input that isn't there gives black's: opaque black, and no height, shine, gloss or glow.  It's
//Blender's Combine, for a sample's channels.
class CSolidCombine : public CSolidOperator
{
CONSTRUCTABLE(CSolidCombine);
DESTRUCTABLE(CSolidCombine);
public:
	Ptr<CSolidTexture>	mpColour;
	Ptr<CSolidTexture>	mpHeight;
	Ptr<CSolidTexture>	mpShine;
	Ptr<CSolidTexture>	mpGloss;
	Ptr<CSolidTexture>	mpGlow;

	void	Init(Ptr<CSolidTexture> pColour, Ptr<CSolidTexture> pHeight, Ptr<CSolidTexture> pShine, Ptr<CSolidTexture> pGloss, Ptr<CSolidTexture> pGlow);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
	float32	InputValue(Ptr<CSolidTexture>& pInput, SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth);
};


#endif // __SOLID_COMBINE_H__

