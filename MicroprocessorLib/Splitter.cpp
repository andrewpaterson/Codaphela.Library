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
#include "Splitter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSplitter::Init(CSubcircuitSimulation* pcContainingSubcircuitSimulation, const char* szName, CArrayChars* paszStartPortNames, CArrayChars* paszEndPortNames)
{
	PreInit();

	CPassive::Init(pcContainingSubcircuitSimulation, szName);
	mapcStartPorts.Init();
	mapcEndPorts.Init();
	AddTracePorts(&mapcStartPorts, paszStartPortNames);
	AddTracePorts(&mapcEndPorts, paszEndPortNames);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSplitter::Class(void)
{
	CPassive::Class();

	U_Data(CArrayTracePortPtr, mapcStartPorts);
	U_Data(CArrayTracePortPtr, mapcEndPorts);
}


//////////////////////////////////////////////////////////////////////////
//
// The ports are freed by CPortHolder.
//
//////////////////////////////////////////////////////////////////////////
void CSplitter::Free(void)
{
	mapcStartPorts.Kill();
	mapcEndPorts.Kill();
	CPassive::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSplitter::AddTracePorts(CArrayTracePortPtr* papcPorts, CArrayChars* paszPortNames)
{
	size		i;
	CTracePort*	pcPort;

	for (i = 0; i < paszPortNames->NumElements(); i++)
	{
		pcPort = NewMalloc<CTracePort>();
		pcPort->Init(paszPortNames->Get(i)->Text(), this);
		papcPorts->Add(pcPort);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CSplitter::GetType(void) { return "Splitter"; }
size CSplitter::NumStartPorts(void) { return mapcStartPorts.NumElements(); }
CTracePort* CSplitter::GetStartPort(size uiIndex) { return mapcStartPorts.SafeGetPtr(uiIndex); }
size CSplitter::NumEndPorts(void) { return mapcEndPorts.NumElements(); }
CTracePort* CSplitter::GetEndPort(size uiIndex) { return mapcEndPorts.SafeGetPtr(uiIndex); }

