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
	const char*	szOperation;
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

	switch (psInstruction->uiOpcode)
	{
	case IRO_FAdd:	szOperation = "Add";		break;
	case IRO_FSub:	szOperation = "Subtract";	break;
	case IRO_FMul:	szOperation = "Multiply";	break;
	default:		szOperation = "Divide";		break;
	}
	Push(psInstruction->asArguments[1], psInstruction->sType);
	Push(psInstruction->asArguments[0], psInstruction->sType);
	szName.Init(bDouble ? "SoftDouble" : "SoftFloat");
	szName.Append(szOperation);
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
	SIRType	sTo;
	bool	bSigned;
	CChars	szName;

	sFrom = mpcFunction->TypeOf(psInstruction->asArguments[0]);
	sTo = psInstruction->sType;
	switch (psInstruction->uiOpcode)
	{
	case IRO_FExt:
	case IRO_FTrunc:
		Push(psInstruction->asArguments[0], sFrom);
		CallRuntime(sTo.Is(IRTK_F64) ? "SoftFloatToDouble" : "SoftDoubleToFloat", ValueBytes(sFrom));
		break;

	case IRO_SIToF:
	case IRO_UIToF:
		bSigned = psInstruction->uiOpcode == IRO_SIToF;
		szName.Init(sTo.Is(IRTK_F64) ? "SoftDoubleFrom" : "SoftFloatFrom");
		if (IsWide(sFrom))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			szName.Append((bSigned || (sFrom.IntegerBits() < 32)) ? "Int" : "Unsigned");
		}
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
		szName.Kill();
		break;

	default:
		bSigned = psInstruction->uiOpcode == IRO_FToSI;
		szName.Init(sFrom.Is(IRTK_F64) ? "SoftDoubleTo" : "SoftFloatTo");
		if (IsWide(sTo))
		{
			szName.Append(bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			szName.Append((bSigned || (sTo.IntegerBits() < 32)) ? "Int" : "Unsigned");
		}
		Push(psInstruction->asArguments[0], sFrom);
		CallRuntime(szName.Text(), ValueBytes(sFrom));
		szName.Kill();
		break;
	}
	msAvailable = IRNone();
	StoreResult(psInstruction->sResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The runtime's compare gives -1, 0 or 1, or 2 when either is a NaN (so
// every comparison but fne and funordered is false then).
//
//////////////////////////////////////////////////////////////////////////
bool CIR65Backend::FloatCompare(SIRInstruction* psInstruction, bool bJumpIf, int32 iLabel)
{
	const char*	szTrue;
	const char*	szFalse;

	Push(psInstruction->asArguments[1], psInstruction->sType);
	Push(psInstruction->asArguments[0], psInstruction->sType);
	CallRuntime(psInstruction->sType.Is(IRTK_F64) ? "SoftDoubleCompare" : "SoftFloatCompare", 2 * ValueBytes(psInstruction->sType));
	msAvailable = IRNone();

	szTrue = "bras z, ";
	szFalse = "brac z, ";
	switch (psInstruction->uiOpcode)
	{
	case IRO_FEq:
		Line("cmp A, 0")->AppendNewLine();
		break;
	case IRO_FNe:
		Line("cmp A, 0")->AppendNewLine();
		szTrue = "brac z, ";
		szFalse = "bras z, ";
		break;
	case IRO_FLt:
		Line("cmp A, -1")->AppendNewLine();
		break;
	case IRO_FGt:
		Line("cmp A, 1")->AppendNewLine();
		break;
	case IRO_FLe:
		//-1 or 0: A + 1 is below 2, unsigned.
		Line("add A, 1")->AppendNewLine();
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac c, ";
		szFalse = "bras c, ";
		break;
	case IRO_FGe:
		//0 or 1.
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac c, ";
		szFalse = "bras c, ";
		break;
	case IRO_FOrdered:
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac z, ";
		szFalse = "bras z, ";
		break;
	default:
		Line("cmp A, 2")->AppendNewLine();
		break;
	}
	Branch(bJumpIf ? szTrue : szFalse, iLabel);
	return true;
}
