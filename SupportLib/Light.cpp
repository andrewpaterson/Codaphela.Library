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
#include "Light.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLight::Init(void)
{
	CStandardTrackerObject::Init();
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
	CStandardTrackerObject::Init();
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
void CLight::Kill(void)
{
	CStandardTrackerObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLight::LoadSpecific(CFileReader* pcFile, int iChunkNum)
{
	ReturnOnFalse(BeginLoadStandardTrackerObject(pcFile, iChunkNum));

	ReturnOnFalse(pcFile->ReadInt((int*)&meType));
	ReturnOnFalse(msColour.Load(pcFile));
	ReturnOnFalse(pcFile->ReadInt((int*)&meDecay));
	ReturnOnFalse(pcFile->ReadFloat(&mfDecayStart));
	ReturnOnFalse(pcFile->ReadBool(&mbCastShadows));
	ReturnOnFalse(pcFile->ReadFloat(&mfIntensity));

	return EndLoadStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLight::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(BeginSaveStandardTrackerObject(pcFile));

	pcFile->WriteInt(meType);
	msColour.Save(pcFile);
	pcFile->WriteInt(meDecay);
	pcFile->WriteFloat(mfDecayStart);
	pcFile->WriteBool(mbCastShadows);
	pcFile->WriteFloat(mfIntensity);

	return EndSaveStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLight::Copy(CLight* pcLight)
{
	//Implement me please.
}

