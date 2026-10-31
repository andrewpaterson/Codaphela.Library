/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "Asm65C64Lockstep.h"


//////////////////////////////////////////////////////////////////////////
//
// pcFast has run reset on pcFastMemory.  The simulator gets a copy of the memory and runs reset too.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65C64Lockstep::Init(CAsm65Memory* pcFastMemory, CFast65C64* pcFast)
{
	mpcFastMemory = pcFastMemory;
	mpcFast = pcFast;
	muiSteps = 0;
	mbStopped = false;
	mszDifferences.Init();

	mpcSimMemory = NewMalloc<CAsm65Memory>();
	mpcSimMemory->Init(mpcFastMemory->GetLowSize());
	mpcSimMemory->CopyFrom(mpcFastMemory);
	mcMeta.Init(CAsm65Memory::TickHigh65C64, CAsm65Memory::TickLow65C64, mpcSimMemory);
	mcMeta.TickInstruction();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65C64Lockstep::Kill(void)
{
	mcMeta.Kill();
	SafeKill(mpcSimMemory);
	mszDifferences.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Steps both until either stops, they differ or uiMaxSteps.  Memory is compared every uiMemoryInterval steps (never
// if 0) and at the end.  Returns the number of steps.
//
//////////////////////////////////////////////////////////////////////////
size CAsm65C64Lockstep::Run(size uiMaxSteps, size uiMemoryInterval)
{
	size	i;

	for (i = 0; i < uiMaxSteps; i++)
	{
		if (!Step())
		{
			break;
		}
		if ((uiMemoryInterval != 0) && ((muiSteps % uiMemoryInterval) == 0) && !CompareMemory())
		{
			return muiSteps;
		}
	}
	if (mszDifferences.Empty())
	{
		CompareMemory();
	}
	return muiSteps;
}


//////////////////////////////////////////////////////////////////////////
//
// One step on both.  Returns false if either stopped or they differ.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Lockstep::Step(void)
{
	C65C64*	pcSim;
	uint32	uiC;
	bool	bSim;
	bool	bFast;
	CChars	szDifferences;

	pcSim = mcMeta.GetMPU();
	uiC = pcSim->GetProgramCounter();
	bSim = mcMeta.TickInstruction();
	bFast = mpcFast->Step();
	muiSteps++;
	mbStopped = !bSim || !bFast;

	szDifferences.Init();
	mpcFast->AppendDifferences(pcSim, &szDifferences);
	mpcFast->AppendDifference(&szDifferences, "Running", bSim, bFast);
	if (!szDifferences.Empty())
	{
		mszDifferences.Append("Step ");
		mszDifferences.Append((int)muiSteps);
		mszDifferences.Append(" at 0x");
		mszDifferences.AppendHexHiLo(&uiC, 4);
		mszDifferences.Append(":");
		mszDifferences.Append(szDifferences);
	}
	szDifferences.Kill();
	return !mbStopped && mszDifferences.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false (and notes it) if the memories or their bad access counts differ.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Lockstep::CompareMemory(void)
{
	bool	bSame;

	bSame = (memcmp(mpcSimMemory->Byte(0), mpcFastMemory->Byte(0), mpcFastMemory->GetLowSize()) == 0);
	bSame &= (memcmp(mpcSimMemory->Byte(ASM65_MEMORY_HIGH_BASE), mpcFastMemory->Byte(ASM65_MEMORY_HIGH_BASE), ASM65_MEMORY_HIGH_SIZE) == 0);
	bSame &= mpcSimMemory->NumBadAccesses() == mpcFastMemory->NumBadAccesses();
	if (!bSame)
	{
		mszDifferences.Append("After step ");
		mszDifferences.Append((int)muiSteps);
		mszDifferences.Append(": the memories or their bad accesses differ.");
	}
	return bSame;
}


//////////////////////////////////////////////////////////////////////////
//
// What differed first, empty if nothing has.
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65C64Lockstep::GetDifferences(void)
{
	return mszDifferences.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65C64Lockstep::IsStopped(void)
{
	return mbStopped;
}

