/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SCENE_PARTICLES_H__
#define __SCENE_PARTICLES_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "StandardLib/Object.h"


typedef CArrayLargeTemplate<float32>	CArrayParticleFloat;  //Large: can pass 64KB with a 16 bit size (W65C816).


//Points, as a field of stars is.  mafPositions and mafColours have three floats for each of them.  Any but mafPositions can be empty.
class CSceneParticles : public CObject
{
CONSTRUCTABLE(CSceneParticles);
DESTRUCTABLE(CSceneParticles);
public:
	CChars				mszName;
	CArrayParticleFloat	mafPositions;
	CArrayParticleFloat	mafColours;
	CArrayParticleFloat	mafRadii;
	CArrayParticleFloat	mafIntensities;

	void 	Init(void);
	void 	Free(void) override;
	void	Class(void) override;

	lsize	NumParticles(void);
};


#endif // __SCENE_PARTICLES_H__

