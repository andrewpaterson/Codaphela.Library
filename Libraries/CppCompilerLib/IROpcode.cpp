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
#include "BaseLib/StringHelper.h"
#include "IROpcode.h"


#define IR_PURE		IROP_Pure
#define IR_SWAPS	(IROP_Pure | IROP_Commutative)
#define IR_TRAPS	(IROP_Pure | IROP_MayTrap)
#define IR_SIGNED	(IROP_Pure | IROP_Signed)
#define IR_UNSIGNED	(IROP_Pure | IROP_Unsigned)
#define IR_SWAP_LT	IROP_SwappedLess
#define IR_ARITH	IROP_IntegerArithmetic
#define IR_LOW		IROP_LowBits
#define IR_WRAPS	IROP_SignedWrapping
#define IR_REM		IROP_Remainder


//Every column but an integer comparison's and a division's, which are 0: IR_INTEGER_COMPARE and IR_DIVIDE_OP have them.
#define IR_OPCODE(name, shape, types, sourcetypes, first, second, result, flags, properties)	{ name, shape, types, sourcetypes, { first, second }, result, flags, properties, 0, 0, 0, 0, 0 }
#define IR_INTEGER_OP(name, properties)		IR_OPCODE(name, IRSH_Operands, IRTM_I1 | IRTM_Integer, 0, IROK_Same, IROK_Same, IRRS_Same, 0, properties)
#define IR_INTEGER_UNARY(name, properties)	IR_OPCODE(name, IRSH_Operands, IRTM_I1 | IRTM_Integer, 0, IROK_Same, IROK_None, IRRS_Same, 0, properties)
#define IR_INTEGER_COMPARE(name, properties, inverted, swapped, holds, unsignedtwin)	{ name, IRSH_Operands, IRTM_I1 | IRTM_Integer | IRTM_Pointer, 0, { IROK_Same, IROK_Same }, IRRS_I1, 0, (properties) | IROP_IntegerComparison, inverted, swapped, holds, unsignedtwin, 0 }
#define IR_FLOAT_OP(name, properties)		IR_OPCODE(name, IRSH_Operands, IRTM_Float, 0, IROK_Same, IROK_Same, IRRS_Same, 0, properties)
#define IR_FLOAT_COMPARE(name, properties)	IR_OPCODE(name, IRSH_Operands, IRTM_Float, 0, IROK_Same, IROK_Same, IRRS_I1, 0, properties)
#define IR_DIVIDE_OP(name, properties, unsignedtwin, partner)	{ name, IRSH_Operands, IRTM_I1 | IRTM_Integer, 0, { IROK_Same, IROK_Same }, IRRS_Same, IRF_Hot, IR_TRAPS | IROP_Division | (properties), 0, 0, 0, unsignedtwin, partner }
#define IR_CONVERSION(name, to, from)		IR_OPCODE(name, IRSH_Conversion, to, from, IROK_Source, IROK_None, IRRS_Same, 0, IR_PURE)
#define IR_INTEGER_CONVERSION(name, to, from)	IR_OPCODE(name, IRSH_Conversion, to, from, IROK_Source, IROK_None, IRRS_Same, 0, IR_PURE | IROP_IntegerConversion)


SIROpcode	gasIROpcodes[IRO_NumGeneric] =
{
	IR_OPCODE("", IRSH_Operands, 0, 0, IROK_None, IROK_None, IRRS_None, 0, 0),

	//             name    properties                               the unsigned twin  the partner
	IR_INTEGER_OP("add",  IR_SWAPS | IR_ARITH | IR_LOW | IR_WRAPS),
	IR_INTEGER_OP("sub",  IR_PURE | IR_ARITH | IR_LOW | IR_WRAPS),
	IR_INTEGER_OP("mul",  IR_SWAPS | IR_ARITH | IR_LOW | IR_WRAPS),
	IR_DIVIDE_OP("sdiv",  IROP_Signed,                              IRO_UDiv,          IRO_SRem),
	IR_DIVIDE_OP("udiv",  IROP_Unsigned,                            IRO_None,          IRO_URem),
	IR_DIVIDE_OP("srem",  IROP_Signed | IR_REM,                     IRO_URem,          IRO_SDiv),
	IR_DIVIDE_OP("urem",  IROP_Unsigned | IR_REM,                   IRO_None,          IRO_UDiv),
	IR_INTEGER_OP("and",  IR_SWAPS | IR_ARITH | IR_LOW),
	IR_INTEGER_OP("or",   IR_SWAPS | IR_ARITH | IR_LOW),
	IR_INTEGER_OP("xor",  IR_SWAPS | IR_ARITH | IR_LOW),
	IR_INTEGER_OP("shl",  IR_PURE | IR_ARITH | IR_WRAPS),
	IR_INTEGER_OP("lshr", IR_PURE | IR_ARITH),
	IR_INTEGER_OP("ashr", IR_PURE | IR_ARITH),
	IR_INTEGER_UNARY("neg", IR_PURE | IR_ARITH | IR_WRAPS),
	IR_INTEGER_UNARY("not", IR_PURE | IR_ARITH),
	IR_OPCODE("copy", IRSH_Operands, IRTM_I1 | IRTM_Memory, 0, IROK_Same, IROK_None, IRRS_Same, 0, IR_PURE),
	IR_INTEGER_CONVERSION("sext", IRTM_Integer, IRTM_I1 | IRTM_Integer),
	IR_INTEGER_CONVERSION("zext", IRTM_Integer, IRTM_I1 | IRTM_Integer),
	IR_INTEGER_CONVERSION("trunc", IRTM_I1 | IRTM_Integer, IRTM_Integer),

	//                 name    properties                 inverted  swapped   holds when the left operand is  the unsigned twin
	IR_INTEGER_COMPARE("eq",   IR_SWAPS,                  IRO_Ne,   IRO_Eq,   IRCH_Equal,                     IRO_None),
	IR_INTEGER_COMPARE("ne",   IR_SWAPS,                  IRO_Eq,   IRO_Ne,   IRCH_Less | IRCH_Greater,       IRO_None),
	IR_INTEGER_COMPARE("slt",  IR_SIGNED,                 IRO_SGe,  IRO_SGt,  IRCH_Less,                      IRO_ULt),
	IR_INTEGER_COMPARE("sle",  IR_SIGNED | IR_SWAP_LT,    IRO_SGt,  IRO_SGe,  IRCH_Less | IRCH_Equal,         IRO_ULe),
	IR_INTEGER_COMPARE("sgt",  IR_SIGNED | IR_SWAP_LT,    IRO_SLe,  IRO_SLt,  IRCH_Greater,                   IRO_UGt),
	IR_INTEGER_COMPARE("sge",  IR_SIGNED,                 IRO_SLt,  IRO_SLe,  IRCH_Greater | IRCH_Equal,      IRO_UGe),
	IR_INTEGER_COMPARE("ult",  IR_UNSIGNED,               IRO_UGe,  IRO_UGt,  IRCH_Less,                      IRO_None),
	IR_INTEGER_COMPARE("ule",  IR_UNSIGNED | IR_SWAP_LT,  IRO_UGt,  IRO_UGe,  IRCH_Less | IRCH_Equal,         IRO_None),
	IR_INTEGER_COMPARE("ugt",  IR_UNSIGNED | IR_SWAP_LT,  IRO_ULe,  IRO_ULt,  IRCH_Greater,                   IRO_None),
	IR_INTEGER_COMPARE("uge",  IR_UNSIGNED,               IRO_ULt,  IRO_ULe,  IRCH_Greater | IRCH_Equal,      IRO_None),

	IR_FLOAT_OP("fadd", IR_SWAPS),
	IR_FLOAT_OP("fsub", IR_PURE),
	IR_FLOAT_OP("fmul", IR_SWAPS),
	IR_FLOAT_OP("fdiv", IR_PURE),
	IR_OPCODE("fneg", IRSH_Operands, IRTM_Float, 0, IROK_Same, IROK_None, IRRS_Same, 0, IR_PURE),
	IR_FLOAT_COMPARE("feq", IR_SWAPS),
	IR_FLOAT_COMPARE("fne", IR_SWAPS),
	IR_FLOAT_COMPARE("flt", IR_PURE),
	IR_FLOAT_COMPARE("fle", IR_PURE),
	IR_FLOAT_COMPARE("fgt", IR_PURE),
	IR_FLOAT_COMPARE("fge", IR_PURE),
	IR_FLOAT_COMPARE("fordered", IR_SWAPS),
	IR_FLOAT_COMPARE("funordered", IR_SWAPS),
	IR_CONVERSION("sitof", IRTM_Float, IRTM_Integer),
	IR_CONVERSION("uitof", IRTM_Float, IRTM_Integer),
	IR_CONVERSION("ftosi", IRTM_Integer, IRTM_Float),
	IR_CONVERSION("ftoui", IRTM_Integer, IRTM_Float),
	IR_CONVERSION("fext", IRTM_Float, IRTM_Float),
	IR_CONVERSION("ftrunc", IRTM_Float, IRTM_Float),

	IR_OPCODE("slot", IRSH_Operands, IRTM_Pointer, 0, IROK_Constant, IROK_Constant, IRRS_Same, IRF_Volatile, IR_PURE),
	IR_OPCODE("argaddr", IRSH_Operands, IRTM_Pointer, 0, IROK_Constant, IROK_None, IRRS_Same, 0, IR_PURE),
	IR_OPCODE("ptradd", IRSH_Operands, IRTM_Pointer, 0, IROK_Same, IROK_Offset, IRRS_Same, IRF_InBank | IRF_InArray, IR_PURE),
	IR_CONVERSION("ptrtoi", IRTM_Integer, IRTM_Pointer),
	IR_CONVERSION("itoptr", IRTM_Pointer, IRTM_Integer),
	IR_CONVERSION("spacecast", IRTM_Pointer, IRTM_Pointer),
	IR_OPCODE("load", IRSH_Operands, IRTM_Memory, 0, IROK_Pointer, IROK_None, IRRS_Same, IRF_Volatile, 0),
	IR_OPCODE("store", IRSH_Operands, IRTM_Memory, 0, IROK_Same, IROK_Pointer, IRRS_None, IRF_Volatile, 0),
	IR_OPCODE("arg", IRSH_Operands, IRTM_Memory, 0, IROK_Same, IROK_None, IRRS_None, 0, 0),
	IR_OPCODE("call", IRSH_Call, IRTM_Memory | IRTM_Void, 0, IROK_Pointer, IROK_None, IRRS_Optional, IRF_ThisCall | IRF_Variadic, 0),

	//Pure, so that it is no write to memory and goes when nothing reads it; but it speaks of the call before it, so it is
	//never the same as another (IROptimiserSameInstruction) and is never moved (IROptimiserHoistable doesn't list it).
	IR_OPCODE("thrown", IRSH_Operands, IRTM_I1, 0, IROK_None, IROK_None, IRRS_Same, 0, IR_PURE),
};


SIRFlagName	gasIRFlagNames[IRF_NumWordFlags] =
{
	{ IRF_Volatile, "volatile" },
	{ IRF_InBank, "inbank" },
	{ IRF_ThisCall, "thiscall" },
	{ IRF_Hot, "hot" },
	{ IRF_InArray, "inarray" },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SIROpcode::NumOperands(void) const
{
	return (auiOperands[0] != IROK_None ? 1 : 0) + (auiOperands[1] != IROK_None ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A generic opcode; NULL for a target's (see CIRModule::GetOpcode).
//
//////////////////////////////////////////////////////////////////////////
SIROpcode* GetIROpcode(uint16 uiOpcode)
{
	if ((uiOpcode > IRO_None) && (uiOpcode < IRO_NumGeneric))
	{
		return &gasIROpcodes[uiOpcode];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The opcode called szName in pasOpcodes (the generic table when NULL),
// whose first entry is opcode uiFirst; IRO_None if there is none.
//
//////////////////////////////////////////////////////////////////////////
uint16 FindIROpcode(SIROpcode* pasOpcodes, uint16 uiNumOpcodes, uint16 uiFirst, const char* szName)
{
	uint16	ui;

	if (pasOpcodes == NULL)
	{
		pasOpcodes = gasIROpcodes;
		uiNumOpcodes = IRO_NumGeneric;
		uiFirst = IRO_None;
	}
	for (ui = 0; ui < uiNumOpcodes; ui++)
	{
		if ((pasOpcodes[ui].szName[0] != '\0') && (StringCompare(pasOpcodes[ui].szName, szName) == 0))
		{
			return uiFirst + ui;
		}
	}
	return IRO_None;
}


//////////////////////////////////////////////////////////////////////////
//
// The opcode's IROP_ properties: a target's opcodes have none.
//
//////////////////////////////////////////////////////////////////////////
static uint16 IRProperties(uint16 uiOpcode)
{
	return (uiOpcode < IRO_NumGeneric) ? gasIROpcodes[uiOpcode].uiProperties : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// True for the integer comparisons: eq, ne and the signed and unsigned
// orderings.
//
//////////////////////////////////////////////////////////////////////////
bool IsIRIntegerComparison(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & IROP_IntegerComparison) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// slt, sle, sgt and sge.
//
//////////////////////////////////////////////////////////////////////////
bool IsIRSignedComparison(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & (IROP_IntegerComparison | IROP_Signed)) == (IROP_IntegerComparison | IROP_Signed);
}


//////////////////////////////////////////////////////////////////////////
//
// sgt, sle, ugt and ule: a less than comparison with the operands swapped.
//
//////////////////////////////////////////////////////////////////////////
bool IsIRSwappedComparison(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & IROP_SwappedLess) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The integer comparison that is true when uiOpcode's is false, IRO_None
// if uiOpcode isn't an integer comparison.
//
//////////////////////////////////////////////////////////////////////////
uint16 IRInvertedComparison(uint16 uiOpcode)
{
	return IsIRIntegerComparison(uiOpcode) ? gasIROpcodes[uiOpcode].uiInverted : (uint16)IRO_None;
}


//////////////////////////////////////////////////////////////////////////
//
// The integer comparison of the operands the other way round: a < b is
// b > a.  Any other opcode is returned as it is.
//
//////////////////////////////////////////////////////////////////////////
uint16 IRSwappedComparison(uint16 uiOpcode)
{
	return IsIRIntegerComparison(uiOpcode) ? gasIROpcodes[uiOpcode].uiSwapped : uiOpcode;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether integer comparison uiOpcode holds for constants ulliLeft and
// ulliRight of sType (their bits as wide as it, signed for the s ones).
//
//////////////////////////////////////////////////////////////////////////
bool IRCompareIntegers(uint16 uiOpcode, SIRType sType, uint64 ulliLeft, uint64 ulliRight)
{
	bool	bLess;
	uint8	uiOrder;

	ulliLeft = IRTruncate(sType, ulliLeft);
	ulliRight = IRTruncate(sType, ulliRight);
	if (ulliLeft == ulliRight)
	{
		uiOrder = IRCH_Equal;
	}
	else
	{
		bLess = IsIRSignedComparison(uiOpcode) ? (IRSignExtend(sType, ulliLeft) < IRSignExtend(sType, ulliRight)) : (ulliLeft < ulliRight);
		uiOrder = bLess ? IRCH_Less : IRCH_Greater;
	}
	return (gasIROpcodes[uiOpcode].uiHolds & uiOrder) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// trunc, sext and zext: an integer to an integer of another width.
//
//////////////////////////////////////////////////////////////////////////
bool IsIRIntegerConversion(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & IROP_IntegerConversion) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// True if an instruction of uiOpcode has no effect and reads no memory
// (IROP_Pure): a target's opcodes never are.
//
//////////////////////////////////////////////////////////////////////////
bool IsIRPure(uint16 uiOpcode)
{
	return (uiOpcode == IRO_None) || ((IRProperties(uiOpcode) & IROP_Pure) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the opcode fails on some operands (IROP_MayTrap: division by 0):
// though pure, it is only moved where it would be done anyway.
//
//////////////////////////////////////////////////////////////////////////
bool IsIRMayTrap(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & IROP_MayTrap) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// a op b is b op a (IROP_Commutative).
//
//////////////////////////////////////////////////////////////////////////
bool IsIRCommutative(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & IROP_Commutative) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// What one integer instruction gives for constant operands: a op b, or a
// conversion of a from sFrom, in sType (iBits wide; a pointer is as wide
// as its space).  False if it has no value (a division by zero, a shift
// by the width or more) or isn't an integer instruction.  The optimiser
// folds with it and the interpreter runs with it, so they can't disagree.
//
//////////////////////////////////////////////////////////////////////////
bool IRFoldInteger(uint16 uiOpcode, SIRType sType, SIRType sFrom, int iBits, uint64 a, uint64 b, uint64* pulliResult)
{
	int64	sa;
	int64	sb;

	sa = IRSignExtend(sFrom, a);
	sb = IRSignExtend(sType, b);
	switch (uiOpcode)
	{
	case IRO_Add:		*pulliResult = a + b;	break;
	case IRO_Sub:		*pulliResult = a - b;	break;
	case IRO_Mul:		*pulliResult = a * b;	break;
	case IRO_And:		*pulliResult = a & b;	break;
	case IRO_Or:		*pulliResult = a | b;	break;
	case IRO_Xor:		*pulliResult = a ^ b;	break;
	case IRO_Neg:		*pulliResult = (uint64)-(int64)a;	break;
	case IRO_Not:		*pulliResult = ~a;	break;
	case IRO_Copy:		*pulliResult = a;	break;
	case IRO_SDiv:
	case IRO_SRem:
		if ((sb == 0) || ((sb == -1) && (sa == (int64)(((uint64)1) << (iBits - 1)) * ((iBits == 64) ? 1 : -1))))
		{
			return false;
		}
		*pulliResult = (uiOpcode == IRO_SDiv) ? (uint64)(sa / sb) : (uint64)(sa % sb);
		break;
	case IRO_UDiv:
	case IRO_URem:
		if (b == 0)
		{
			return false;
		}
		*pulliResult = (uiOpcode == IRO_UDiv) ? a / b : a % b;
		break;
	case IRO_Shl:
	case IRO_LShr:
	case IRO_AShr:
		if (b >= (uint64)iBits)
		{
			return false;
		}
		if (uiOpcode == IRO_Shl)
		{
			*pulliResult = a << b;
		}
		else if (uiOpcode == IRO_LShr)
		{
			*pulliResult = a >> b;
		}
		else
		{
			*pulliResult = (uint64)(sa >> b);
		}
		break;
	case IRO_Eq:
	case IRO_Ne:
	case IRO_ULt:
	case IRO_ULe:
	case IRO_UGt:
	case IRO_UGe:
	case IRO_SLt:
	case IRO_SLe:
	case IRO_SGt:
	case IRO_SGe:
		*pulliResult = IRCompareIntegers(uiOpcode, sType, a, b);
		break;
	case IRO_SExt:	*pulliResult = (uint64)sa;	break;
	case IRO_ZExt:
	case IRO_Trunc:
	case IRO_PtrToI:
	case IRO_IToPtr:
		*pulliResult = a;
		break;
	default:
		return false;
	}
	if (sType.IsPointer() && (iBits < 64))
	{
		//A pointer is as wide as its space (IRTruncate doesn't know the module's sizes).
		*pulliResult &= (((uint64)1) << iBits) - 1;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the opcode is one a signed overflow of is undefined behaviour
// in (IRF_NoSignedWrap): add, sub, mul, neg and shl.
//
//////////////////////////////////////////////////////////////////////////
bool IsIRSignedWrapping(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & IROP_SignedWrapping) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The orders of its operands an integer comparison holds for (IRCH_Less,
// _Equal, _Greater: of the left against the right); 0 for any other
// opcode.
//
//////////////////////////////////////////////////////////////////////////
uint8 IRComparisonHolds(uint16 uiOpcode)
{
	return IsIRIntegerComparison(uiOpcode) ? gasIROpcodes[uiOpcode].uiHolds : (uint8)0;
}


//////////////////////////////////////////////////////////////////////////
//
// IROP_Signed for an opcode that takes its operands as signed (slt, sdiv),
// IROP_Unsigned for one that takes them as unsigned (ult, udiv), and 0 for
// one that is the same for both (eq, add).
//
//////////////////////////////////////////////////////////////////////////
uint16 IRSignedness(uint16 uiOpcode)
{
	return IRProperties(uiOpcode) & (IROP_Signed | IROP_Unsigned);
}


//////////////////////////////////////////////////////////////////////////
//
// The opcode that does to unsigned operands what a signed comparison or a
// signed division does to signed ones: slt's is ult, srem's is urem.  Any
// other opcode is returned as it is.
//
//////////////////////////////////////////////////////////////////////////
uint16 IRUnsignedOpcode(uint16 uiOpcode)
{
	return ((uiOpcode < IRO_NumGeneric) && (gasIROpcodes[uiOpcode].uiUnsigned != IRO_None)) ? gasIROpcodes[uiOpcode].uiUnsigned : uiOpcode;
}


//////////////////////////////////////////////////////////////////////////
//
// add, sub, mul, and, or, xor, shl, lshr, ashr, neg and not.
//
//////////////////////////////////////////////////////////////////////////
bool IsIRIntegerArithmetic(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & IROP_IntegerArithmetic) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the low bits of the opcode's result depend only on the low bits
// of its operands: add, sub, mul, and, or and xor.
//
//////////////////////////////////////////////////////////////////////////
bool IsIRLowBits(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & IROP_LowBits) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// sdiv, udiv, srem and urem; those of them that take signed operands, and
// unsigned; the two that give the remainder, and the two that give the
// quotient.
//
//////////////////////////////////////////////////////////////////////////
bool IsIRDivision(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & IROP_Division) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsIRSignedDivision(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & (IROP_Division | IROP_Signed)) == (IROP_Division | IROP_Signed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsIRUnsignedDivision(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & (IROP_Division | IROP_Unsigned)) == (IROP_Division | IROP_Unsigned);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsIRRemainder(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & IROP_Remainder) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsIRQuotient(uint16 uiOpcode)
{
	return (IRProperties(uiOpcode) & (IROP_Division | IROP_Remainder)) == IROP_Division;
}


//////////////////////////////////////////////////////////////////////////
//
// The division of signed or of unsigned operands that gives the quotient,
// or the remainder.
//
//////////////////////////////////////////////////////////////////////////
uint16 IRDivisionOpcode(bool bSigned, bool bRemainder)
{
	if (bRemainder)
	{
		return bSigned ? IRO_SRem : IRO_URem;
	}
	return bSigned ? IRO_SDiv : IRO_UDiv;
}


//////////////////////////////////////////////////////////////////////////
//
// The remainder of the division a quotient is of, and the quotient of the
// one a remainder is of: sdiv's is srem, urem's is udiv.  IRO_None for an
// opcode that is no division.
//
//////////////////////////////////////////////////////////////////////////
uint16 IRDivisionPartner(uint16 uiOpcode)
{
	return IsIRDivision(uiOpcode) ? gasIROpcodes[uiOpcode].uiPartner : (uint16)IRO_None;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the opcode's result is an i1 whatever its instruction's type:
// the comparisons, of integers and of floats.
//
//////////////////////////////////////////////////////////////////////////
bool IRResultIsI1(uint16 uiOpcode)
{
	return (uiOpcode < IRO_NumGeneric) && (gasIROpcodes[uiOpcode].uiResult == IRRS_I1);
}


//////////////////////////////////////////////////////////////////////////
//
// A number iBits wide as a signed one.
//
//////////////////////////////////////////////////////////////////////////
static int64 IRSignExtendBits(int iBits, uint64 ulliBits)
{
	return (iBits >= 64) ? (int64)ulliBits : ((int64)(ulliBits << (64 - iBits)) >> (64 - iBits));
}


//////////////////////////////////////////////////////////////////////////
//
// True if a signed add, sub, mul, neg or shl of a and b, iBits wide, that
// gave ulliResult (IRFoldInteger's) overflows: its exact value isn't one
// of the type's, which is undefined behaviour, so no constant.  A left
// shift's left operand mustn't be negative, and what it makes must fit the
// unsigned type as wide.  [expr] 4, [expr.shift] 2
//
//////////////////////////////////////////////////////////////////////////
bool IRSignedOverflows(uint16 uiOpcode, int iBits, uint64 a, uint64 b, uint64 ulliResult)
{
	int64	sa;
	int64	sb;
	int64	sr;
	int64	lliLeast;

	sa = IRSignExtendBits(iBits, a);
	sb = IRSignExtendBits(iBits, b);
	sr = IRSignExtendBits(iBits, ulliResult);
	lliLeast = (int64)(((uint64)1) << 63) >> (64 - iBits);
	switch (uiOpcode)
	{
	case IRO_Add:
		return ((sa ^ sr) & (sb ^ sr)) < 0;
	case IRO_Sub:
		return ((sa ^ sb) & (sa ^ sr)) < 0;
	case IRO_Neg:
		return sa == lliLeast;
	case IRO_Mul:
		if (iBits <= 32)
		{
			return (sa * sb) != sr;
		}
		if ((sa == 0) || (sb == 0))
		{
			return false;
		}
		if (((sa == -1) && (sb == lliLeast)) || ((sb == -1) && (sa == lliLeast)))
		{
			return true;
		}
		return (sr / sa) != sb;
	case IRO_Shl:
		if (sa < 0)
		{
			return true;
		}
		return (b > 0) && (b < (uint64)iBits) && (((a & ((iBits >= 64) ? ~(uint64)0 : ((((uint64)1) << iBits) - 1))) >> (iBits - (int)b)) != 0);
	default:
		return false;
	}
}
