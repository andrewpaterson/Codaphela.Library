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
#include "Trace.h"
#include "PowerInPort.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPowerInPort::Init(EPortType eType, const char* szName, CPortHolder* pcHolder)
{
	CPort::Init(eType, szName, pcHolder);
}


//////////////////////////////////////////////////////////////////////////
//
// NaN when not connected.
//
//////////////////////////////////////////////////////////////////////////
float32 CPowerInPort::GetVoltageIn(int64 iTime)
{
	if (mpcTrace != NULL)
	{
		return mpcTrace->GetVoltage(iTime);
	}
	else
	{
		return NoVoltage();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPowerInPort::IsPowerIn(void)
{
	return true;
}

