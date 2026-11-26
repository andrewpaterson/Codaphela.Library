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
#include "PowerInPort.h"
#include "PullPort.h"


//////////////////////////////////////////////////////////////////////////
//
// pcSupply is the pull's port on the supply it pulls to.
//
//////////////////////////////////////////////////////////////////////////
void CPullPort::Init(const char* szName, CPortHolder* pcHolder, CPowerInPort* pcSupply, float32 fResistance)
{
	CPort::Init(PT_Passive, szName, pcHolder);
	mpcSupply = pcSupply;
	mfResistance = fResistance;
}


//////////////////////////////////////////////////////////////////////////
//
// The supply's voltage, NaN while it has none.
//
//////////////////////////////////////////////////////////////////////////
float32 CPullPort::GetVoltageOut(int64 iTime)
{
	return mpcSupply->GetVoltageIn(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
// The supply's voltage, weakly.
//
//////////////////////////////////////////////////////////////////////////
float32 CPullPort::GetDrive(int64 iTime, float32* pfResistance)
{
	*pfResistance = mfResistance;
	return mpcSupply->GetVoltageIn(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CPullPort::GetResistance(void) const { return mfResistance; }

