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
#include <string.h>
#include "CTFloat.h"
#include "CTDouble.h"
#include "CTLongDouble.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"


//////////////////////////////////////////////////////////////////////////
//
// Floating point is worked out by Libc65's soft float runtime (softfloat.cpp): the
// generator calls its functions with the values' bits pushed (the right operand
// first, as arguments are) and gets the result in A.  Only negation and tests
// against zero are written in line.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
// The type of a floating point operation: long double if either operand is, then
// double, otherwise float (an integer operand takes the other's type).  [expr] 10
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::FloatArithmetic(SCPP65Type* psLeft, SCPP65Type* psRight)
{
	if ((psLeft && psLeft->bLongDouble) || (psRight && psRight->bLongDouble))
	{
		return mcTypes.LongDouble();
	}
	if ((psLeft && psLeft->IsFloat() && (psLeft->iBytes == 8)) || (psRight && psRight->IsFloat() && (psRight->iBytes == 8)))
	{
		return mcTypes.Float(8);
	}
	return mcTypes.Float(4);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a soft float runtime function (the label is _ and its name) and removes
// iBytes from the stack.  The label is an external of the unit.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::CallRuntime(const char* szName, int iBytes)
{
	CChars	szLabel;

	szLabel.Init("_");
	szLabel.Append(szName);
	maszRuntime.AddIfUnique(szLabel);
	Line("jsr ")->Append(szLabel)->AppendNewLine();
	szLabel.Kill();
	ChangeX();
	if (iBytes > 0)
	{
		Line("add S, ")->Append(iBytes)->AppendNewLine();
		miDepth -= iBytes;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Copies B to A, all 8 bytes of an 8 byte value.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::MoveBToA(SCPP65Type* psType)
{
	if (psType->Is64())
	{
		Line("wtth (A, B), 8")->AppendNewLine();
		Line("cpy A, B")->AppendNewLine();
		Line("wtth (A, B), 4")->AppendNewLine();
	}
	else
	{
		Line("cpy A, B")->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the operands of a runtime call with two psType arguments: the right
// (pushed first, it is the second argument) then the left, each converted to
// psType.  The left is in A and the right is psRight.  Returns the bytes to remove
// after the call, or -1 after an error.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::PushFloatOperands(SCPP65Type* psLeft, SCPP65Right* psRight, SCPP65Type* psType, CCPPSyntax* pcNode)
{
	int		iSaved;

	if (psLeft->IsPointer() || psRight->psType->IsPointer() || !psLeft->IsScalar() || !psRight->psType->IsScalar())
	{
		Error(pcNode, "Only numbers can be used with floating point");
		return -1;
	}

	RightInB(psRight);
	if (psLeft->Equals(psType) && psRight->psType->Equals(psType))
	{
		PushWide("B", psType->Is64());
		PushValue(psType);
		return 2 * psType->iBytes;
	}

	//The left waits on the stack while the right is converted.
	PushWide("A", psLeft->Is64());
	iSaved = psLeft->Is64() ? 8 : 4;
	MoveBToA(psRight->psType);
	if (!Convert(psRight->psType, psType, pcNode))
	{
		return -1;
	}
	PushValue(psType);

	if (psLeft->Is64())
	{
		Line("wth A, 8")->AppendNewLine();
	}
	Line("cpy A, [")->Append(1 + psType->iBytes)->Append(" + S]")->AppendNewLine();
	if (psLeft->Is64())
	{
		Line("wth A, 4")->AppendNewLine();
	}
	if (!Convert(psLeft, psType, pcNode))
	{
		return -1;
	}
	PushValue(psType);
	return 2 * psType->iBytes + iSaved;
}


//////////////////////////////////////////////////////////////////////////
//
// A = A op right for + - * and / when either operand is floating point.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::OperateFloat(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	const char*		szOperation;
	CChars			szName;
	int				iBytes;

	switch (eOperator)
	{
	case CT_Add:		szOperation = "Add";		break;
	case CT_Subtract:	szOperation = "Subtract";	break;
	case CT_Multiply:	szOperation = "Multiply";	break;
	case CT_Divide:		szOperation = "Divide";		break;
	default:
		Error(pcNode, "The operands must be integers");
		return NULL;
	}

	psType = FloatArithmetic(psLeft, psRight->psType);
	iBytes = PushFloatOperands(psLeft, psRight, psType, pcNode);
	if (iBytes < 0)
	{
		return NULL;
	}
	szName.Init((psType->iBytes == 8) ? "SoftDouble" : "SoftFloat");
	szName.Append(szOperation);
	CallRuntime(szName.Text(), iBytes);
	szName.Kill();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Compares the left (in A) with the right when either is floating point, and
// branches to iLabel when the comparison is bJumpIf.  The runtime gives -1, 0 or 1,
// or 2 when either is a NaN (so every comparison but != is false).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CompareFloat(ECppTokens eOperator, SCPP65Type* psLeft, SCPP65Right* psRight, bool bJumpIf, int iLabel, CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	int				iBytes;
	const char*		szTrue;
	const char*		szFalse;

	psType = FloatArithmetic(psLeft, psRight->psType);
	iBytes = PushFloatOperands(psLeft, psRight, psType, pcNode);
	if (iBytes < 0)
	{
		return false;
	}
	CallRuntime((psType->iBytes == 8) ? "SoftDoubleCompare" : "SoftFloatCompare", iBytes);

	//szTrue branches when the comparison is true, szFalse when it is false.
	szTrue = "bras z, ";
	szFalse = "brac z, ";
	switch (eOperator)
	{
	case CT_EqualTo:
		Line("cmp A, 0")->AppendNewLine();
		break;
	case CT_NotEqualTo:
		Line("cmp A, 0")->AppendNewLine();
		szTrue = "brac z, ";
		szFalse = "bras z, ";
		break;
	case CT_LessThan:
		Line("cmp A, -1")->AppendNewLine();
		break;
	case CT_GreaterThan:
		Line("cmp A, 1")->AppendNewLine();
		break;
	case CT_LessThanEqualTo:
		//-1 or 0: A + 1 is below 2, unsigned.
		Line("add A, 1")->AppendNewLine();
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac c, ";
		szFalse = "bras c, ";
		break;
	default:
		//0 or 1.
		Line("cmp A, 2")->AppendNewLine();
		szTrue = "brac c, ";
		szFalse = "bras c, ";
		break;
	}
	Line(bJumpIf ? szTrue : szFalse);
	AppendLabel(mpszOut, iLabel);
	mpszOut->AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Converts A from psFrom to psTo when either is floating point (but psTo is not
// bool).  Integers narrower than int are converted from int, and to int then
// truncated.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ConvertFloat(SCPP65Type* psFrom, SCPP65Type* psTo, CCPPSyntax* pcNode)
{
	CChars	szName;

	if (psFrom->IsPointer() || psTo->IsPointer())
	{
		return Error(pcNode, "A pointer cannot be converted to or from floating point");
	}

	if (psFrom->IsFloat() && psTo->IsFloat())
	{
		if (psFrom->iBytes == psTo->iBytes)
		{
			return true;
		}
		PushValue(psFrom);
		CallRuntime((psTo->iBytes == 8) ? "SoftFloatToDouble" : "SoftDoubleToFloat", psFrom->iBytes);
		return true;
	}

	if (psTo->IsFloat())
	{
		szName.Init((psTo->iBytes == 8) ? "SoftDoubleFrom" : "SoftFloatFrom");
		if (psFrom->iBytes == 8)
		{
			szName.Append(psFrom->bSigned ? "LongLong" : "UnsignedLongLong");
		}
		else
		{
			//Narrow integers are already extended to 32 bits, so they fit an int.
			szName.Append((psFrom->bSigned || (psFrom->iBytes < 4)) ? "Int" : "Unsigned");
		}
		PushWide("A", psFrom->Is64());
		CallRuntime(szName.Text(), psFrom->Is64() ? 8 : 4);
		szName.Kill();
		return true;
	}

	szName.Init((psFrom->iBytes == 8) ? "SoftDoubleTo" : "SoftFloatTo");
	if (psTo->iBytes == 8)
	{
		szName.Append(psTo->bSigned ? "LongLong" : "UnsignedLongLong");
	}
	else
	{
		szName.Append((psTo->bSigned || (psTo->iBytes < 4)) ? "Int" : "Unsigned");
	}
	PushValue(psFrom);
	CallRuntime(szName.Text(), psFrom->iBytes);
	szName.Kill();
	if (psTo->iBytes < 4)
	{
		Truncate("A", psTo->iBytes, psTo->bSigned);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// ++ and -- of a floating point place (loaded into A): plus or minus one.  An
// address in X is kept on the stack across the runtime call.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::IncrementFloat(CCPPSyntaxUnaryExpression* pcUnary, SCPP65Place* psPlace, const char* szAddress)
{
	SCPP65Right		sOne;
	SCPP65Type*		psType;
	bool			bWide;

	psType = psPlace->psType;
	bWide = psType->Is64();
	if (szAddress)
	{
		Push("X");
	}
	if (pcUnary->mbPostfix)
	{
		PushWide("A", bWide);
	}

	sOne.eKind = CPP65RK_Constant;
	sOne.psType = psType;
	sOne.lliValue = FloatBits(psType, 1.0);
	if (!OperateFloat((pcUnary->meOperator == CT_Increment) ? CT_Add : CT_Subtract, psType, &sOne, pcUnary))
	{
		return NULL;
	}

	if (szAddress)
	{
		Line("cpy X, [")->Append(1 + (pcUnary->mbPostfix ? (bWide ? 8 : 4) : 0))->Append(" + S]")->AppendNewLine();
		ChangeX();
	}
	Store(psPlace, "A", szAddress);
	if (pcUnary->mbPostfix)
	{
		PullWide("A", bWide);
	}
	if (szAddress)
	{
		Line("add S, 4")->AppendNewLine();
		miDepth -= 4;
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The bits of a floating point value of psType (as a 32 bit value for a float).
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::FloatBits(SCPP65Type* psType, double dValue)
{
	float	fValue;
	uint32	uiBits;
	uint64	ulliBits;

	if (psType->iBytes == 4)
	{
		fValue = (float)dValue;
		memcpy(&uiBits, &fValue, sizeof(uint32));
		return (int64)(int32)uiBits;
	}
	memcpy(&ulliBits, &dValue, sizeof(uint64));
	return (int64)ulliBits;
}


//////////////////////////////////////////////////////////////////////////
//
// A floating point constant expression (literals, integer constants, + - * /, unary
// - and +, and casts), worked out as the target would: each float result is rounded
// to float.  Rounding a double result of float operands to float rounds correctly,
// as a double has more than twice a float's precision.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::FloatConstant(CCPPSyntax* pcExpression, double* pdValue)
{
	SCPP65Type*		psType;

	return FloatValue(pcExpression, pdValue, &psType) && psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a constant expression; *ppsType is its floating point type, or NULL
// for an integer.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::FloatValue(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType)
{
	CCPPToken*						pcToken;
	CCPPSyntaxUnaryExpression*		pcUnary;
	CCPPSyntaxBinaryExpression*		pcBinary;
	CCPPSyntaxCastExpression*		pcCast;
	SCPP65Type*						psLeft;
	SCPP65Type*						psRight;
	SCPP65Type*						psType;
	double							dLeft;
	double							dRight;
	int64							lliValue;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression)
	{
		return false;
	}
	*ppsType = NULL;

	switch (pcExpression->GetKind())
	{
	case CPPS_LiteralExpression:
		pcToken = mpcTokeniser->Get(((CCPPSyntaxLiteralExpression*)pcExpression)->miLiteral);
		if (pcToken->IsNumber())
		{
			switch (((CCTNumber*)pcToken)->meType)
			{
			case CTNT_Float:
				*pdValue = ((CCTFloat*)pcToken)->mcValue;
				*ppsType = mcTypes.Float(4);
				return true;
			case CTNT_Double:
				*pdValue = ((CCTDouble*)pcToken)->mcValue;
				*ppsType = mcTypes.Float(8);
				return true;
			case CTNT_LongDouble:
				*pdValue = (double)((CCTLongDouble*)pcToken)->mcValue;
				*ppsType = mcTypes.LongDouble();
				return true;
			default:
				break;
			}
		}
		break;

	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		if (((pcUnary->meOperator == CT_Subtract) || (pcUnary->meOperator == CT_Add)) && FloatValue(pcUnary->mpcOperand, pdValue, ppsType) && *ppsType)
		{
			*pdValue = (pcUnary->meOperator == CT_Subtract) ? -*pdValue : *pdValue;
			return true;
		}
		break;

	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		if ((pcBinary->meOperator != CT_Add) && (pcBinary->meOperator != CT_Subtract) && (pcBinary->meOperator != CT_Multiply) && (pcBinary->meOperator != CT_Divide))
		{
			break;
		}
		if (!FloatValue(pcBinary->mpcLeft, &dLeft, &psLeft) || !FloatValue(pcBinary->mpcRight, &dRight, &psRight))
		{
			return false;
		}
		if (!psLeft && !psRight)
		{
			break;
		}
		psType = FloatArithmetic(psLeft, psRight);
		switch (pcBinary->meOperator)
		{
		case CT_Add:		*pdValue = dLeft + dRight;	break;
		case CT_Subtract:	*pdValue = dLeft - dRight;	break;
		case CT_Multiply:	*pdValue = dLeft * dRight;	break;
		default:			*pdValue = dLeft / dRight;	break;
		}
		if (psType->iBytes == 4)
		{
			*pdValue = (float)*pdValue;
		}
		*ppsType = psType;
		return true;

	case CPPS_CastExpression:
		pcCast = (CCPPSyntaxCastExpression*)pcExpression;
		psType = GetTypeIdType(pcCast->mpcType);
		if (psType && psType->IsFloat() && FloatValue(pcCast->mpcOperand, pdValue, &psLeft))
		{
			if (psType->iBytes == 4)
			{
				*pdValue = (float)*pdValue;
			}
			*ppsType = psType;
			return true;
		}
		break;

	default:
		break;
	}

	//An integer constant.
	if (!Constant(pcExpression, &lliValue))
	{
		return false;
	}
	*pdValue = (double)lliValue;
	*ppsType = NULL;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if working out the expression may call a soft float runtime function, which
// may use the pseudo registers.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::UsesFloat(CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	size			i;

	switch (pcNode->GetKind())
	{
	case CPPS_BinaryExpression:
	case CPPS_UnaryExpression:
	case CPPS_CastExpression:
	case CPPS_ConditionalExpression:
		psType = TypeOf(pcNode);
		if (psType && psType->IsFloat())
		{
			return true;
		}
		for (i = 0; i < pcNode->NumChildren(); i++)
		{
			psType = TypeOf(pcNode->GetChild(i));
			if (psType && psType->IsFloat())
			{
				return true;
			}
		}
		return false;
	default:
		return false;
	}
}

