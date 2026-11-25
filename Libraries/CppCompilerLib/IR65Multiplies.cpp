/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/IntegerHelper.h"
#include "IR65BackendInternal.h"


//The 65C64 back end's multiplies and divides: mul and div (one div for a udiv and urem pair), and multiplies by
//constants as shifts.


//////////////////////////////////////////////////////////////////////////
//
// mul and div count their steps in W, from 0, and leave it at twice their
// width in bytes.  W is a register like any other (a call may change it,
// and it may be a value's home), so it is set to 0 before them; CIR65Peephole
// takes that out where W is 0 already.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Selector::Multiply(uint8 uiMnemonic)
{
	mpcBackend->Line(IR65M_Cpy, "W, 0");
	mpcBackend->Line(uiMnemonic);
}


//////////////////////////////////////////////////////////////////////////
//
// A x B -> Y.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::MultiplyInstruction(SIRInstruction* psInstruction)
{
	bool	bWide;
	bool	bByte;

	bWide = mpcBackend->IsWide(psInstruction->sType);
	if (!bWide && mpcBackend->mcPlaces.IsInA(psInstruction->asArguments[1]) && !mpcBackend->mcPlaces.IsInA(psInstruction->asArguments[0]))
	{
		LoadValue(psInstruction->asArguments[0], "B", IR65E_Zero);
	}
	else
	{
		LoadOperands(psInstruction, IR65E_Zero, false);
	}
	if (bWide)
	{
		mpcBackend->Line(IR65M_Wtth, "(A, B), 64");
		mpcBackend->Line(IR65M_Wth, "Y, 64");
	}
	bByte = !bWide && psInstruction->asArguments[1].IsConstant() && (IRTruncate(psInstruction->sType, mpcBackend->mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) < 256);
	if (bByte)
	{
		//mul takes a step for each byte of B: a multiplier below 256 takes one.
		mpcBackend->Line(IR65M_Wth, "B, 8");
	}
	Multiply(IR65M_Mul);
	if (bByte)
	{
		mpcBackend->Line(IR65M_Wth, "B, 32");
	}
	mpcBackend->msAvailable = IRNone();
	if (bWide)
	{
		mpcBackend->Line(IR65M_Cpy, "A, Y");
		mpcBackend->Line(IR65M_Wth, "Y, 32");
		mpcBackend->Line(IR65M_Wtth, "(A, B), 32");
		StoreResult(psInstruction->sResult);
	}
	else
	{
		StoreResult(psInstruction->sResult, "Y");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Roughly the cycles of a mul by a constant (MultiplyInstruction, for
// ExpandMultiplies): the constant into B (narrowed to 8 bits round the mul
// when it fits), a step a byte of B, and W cleared before.  A 64 bit mul
// stays one: its shifts and adds are no cheaper.
//
//////////////////////////////////////////////////////////////////////////
int CIR65Backend::MultiplyCost(SIRType sType, uint64 ulliMultiplier)
{
	if (ulliMultiplier < 256)
	{
		return 2 + 2 + 1 + 1;
	}
	return 2 + 4 + 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Roughly the cycles of ExpandMultiplies' shifts and adds: a copy keeping
// x, a neg for a top digit of -1, a shift (a cycle when it is 1 to 8 bits,
// embedded, else two) and an add or sub for each digit after the top, and
// the last shift.  Not for 64 bits (MultiplyCost).
//
//////////////////////////////////////////////////////////////////////////
int CIR65Backend::ShiftAddCost(SIRType sType, int* aiPlaces, int* aiSigns, int iDigits)
{
	int	iCost;
	int	i;

	if (IsWide(sType))
	{
		return MAX_INT;
	}
	iCost = 1 + ((aiSigns[iDigits - 1] < 0) ? 1 : 0);
	for (i = iDigits - 1; i > 0; i--)
	{
		iCost += ((aiPlaces[i] - aiPlaces[i - 1] <= 8) ? 1 : 2) + 1;
	}
	if (aiPlaces[0] > 0)
	{
		iCost += (aiPlaces[0] <= 8) ? 1 : 2;
	}
	return iCost;
}


//////////////////////////////////////////////////////////////////////////
//
// Unsigned: div (Y / A -> Y, the remainder in B, which starts at 0); a
// psNext that is its DividePartner gets its result from the same div.
// Signed: R_DivideSigned (A / B -> A, the remainder in B).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::Divide(SIRInstruction* psInstruction, SIRInstruction* psNext)
{
	bool			bWide;
	bool			bSigned;
	bool			bRemainder;
	SIRReference	sLeft;
	SIRReference	sRight;

	bWide = mpcBackend->IsWide(psInstruction->sType);
	bSigned = (psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem);
	bRemainder = (psInstruction->uiOpcode == IRO_SRem) || (psInstruction->uiOpcode == IRO_URem);
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];

	if (bSigned || bWide)
	{
		LoadOperands(psInstruction, bSigned ? IR65E_Sign : IR65E_Zero, false);
		if (bWide)
		{
			mpcBackend->Line(IR65M_Wtth, "(A, B), 64");
		}
		if (bSigned)
		{
			mpcBackend->Line(IR65M_Jsr, bWide ? "R_DivideSigned64" : "R_DivideSigned");
			mpcBackend->UseRuntime(bWide ? "R_DivideSigned64" : "R_DivideSigned");
		}
		else
		{
			mpcBackend->Line(IR65M_Wth, "Y, 64");
			mpcBackend->Line(IR65M_Cpy, "Y, A");
			mpcBackend->Line(IR65M_Cpy, "A, B");
			mpcBackend->Line(IR65M_Cpy, "B, 0");
			Multiply(IR65M_Div);
			mpcBackend->Line(IR65M_Cpy, "A, Y");
			mpcBackend->Line(IR65M_Wth, "Y, 32");
		}
		if (bRemainder)
		{
			mpcBackend->Line(IR65M_Cpy, "A, B");
		}
		if (bWide)
		{
			mpcBackend->Line(IR65M_Wtth, "(A, B), 32");
		}
		mpcBackend->msAvailable = IRNone();
		StoreResult(psInstruction->sResult);
		return true;
	}

	//The dividend to Y and the divisor to A, in an order that loses neither.
	if (mpcBackend->mcPlaces.IsInA(sRight) && !mpcBackend->mcPlaces.IsInA(sLeft))
	{
		LoadValue(sLeft, "Y", IR65E_Zero);
	}
	else if (mpcBackend->mcPlaces.InRegister(sRight) && (mpcBackend->mcPlaces.PlaceOf(sRight)->iOffset == IR65R_Y) && !sLeft.Equals(sRight))
	{
		if (mpcBackend->mcPlaces.IsInA(sLeft))
		{
			mpcBackend->Line(IR65M_Cpy, "B, A");
			mpcBackend->Line(IR65M_Cpy, "A, Y");
			mpcBackend->Line(IR65M_Cpy, "Y, B");
		}
		else
		{
			LoadValue(sRight, "A", IR65E_Zero);
			LoadValue(sLeft, "Y", IR65E_Zero);
		}
	}
	else
	{
		LoadValue(sLeft, "Y", IR65E_Zero);
		LoadValue(sRight, "A", IR65E_Zero);
	}
	mpcBackend->Line(IR65M_Cpy, "B, 0");
	Multiply(IR65M_Div);
	mpcBackend->msAvailable = IRNone();
	if (psNext && PairsWithDivide(psInstruction, psNext))
	{
		//The result from Y goes first, as the other may be kept in Y.
		StoreResult(bRemainder ? psNext->sResult : psInstruction->sResult, "Y");
		StoreResult(bRemainder ? psInstruction->sResult : psNext->sResult, "B");
		return true;
	}
	StoreResult(psInstruction->sResult, bRemainder ? "B" : "Y");
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if psPartner is the unsigned udiv (not 64 bits) of psFirst's urem
// (or the urem of its udiv): the same operands, so the one div does both.
// The optimiser puts them next to each other (SetDivideRemainders).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::DividePartner(SIRInstruction* psFirst, SIRInstruction* psPartner)
{
	if (((psFirst->uiOpcode != IRO_UDiv) && (psFirst->uiOpcode != IRO_URem)) || mpcBackend->IsWide(psFirst->sType))
	{
		return false;
	}
	return (psPartner->uiOpcode == ((psFirst->uiOpcode == IRO_UDiv) ? IRO_URem : IRO_UDiv)) && psPartner->sType.Equals(psFirst->sType) &&
		psPartner->asArguments[0].Equals(psFirst->asArguments[0]) && psPartner->asArguments[1].Equals(psFirst->asArguments[1]) && psPartner->sResult.IsTemporary();
}


//////////////////////////////////////////////////////////////////////////
//
// The partner (DividePartner) just after the udiv or urem uiIndex that one
// div gives the results of both, the one kept in Y not kept in A; NULL for
// none.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR65Selector::PairedDivide(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psPartner;

	psPartner = pcBlock->masInstructions.SafeGet(uiIndex + 1);
	return (psPartner && PairsWithDivide(pcBlock->masInstructions.Get(uiIndex), psPartner)) ? psPartner : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the udiv or urem psInstruction's div gives psPartner's result too
// (DividePartner): Divide writes both, the one from Y first, so it isn't
// kept in A (a udiv's quotient, a urem's partner's).  Divide and CanFuse
// (through PairedDivide) both ask this, so they agree.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Selector::PairsWithDivide(SIRInstruction* psInstruction, SIRInstruction* psPartner)
{
	if (!DividePartner(psInstruction, psPartner) || mpcBackend->mcPlaces.PlaceOf(psInstruction->sResult)->bInA)
	{
		return false;
	}
	return (psInstruction->uiOpcode != IRO_URem) || !mpcBackend->mcPlaces.PlaceOf(psPartner->sResult)->bInA;
}
