/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "StandardLib/ClassDefines.h"
#include "Camera.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCamera> CCamera::Init(void)
{
	PreInit();

	meCameraType = CT_Perspective;
	mfFOV = 0.0f;

	PostInit();

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CCamera> CCamera::Init(ECameraType eCameraType, float fFOV)
{
	PreInit();

	mfFOV = fFOV;
	meCameraType = eCameraType;

	PostInit();

	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCamera::Free(void)
{
	meCameraType = CT_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCamera::Class(void)
{
	U_Float32(mfFOV);
	U_Enum(meCameraType);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCamera::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadSInt((int*)&meCameraType));
	ReturnOnFalse(pcFile->ReadFloat(&mfFOV));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCamera::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteSInt(meCameraType));
	ReturnOnFalse(pcFile->WriteFloat(mfFOV));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCamera::Copy(CCamera* pcCamera)
{
	//Implement me please.
}

