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
#include "Camera.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCamera::Init(void)
{
	meCameraType = CT_Perspective;
	mfFOV = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCamera::Init(ECameraType eCameraType, float fFOV)
{
	mfFOV = fFOV;
	meCameraType = eCameraType;
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
	CObject::ClassNotImplemented();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCamera::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadInt((int*)&meCameraType));
	ReturnOnFalse(pcFile->ReadFloat(&mfFOV));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCamera::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt(meCameraType));
	ReturnOnFalse(pcFile->WriteFloat(mfFOV));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCamera::Copy(CCamera* pcCamera)
{
	//Implement me please.
}
