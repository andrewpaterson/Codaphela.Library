/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/IntegerHelper.h"
#include "IR65BackendInternal.h"


//The 65C64 back end's multiplies and divides: mul and div (one div for a udiv and urem pair), and multiplies by
//constants as shifts.


//////////////////////////////////////////////////////////////////////////
//
// mul and div count their steps in W, from 0.  W is always 0 (the calling
// convention, CPP65Generator.txt), so it is only set back afterwards.
//
//////////////////////////////////////////////////////////////////////////
void CIR65Backend::Multiply(const char* szInstruction)
{
	Line(szInstruction)->AppendNewLine();
	Line("cpy W, 0")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A x B -> Y.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::MultiplyInstruction(SIRInstruction* psInstruction)
{
	bool	bWide;
	bool	bByte;

	bWide = IsWide(psInstruction->sType);
	if (!bWide && MultiplyByShifts(psInstruction))
	{
		return true;
	}
	if (!bWide && IsInA(psInstruction->asArguments[1]) && !IsInA(psInstruction->asArguments[0]))
	{
		LoadValue(psInstruction->asArguments[0], "B", IR65E_Zero);
	}
	else
	{
		LoadOperands(psInstruction, IR65E_Zero, false);
	}
	if (bWide)
	{
		Line("wtth (A, B), 64")->AppendNewLine();
		Line("wth Y, 64")->AppendNewLine();
	}
	bByte = !bWide && psInstruction->asArguments[1].IsConstant() && (IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits) < 256);
	if (bByte)
	{
		//mul takes a step for each byte of B: a multiplier below 256 takes one.
		Line("wth B, 8")->AppendNewLine();
	}
	Multiply("mul");
	if (bByte)
	{
		Line("wth B, 32")->AppendNewLine();
	}
	msAvailable = IRNone();
	if (bWide)
	{
		Line("cpy A, Y")->AppendNewLine();
		Line("wth Y, 32")->AppendNewLine();
		Line("wtth (A, B), 32")->AppendNewLine();
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
// x * (2^a + 2^b) is ((x << (a - b)) + x) << b and x * (2^a - 2^b) is
// ((x << (a - b)) - x) << b, which beat mul (a step a byte of B, with W
// saved round it).  Returns false for other multipliers.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::MultiplyByShifts(SIRInstruction* psInstruction)
{
	uint32		uiMultiplier;
	uint32		uiLow;
	uint32		uiRest;
	int			iLow;
	int			iHigh;
	const char*	szCombine;

	if (!psInstruction->asArguments[1].IsConstant())
	{
		return false;
	}
	uiMultiplier = (uint32)IRTruncate(psInstruction->sType, mpcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits);
	if (uiMultiplier == 0)
	{
		return false;
	}
	uiLow = uiMultiplier & (0 - uiMultiplier);
	uiRest = uiMultiplier - uiLow;
	szCombine = "add A, B";
	if ((uiRest == 0) || ((uiRest & (uiRest - 1)) != 0))
	{
		//Not two bits: maybe a run of them, 2^a - 2^b.
		uiRest = uiMultiplier + uiLow;
		if ((uiRest == 0) || ((uiRest & (uiRest - 1)) != 0))
		{
			return false;
		}
		szCombine = "sub A, B";
	}
	iLow = (int)FindFirstSetBit(uiLow);
	iHigh = (int)FindFirstSetBit(uiRest);
	if ((iHigh - iLow > 8) || (iLow > 8))
	{
		return false;
	}

	LoadValue(psInstruction->asArguments[0], "A", IR65E_Zero);
	Line("cpy B, A")->AppendNewLine();
	Line("shl A, ")->Append(iHigh - iLow)->AppendNewLine();
	Line(szCombine)->AppendNewLine();
	if (iLow > 0)
	{
		Line("shl A, ")->Append(iLow)->AppendNewLine();
	}
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult, "A");
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Unsigned: div (Y / A -> Y, the remainder in B, which starts at 0); a
// psNext that is its DividePartner gets its result from the same div.
// Signed: R_DivideSigned (A / B -> A, the remainder in B).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::Divide(SIRInstruction* psInstruction, SIRInstruction* psNext)
{
	bool			bWide;
	bool			bSigned;
	bool			bRemainder;
	SIRReference	sLeft;
	SIRReference	sRight;

	bWide = IsWide(psInstruction->sType);
	bSigned = (psInstruction->uiOpcode == IRO_SDiv) || (psInstruction->uiOpcode == IRO_SRem);
	bRemainder = (psInstruction->uiOpcode == IRO_SRem) || (psInstruction->uiOpcode == IRO_URem);
	sLeft = psInstruction->asArguments[0];
	sRight = psInstruction->asArguments[1];

	if (bSigned || bWide)
	{
		LoadOperands(psInstruction, bSigned ? IR65E_Sign : IR65E_Zero, false);
		if (bWide)
		{
			Line("wtth (A, B), 64")->AppendNewLine();
		}
		if (bSigned)
		{
			Line(bWide ? "jsr R_DivideSigned64" : "jsr R_DivideSigned")->AppendNewLine();
			UseRuntime(bWide ? "R_DivideSigned64" : "R_DivideSigned");
		}
		else
		{
			Line("wth Y, 64")->AppendNewLine();
			Line("cpy Y, A")->AppendNewLine();
			Line("cpy A, B")->AppendNewLine();
			Line("cpy B, 0")->AppendNewLine();
			Multiply("div");
			Line("cpy A, Y")->AppendNewLine();
			Line("wth Y, 32")->AppendNewLine();
		}
		if (bRemainder)
		{
			Line("cpy A, B")->AppendNewLine();
		}
		if (bWide)
		{
			Line("wtth (A, B), 32")->AppendNewLine();
		}
		msAvailable = IRNone();
		StoreResult(psInstruction->sResult);
		return true;
	}

	//The dividend to Y and the divisor to A, in an order that loses neither.
	if (IsInA(sRight) && !IsInA(sLeft))
	{
		LoadValue(sLeft, "Y", IR65E_Zero);
	}
	else if (InRegister(sRight) && (PlaceOf(sRight)->iOffset == IR65R_Y) && !sLeft.Equals(sRight))
	{
		if (IsInA(sLeft))
		{
			Line("cpy B, A")->AppendNewLine();
			Line("cpy A, Y")->AppendNewLine();
			Line("cpy Y, B")->AppendNewLine();
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
	Line("cpy B, 0")->AppendNewLine();
	Multiply("div");
	msAvailable = IRNone();
	if (psNext && DividePartner(psInstruction, psNext) && !PlaceOf(psInstruction->sResult)->bInA)
	{
		//The result from Y goes first, as the other may be kept in Y; one kept in A goes last.
		if (!bRemainder)
		{
			StoreResult(psInstruction->sResult, "Y");
			StoreResult(psNext->sResult, "B");
			return true;
		}
		if (!PlaceOf(psNext->sResult)->bInA)
		{
			StoreResult(psNext->sResult, "Y");
			StoreResult(psInstruction->sResult, "B");
			return true;
		}
	}
	StoreResult(psInstruction->sResult, bRemainder ? "B" : "Y");
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The partner (DividePartner) just after the udiv or urem uiIndex that one
// div gives the results of both, the one kept in Y not kept in A; NULL for
// none.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIR65Backend::PairedDivide(CIRBlock* pcBlock, size uiIndex)
{
	SIRInstruction*	psInstruction;
	SIRInstruction*	psPartner;

	psInstruction = pcBlock->masInstructions.Get(uiIndex);
	psPartner = pcBlock->masInstructions.SafeGet(uiIndex + 1);
	if ((psPartner == NULL) || !DividePartner(psInstruction, psPartner) || PlaceOf(psInstruction->sResult)->bInA)
	{
		return NULL;
	}
	if ((psInstruction->uiOpcode == IRO_URem) && PlaceOf(psPartner->sResult)->bInA)
	{
		return NULL;
	}
	return psPartner;
}
