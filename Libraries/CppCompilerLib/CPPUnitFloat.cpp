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
#include "CPPUnit.h"


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
int64 CCPPUnit::FloatBits(SCPP65Type* psType, double dValue)
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
bool CCPPUnit::FloatConstant(CCPPSyntax* pcExpression, double* pdValue)
{
	SCPP65Type*		psType;

	mpcStatistics->Count(CPPCC_FloatConstants);
	if (FloatValue(pcExpression, pdValue, &psType) && psType)
	{
		mpcStatistics->Count(CPPCC_FloatConstantsFolded);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A floating point number token's value and type (float, double or long
// double).  False for any other token.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::TokenFloat(CCPPToken* pcToken, double* pdValue, SCPP65Type** ppsType)
{
	if (!pcToken->IsNumber())
	{
		return false;
	}
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
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a constant expression; *ppsType is its floating point type, or NULL
// for an integer.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::FloatValue(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType)
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
	case CPPS_ImplicitArgument:
		if (((CCPPSyntaxImplicitArgument*)pcExpression)->mbConstant && ((CCPPSyntaxImplicitArgument*)pcExpression)->mpsType->IsFloat())
		{
			*pdValue = ((CCPPSyntaxImplicitArgument*)pcExpression)->mdValue;
			*ppsType = ((CCPPSyntaxImplicitArgument*)pcExpression)->mpsType;
			return true;
		}
		break;

	case CPPS_LiteralExpression:
		pcToken = mpcTokeniser->Get(((CCPPSyntaxLiteralExpression*)pcExpression)->miLiteral);
		if (TokenFloat(pcToken, pdValue, ppsType))
		{
			return true;
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

	case CPPS_IdExpression:
		if (ConstexprFloat((CCPPSyntaxIdExpression*)pcExpression, pdValue, ppsType))
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
// The value of a name that is a floating point variable declared constexpr
// (a const one isn't a constant: [expr.const] 2), which is its
// initialiser's as its type; *ppsType is the type.  False for any other
// name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::ConstexprFloat(CCPPSyntaxIdExpression* pcId, double* pdValue, SCPP65Type** ppsType)
{
	CCPPSymbol*		pcSymbol;
	CCPPVariable*	pcVariable;
	CCPPSyntax*		pcInitialiser;
	SCPP65Type*		psValue;
	CCPPBoundName	cBound;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;

	//S::d, a static member of a class (a field's name in it), is the variable its name is bound to.
	pcSymbol = pcId->mpcName->mpcSymbol;
	if (pcSymbol && (pcSymbol->meKind == CPPSK_Variable))
	{
		pcVariable = GetVariable(pcSymbol);
	}
	else
	{
		mcBinder.BindName(pcId, &cBound);
		pcVariable = (cBound.meWhat == CPPBN_Variable) ? cBound.mpcVariable : NULL;
	}
	if (!pcVariable || !pcVariable->mpsType->IsFloat())
	{
		return false;
	}
	//Its declaration says constexpr; the initialiser is there or, for a static member, in its class.
	pcInit = VariableDeclarator(pcVariable);
	pcDeclaration = pcInit ? pcInit->GetDeclaration() : NULL;
	if (!pcDeclaration || !pcDeclaration->mpcSpecifiers || !(pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_CONSTEXPR))
	{
		return false;
	}
	pcInitialiser = pcInit->mpcInitializer ? pcInit->mpcInitializer : pcVariable->mpcInClassInitialiser;
	if (!pcInitialiser)
	{
		return false;
	}
	if ((pcInitialiser->GetKind() == CPPS_InitializerList) && (ListElements(pcInitialiser)->NumElements() == 1))
	{
		pcInitialiser = ListElements(pcInitialiser)->GetPtr(0);
	}
	if (!FloatValue(pcInitialiser, pdValue, &psValue))
	{
		return false;
	}
	if (pcVariable->mpsType->iBytes == 4)
	{
		*pdValue = (float)*pdValue;
	}
	*ppsType = pcVariable->mpsType->Unqualified();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// GCC's floating point constant builtins (math.h's HUGE_VAL, INFINITY and NAN):
// __builtin_huge_val, __builtin_inf and __builtin_nan("") are double, and their f
// forms float.  NaNs are quiet and positive; a payload string is ignored.
//
//////////////////////////////////////////////////////////////////////////
static bool IsBuiltinNamed(const char* szName, int iLength, const char* szBuiltin)
{
	return ((size)iLength == strlen(szBuiltin)) && (memcmp(szName, szBuiltin, iLength) == 0);
}


bool CCPPUnit::FloatBuiltin(CCPPSyntax* pcExpression, double* pdValue, SCPP65Type** ppsType)
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
	if (IsBuiltinNamed(szName, iLength, "huge_val") || IsBuiltinNamed(szName, iLength, "inf"))
	{
		bNaN = false;
	}
	else if (IsBuiltinNamed(szName, iLength, "nan"))
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
