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
#ifndef __I_R_TYPES_H__
#define __I_R_TYPES_H__
#include "BaseLib/PrimitiveTypes.h"


//The type of an IR value (see IR.txt Types).  Pointers have an address space: an index into the module's spaces.
enum EIRTypeKind
{
	IRTK_None,
	IRTK_I1,
	IRTK_I8,
	IRTK_I16,
	IRTK_I32,
	IRTK_I64,
	IRTK_F32,
	IRTK_F64,
	IRTK_Pointer,

	IRTK_NumKinds
};


//Sets of types, for what an opcode allows.
#define IRTM_I1			0x01
#define IRTM_Integer	0x02		//i8 to i64.
#define IRTM_Float		0x04
#define IRTM_Pointer	0x08
#define IRTM_Void		0x10
#define IRTM_Memory		(IRTM_Integer | IRTM_Float | IRTM_Pointer)		//What can be loaded, stored and passed.


struct SIRType
{
	uint8	uiKind;
	uint8	uiSpace;

	bool	IsNone(void)		{ return uiKind == IRTK_None; }
	bool	IsInteger(void)		{ return uiKind >= IRTK_I1 && uiKind <= IRTK_I64; }
	bool	IsFloat(void)		{ return uiKind == IRTK_F32 || uiKind == IRTK_F64; }
	bool	IsPointer(void)		{ return uiKind == IRTK_Pointer; }
	bool	Is(EIRTypeKind eKind)	{ return uiKind == eKind; }
	bool	Equals(SIRType sOther)	{ return uiKind == sOther.uiKind && uiSpace == sOther.uiSpace; }
	int		Bits(void);
	int		IntegerBits(void);
	uint8	Mask(void);
};


SIRType		IRType(EIRTypeKind eKind, uint8 uiSpace = 0);
SIRType		IRIntegerType(int iBytes);
uint64		IRTruncate(SIRType sType, uint64 ulliBits);
int64		IRSignExtend(SIRType sType, uint64 ulliBits);
const char*	GetIRTypeName(SIRType sType);
EIRTypeKind	FindIRTypeKind(const char* szName);


//What an operand refers to.  Temporaries and constants are the function's, symbols the module's.  Registers and
//memory operands are only in machine IR.
enum EIRReferenceKind
{
	IRRK_None,
	IRRK_Temporary,
	IRRK_Constant,
	IRRK_Symbol,
	IRRK_Register,
	IRRK_Memory,
};


#define IR_REFERENCE_INDEX_BITS		29
#define IR_REFERENCE_INDEX_MASK		((1u << IR_REFERENCE_INDEX_BITS) - 1)


//A kind (top 3 bits) and an index, as QBE's Ref.
struct SIRReference
{
	uint32	uiValue;

	EIRReferenceKind	Kind(void)		{ return (EIRReferenceKind)(uiValue >> IR_REFERENCE_INDEX_BITS); }
	uint32				Index(void)		{ return uiValue & IR_REFERENCE_INDEX_MASK; }
	bool				IsNone(void)	{ return uiValue == 0; }
	bool				IsTemporary(void)	{ return Kind() == IRRK_Temporary; }
	bool				IsConstant(void)	{ return Kind() == IRRK_Constant; }
	bool				IsSymbol(void)		{ return Kind() == IRRK_Symbol; }
	bool				Equals(SIRReference sOther)	{ return uiValue == sOther.uiValue; }
};


SIRReference	IRReference(EIRReferenceKind eKind, uint32 uiIndex);
SIRReference	IRNone(void);


#endif // __I_R_TYPES_H__
