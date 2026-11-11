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
#include "SceneParticles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneParticles::Init(void)
{
	PreInit();

	mszName.Init();
	mafPositions.Init();
	mafColours.Init();
	mafRadii.Init();
	mafIntensities.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneParticles::Free(void)
{
	mafIntensities.Kill();
	mafRadii.Kill();
	mafColours.Kill();
	mafPositions.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneParticles::Class(void)
{
	U_String(mszName);
	U_Unknown(CArrayParticleFloat, mafPositions);
	U_Unknown(CArrayParticleFloat, mafColours);
	U_Unknown(CArrayParticleFloat, mafRadii);
	U_Unknown(CArrayParticleFloat, mafIntensities);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CSceneParticles::NumParticles(void)
{
	return mafPositions.NumElements() / 3;
}

