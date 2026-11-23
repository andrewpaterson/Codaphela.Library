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
#ifndef __I_R_OPCODE_H__
#define __I_R_OPCODE_H__
#include "IRTypes.h"


//The generic (mid level) opcodes.  A target's opcodes come after IRO_NumGeneric (CIRModule::SetTargetOpcodes).
enum EIROpcode
{
	IRO_None,

	IRO_Add,
	IRO_Sub,
	IRO_Mul,
	IRO_SDiv,
	IRO_UDiv,
	IRO_SRem,
	IRO_URem,
	IRO_And,
	IRO_Or,
	IRO_Xor,
	IRO_Shl,
	IRO_LShr,
	IRO_AShr,
	IRO_Neg,
	IRO_Not,
	IRO_Copy,
	IRO_SExt,
	IRO_ZExt,
	IRO_Trunc,

	IRO_Eq,
	IRO_Ne,
	IRO_SLt,
	IRO_SLe,
	IRO_SGt,
	IRO_SGe,
	IRO_ULt,
	IRO_ULe,
	IRO_UGt,
	IRO_UGe,

	IRO_FAdd,
	IRO_FSub,
	IRO_FMul,
	IRO_FDiv,
	IRO_FNeg,
	IRO_FEq,
	IRO_FNe,
	IRO_FLt,
	IRO_FLe,
	IRO_FGt,
	IRO_FGe,
	IRO_FOrdered,
	IRO_FUnordered,
	IRO_SIToF,
	IRO_UIToF,
	IRO_FToSI,
	IRO_FToUI,
	IRO_FExt,
	IRO_FTrunc,

	IRO_Slot,
	IRO_ArgAddress,
	IRO_PtrAdd,
	IRO_PtrToI,
	IRO_IToPtr,
	IRO_SpaceCast,
	IRO_Load,
	IRO_Store,
	IRO_Arg,
	IRO_Call,
	IRO_Thrown,			//%t = thrown i1, straight after a call: whether the call was left with an exception.

	IRO_NumGeneric
};


//What an operand must be.
enum EIROperandKind
{
	IROK_None,
	IROK_Same,			//The instruction's type.
	IROK_Pointer,		//Any pointer.
	IROK_Offset,		//An integer the size of the instruction's pointer type.
	IROK_Constant,		//An i32 constant.
	IROK_Source,		//A conversion's operand: one of the opcode's source types.
};


//What the result is.
enum EIRResultKind
{
	IRRS_None,
	IRRS_Same,			//The instruction's type.
	IRRS_I1,
	IRRS_Optional,		//A call: the instruction's type, or no result.
};


//How an instruction is written.
enum EIRShape
{
	IRSH_Operands,		//{%r = }op{ flags} T a{, b}
	IRSH_Conversion,	//%r = op S a to T
	IRSH_Call,			//{%r = }call T f(T a, ...)
};


//Instruction flags.  Variadic is written as ... in a call, the others as words after the opcode.
#define IRF_Volatile	0x01		//load, store; slot: a variable a debugger reads, its loads and stores volatile (CIRFunction::Slot)
#define IRF_InBank		0x02		//ptradd: the result is in the pointer's 64KB bank
#define IRF_ThisCall	0x04		//call: the first argument is a member function's this (in X on the 65C64)
#define IRF_Variadic	0x08		//call: uiNamed is the number of named arguments
#define IRF_Hot			0x10		//sdiv, udiv, srem, urem: the profile found it hot (a target may spend memory on it)
#define IRF_InArray		0x20		//ptradd: the result is in an array of under 64KB that starts at the pointer, so the offset
									//and the bytes read or written through it stay under 64KB
#define IRF_NumWordFlags	5
#define IRF_NoSignedWrap	0x80		//add, sub, mul, neg, shl in a constant's IR: signed, so an overflow means it isn't a
									//constant (IRSignedOverflows).  Not written.
#define IRF_FullExpression	0x40		//any: ends a full-expression in a constant's IR (CIRInterpreter counts them).  Not
									//written, and allowed on every opcode.


//What optimising may do with an opcode's instructions.  A target's opcodes have none, so are left alone.
#define IROP_Pure			0x01		//No effect and reads no memory: removed when unused, worked out once, moved.
#define IROP_Commutative	0x02		//a op b is b op a.
#define IROP_MayTrap		0x04		//Fails on some operands (division by 0): only moved where it would be done anyway.
#define IROP_IntegerComparison	0x08	//eq, ne and the signed and unsigned orderings: they have the comparison columns.
#define IROP_Signed			0x10		//An integer comparison or a division of signed operands...
#define IROP_SwappedLess	0x20		//sgt, sle, ugt and ule: a less than comparison with the operands swapped.
#define IROP_Unsigned		0x40		//...and one of unsigned operands.  eq and ne are neither: they are the same for both.
#define IROP_IntegerArithmetic	0x80	//add, sub, mul, and, or, xor, the shifts, neg and not: integers in, an integer out, no trap.
#define IROP_LowBits		0x100		//add, sub, mul, and, or and xor: the result's low bits come of the operands' low bits alone.
#define IROP_Division		0x200		//sdiv, udiv, srem and urem...
#define IROP_Remainder		0x400		//...and of them srem and urem.
#define IROP_IntegerConversion	0x800	//trunc, sext and zext: an integer to an integer of another width.
#define IROP_SignedWrapping	0x1000		//add, sub, mul, neg and shl: a signed overflow of one is undefined behaviour (IRF_NoSignedWrap).


//The orders of an integer comparison's operands it holds for (SIROpcode::uiHolds).
#define IRCH_Less			0x01
#define IRCH_Equal			0x02
#define IRCH_Greater		0x04
#define IRCH_All			(IRCH_Less | IRCH_Equal | IRCH_Greater)


struct SIROpcode
{
	const char*	szName;
	uint8		uiShape;
	uint8		uiTypes;			//IRTM_ set of the instruction's type.
	uint8		uiSourceTypes;		//IRTM_ set of a conversion's operand type.
	uint8		auiOperands[2];		//EIROperandKind.
	uint8		uiResult;			//EIRResultKind.
	uint8		uiFlags;			//The IRF_ flags allowed.
	uint16		uiProperties;		//IROP_.

	//An integer comparison's columns.
	uint8		uiInverted;			//The comparison that is true when this one is false.
	uint8		uiSwapped;			//The comparison of the operands the other way round.
	uint8		uiHolds;			//IRCH_ set of the orders it holds for.

	//A signed comparison's and a signed division's.
	uint8		uiUnsigned;			//The opcode that does the same with unsigned operands: slt's is ult, sdiv's udiv.

	//A division's.
	uint8		uiPartner;			//The remainder of the same division, and a remainder's quotient: sdiv's is srem.

	int			NumOperands(void) const;
};


struct SIRFlagName
{
	uint8		uiFlag;
	const char*	szName;
};


extern SIRFlagName	gasIRFlagNames[IRF_NumWordFlags];


SIROpcode*	GetIROpcode(uint16 uiOpcode);
uint16		FindIROpcode(SIROpcode* pasOpcodes, uint16 uiNumOpcodes, uint16 uiFirst, const char* szName);
bool		IsIRIntegerComparison(uint16 uiOpcode);
bool		IsIRSignedComparison(uint16 uiOpcode);
bool		IsIRSwappedComparison(uint16 uiOpcode);
uint16		IRInvertedComparison(uint16 uiOpcode);
uint16		IRSwappedComparison(uint16 uiOpcode);
uint8		IRComparisonHolds(uint16 uiOpcode);
uint16		IRSignedness(uint16 uiOpcode);
uint16		IRUnsignedOpcode(uint16 uiOpcode);
bool		IRCompareIntegers(uint16 uiOpcode, SIRType sType, uint64 ulliLeft, uint64 ulliRight);
bool		IsIRIntegerConversion(uint16 uiOpcode);
bool		IsIRPure(uint16 uiOpcode);
bool		IsIRMayTrap(uint16 uiOpcode);
bool		IsIRCommutative(uint16 uiOpcode);
bool		IRFoldInteger(uint16 uiOpcode, SIRType sType, SIRType sFrom, int iBits, uint64 a, uint64 b, uint64* pulliResult);
bool		IRSignedOverflows(uint16 uiOpcode, int iBits, uint64 a, uint64 b, uint64 ulliResult);
bool		IsIRSignedWrapping(uint16 uiOpcode);
bool		IsIRIntegerArithmetic(uint16 uiOpcode);
bool		IsIRLowBits(uint16 uiOpcode);
bool		IsIRDivision(uint16 uiOpcode);
bool		IsIRSignedDivision(uint16 uiOpcode);
bool		IsIRUnsignedDivision(uint16 uiOpcode);
bool		IsIRRemainder(uint16 uiOpcode);
bool		IsIRQuotient(uint16 uiOpcode);
uint16		IRDivisionOpcode(bool bSigned, bool bRemainder);
uint16		IRDivisionPartner(uint16 uiOpcode);
bool		IRResultIsI1(uint16 uiOpcode);


#endif // __I_R_OPCODE_H__
