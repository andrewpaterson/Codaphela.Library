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
#include "W65DataOperation.h"
#include "W65C02InstructionFactory.h"
#include "W65C02.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C02::Init(CW65C02Pins* pcPins)
{
	CW65CCore::Init(pcPins);
	mpcW65C02Pins = pcPins;

	//P reads bit 5 as 1, B (bit 4) only exists on the stack.
	muiEmulationStatusBits = STATUS_ACCUMULATOR_WIDTH;
	mbBitBranch = false;
	mbPreviousSOB = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65C02Pins* CW65C02::GetPins(void)
{
	return mpcW65C02Pins;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CW65C02::GetType(void)
{
	return "W65C02 Microprocessor";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02::GetInstruction(uint16 uiOpcode)
{
	return CW65C02InstructionFactory::GetInstance()->GetInstruction(uiOpcode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02::GetResetOpcode(void)
{
	return CW65C02InstructionFactory::GetInstance()->GetReset();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02::GetIrqOpcode(void)
{
	return CW65C02InstructionFactory::GetInstance()->GetIRQ();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02::GetNmiOpcode(void)
{
	return CW65C02InstructionFactory::GetInstance()->GetNMI();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02::GetAbortOpcode(void)
{
	//The W65C02S has no ABORTB.
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CW65Instruction* CW65C02::GetFetchNextOpcode(void)
{
	return CW65C02InstructionFactory::GetInstance()->GetFetchNext();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C02::WriteSignalsPhi2Falling(CTimeline* pcTimeline, CW65DataOperation* pcDataOperation, CW65BusCycle* pcBusCycle)
{
	bool	bSOB;

	mpcW65C02Pins->WriteSYNC(pcTimeline, pcDataOperation->IsFetchOpcode());

	//A falling edge on SOB sets the overflow flag.
	bSOB = !mpcW65C02Pins->ReadSOB(pcTimeline).IsLow();
	if (mbPreviousSOB && !bSOB)
	{
		SetOverflowFlag(true);
	}
	mbPreviousSOB = bSOB;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 CW65C02::GetOpcodeBit(void)
{
	//RMBx, SMBx, BBRx and BBSx encode the bit in bits 4 to 6 of the opcode.
	return (muiOpcodeIndex >> 4) & 0x7;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C02::RMB(void)
{
	SetDataLow(GetDataLow() & ~(1 << GetOpcodeBit()));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C02::SMB(void)
{
	SetDataLow(GetDataLow() | (1 << GetOpcodeBit()));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C02::BitBranchTest(void)
{
	//BBRx (opcode bit 7 clear) branches if the bit is clear, BBSx if it is set.
	bool	bBitSet;

	bBitSet = (GetDataLow() & (1 << GetOpcodeBit())) != 0;
	mbBitBranch = bBitSet == ((muiOpcodeIndex & 0x80) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C02::BitBranch(void)
{
	Branch(mbBitBranch);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CW65C02::Execute8BitBCDSBC(void)
{
	//The W65C02S decimal subtract (Bruce Clark, "Decimal Mode", appendix A).  Carry and
	//overflow come from the binary subtraction.
	int32	iAccumulator;
	int32	iOperand;
	int32	iBorrow;
	int32	iLow;
	int32	iResult;
	uint8	uiBinary;

	iAccumulator = GetA();
	iOperand = GetMemoryData() & 0xFF;
	iBorrow = IsCarrySet() ? 0 : 1;

	uiBinary = (uint8)(iAccumulator - iOperand - iBorrow);
	SetOverflowFlag(((iAccumulator ^ iOperand) & (iAccumulator ^ uiBinary) & 0x80) != 0);

	iLow = (iAccumulator & 0x0F) - (iOperand & 0x0F) - iBorrow;
	iResult = iAccumulator - iOperand - iBorrow;
	SetCarryFlag(iResult >= 0);
	if (iResult < 0)
	{
		iResult -= 0x60;
	}
	if (iLow < 0)
	{
		iResult -= 0x06;
	}
	SetA((uint16)(iResult & 0xFF));
}

