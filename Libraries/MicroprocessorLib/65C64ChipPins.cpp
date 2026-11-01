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
#include "65C64ChipPins.h"


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
void C65C64ChipPins::Init(CVoltageConfigurationSource* pcVoltageConfiguration)
{
	size	i;
	CChars	szName;

	PreInit();

	CPins::Init();
	mcAddress.Init();
	for (i = 0; i < 32; i++)
	{
		szName.Init("Address ");
		szName.Append(i);
		mcAddress.Add(AddChipPort(this, PT_Output, szName.Text(), pcVoltageConfiguration));
		szName.Kill();
	}
	mcData.Init();
	for (i = 0; i < 16; i++)
	{
		szName.Init("Data ");
		szName.Append(i);
		mcData.Add(AddChipPort(this, PT_Bidirectional, szName.Text(), pcVoltageConfiguration));
		szName.Kill();
	}

	mpcPHI2 = AddChipPort(this, PT_Input, "PHI2", pcVoltageConfiguration);
	mpcRESB = AddChipPort(this, PT_Input, "RESB", pcVoltageConfiguration);
	mpcRWB = AddChipPort(this, PT_Output, "RWB", pcVoltageConfiguration);
	mpcLowerByte = AddChipPort(this, PT_Output, "Lower Byte", pcVoltageConfiguration);
	mpcUpperByte = AddChipPort(this, PT_Output, "Upper Byte", pcVoltageConfiguration);
	mpcOPV = AddChipPort(this, PT_Output, "OPV", pcVoltageConfiguration);
	mpcVPA = AddChipPort(this, PT_Output, "VPA", pcVoltageConfiguration);
	mpcVDA = AddChipPort(this, PT_Output, "VDA", pcVoltageConfiguration);
	mpcVPB = AddChipPort(this, PT_Output, "VPB", pcVoltageConfiguration);
	for (i = 0; i < NUM_65C64_EDGE_INTERRUPTS; i++)
	{
		szName.Init("EIRQB ");
		szName.Append(i);
		mapcEIRQB[i] = AddChipPort(this, PT_Input, szName.Text(), pcVoltageConfiguration);
		szName.Kill();
	}
	mpcLIRQB = AddChipPort(this, PT_Input, "LIRQB", pcVoltageConfiguration);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64ChipPins::Class(void)
{
	CPins::Class();

	U_Data(CLogicPortBus, mcAddress);
	U_Data(CLogicPortBus, mcData);
	U_Pointer(mpcPHI2);
	U_Pointer(mpcRESB);
	U_Pointer(mpcRWB);
	U_Pointer(mpcLowerByte);
	U_Pointer(mpcUpperByte);
	U_Pointer(mpcOPV);
	U_Pointer(mpcVPA);
	U_Pointer(mpcVDA);
	U_Pointer(mpcVPB);
	U_Data(CLogicPort*[NUM_65C64_EDGE_INTERRUPTS], mapcEIRQB);
	U_Pointer(mpcLIRQB);
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void C65C64ChipPins::Free(void)
{
	mcAddress.Kill();
	mcData.Kill();
	CPins::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// NULL past the last.
//
//////////////////////////////////////////////////////////////////////////
CLogicPort* C65C64ChipPins::GetEIRQB(size uiInterrupt)
{
	if (uiInterrupt < NUM_65C64_EDGE_INTERRUPTS)
	{
		return mapcEIRQB[uiInterrupt];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogicPortBus* C65C64ChipPins::GetAddress(void) { return &mcAddress; }
CLogicPortBus* C65C64ChipPins::GetData(void) { return &mcData; }
CLogicPort* C65C64ChipPins::GetPHI2(void) { return mpcPHI2; }
CLogicPort* C65C64ChipPins::GetRESB(void) { return mpcRESB; }
CLogicPort* C65C64ChipPins::GetRWB(void) { return mpcRWB; }
CLogicPort* C65C64ChipPins::GetLowerByte(void) { return mpcLowerByte; }
CLogicPort* C65C64ChipPins::GetUpperByte(void) { return mpcUpperByte; }
CLogicPort* C65C64ChipPins::GetOPV(void) { return mpcOPV; }
CLogicPort* C65C64ChipPins::GetVPA(void) { return mpcVPA; }
CLogicPort* C65C64ChipPins::GetVDA(void) { return mpcVDA; }
CLogicPort* C65C64ChipPins::GetVPB(void) { return mpcVPB; }
CLogicPort* C65C64ChipPins::GetLIRQB(void) { return mpcLIRQB; }

