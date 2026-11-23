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
#include <math.h>
#include <string.h>
#include "CppTokeniser.h"
#include "CPPUnit.h"
#include "CPPBuiltins.h"


#define CPP65_BCD_ONLY	"__builtin_bcd_add and __builtin_bcd_subtract are the W65C816's decimal mode: use BaseLib's BCDAdd and BCDSubtract"


//The builtins the compiler does: the type checker and the IR front end both look them up here.
static const SCPPBuiltin gasCPP65Builtins[] =
{
	{ "unreachable",	CPPB_Unreachable,	0, CPPBO_None,				NULL },
	{ "trap",			CPPB_Unreachable,	0, CPPBO_None,				NULL },
	{ "ia32_pause",		CPPB_Pause,		0, CPPBO_None,				NULL },
	{ "expect",			CPPB_Expect,		2, CPPBO_None,				NULL },
	{ "memcpy",			CPPB_Memcpy,		3, CPPBO_None,				NULL },
	{ "alloca",			CPPB_Alloca,		1, CPPBO_None,				NULL },
	{ "bcd_add",		CPPB_BCDAdd,		4, CPPBO_None,				CPP65_BCD_ONLY },
	{ "bcd_subtract",	CPPB_BCDSubtract,	4, CPPBO_None,				CPP65_BCD_ONLY },
	{ "ctz",			CPPB_Ctz,			1, CPPBO_UnsignedInt,			NULL },
	{ "ctzl",			CPPB_Ctz,			1, CPPBO_UnsignedLong,		NULL },
	{ "ctzll",			CPPB_Ctz,			1, CPPBO_UnsignedLongLong,	NULL },
	{ "clz",			CPPB_Clz,			1, CPPBO_UnsignedInt,			NULL },
	{ "clzl",			CPPB_Clz,			1, CPPBO_UnsignedLong,		NULL },
	{ "clzll",			CPPB_Clz,			1, CPPBO_UnsignedLongLong,	NULL },
	{ "popcount",		CPPB_Popcount,	1, CPPBO_UnsignedInt,			NULL },
	{ "popcountl",		CPPB_Popcount,	1, CPPBO_UnsignedLong,		NULL },
	{ "popcountll",		CPPB_Popcount,	1, CPPBO_UnsignedLongLong,	NULL },
	{ NULL,				CPPB_Unreachable,	0, CPPBO_None,				NULL },
};


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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuiltins::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuiltins::Kill(void)
{
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A name reserved for the compiler (__debugbreak): the host compiler's
// intrinsics, which no header declares.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuiltins::IsIntrinsic(int64 lliName)
{
	char*	szName;
	int		iLength;

	if (lliName < 0)
	{
		return false;
	}
	szName = mpcUnit->mpcTokeniser->GetIdentifier(lliName, &iLength);
	return szName && (iLength > 2) && (szName[0] == '_') && (szName[1] == '_');
}


//////////////////////////////////////////////////////////////////////////
//
// True for the names of GCC's builtin functions, which are not declared.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuiltins::IsBuiltin(int64 lliName)
{
	char*	szName;
	int		iLength;

	if (lliName < 0)
	{
		return false;
	}
	szName = mpcUnit->mpcTokeniser->GetIdentifier(lliName, &iLength);
	return szName && (iLength > CPP65_BUILTIN_PREFIX_LENGTH) && (memcmp(szName, CPP65_BUILTIN_PREFIX, CPP65_BUILTIN_PREFIX_LENGTH) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The builtin lliName names (__builtin_ and an entry of gasCPP65Builtins),
// NULL for one the compiler doesn't do (or a name that isn't a builtin's).
//
//////////////////////////////////////////////////////////////////////////
const SCPPBuiltin* CCPPBuiltins::FindBuiltin(int64 lliName)
{
	char*	szName;
	int		iLength;
	size	i;

	if (!IsBuiltin(lliName))
	{
		return NULL;
	}
	szName = BuiltinName(lliName, &iLength);
	for (i = 0; gasCPP65Builtins[i].szName; i++)
	{
		if ((StrLen(gasCPP65Builtins[i].szName) == (size)iLength) && (memcmp(gasCPP65Builtins[i].szName, szName, iLength) == 0))
		{
			return &gasCPP65Builtins[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type ctz, clz or popcount counts the bits of (NULL for the others).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPBuiltins::BuiltinOperandType(const SCPPBuiltin* psBuiltin)
{
	switch (psBuiltin->eOperand)
	{
	case CPPBO_UnsignedInt:		return &mpcUnit->mcTypes.msUnsignedInt;
	case CPPBO_UnsignedLong:		return mpcUnit->mcTypes.Long(false);
	case CPPBO_UnsignedLongLong:	return &mpcUnit->mcTypes.msUnsignedLongLong;
	default:						return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A builtin's name without "__builtin_" (IsBuiltin) and its length.
//
//////////////////////////////////////////////////////////////////////////
char* CCPPBuiltins::BuiltinName(int64 lliName, int* piLength)
{
	char*	szName;

	szName = mpcUnit->mpcTokeniser->GetIdentifier(lliName, piLength);
	*piLength -= CPP65_BUILTIN_PREFIX_LENGTH;
	return szName + CPP65_BUILTIN_PREFIX_LENGTH;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call to lliName is memcpy (or __builtin_memcpy) with a constant count
// of 1 to CPP65_INLINE_MEMCPY bytes (CPP65_INLINE_MEMCPY_SIZE preferring size), which InlineMemcpy copies in
// place.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuiltins::IsInlineMemcpy(int64 lliName, CArraySyntax* papcArguments)
{
	int64	lliBytes;

	if (((lliName != mpcUnit->mpcTokeniser->mlliMemcpy) && (lliName != mpcUnit->mpcTokeniser->mlliBuiltinMemcpy)) || (papcArguments->NumElements() != 3))
	{
		return false;
	}
	return mpcUnit->mcFolder.Constant(papcArguments->GetPtr(2), &lliBytes) && (lliBytes > 0) && (lliBytes <= (mpcUnit->mpcOptions->PrefersSize() ? CPP65_INLINE_MEMCPY_SIZE : CPP65_INLINE_MEMCPY));
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a call of GCC's floating point constant builtins (math.h's
// HUGE_VAL, INFINITY and NAN) named lliName, with uiArguments: __builtin_
// huge_val and __builtin_inf are infinity, __builtin_nan("") a quiet
// positive NaN (its payload string ignored), double, and *pbFloat for
// their f forms, float.  False for any other call.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuiltins::FloatBuiltinValue(int64 lliName, size uiArguments, double* pdValue, bool* pbFloat)
{
	char*	szName;
	int		iLength;
	bool	bNaN;

	if (!IsBuiltin(lliName))
	{
		return false;
	}
	szName = BuiltinName(lliName, &iLength);
	*pbFloat = (iLength > 0) && (szName[iLength - 1] == 'f');
	iLength -= *pbFloat ? 1 : 0;
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
	if (uiArguments != (bNaN ? 1 : 0))
	{
		return false;
	}
	*pdValue = bNaN ? (double)NAN : (double)INFINITY;
	return true;
}

