/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela EmulatorDebugLib

Codaphela EmulatorDebugLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela EmulatorDebugLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela EmulatorDebugLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "DebugTarget.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetDebugRegister(SDebugRegister* psDest, const char* szName, uint8 uiBits, EDebugRegisterKind eKind, const char* szFlags)
{
	memset(psDest, 0, sizeof(SDebugRegister));
	strncpy(psDest->szName, szName, DEBUG_REGISTER_NAME_SIZE - 1);
	psDest->uiBits = uiBits;
	psDest->uiKind = (uint8)eKind;
	strncpy(psDest->szFlags, szFlags, DEBUG_REGISTER_FLAGS_SIZE - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size ClampDebugMemory(uint32 uiAddress, size uiSize, uint32 uiMemorySize)
{
	if (uiAddress >= uiMemorySize)
	{
		return 0;
	}
	if (uiSize > uiMemorySize - uiAddress)
	{
		return uiMemorySize - uiAddress;
	}
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size DecodeDebugInstructions(CDebugTarget* pcTarget, uint32 uiAddress, size uiCount, SDebugInstruction* asDest)
{
	CDebugDecoder*	pcDecoder;

	pcDecoder = pcTarget->GetDecoder();
	if (pcDecoder == NULL)
	{
		return 0;
	}
	return pcDecoder->Decode(pcTarget, uiAddress, uiCount, asDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void StartDebugInstruction(SDebugInstruction* psDest, CDebugTarget* pcTarget, uint32 uiAddress, size uiLength)
{
	memset(psDest, 0, sizeof(SDebugInstruction));
	if (uiLength > DEBUG_INSTRUCTION_BYTES)
	{
		uiLength = DEBUG_INSTRUCTION_BYTES;
	}
	psDest->uiAddress = uiAddress;
	psDest->uiLength = (uint8)uiLength;
	psDest->uiKind = DI_Other;
	pcTarget->PeekMemory(uiAddress, psDest->auiBytes, uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
// One breakpoint space unless the target has more.
//
//////////////////////////////////////////////////////////////////////////
size CDebugTarget::NumBreakpointSpaces(void)
{
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The name a debugger chooses the space by ("Kernel"); the only space of
// a target with one has none.
//
//////////////////////////////////////////////////////////////////////////
const char* CDebugTarget::GetBreakpointSpaceName(size uiSpace)
{
	return "";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBreakpoints* CDebugTarget::GetSpaceBreakpoints(size uiSpace)
{
	return (uiSpace == 0) ? GetBreakpoints() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A target without processes has no process's breakpoints.
//
//////////////////////////////////////////////////////////////////////////
CBreakpoints* CDebugTarget::GetProcessBreakpoints(size uiSpace, uint32 uiProcess)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Every space's breakpoints (and its processes', for a target with
// processes).
//
//////////////////////////////////////////////////////////////////////////
void CDebugTarget::ClearBreakpoints(void)
{
	size	i;

	for (i = 0; i < NumBreakpointSpaces(); i++)
	{
		GetSpaceBreakpoints(i)->Clear();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugTarget::EnableAllBreakpoints(bool bEnabled)
{
	size	i;

	for (i = 0; i < NumBreakpointSpaces(); i++)
	{
		GetSpaceBreakpoints(i)->EnableAll(bEnabled);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A bare processor runs no processes.
//
//////////////////////////////////////////////////////////////////////////
bool CDebugTarget::HasProcesses(void)							{ return false; }
size CDebugTarget::NumProcesses(void)							{ return 0; }
bool CDebugTarget::GetProcess(size uiIndex, SDebugProcess* psDest)	{ return false; }
void CDebugTarget::WatchProcesses(bool bWatch)					{ }
bool CDebugTarget::TakeProcessEvent(EDebugProcessEvent* peEvent, SDebugProcess* psDest)	{ return false; }

