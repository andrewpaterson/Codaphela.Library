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
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConnection::Init(void)
{
	msWorldMatrix.Init();
	msLocalMatrix.Init();
	mbSelected = false;
	mbTopLevel = false;
	mbParity = false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConnection::Free(void)
{	
	mbSelected = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnection::Class(void)
{
	CObject::ClassNotImplemented();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CConnection::Load(CObjectReader* pcFile)
{
	//Read in the matrix.
	if (!msWorldMatrix.Load(pcFile))
	{
		gcUserError.Set("Could not read connection world matrix.");
		return false;
	}

	//Read in the matrix.
	if (!msLocalMatrix.Load(pcFile))
	{
		gcUserError.Set("Could not read connection local matrix.");
		return false;
	}	
	
	if (!pcFile->ReadBool(&mbSelected))
	{
		return false;
	}
	if (!pcFile->ReadBool(&mbTopLevel))
	{
		return false;
	}
	if (!pcFile->ReadBool(&mbParity))
	{
		return false;;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CConnection::Save(CObjectWriter* pcFile)
{
	//Write out the actual matrix.
	if (!msWorldMatrix.Save(pcFile))
	{
		gcUserError.Set("Could not write connection world matrix.");
		return false;
	}

	if (!msLocalMatrix.Save(pcFile))
	{
		gcUserError.Set("Could not write connection local matrix.");
		return false;
	}

	ReturnOnFalse(pcFile->WriteInt(mbSelected));
	ReturnOnFalse(pcFile->WriteInt(mbTopLevel));
	ReturnOnFalse(pcFile->WriteInt(mbParity));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConnection::Copy(CConnection* pcConnection)
{
	Init();
	memcpy(&msWorldMatrix, &pcConnection->msWorldMatrix, sizeof(SFloat4x4));
	memcpy(&msLocalMatrix, &pcConnection->msLocalMatrix, sizeof(SFloat4x4));
}
