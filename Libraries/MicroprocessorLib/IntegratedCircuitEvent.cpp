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
#include "IntegratedCircuit.h"
#include "IntegratedCircuitEvent.h"


//////////////////////////////////////////////////////////////////////////
//
// iTime is when the event executes.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitEvent::Init(int64 iTime, CIntegratedCircuit* pcIntegratedCircuit)
{
	CEvent::Init(iTime);
	mpcIntegratedCircuit = pcIntegratedCircuit;
	mpcIntegratedCircuit->Add(this);
}


//////////////////////////////////////////////////////////////////////////
//
// For a loaded event.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitEvent::Init(int64 iTime, int64 iID, CIntegratedCircuit* pcIntegratedCircuit)
{
	CEvent::Init(iTime, iID);
	mpcIntegratedCircuit = pcIntegratedCircuit;
	mpcIntegratedCircuit->Add(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitEvent::RemoveFromOwner(void)
{
	CEvent::RemoveFromOwner();
	mpcIntegratedCircuit->Remove(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CIntegratedCircuitEvent::GetIntegratedCircuit(void)
{
	return mpcIntegratedCircuit;
}

