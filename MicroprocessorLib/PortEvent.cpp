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
#include "LogicPort.h"
#include "Pins.h"
#include "PortEvent.h"


//////////////////////////////////////////////////////////////////////////
//
// iTime is when the event executes.  It is added to pcPort's events.
//
//////////////////////////////////////////////////////////////////////////
void CPortEvent::Init(CLogicPort* pcPort, int64 iTime)
{
	CEvent::Init(iTime);
	mpcPort = pcPort;
	mpcPort->Add(this);
}


//////////////////////////////////////////////////////////////////////////
//
// For a loaded event.
//
//////////////////////////////////////////////////////////////////////////
void CPortEvent::Init(CLogicPort* pcPort, int64 iTime, int64 iID)
{
	CEvent::Init(iTime, iID);
	mpcPort = pcPort;
	mpcPort->Add(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPortEvent::RemoveFromOwner(void)
{
	CEvent::RemoveFromOwner();
	mpcPort->Remove(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPortEvent::IsDriveEvent(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True for the drives and slews that set what the port drives.
//
//////////////////////////////////////////////////////////////////////////
bool CPortEvent::IsOutputEvent(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogicPort* CPortEvent::GetPort(void)
{
	return mpcPort;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CPortEvent::GetIntegratedCircuit(void)
{
	return mpcPort->GetPins()->GetIntegratedCircuit();
}

