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
#include "65Instruction.h"
#include "65C64Meta.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Meta::Init(C65C64MetaTick fTickHigh, C65C64MetaTick fTickLow, void* pvContext)
{
	size	i;

	mcAddress.Init();
	mcData.Init();
	mcPHI2.Init(false);
	mcRESB.Init(false);
	mcRWB.Init();
	mcLowerByte.Init();
	mcUpperByte.Init();
	mcOPV.Init();
	mcVPA.Init();
	mcVDA.Init();
	mcVPB.Init();
	for (i = 0; i < NUM_65C64_EDGE_INTERRUPTS; i++)
	{
		macEIRQB[i].Init(true);
	}
	mcLIRQB.Init(true);

	mcPins.Init(&mcAddress, &mcData, &mcPHI2, &mcRESB, &mcRWB);
	mcPins.SetByteTraces(&mcLowerByte, &mcUpperByte);
	mcPins.SetValidTraces(&mcOPV, &mcVPA, &mcVDA, &mcVPB);
	mcPins.SetInterruptTraces(&macEIRQB[0], &macEIRQB[1], &macEIRQB[2], &macEIRQB[3], &mcLIRQB);

	mcMPU.Init(&mcPins);

	mfTickHigh = fTickHigh;
	mfTickLow = fTickLow;
	mpvContext = pvContext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Meta::Kill(void)
{
	size	i;

	mcMPU.Kill();
	mcPins.Kill();

	mcLIRQB.Kill();
	for (i = 0; i < NUM_65C64_EDGE_INTERRUPTS; i++)
	{
		macEIRQB[i].Kill();
	}
	mcVPB.Kill();
	mcVDA.Kill();
	mcVPA.Kill();
	mcOPV.Kill();
	mcUpperByte.Kill();
	mcLowerByte.Kill();
	mcRWB.Kill();
	mcRESB.Kill();
	mcPHI2.Kill();
	mcData.Kill();
	mcAddress.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Meta::InputTransition(void)
{
	mcMPU.InputTransition(&mcTimeline);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Meta::CallTickHigh(void)
{
	if (mfTickHigh)
	{
		mfTickHigh(this, mpvContext);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Meta::CallTickLow(void)
{
	if (mfTickLow)
	{
		mfTickLow(this, mpvContext);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Runs until the next opcode fetch is on the bus, the processor waits (one cycle
// per call while it waits) or stops.  Returns false if the processor stopped.
//
//////////////////////////////////////////////////////////////////////////
bool C65C64Meta::TickInstruction(void)
{
	size	i;

	if (mcRESB.IsLow())
	{
		for (i = 0; i < 4; i++)
		{
			mcPHI2.Invert();
			InputTransition();
		}
		mcRESB.Set(true);
		InputTransition();
	}

	for (;;)
	{
		mcPHI2.Invert();
		InputTransition();

		if (mcPHI2.IsHigh())
		{
			CallTickHigh();
		}
		else
		{
			CallTickLow();

			if (mcMPU.IsStopped())
			{
				return false;
			}
			if (mcOPV.IsHigh() || mcMPU.IsWaiting())
			{
				return true;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Only for tests.  Runs one PHI2 cycle, high then low.  Returns true if the next
// cycle fetches an opcode.
//
//////////////////////////////////////////////////////////////////////////
bool C65C64Meta::TestTickCycle(void)
{
	size	i;

	for (i = 0; i < 2; i++)
	{
		mcPHI2.Invert();
		InputTransition();

		if (mcPHI2.IsHigh())
		{
			CallTickHigh();
		}
		else
		{
			CallTickLow();
		}
	}
	return mcOPV.IsHigh();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Meta::Print(CChars* psz)
{
	CChars	sz;
	uint64	uiA;
	uint64	uiB;
	uint64	uiX;
	uint64	uiY;
	uint32	uiS;
	uint32	uiV;
	uint32	uiW;
	uint32	uiC;

	sz.Init();
	mcMPU.GetMnemonicString(&sz);
	psz->LeftAlign(sz, ' ', 6);
	sz.Kill();

	psz->Append("(");
	psz->Append((int32)mcMPU.GetCycle() + 1);
	psz->Append(")  ");

	sz.Init();
	mcMPU.GetCycleOperationString(&sz);
	psz->LeftAlign(sz, ' ', 28);
	sz.Kill();

	uiA = mcMPU.GetA();
	uiB = mcMPU.GetB();
	uiX = mcMPU.GetX();
	uiY = mcMPU.GetY();
	uiS = mcMPU.GetS();
	uiV = mcMPU.GetV();
	uiW = mcMPU.GetW();
	uiC = mcMPU.GetProgramCounter();

	psz->Append("A.");
	psz->AppendHexHiLo(&uiA, 8);
	psz->Append("  B.");
	psz->AppendHexHiLo(&uiB, 8);
	psz->Append("  X.");
	psz->AppendHexHiLo(&uiX, 8);
	psz->Append("  Y.");
	psz->AppendHexHiLo(&uiY, 8);
	psz->Append("  S.");
	psz->AppendHexHiLo(&uiS, 4);
	psz->Append("  V.");
	psz->AppendHexHiLo(&uiV, 4);
	psz->Append("  W.");
	psz->AppendHexHiLo(&uiW, 4);
	psz->Append("  C.");
	psz->AppendHexHiLo(&uiC, 4);
	psz->Append("  P(");
	mcMPU.GetStatusString(psz);
	psz->Append(")  Address.");
	mcAddress.Print(psz, false);
	psz->Append("  Data.");
	mcData.Print(psz, false);
	psz->Append("  RWB.");
	mcRWB.Print(psz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void C65C64Meta::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.AppendNewLine();
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
C65C64* C65C64Meta::GetMPU(void) { return &mcMPU; }
CW65MetaBus32* C65C64Meta::GetAddress(void) { return &mcAddress; }
CW65MetaBus16* C65C64Meta::GetData(void) { return &mcData; }
CW65MetaTrace* C65C64Meta::GetPHI2(void) { return &mcPHI2; }
CW65MetaTrace* C65C64Meta::GetRESB(void) { return &mcRESB; }
CW65MetaTrace* C65C64Meta::GetRWB(void) { return &mcRWB; }
CW65MetaTrace* C65C64Meta::GetLowerByte(void) { return &mcLowerByte; }
CW65MetaTrace* C65C64Meta::GetUpperByte(void) { return &mcUpperByte; }
CW65MetaTrace* C65C64Meta::GetOPV(void) { return &mcOPV; }
CW65MetaTrace* C65C64Meta::GetVPA(void) { return &mcVPA; }
CW65MetaTrace* C65C64Meta::GetVDA(void) { return &mcVDA; }
CW65MetaTrace* C65C64Meta::GetVPB(void) { return &mcVPB; }
CW65MetaTrace* C65C64Meta::GetEIRQB(size uiInterrupt) { return &macEIRQB[uiInterrupt]; }
CW65MetaTrace* C65C64Meta::GetLIRQB(void) { return &mcLIRQB; }

