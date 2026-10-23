/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/StringHelper.h"
#include "PortHolder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortHolder::Init(void)
{
	mapcPorts.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// Kills and frees the ports.
//
//////////////////////////////////////////////////////////////////////////
void CPortHolder::Kill(void)
{
	size	i;
	CPort*	pcPort;

	for (i = 0; i < mapcPorts.NumElements(); i++)
	{
		pcPort = mapcPorts.GetPtr(i);
		SafeKill(pcPort);
	}
	mapcPorts.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Called by CPort::Init.
//
//////////////////////////////////////////////////////////////////////////
void CPortHolder::AddPort(CPort* pcPort)
{
	mapcPorts.Add(pcPort);
}


//////////////////////////////////////////////////////////////////////////
//
// Logs an error and returns NULL if there is no port named szName.
//
//////////////////////////////////////////////////////////////////////////
CPort* CPortHolder::GetPort(const char* szName)
{
	size	i;
	CPort*	pcPort;
	CChars	sz;

	for (i = 0; i < mapcPorts.NumElements(); i++)
	{
		pcPort = mapcPorts.GetPtr(i);
		if (StringCompare(pcPort->GetName(), szName) == 0)
		{
			return pcPort;
		}
	}

	sz.Init(" Cannot get port named [");
	sz.Append(szName);
	sz.Append("] on ");
	AppendDescription(&sz);
	sz.Append(".");
	gcLogger.Error2(__METHOD__, sz.Text(), NULL);
	sz.Kill();
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPortHolder::NumPorts(void) { return mapcPorts.NumElements(); }
CPort* CPortHolder::GetPort(size uiIndex) { return mapcPorts.SafeGetPtr(uiIndex); }

