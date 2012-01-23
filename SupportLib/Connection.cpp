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
#include "Connection.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConnection::Init(void)
{
	CStandardTrackerObject::Init();
	msWorldMatrix.Init();
	msLocalMatrix.Init();
	mbSelected = FALSE;
	mbTopLevel = FALSE;
	mbParity = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConnection::Kill(void)
{	
	mbSelected = 0;
	CStandardTrackerObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConnection::Load(CFileReader* pcFile)
{
	//Fixes: ambiguous access of 'Load'
	return CStandardTrackerObject::Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CConnection::LoadSpecific(CFileReader* pcFile, int iChunkNum)
{
	ReturnOnFalse(BeginLoadStandardTrackerObject(pcFile, iChunkNum));

	//Read in the matrix.
	if (!msWorldMatrix.Load(pcFile))
	{
		gcUserError.Set("Could not read connection world matrix.");
		pcFile->ReadChunkEnd();
		return FALSE;
	}

	//Read in the matrix.
	if (!msLocalMatrix.Load(pcFile))
	{
		gcUserError.Set("Could not read connection local matrix.");
		pcFile->ReadChunkEnd();
		return FALSE;
	}	
	
	ReturnOnFalse(pcFile->ReadInt(&mbSelected));
	ReturnOnFalse(pcFile->ReadInt(&mbTopLevel));
	ReturnOnFalse(pcFile->ReadInt(&mbParity));
	
	return EndLoadStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CConnection::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(BeginSaveStandardTrackerObject(pcFile));


	//Write out the actual matrix.
	if (!msWorldMatrix.Save(pcFile))
	{
		gcUserError.Set("Could not write connection world matrix.");
		return FALSE;
	}

	if (!msLocalMatrix.Save(pcFile))
	{
		gcUserError.Set("Could not write connection local matrix.");
		return FALSE;
	}

	ReturnOnFalse(pcFile->WriteInt(mbSelected));
	ReturnOnFalse(pcFile->WriteInt(mbTopLevel));
	ReturnOnFalse(pcFile->WriteInt(mbParity));

	return EndSaveStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnection::Copy(CConnection* pcConnection)
{
	Init();
	SetID(pcConnection->miUniqueID);
	memcpy(&msWorldMatrix, &pcConnection->msWorldMatrix, sizeof(SFloat4x4));
	memcpy(&msLocalMatrix, &pcConnection->msLocalMatrix, sizeof(SFloat4x4));
}
