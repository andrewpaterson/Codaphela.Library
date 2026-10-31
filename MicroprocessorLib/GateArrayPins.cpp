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
#include "GateArrayPins.h"


//////////////////////////////////////////////////////////////////////////
//
// Gates are numbered from 1 and their inputs lettered from A.
//
//////////////////////////////////////////////////////////////////////////
void CGateArrayPins::Init(size uiGateCount, size uiInputCount, CVoltageConfigurationSource* pcVoltageConfiguration)
{
	size		i;
	size		j;
	CLogicPort*	pcPort;
	CChars		szName;

	PreInit();

	CPins::Init();
	muiGateCount = uiGateCount;
	muiInputCount = uiInputCount;
	mapcInputs.Init();
	mapcOutputs.Init();
	for (i = 0; i < uiGateCount; i++)
	{
		for (j = 0; j < uiInputCount; j++)
		{
			szName.Init();
			GetInputName(&szName, i, j);
			pcPort = NewMalloc<CLogicPort>();
			pcPort->Init(PT_Input, this, szName.Text(), pcVoltageConfiguration);
			mapcInputs.Add(pcPort);
			szName.Kill();
		}

		szName.Init();
		GetOutputName(&szName, i);
		pcPort = NewMalloc<CLogicPort>();
		pcPort->Init(PT_Output, this, szName.Text(), pcVoltageConfiguration);
		mapcOutputs.Add(pcPort);
		szName.Kill();
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGateArrayPins::Class(void)
{
	CPins::Class();

	U_Size(muiGateCount);
	U_Size(muiInputCount);
	U_Data(CArrayLogicPortPtr, mapcInputs);
	U_Data(CArrayLogicPortPtr, mapcOutputs);
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void CGateArrayPins::Free(void)
{
	mapcInputs.Kill();
	mapcOutputs.Kill();
	CPins::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// uiGate and uiInput are from 0: "1A" is gate 0's input 0.
//
//////////////////////////////////////////////////////////////////////////
void CGateArrayPins::GetInputName(CChars* psz, size uiGate, size uiInput)
{
	psz->Append((int32)(uiGate + 1));
	psz->Append((char)('A' + uiInput));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGateArrayPins::GetOutputName(CChars* psz, size uiGate)
{
	psz->Append((int32)(uiGate + 1));
	psz->Append('Y');
}


//////////////////////////////////////////////////////////////////////////
//
// The gate the input is of, or ARRAY_ELEMENT_NOT_FOUND.
//
//////////////////////////////////////////////////////////////////////////
size CGateArrayPins::FindGate(CLogicPort* pcInput)
{
	size	uiIndex;

	uiIndex = mapcInputs.Find(pcInput);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	return uiIndex / muiInputCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGateArrayPins::GetGateCount(void) { return muiGateCount; }
size CGateArrayPins::GetInputCount(void) { return muiInputCount; }
CLogicPort* CGateArrayPins::GetInput(size uiGate, size uiInput) { return mapcInputs.SafeGetPtr(uiGate * muiInputCount + uiInput); }
CLogicPort* CGateArrayPins::GetOutput(size uiGate) { return mapcOutputs.SafeGetPtr(uiGate); }

