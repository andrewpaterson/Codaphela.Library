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
#include <string.h>
#include <math.h>
#include "CTFloat.h"
#include "CTDouble.h"
#include "CTLongDouble.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"


//////////////////////////////////////////////////////////////////////////
//
// Floating point is worked out by Libc65's soft float runtime (softfloat.cpp and
// its assembly): the IR back ends call its functions.  Here are the float
// builtins' constants and the types and constant values of float expressions.
//
//////////////////////////////////////////////////////////////////////////


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
		psType = mcTypes.FloatArithmetic(psLeft, psRight);
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
		psType = mcDeclarators.GetTypeIdType(pcCast->mpcType);
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

	case CPPS_CallExpression:
		if (FloatBuiltin(pcExpression, pdValue, ppsType))
		{
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
// GCC's floating point constant builtins (math.h's HUGE_VAL, INFINITY and NAN):
// __builtin_huge_val, __builtin_inf and __builtin_nan("") are double, and their f
// forms float.  NaNs are quiet and positive; a payload string is ignored.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::FloatBuiltin(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType)
{
	CCPPSyntaxCallExpression*	pcCall;
	CCPPSyntax*					pcCallee;
	CCPPSyntaxName*				pcName;
	char*						szName;
	int							iLength;
	int64						lliName;
	bool						bFloat;
	bool						bNaN;

	pcCall = (CCPPSyntaxCallExpression*)pcExpression;
	pcCallee = SkipParentheses(pcCall->mpcCallee);
	if (!pcCallee || (pcCallee->GetKind() != CPPS_IdExpression))
	{
		return false;
	}
	pcName = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName;
	if (!pcName || (pcName->macpComponents.NumElements() != 1))
	{
		return false;
	}
	lliName = GetFunctionName(pcName->GetLast());
	if (!IsBuiltin(lliName))
	{
		return false;
	}
	szName = BuiltinName(lliName, &iLength);
	bFloat = (iLength > 0) && (szName[iLength - 1] == 'f');
	iLength -= bFloat ? 1 : 0;
	if (((iLength == 8) && (memcmp(szName, "huge_val", 8) == 0)) || ((iLength == 3) && (memcmp(szName, "inf", 3) == 0)))
	{
		bNaN = false;
	}
	else if ((iLength == 3) && (memcmp(szName, "nan", 3) == 0))
	{
		bNaN = true;
	}
	else
	{
		return false;
	}
	if (pcCall->macpArguments.NumElements() != (bNaN ? 1 : 0))
	{
		return false;
	}
	*pdValue = bNaN ? (double)NAN : (double)INFINITY;
	*ppsType = bFloat ? &mcTypes.msFloat : &mcTypes.msDouble;
	return true;
}
