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
#include "CounterPins.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static CLogicPort* AddCounterPort(CPins* pcPins, EPortType eType, const char* szName, size i, CVoltageConfigurationSource* pcVoltageConfiguration)
{
	CLogicPort*	pcPort;
	CChars		szPortName;

	szPortName.Init(szName);
	if (i != (size)-1)
	{
		szPortName.Append(' ');
		szPortName.Append(i);
	}
	pcPort = NewMalloc<CLogicPort>();
	pcPort->Init(eType, pcPins, szPortName.Text(), pcVoltageConfiguration);
	szPortName.Kill();
	return pcPort;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounterPins::Init(CVoltageConfigurationSource* pcVoltageConfiguration, size uiBitWidth)
{
	size	i;

	PreInit();

	CPins::Init();
	mpcClock = AddCounterPort(this, PT_Input, "Clock", -1, pcVoltageConfiguration);
	mapcData.Init();
	for (i = 0; i < uiBitWidth; i++)
	{
		mapcData.Add(AddCounterPort(this, PT_Input, "Data", i, pcVoltageConfiguration));
	}
	mpcReset = AddCounterPort(this, PT_Input, "Reset", -1, pcVoltageConfiguration);
	mpcLoad = AddCounterPort(this, PT_Input, "Load", -1, pcVoltageConfiguration);
	mpcCountEnable = AddCounterPort(this, PT_Input, "Enable", -1, pcVoltageConfiguration);
	mpcCountEnableTerminal = AddCounterPort(this, PT_Input, "Enable Terminal", -1, pcVoltageConfiguration);

	mapcOutput.Init();
	for (i = 0; i < uiBitWidth; i++)
	{
		mapcOutput.Add(AddCounterPort(this, PT_Output, "Count", i, pcVoltageConfiguration));
	}
	mpcTerminalOutput = AddCounterPort(this, PT_Output, "Terminal Count", -1, pcVoltageConfiguration);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounterPins::Class(void)
{
	CPins::Class();

	U_Pointer(mpcClock);
	U_Pointer(mpcReset);
	U_Pointer(mpcLoad);
	U_Data(CArrayLogicPortPtr, mapcData);
	U_Pointer(mpcCountEnable);
	U_Pointer(mpcCountEnableTerminal);
	U_Data(CArrayLogicPortPtr, mapcOutput);
	U_Pointer(mpcTerminalOutput);
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void CCounterPins::Free(void)
{
	mapcData.Kill();
	mapcOutput.Kill();
	CPins::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// A bit is 1 only when its input reads high.
//
//////////////////////////////////////////////////////////////////////////
uint32 CCounterPins::ReadData(CTimeline* pcTimeline, size uiBitWidth)
{
	uint32	uiData;
	size	i;

	uiData = 0;
	for (i = 0; i < uiBitWidth; i++)
	{
		if (mapcData.GetPtr(i)->Read(pcTimeline).IsHigh())
		{
			uiData |= (uint32)1 << i;
		}
	}
	return uiData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCounterPins::WriteOutput(CTimeline* pcTimeline, uint32 uiData, size uiBitWidth)
{
	size	i;

	for (i = 0; i < uiBitWidth; i++)
	{
		mapcOutput.GetPtr(i)->WriteBool(pcTimeline, (uiData & 1) == 1);
		uiData >>= 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCounterPins::IsClock(CLogicPort* pcPort) { return pcPort == mpcClock; }
CLogicPort* CCounterPins::GetClock(void) { return mpcClock; }
CLogicPort* CCounterPins::GetReset(void) { return mpcReset; }
CLogicPort* CCounterPins::GetLoad(void) { return mpcLoad; }
CArrayLogicPortPtr* CCounterPins::GetData(void) { return &mapcData; }
CLogicPort* CCounterPins::GetCountEnable(void) { return mpcCountEnable; }
CLogicPort* CCounterPins::GetCountEnableTerminal(void) { return mpcCountEnableTerminal; }
CArrayLogicPortPtr* CCounterPins::GetOutput(void) { return &mapcOutput; }
CLogicPort* CCounterPins::GetTerminalOutput(void) { return mpcTerminalOutput; }

