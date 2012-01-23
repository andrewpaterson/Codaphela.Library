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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCamera::Init(void)
{
	CStandardTrackerObject::Init();
	meCameraType = CT_Perspective;
	mfFOV = 0.0f;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCamera::Init(ECameraType eCameraType, float fFOV)
{
	CStandardTrackerObject::Init();
	mfFOV = fFOV;
	meCameraType = eCameraType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCamera::Kill(void)
{
	CStandardTrackerObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCamera::LoadSpecific(CFileReader* pcFile, int iChunkNum)
{
	ReturnOnFalse(BeginLoadStandardTrackerObject(pcFile, iChunkNum));

	pcFile->ReadInt((int*)&meCameraType);
	pcFile->ReadFloat(&mfFOV);

	return EndLoadStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCamera::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(BeginSaveStandardTrackerObject(pcFile));

	pcFile->WriteInt(meCameraType);
	pcFile->WriteFloat(mfFOV);

	return EndSaveStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCamera::Copy(CCamera* pcCamera)
{
	//Implement me please.
}
