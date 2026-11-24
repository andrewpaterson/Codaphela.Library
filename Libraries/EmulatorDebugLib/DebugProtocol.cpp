/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela EmulatorDebugLib

Codaphela EmulatorDebugLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela EmulatorDebugLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela EmulatorDebugLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "DebugProtocol.h"


//////////////////////////////////////////////////////////////////////////
//
// An execute breakpoint, or (eKind DBK_Write or DBK_ReadWrite) a data
// breakpoint over uiBytes.
//
//////////////////////////////////////////////////////////////////////////
void SDebugBreakpoint::Init(uint32 uiAddress, bool bEnabled, uint32 uiSpace, uint32 uiProcess, EDebugBreakpointKind eKind, uint32 uiBytes)
{
	this->uiAddress = uiAddress;
	uiEnabled = bEnabled;
	this->uiSpace = uiSpace;
	this->uiProcess = uiProcess;
	uiKind = eKind;
	this->uiBytes = (eKind == DBK_Execute) ? 0 : uiBytes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SDebugBreakpoint::IsData(void) const
{
	return uiKind != DBK_Execute;
}


//////////////////////////////////////////////////////////////////////////
//
// The same breakpoint (enabled or not): where it is, its space, process,
// kind and bytes.
//
//////////////////////////////////////////////////////////////////////////
bool SDebugBreakpoint::SameAs(SDebugBreakpoint* psOther) const
{
	return (uiAddress == psOther->uiAddress) && (uiSpace == psOther->uiSpace) && (uiProcess == psOther->uiProcess) &&
		   (uiKind == psOther->uiKind) && (uiBytes == psOther->uiBytes);
}

