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
#include "BufferPins.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBufferPins::Init(size uiBufferCount, CVoltageConfigurationSource* pcVoltageConfiguration)
{
	size		i;
	CLogicPort*	pcPort;
	CChars		szName;

	CPins::Init();
	mapcInputs.Init();
	mapcOutputs.Init();
	for (i = 0; i < uiBufferCount; i++)
	{
		szName.Init();
		CalculatePortName(&szName, "Input", i, uiBufferCount);
		pcPort = NewMalloc<CLogicPort>();
		pcPort->Init(PT_Input, this, szName.Text(), pcVoltageConfiguration);
		mapcInputs.Add(pcPort);
		szName.Kill();

		szName.Init();
		CalculatePortName(&szName, "Output", i, uiBufferCount);
		pcPort = NewMalloc<CLogicPort>();
		pcPort->Init(PT_Output, this, szName.Text(), pcVoltageConfiguration);
		mapcOutputs.Add(pcPort);
		szName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void CBufferPins::Kill(void)
{
	mapcInputs.Kill();
	mapcOutputs.Kill();
	CPins::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayLogicPortPtr* CBufferPins::GetInputs(void) { return &mapcInputs; }
CArrayLogicPortPtr* CBufferPins::GetOutputs(void) { return &mapcOutputs; }
CLogicPort* CBufferPins::GetInput(size uiIndex) { return mapcInputs.SafeGetPtr(uiIndex); }
CLogicPort* CBufferPins::GetOutput(size uiIndex) { return mapcOutputs.SafeGetPtr(uiIndex); }

