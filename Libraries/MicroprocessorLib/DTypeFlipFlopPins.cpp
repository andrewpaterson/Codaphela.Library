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
#include "DTypeFlipFlopPins.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlopPins::Init(CVoltageConfigurationSource* pcVoltageConfiguration)
{
	PreInit();

	CPins::Init();
	mpcOutputQ = NewMalloc<CLogicPort>();
	mpcOutputQ->Init(PT_Output, this, "Q", pcVoltageConfiguration);
	mpcOutputQB = NewMalloc<CLogicPort>();
	mpcOutputQB->Init(PT_Output, this, "QB", pcVoltageConfiguration);
	mpcClock = NewMalloc<CLogicPort>();
	mpcClock->Init(PT_Input, this, "CP", pcVoltageConfiguration);
	mpcData = NewMalloc<CLogicPort>();
	mpcData->Init(PT_Input, this, "D", pcVoltageConfiguration);
	mpcSet = NewMalloc<CLogicPort>();
	mpcSet->Init(PT_Input, this, "SD", pcVoltageConfiguration);
	mpcReset = NewMalloc<CLogicPort>();
	mpcReset->Init(PT_Input, this, "RD", pcVoltageConfiguration);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlopPins::Class(void)
{
	CPins::Class();

	U_Pointer(mpcOutputQ);
	U_Pointer(mpcOutputQB);
	U_Pointer(mpcClock);
	U_Pointer(mpcData);
	U_Pointer(mpcSet);
	U_Pointer(mpcReset);
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void CDTypeFlipFlopPins::Free(void)
{
	mpcOutputQ = NULL;
	mpcOutputQB = NULL;
	mpcClock = NULL;
	mpcData = NULL;
	mpcSet = NULL;
	mpcReset = NULL;
	CPins::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDTypeFlipFlopPins::IsClock(CLogicPort* pcPort) { return pcPort == mpcClock; }
CLogicPort* CDTypeFlipFlopPins::GetOutputQ(void) { return mpcOutputQ; }
CLogicPort* CDTypeFlipFlopPins::GetOutputQB(void) { return mpcOutputQB; }
CLogicPort* CDTypeFlipFlopPins::GetClock(void) { return mpcClock; }
CLogicPort* CDTypeFlipFlopPins::GetData(void) { return mpcData; }
CLogicPort* CDTypeFlipFlopPins::GetSet(void) { return mpcSet; }
CLogicPort* CDTypeFlipFlopPins::GetReset(void) { return mpcReset; }

