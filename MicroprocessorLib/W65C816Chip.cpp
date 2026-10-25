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
#include "W65C816InstructionFactory.h"
#include "W65C816Chip.h"


//////////////////////////////////////////////////////////////////////////
//
// Takes ownership of pcPins and wires the processor's signals to them.
// The instruction factory is made ready for the chip (see
// CW65C816InstructionFactory) until it's killed.
//
//////////////////////////////////////////////////////////////////////////
void CW65C816Chip::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CW65C816ChipPins* pcPins)
{
	CIntegratedCircuit::Init(pcContainingSubcircuitSimulation, szName, pcPins);

	CW65C816InstructionFactory::GetInstance()->Init();
	mcSignals.Init(pcPins->GetAddress(), pcPins->GetData(), pcPins->GetPHI2(), pcPins->GetRESB(), pcPins->GetRWB());
	mcSignals.SetImportantTraces(pcPins->GetVPA(), pcPins->GetVDA(), pcPins->GetVPB());
	mcSignals.SetOtherTraces(pcPins->GetMLB(), pcPins->GetMX(), pcPins->GetBE(), pcPins->GetE(), pcPins->GetRDY());
	mcSignals.SetInterruptTraces(pcPins->GetABORTB(), pcPins->GetIRQB(), pcPins->GetNMIB());
	mcMPU.Init(&mcSignals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816Chip::Kill(void)
{
	mcMPU.Kill();
	mcSignals.Kill();
	CW65C816InstructionFactory::GetInstance()->Kill();
	CIntegratedCircuit::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's reset, the processor starts out being reset.
//
//////////////////////////////////////////////////////////////////////////
void CW65C816Chip::SimulationStarted(CTimeline* pcTimeline)
{
	mcMPU.ResetPulled();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816Chip::InputTransition(CTimeline* pcTimeline, CLogicPort* pcPort)
{
	mcMPU.InputTransition(pcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CW65C816Chip::GetType(void) { return W65C816_CHIP_TYPE; }
CW65C816* CW65C816Chip::GetMPU(void) { return &mcMPU; }
CW65C816ChipPins* CW65C816Chip::GetChipPins(void) { return (CW65C816ChipPins*)mpcPins; }

