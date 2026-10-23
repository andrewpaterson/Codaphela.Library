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
#include "BaseLib/ConstructorCall.h"
#include "PowerPinNames.h"
#include "Pin.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPin::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, size uiPortCount)
{
	size		i;
	CTracePort*	pcPort;
	CChars		szPortName;

	CPassive::Init(pcContainingSubcircuitSimulation, szName);
	mapcTracePorts.Init();
	for (i = 0; i < uiPortCount; i++)
	{
		szPortName.Init("Port ");
		szPortName.Append(i);
		pcPort = NewMalloc<CTracePort>();
		pcPort->Init(szPortName.Text(), this);
		mapcTracePorts.Add(pcPort);
		szPortName.Kill();
	}

	mpcVCC = NewMalloc<CPowerInPort>();
	mpcVCC->Init(PT_PowerIn, POWER_PIN_VCC, this);
	mpcGND = NewMalloc<CPowerInPort>();
	mpcGND->Init(PT_PowerIn, POWER_PIN_GND, this);
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void CPin::Kill(void)
{
	mapcTracePorts.Kill();
	mpcVCC = NULL;
	mpcGND = NULL;
	CPassive::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CPin::GetVCC(int64 iTime)
{
	return mpcVCC->GetVoltageIn(iTime);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CPin::GetType(void) { return "Pin"; }
size CPin::NumTracePorts(void) { return mapcTracePorts.NumElements(); }
CTracePort* CPin::GetTracePort(size uiIndex) { return mapcTracePorts.SafeGetPtr(uiIndex); }
CPowerInPort* CPin::GetVoltageCommon(void) { return mpcVCC; }
CPowerInPort* CPin::GetVoltageGround(void) { return mpcGND; }

