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
#include "ConstantPins.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstantPins::Init(CVoltageConfigurationSource* pcVoltageConfiguration, size uiBitWidth)
{
	size		i;
	CLogicPort*	pcPort;
	CChars		szName;

	CPins::Init();
	mapcOutputs.Init();
	for (i = 0; i < uiBitWidth; i++)
	{
		szName.Init("Output ");
		szName.Append(i);
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
void CConstantPins::Kill(void)
{
	mapcOutputs.Kill();
	CPins::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayLogicPortPtr* CConstantPins::GetOutputs(void) { return &mapcOutputs; }
CLogicPort* CConstantPins::GetOutput(size uiIndex) { return mapcOutputs.SafeGetPtr(uiIndex); }

