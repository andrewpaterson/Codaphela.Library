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
#include "65InstructionCycles.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65InstructionCycles::Init(C65BusCycleArray* papcCycles)
{
	C65BusCycle*	pcBusCycle;
	size			uiNumCycles;
	size			i;

	memcpy(&mapcCycles, papcCycles, sizeof(C65BusCycleArray));
	free(papcCycles);

	uiNumCycles = mapcCycles.NumElements();
	for (i = 0; i < uiNumCycles; i++)
	{
		pcBusCycle = mapcCycles.GetPtr(i);
		pcBusCycle->SetCycle((int16)(i + 1));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65InstructionCycles::Kill(void)
{
	mapcCycles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65BusCycle* C65InstructionCycles::GetBusCycle(int16 iIndex)
{
	if ((iIndex >= 0) && (iIndex < (int16)mapcCycles.NumElements()))
	{
		return mapcCycles.GetPtr(iIndex);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size C65InstructionCycles::Size(void)
{
	return mapcCycles.NumElements();
}

