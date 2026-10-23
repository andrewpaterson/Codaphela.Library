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
#include "65C64Chip.h"


//////////////////////////////////////////////////////////////////////////
//
// Takes ownership of pcPins and wires the processor's signals to them.
//
//////////////////////////////////////////////////////////////////////////
void C65C64Chip::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, C65C64ChipPins* pcPins)
{
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pcPins);

	mcSignals.Init(pcPins->GetAddress(), pcPins->GetData(), pcPins->GetPHI2(), pcPins->GetRESB(), pcPins->GetRWB());
	mcSignals.SetByteTraces(pcPins->GetLowerByte(), pcPins->GetUpperByte());
	mcSignals.SetValidTraces(pcPins->GetOPV(), pcPins->GetVPA(), pcPins->GetVDA(), pcPins->GetVPB());
	mcSignals.SetInterruptTraces(pcPins->GetEIRQB(0), pcPins->GetEIRQB(1), pcPins->GetEIRQB(2), pcPins->GetEIRQB(3), pcPins->GetLIRQB());
	mcMPU.Init(&mcSignals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Chip::Kill(void)
{
	mcMPU.Kill();
	mcSignals.Kill();
	CIntegratedCircuit::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The processor starts out being reset.
//
//////////////////////////////////////////////////////////////////////////
void C65C64Chip::SimulationStarted(CTimeline* pcTimeline)
{
	mcMPU.ResetPulled();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Chip::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	mcMPU.InputTransition(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
// The processor holds its own state.
//
//////////////////////////////////////////////////////////////////////////
CState* C65C64Chip::CreateState(void)
{
	return NewMalloc<CState>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* C65C64Chip::GetType(void) { return C65C64_CHIP_TYPE; }
C65C64* C65C64Chip::GetMPU(void) { return &mcMPU; }
C65C64ChipPins* C65C64Chip::GetChipPins(void) { return (C65C64ChipPins*)mpcPins; }

