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
#include "ProbePins.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProbePins::Init(CVoltageConfigurationSource* pcVoltageConfiguration, size uiBitWidth)
{
	size		i;
	CLogicPort*	pcPort;
	CChars		szName;

	PreInit();

	CPins::Init();
	mapcInputs.Init();
	for (i = 0; i < uiBitWidth; i++)
	{
		szName.Init("Input ");
		szName.Append(i);
		pcPort = NewMalloc<CLogicPort>();
		pcPort->Init(PT_Input, this, szName.Text(), pcVoltageConfiguration);
		mapcInputs.Add(pcPort);
		szName.Kill();
	}

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProbePins::Class(void)
{
	CPins::Class();

	U_Data(CArrayLogicPortPtr, mapcInputs);
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void CProbePins::Free(void)
{
	mapcInputs.Kill();
	CPins::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayLogicPortPtr* CProbePins::GetInputs(void) { return &mapcInputs; }
CLogicPort* CProbePins::GetInput(size uiIndex) { return mapcInputs.SafeGetPtr(uiIndex); }

