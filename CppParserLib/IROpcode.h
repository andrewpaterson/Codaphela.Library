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
#define IRF_Volatile	0x01		//load, store
#define IRF_InBank		0x02		//ptradd: the result is in the pointer's 64KB bank
#define IRF_ThisCall	0x04		//call: the first argument is a member function's this (in X on the 65C64)
#define IRF_Variadic	0x08		//call: uiNamed is the number of named arguments
#define IRF_NumWordFlags	3


struct SIROpcode
{
	const char*	szName;
	uint8		uiShape;
	uint8		uiTypes;			//IRTM_ set of the instruction's type.
	uint8		uiSourceTypes;		//IRTM_ set of a conversion's operand type.
	uint8		auiOperands[2];		//EIROperandKind.
	uint8		uiResult;			//EIRResultKind.
	uint8		uiFlags;			//The IRF_ flags allowed.

	int			NumOperands(void);
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


#endif // __I_R_OPCODE_H__
