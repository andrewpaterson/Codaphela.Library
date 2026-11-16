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
#ifndef __SOLID_RAMP_H__
#define __SOLID_RAMP_H__
#include "BaseLib/ArrayTemplate.h"
#include "SolidOperator.h"


//What a ramp gives where its input's value is fValue.
struct SSolidRampKey
{
	float32			fValue;
	SSolidSample	sSample;
};


typedef CArrayTemplate<SSolidRampKey>	CArraySolidRampKey;


//Its input's value (its brightness) through a table of keys: the sample of the key at that value, between two keys a blend
//of theirs, and before the first or after the last, that key's (TexturePlan.txt, Operators).  It's what gives a noise its
//colours, heights and shine: most materials are a noise through a ramp.  Two keys at one value are a hard edge.  With no keys
//it's opaque black.
class CSolidRamp : public CSolidOperator
{
CONSTRUCTABLE(CSolidRamp);
DESTRUCTABLE(CSolidRamp);
public:
	Ptr<CSolidTexture>	mpInput;
	CArraySolidRampKey	masKeys;  //In order of their values, those at one value in the order they were added.

	void	Init(Ptr<CSolidTexture> pInput);
	void	Free(void) override;
	void	Class(void) override;

	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;

	void	AddKey(float32 fValue, ARGB32 uiColour);
	void	AddKey(float32 fValue, SSolidSample* psSample);

	void	At(float32 fValue, SSolidSample* psSample);  //The ramp at a value.

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_RAMP_H__

