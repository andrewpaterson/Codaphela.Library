/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardHeader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStandardTrackerObject::Init(void)
{
	Init(0, "Unnamed");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CStandardTrackerObject::Init(int iUniqueID, char* szName)
{
	miUniqueID = iUniqueID;
	mszName.Init(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CStandardTrackerObject::Load(CFileReader* pcFile)
{
	int iChunkNum;

	iChunkNum = pcFile->FindFirstChunkWithName(ClassName());
	return LoadSpecific(pcFile, iChunkNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CStandardTrackerObject::BeginLoadStandardTrackerObject(CFileReader* pcFile, int iChunkNum)
{
	ReturnOnFalse(pcFile->ReadChunkBegin(iChunkNum));

	if (pcFile->ReadInt(&miUniqueID) != TRITRUE)
	{
		gcUserError.Set("Could not read ID.");
		return FALSE;
	}

	if (pcFile->ReadString(&mszName) != TRITRUE)
	{
		gcUserError.Set("Could not read Name.");
		return FALSE;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CStandardTrackerObject::BeginSaveStandardTrackerObject(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteChunkBegin());
	if (pcFile->WriteInt(miUniqueID) != TRITRUE)
	{
		gcUserError.Set("Could not write ID.");
		return FALSE;
	}
	if (pcFile->WriteString(&mszName) != TRITRUE)
	{
		gcUserError.Set("Could not write Name.");
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CStandardTrackerObject::EndLoadStandardTrackerObject(CFileReader* pcFile)
{
	return pcFile->ReadChunkEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CStandardTrackerObject::EndSaveStandardTrackerObject(CFileWriter* pcFile)
{
	return pcFile->WriteChunkEnd(ClassName());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CStandardTrackerObject::GetID(void) { return miUniqueID; }
char* CStandardTrackerObject::GetName(void) { return mszName.Text(); }
void CStandardTrackerObject::SetName(char* szName) { mszName.Set(szName); }
void CStandardTrackerObject::SetID(int iID) { miUniqueID = iID; }
BOOL CStandardTrackerObject::HasName(void) { return !mszName.Empty(); }
