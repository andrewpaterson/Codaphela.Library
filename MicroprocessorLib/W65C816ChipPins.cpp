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
#include "W65C816ChipPins.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static CLogicPort* AddChipPort(CPins* pcPins, EPortType eType, const char* szName, CVoltageConfigurationSource* pcVoltageConfiguration)
{
	CLogicPort*	pcPort;

	pcPort = NewMalloc<CLogicPort>();
	pcPort->Init(eType, pcPins, szName, pcVoltageConfiguration);
	return pcPort;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C816ChipPins::Init(CVoltageConfigurationSource* pcVoltageConfiguration)
{
	size	i;
	CChars	szName;

	CPins::Init();
	mcAddress.Init();
	for (i = 0; i < 16; i++)
	{
		szName.Init("Address ");
		szName.Append(i);
		mcAddress.Add(AddChipPort(this, PT_Output, szName.Text(), pcVoltageConfiguration));
		szName.Kill();
	}
	mcData.Init();
	for (i = 0; i < 8; i++)
	{
		szName.Init("Data ");
		szName.Append(i);
		mcData.Add(AddChipPort(this, PT_Bidirectional, szName.Text(), pcVoltageConfiguration));
		szName.Kill();
	}

	mpcVPB = AddChipPort(this, PT_Output, "VPB", pcVoltageConfiguration);
	mpcRDY = AddChipPort(this, PT_Bidirectional, "RDY", pcVoltageConfiguration);
	mpcABORTB = AddChipPort(this, PT_Input, "ABORTB", pcVoltageConfiguration);
	mpcIRQB = AddChipPort(this, PT_Input, "IRQB", pcVoltageConfiguration);
	mpcNMIB = AddChipPort(this, PT_Input, "NMIB", pcVoltageConfiguration);
	mpcMLB = AddChipPort(this, PT_Output, "MLB", pcVoltageConfiguration);
	mpcVPA = AddChipPort(this, PT_Output, "VPA", pcVoltageConfiguration);
	mpcVDA = AddChipPort(this, PT_Output, "VDA", pcVoltageConfiguration);
	mpcRESB = AddChipPort(this, PT_Input, "RESB", pcVoltageConfiguration);
	mpcMX = AddChipPort(this, PT_Output, "MX", pcVoltageConfiguration);
	mpcPHI2 = AddChipPort(this, PT_Input, "PHI2", pcVoltageConfiguration);
	mpcBE = AddChipPort(this, PT_Input, "BE", pcVoltageConfiguration);
	mpcE = AddChipPort(this, PT_Output, "E", pcVoltageConfiguration);
	mpcRWB = AddChipPort(this, PT_Output, "RWB", pcVoltageConfiguration);
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void CW65C816ChipPins::Kill(void)
{
	mcAddress.Kill();
	mcData.Kill();
	CPins::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogicPortBus* CW65C816ChipPins::GetAddress(void) { return &mcAddress; }
CLogicPortBus* CW65C816ChipPins::GetData(void) { return &mcData; }
CLogicPort* CW65C816ChipPins::GetVPB(void) { return mpcVPB; }
CLogicPort* CW65C816ChipPins::GetRDY(void) { return mpcRDY; }
CLogicPort* CW65C816ChipPins::GetABORTB(void) { return mpcABORTB; }
CLogicPort* CW65C816ChipPins::GetIRQB(void) { return mpcIRQB; }
CLogicPort* CW65C816ChipPins::GetNMIB(void) { return mpcNMIB; }
CLogicPort* CW65C816ChipPins::GetMLB(void) { return mpcMLB; }
CLogicPort* CW65C816ChipPins::GetVPA(void) { return mpcVPA; }
CLogicPort* CW65C816ChipPins::GetVDA(void) { return mpcVDA; }
CLogicPort* CW65C816ChipPins::GetRESB(void) { return mpcRESB; }
CLogicPort* CW65C816ChipPins::GetMX(void) { return mpcMX; }
CLogicPort* CW65C816ChipPins::GetPHI2(void) { return mpcPHI2; }
CLogicPort* CW65C816ChipPins::GetBE(void) { return mpcBE; }
CLogicPort* CW65C816ChipPins::GetE(void) { return mpcE; }
CLogicPort* CW65C816ChipPins::GetRWB(void) { return mpcRWB; }

