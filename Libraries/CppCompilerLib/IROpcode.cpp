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
#define IR_SWAP_LT	IROP_SwappedLess


//Every column but an integer comparison's, which are 0: IR_INTEGER_COMPARE has them.
#define IR_OPCODE(name, shape, types, sourcetypes, first, second, result, flags, properties)	{ name, shape, types, sourcetypes, { first, second }, result, flags, properties, 0, 0, 0 }
#define IR_INTEGER_OP(name, properties)		IR_OPCODE(name, IRSH_Operands, IRTM_I1 | IRTM_Integer, 0, IROK_Same, IROK_Same, IRRS_Same, 0, properties)
#define IR_INTEGER_UNARY(name)				IR_OPCODE(name, IRSH_Operands, IRTM_I1 | IRTM_Integer, 0, IROK_Same, IROK_None, IRRS_Same, 0, IR_PURE)
#define IR_INTEGER_COMPARE(name, properties, inverted, swapped, holds)	{ name, IRSH_Operands, IRTM_I1 | IRTM_Integer | IRTM_Pointer, 0, { IROK_Same, IROK_Same }, IRRS_I1, 0, (properties) | IROP_IntegerComparison, inverted, swapped, holds }
#define IR_FLOAT_OP(name, properties)		IR_OPCODE(name, IRSH_Operands, IRTM_Float, 0, IROK_Same, IROK_Same, IRRS_Same, 0, properties)
#define IR_FLOAT_COMPARE(name, properties)	IR_OPCODE(name, IRSH_Operands, IRTM_Float, 0, IROK_Same, IROK_Same, IRRS_I1, 0, properties)
#define IR_DIVIDE_OP(name)					IR_OPCODE(name, IRSH_Operands, IRTM_I1 | IRTM_Integer, 0, IROK_Same, IROK_Same, IRRS_Same, IRF_Hot, IR_TRAPS)
#define IR_CONVERSION(name, to, from)		IR_OPCODE(name, IRSH_Conversion, to, from, IROK_Source, IROK_None, IRRS_Same, 0, IR_PURE)


SIROpcode	gasIROpcodes[IRO_NumGeneric] =
{
	IR_OPCODE("", IRSH_Operands, 0, 0, IROK_None, IROK_None, IRRS_None, 0, 0),

	IR_INTEGER_OP("add", IR_SWAPS),
	IR_INTEGER_OP("sub", IR_PURE),
	IR_INTEGER_OP("mul", IR_SWAPS),
	IR_DIVIDE_OP("sdiv"),
	IR_DIVIDE_OP("udiv"),
	IR_DIVIDE_OP("srem"),
	IR_DIVIDE_OP("urem"),
	IR_INTEGER_OP("and", IR_SWAPS),
	IR_INTEGER_OP("or", IR_SWAPS),
	IR_INTEGER_OP("xor", IR_SWAPS),
	IR_INTEGER_OP("shl", IR_PURE),
	IR_INTEGER_OP("lshr", IR_PURE),
	IR_INTEGER_OP("ashr", IR_PURE),
	IR_INTEGER_UNARY("neg"),
	IR_INTEGER_UNARY("not"),
	IR_OPCODE("copy", IRSH_Operands, IRTM_I1 | IRTM_Memory, 0, IROK_Same, IROK_None, IRRS_Same, 0, IR_PURE),
	IR_CONVERSION("sext", IRTM_Integer, IRTM_I1 | IRTM_Integer),
	IR_CONVERSION("zext", IRTM_Integer, IRTM_I1 | IRTM_Integer),
	IR_CONVERSION("trunc", IRTM_I1 | IRTM_Integer, IRTM_Integer),

	//                 name    properties               inverted  swapped   holds when the left operand is
	IR_INTEGER_COMPARE("eq",   IR_SWAPS,                IRO_Ne,   IRO_Eq,   IRCH_Equal),
	IR_INTEGER_COMPARE("ne",   IR_SWAPS,                IRO_Eq,   IRO_Ne,   IRCH_Less | IRCH_Greater),
	IR_INTEGER_COMPARE("slt",  IR_SIGNED,               IRO_SGe,  IRO_SGt,  IRCH_Less),
	IR_INTEGER_COMPARE("sle",  IR_SIGNED | IR_SWAP_LT,  IRO_SGt,  IRO_SGe,  IRCH_Less | IRCH_Equal),
	IR_INTEGER_COMPARE("sgt",  IR_SIGNED | IR_SWAP_LT,  IRO_SLe,  IRO_SLt,  IRCH_Greater),
	IR_INTEGER_COMPARE("sge",  IR_SIGNED,               IRO_SLt,  IRO_SLe,  IRCH_Greater | IRCH_Equal),
	IR_INTEGER_COMPARE("ult",  IR_PURE,                 IRO_UGe,  IRO_UGt,  IRCH_Less),
	IR_INTEGER_COMPARE("ule",  IR_PURE | IR_SWAP_LT,    IRO_UGt,  IRO_UGe,  IRCH_Less | IRCH_Equal),
	IR_INTEGER_COMPARE("ugt",  IR_PURE | IR_SWAP_LT,    IRO_ULe,  IRO_ULt,  IRCH_Greater),
	IR_INTEGER_COMPARE("uge",  IR_PURE,                 IRO_ULt,  IRO_ULe,  IRCH_Greater | IRCH_Equal),

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

	IR_OPCODE("slot", IRSH_Operands, IRTM_Pointer, 0, IROK_Constant, IROK_Constant, IRRS_Same, 0, IR_PURE),
	IR_OPCODE("argaddr", IRSH_Operands, IRTM_Pointer, 0, IROK_Constant, IROK_None, IRRS_Same, 0, IR_PURE),
	IR_OPCODE("ptradd", IRSH_Operands, IRTM_Pointer, 0, IROK_Same, IROK_Offset, IRRS_Same, IRF_InBank | IRF_InArray, IR_PURE),
	IR_CONVERSION("ptrtoi", IRTM_Integer, IRTM_Pointer),
	IR_CONVERSION("itoptr", IRTM_Pointer, IRTM_Integer),
	IR_CONVERSION("spacecast", IRTM_Pointer, IRTM_Pointer),
	IR_OPCODE("load", IRSH_Operands, IRTM_Memory, 0, IROK_Pointer, IROK_None, IRRS_Same, IRF_Volatile, 0),
	IR_OPCODE("store", IRSH_Operands, IRTM_Memory, 0, IROK_Same, IROK_Pointer, IRRS_None, IRF_Volatile, 0),
	IR_OPCODE("arg", IRSH_Operands, IRTM_Memory, 0, IROK_Same, IROK_None, IRRS_None, 0, 0),
	IR_OPCODE("call", IRSH_Call, IRTM_Memory | IRTM_Void, 0, IROK_Pointer, IROK_None, IRRS_Optional, IRF_ThisCall | IRF_Variadic, 0),
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
int SIROpcode::NumOperands(void)
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
static uint8 IRProperties(uint16 uiOpcode)
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
	return (IRProperties(uiOpcode) & IROP_Signed) != 0;
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
	return (uiOpcode == IRO_Trunc) || (uiOpcode == IRO_SExt) || (uiOpcode == IRO_ZExt);
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
