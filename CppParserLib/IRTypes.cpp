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
#include "BaseLib/StringHelper.h"
#include "IRTypes.h"


const char*	gaszIRTypeNames[IRTK_NumKinds] = { "void", "i1", "i8", "i16", "i32", "i64", "f32", "f64", "ptr" };
int			gaiIRTypeBits[IRTK_NumKinds] = { 0, 1, 8, 16, 32, 64, 32, 64, 0 };


//////////////////////////////////////////////////////////////////////////
//
// The width of an integer or float in bits; 0 for void and pointers (whose
// size is the module's).
//
//////////////////////////////////////////////////////////////////////////
int SIRType::Bits(void)
{
	return (uiKind < IRTK_NumKinds) ? gaiIRTypeBits[uiKind] : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The width of an integer type in bits, 0 if it is not an integer.
//
//////////////////////////////////////////////////////////////////////////
int SIRType::IntegerBits(void)
{
	return IsInteger() ? Bits() : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The IRTM_ set the type is in.
//
//////////////////////////////////////////////////////////////////////////
uint8 SIRType::Mask(void)
{
	if (IsNone())
	{
		return IRTM_Void;
	}
	else if (Is(IRTK_I1))
	{
		return IRTM_I1;
	}
	else if (IsInteger())
	{
		return IRTM_Integer;
	}
	else if (IsFloat())
	{
		return IRTM_Float;
	}
	else if (IsPointer())
	{
		return IRTM_Pointer;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRType IRType(EIRTypeKind eKind, uint8 uiSpace)
{
	SIRType	sType;

	sType.uiKind = (uint8)eKind;
	sType.uiSpace = uiSpace;
	return sType;
}


//////////////////////////////////////////////////////////////////////////
//
// The integer type of iBytes: i8 for 1 or less, i16 for 2, i32 for 3 or 4
// and i64 for more (a pointer's offset type).
//
//////////////////////////////////////////////////////////////////////////
SIRType IRIntegerType(int iBytes)
{
	if (iBytes <= 1)
	{
		return IRType(IRTK_I8);
	}
	else if (iBytes == 2)
	{
		return IRType(IRTK_I16);
	}
	else if (iBytes <= 4)
	{
		return IRType(IRTK_I32);
	}
	return IRType(IRTK_I64);
}


//////////////////////////////////////////////////////////////////////////
//
// The bits kept for a value of sType: integers zero extended from their
// width, f32 in the low 32 bits.
//
//////////////////////////////////////////////////////////////////////////
uint64 IRTruncate(SIRType sType, uint64 ulliBits)
{
	int		iBits;

	iBits = sType.Bits();
	if ((iBits > 0) && (iBits < 64))
	{
		return ulliBits & ((((uint64)1) << iBits) - 1);
	}
	return ulliBits;
}


//////////////////////////////////////////////////////////////////////////
//
// An integer's bits sign extended from its width.
//
//////////////////////////////////////////////////////////////////////////
int64 IRSignExtend(SIRType sType, uint64 ulliBits)
{
	int		iBits;
	uint64	ulliSign;

	iBits = sType.IntegerBits();
	if ((iBits == 0) || (iBits == 64))
	{
		return (int64)ulliBits;
	}
	ulliSign = ((uint64)1) << (iBits - 1);
	return (int64)((IRTruncate(sType, ulliBits) ^ ulliSign) - ulliSign);
}


//////////////////////////////////////////////////////////////////////////
//
// The least value an iBits wide signed integer holds (iBits 1 to 63).
//
//////////////////////////////////////////////////////////////////////////
int64 IRSignedMin(int iBits)
{
	return -((int64)1 << (iBits - 1));
}


//////////////////////////////////////////////////////////////////////////
//
// The most an iBits wide signed integer holds (iBits 1 to 63).
//
//////////////////////////////////////////////////////////////////////////
int64 IRSignedMax(int iBits)
{
	return ((int64)1 << (iBits - 1)) - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The most an iBits wide unsigned integer holds (iBits 1 to 63, so it fits
// an int64).
//
//////////////////////////////////////////////////////////////////////////
int64 IRUnsignedMax(int iBits)
{
	return (int64)(((uint64)1 << iBits) - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// "void" for none; a pointer's space is not included.
//
//////////////////////////////////////////////////////////////////////////
const char* GetIRTypeName(SIRType sType)
{
	return (sType.uiKind < IRTK_NumKinds) ? gaszIRTypeNames[sType.uiKind] : "?";
}


//////////////////////////////////////////////////////////////////////////
//
// IRTK_None for "void" and for a name that is not a type.
//
//////////////////////////////////////////////////////////////////////////
EIRTypeKind FindIRTypeKind(const char* szName)
{
	int		i;

	for (i = IRTK_I1; i < IRTK_NumKinds; i++)
	{
		if (StringCompare(szName, gaszIRTypeNames[i]) == 0)
		{
			return (EIRTypeKind)i;
		}
	}
	return IRTK_None;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference IRReference(EIRReferenceKind eKind, uint32 uiIndex)
{
	SIRReference	sReference;

	sReference.uiValue = ((uint32)eKind << IR_REFERENCE_INDEX_BITS) | (uiIndex & IR_REFERENCE_INDEX_MASK);
	return sReference;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference IRNone(void)
{
	SIRReference	sReference;

	sReference.uiValue = 0;
	return sReference;
}
