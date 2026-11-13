/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/PointerFunctions.h"
#include "65C64Instruction.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Instruction::Init(uint32 uiForm, C65C64InstructionCycles* pcCycles, char* szName, char* szDescription)
{
	muiForm = uiForm;
	mszName.Init(szName);
	mszDescription.Init(szDescription);
	mpcCycles = pcCycles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Instruction::Kill(void)
{
	mpcCycles->Kill();
	SafeFree(mpcCycles);

	mszName.Kill();
	mszDescription.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 C65C64Instruction::GetForm(void)
{
	return muiForm;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* C65C64Instruction::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* C65C64Instruction::GetDescription(void)
{
	return mszDescription.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65C64InstructionCycles* C65C64Instruction::GetCycles(void)
{
	return mpcCycles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65C64BusCycle* C65C64Instruction::GetBusCycle(int16 iCycle)
{
	return mpcCycles->GetBusCycle(iCycle);
}

