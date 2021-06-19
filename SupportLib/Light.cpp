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
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ObjectReader.h"
#include "Light.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLight::Init(void)
{
	meType = LT_Omni;
	msColour.Init(0, 0, 0, 0);
	meDecay = LDT_None;
	mfDecayStart = 0.0f;
	mbCastShadows = FALSE;
	mfIntensity = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLight::Init(ELightType eType, CImageColourARGB sColour, ELightDecayType eDecay, float fDecayStart, BOOL bCastShadows, float fIntensity)
{
	msColour = sColour;
	meDecay = eDecay;
	mfDecayStart = fDecayStart;
	mbCastShadows = bCastShadows;
	mfIntensity = fIntensity;
	meType = eType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLight::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadInt((int*)&meType));
	ReturnOnFalse(msColour.Load(pcFile));
	ReturnOnFalse(pcFile->ReadInt((int*)&meDecay));
	ReturnOnFalse(pcFile->ReadFloat(&mfDecayStart));
	ReturnOnFalse(pcFile->ReadBool(&mbCastShadows));
	ReturnOnFalse(pcFile->ReadFloat(&mfIntensity));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLight::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt(meType));
	ReturnOnFalse(msColour.Save(pcFile));
	ReturnOnFalse(pcFile->WriteInt(meDecay));
	ReturnOnFalse(pcFile->WriteFloat(mfDecayStart));
	ReturnOnFalse(pcFile->WriteBool(mbCastShadows));
	ReturnOnFalse(pcFile->WriteFloat(mfIntensity));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLight::Copy(CLight* pcLight)
{
	//Implement me please.
}

