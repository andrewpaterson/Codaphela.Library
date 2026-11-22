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
