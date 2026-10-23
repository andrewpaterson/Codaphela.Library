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
#include "Voltage.h"
#include "PowerOutPort.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::Init(const char* szName, CPortHolder* pcHolder, float32 fVoltage)
{
	CPort::Init(PT_PowerOut, szName, pcHolder);
	mfVoltage = fVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CPowerOutPort::GetVoltageOut(int64 iTime)
{
	return mfVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
// Like "VCC (3.3V)".
//
//////////////////////////////////////////////////////////////////////////
void CPowerOutPort::AppendDescription(CChars* psz)
{
	psz->Append(mszName);
	psz->Append(" (");
	AppendVoltage(psz, mfVoltage);
	psz->Append(")");
}

