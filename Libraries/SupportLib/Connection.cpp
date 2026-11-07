/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "Connection.h"
#include "BaseLib/ErrorHandler.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CConnection> CConnection::Init(void)
{
	PreInit();

	msWorldMatrix.Init();
	msLocalMatrix.Init();
	mbSelected = false;
	mbTopLevel = false;
	mbParity = false;

	PostInit();

	return this;
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
	U_4x4Float32(msWorldMatrix);
	U_4x4Float32(msLocalMatrix);
	U_Bool(mbSelected);
	U_Bool(mbTopLevel);
	U_Bool(mbParity);
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

