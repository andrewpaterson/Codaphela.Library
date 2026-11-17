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
#ifndef __SOLID_CHANNEL_H__
#define __SOLID_CHANNEL_H__
#include "SolidOperator.h"


//The channels of a sample, in the order SSolidSample::Channels has them.
enum ESolidChannel
{
	SCH_Red,
	SCH_Green,
	SCH_Blue,
	SCH_Alpha,
	SCH_Height,
	SCH_Shine,
	SCH_Gloss,
	SCH_Glow,
};


//One channel of its input as a grey (TexturePlan.txt, step 39): an opaque colour whose red, green and blue are all the
//channel's number, flat, dull and dark.  It's how a channel is looked at alone, and how one is taken out of a sample to
//be put into another (CSolidCombine).  It's Blender's Separate, one channel at a time.
class CSolidChannel : public CSolidOperator
{
CONSTRUCTABLE(CSolidChannel);
DESTRUCTABLE(CSolidChannel);
public:
	Ptr<CSolidTexture>	mpInput;
	ESolidChannel		meChannel;

	void	Init(ESolidChannel eChannel, Ptr<CSolidTexture> pInput);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_CHANNEL_H__

