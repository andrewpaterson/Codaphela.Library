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
#include "Instance.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CInstance::Init(void)
{
	CStandardTrackerObject::Init();
	maiConnections.Init(16);
	meType = TT_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CInstance::Kill(void)
{	
	maiConnections.Kill();
	CStandardTrackerObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CInstance::LoadSpecific(CFileReader* pcFile, int iChunkNum)
{
	ReturnOnFalse(BeginLoadStandardTrackerObject(pcFile, iChunkNum));

	ReturnOnFalse(pcFile->ReadInt((int*)&meType));
	ReturnOnFalse(pcFile->ReadInt((int*)&miObjectIndex));
	ReturnOnFalse(pcFile->ReadArraySimple(&maiConnections));

	return EndLoadStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CInstance::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(BeginSaveStandardTrackerObject(pcFile));

	ReturnOnFalse(pcFile->WriteInt(meType));
	ReturnOnFalse(pcFile->WriteInt(miObjectIndex));
	ReturnOnFalse(pcFile->WriteArraySimple(&maiConnections));

	return EndSaveStandardTrackerObject(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInstance::Copy(CInstance* pCInstance)
{
	Init();
}

