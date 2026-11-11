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
#include "IR65BackendInternal.h"


//The 65C64 back end's floating point instructions: arithmetic, conversions and compares, as calls of the soft float
//runtime.


//////////////////////////////////////////////////////////////////////////
//
// Floating point is the soft float runtime's: the operands' bits are pushed
// (the right first, as arguments are) and the result is in A.  fneg flips the
// sign bit in line.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FloatOperation(SIRInstruction* psInstruction)
{
	bool		bDouble;
	CChars		szName;

	bDouble = psInstruction->sType.Is(IRTK_F64);
	if (psInstruction->uiOpcode == IRO_FNeg)
	{
		LoadValue(psInstruction->asArguments[0], "A", IR65E_Zero);
		if (bDouble)
		{
			Line("wth A, 64")->AppendNewLine();
			Line("xor A, -9223372036854775808")->AppendNewLine();
			Line("wth A, 32")->AppendNewLine();
		}
		else
		{
			Line("xor A, -2147483648")->AppendNewLine();
		}
		msAvailable = IRNone();
		StoreResult(psInstruction->sResult);
		return true;
	}

	Push(psInstruction->asArguments[1], psInstruction->sType);
	Push(psInstruction->asArguments[0], psInstruction->sType);
	szName.Init();
	AppendSoftFloatName(&szName, psInstruction, psInstruction->sType);
	CallRuntime(szName.Text(), 2 * ValueBytes(psInstruction->sType));
	szName.Kill();
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Conversions to, from and between floats, by the soft float runtime.
// Integers narrower than 32 bits convert as ints.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FloatConversion(SIRInstruction* psInstruction)
{
	SIRType	sFrom;
	bool	bSigned;
	CChars	szName;

	sFrom = mpcFunction->TypeOf(psInstruction->asArguments[0]);
	szName.Init();
	AppendSoftFloatName(&szName, psInstruction, sFrom);
	switch (psInstruction->uiOpcode)
	{
	case IRO_FExt:
	case IRO_FTrunc:
		Push(psInstruction->asArguments[0], sFrom);
		CallRuntime(szName.Text(), ValueBytes(sFrom));
		break;

	case IRO_SIToF:
	case IRO_UIToF:
		bSigned = psInstruction->uiOpcode == IRO_SIToF;
		if (IsWide(sFrom))
		{
			Push(psInstruction->asArguments[0], sFrom);
		}
		else
		{
			//Narrow integers are pushed as ints, extended.
			LoadValue(psInstruction->asArguments[0], "A", bSigned ? IR65E_Sign : IR65E_Zero);
			Line("psh A")->AppendNewLine();
		}
		CallRuntime(szName.Text(), IsWide(sFrom) ? 8 : 4);
		break;

	default:
		Push(psInstruction->asArguments[0], sFrom);
		CallRuntime(szName.Text(), ValueBytes(sFrom));
		break;
	}
	szName.Kill();
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime's compare gives -1, 0 or 1, or 2 when either is a NaN (so
// every comparison but fne and funordered is false then): the values that
// make each true are FloatCompareValues'.
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FloatCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
{
	const char*	szTrue;
	const char*	szFalse;
	CChars		szName;
	int			aiValues[2];
	int			iValues;
	bool		bNot;

	Push(psInstruction->asArguments[1], psInstruction->sType);
	Push(psInstruction->asArguments[0], psInstruction->sType);
	szName.Init();
	AppendSoftFloatName(&szName, psInstruction, psInstruction->sType);
	CallRuntime(szName.Text(), 2 * ValueBytes(psInstruction->sType));
	szName.Kill();
	msAvailable = IRNone();

	//One value is cmp A, v; two (v, v + 1) are A - v below 2, unsigned.
	iValues = FloatCompareValues(psInstruction->uiOpcode, aiValues, &bNot);
	if (iValues == 1)
	{
		Line("cmp A, ")->Append(aiValues[0])->AppendNewLine();
		szTrue = bNot ? "brac z, " : "bras z, ";
		szFalse = bNot ? "bras z, " : "brac z, ";
	}
	else
	{
		if (aiValues[0] != 0)
		{
			Line("add A, ")->Append(-aiValues[0])->AppendNewLine();
		}
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac c, ";
		szFalse = "bras c, ";
	}
	Branch(bJumpIf ? szTrue : szFalse, iLabel);
	return true;
}
