/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __LIGHT_H__
#define __LIGHT_H__
#include "BaseLib/Chars.h"
#include "StandardLib/Object.h"
#include "Connection.h"
#include "ImageColour.h"


enum ELightDecayType
{
	LDT_None,
	LDT_Linear,
	LDT_Square,
};


//The same values as ESceneFileLightType.
enum ELightType
{
	LT_Omni,
	LT_Directional,
	LT_Spot,
	LT_Ambient,
};


//Shines along its connection's -z.  mpConnection is NULL if it isn't placed anywhere.  An ambient light lights everything the
//same, so it needs no connection.  GetStrength is how much it lights, its intensity times its colour's brightness: 1 for a
//white light of intensity 1.
class CLight : public CObject
{
CONSTRUCTABLE(CLight);
DESTRUCTABLE(CLight);
public:
	CChars				mszName;
	ELightType			meType;
	CImageColourARGB	msColour;
	ELightDecayType		meDecay;
	float32				mfDecayStart;
	float32				mfRange;
	bool				mbCastShadows;
	float32				mfIntensity;
	Ptr<CConnection>	mpConnection;

	void 	Init(void);
	void	Free(void) override;
	void	Class(void) override;

	float64	GetStrength(void);
};


#endif // __LIGHT_H__

