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
#ifndef __I_R_BLOCK_H__
#define __I_R_BLOCK_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/Chars.h"
#include "IRTypes.h"
#include "IROpcode.h"


//A fixed size instruction, as QBE's Ins.  sType is the result's type, except for comparisons (the operands' type;
//the result is i1), store and arg (the value's type).  A conversion's operand type is its operand's.  A call's
//callee is asArguments[0]; its arguments are the arg instructions straight before it.
struct SIRInstruction
{
	uint16			uiOpcode;
	uint8			uiFlags;
	uint8			uiNamed;		//A variadic call's number of named arguments.
	SIRType			sType;
	SIRReference	sResult;
	SIRReference	asArguments[2];
};


class CIRBlock;


struct SIRPhiArgument
{
	CIRBlock*		pcBlock;
	SIRReference	sValue;
};


typedef CArrayTemplate<SIRPhiArgument>	CArrayIRPhiArguments;


struct SIRPhi
{
	SIRReference			sResult;
	SIRType					sType;
	CArrayIRPhiArguments	asArguments;
};


enum EIRJumpKind
{
	IRJ_None,
	IRJ_Jump,
	IRJ_Branch,
	IRJ_Return,
	IRJ_Unreachable,
};


struct SIRJump
{
	uint8			uiKind;
	SIRType			sType;			//A return's value type, IRTK_None for ret.
	SIRReference	sArgument;		//A branch's condition or a return's value.
	CIRBlock*		pcThen;			//A jump's target or a branch's target when the condition is 1.
	CIRBlock*		pcElse;
};


typedef CArrayTemplate<SIRInstruction>	CArrayIRInstructions;


SIRReference	IRAddressOf(SIRInstruction* psInstruction);
typedef CArrayTemplate<SIRPhi>			CArrayIRPhis;
typedef CArrayTemplatePtr<CIRBlock>		CArrayIRBlocks;


//A basic block: phis, instructions and the jump that ends it.  Predecessors are worked out from the jumps by
//CIRFunction::ComputePredecessors.  An unnamed block (mszName empty) is numbered when printed.
class CIRBlock
{
public:
	CChars					mszName;
	CArrayIRPhis			masPhis;
	CArrayIRInstructions	masInstructions;
	SIRJump					msJump;
	CArrayIRBlocks			mapcPredecessors;
	size					muiIndex;			//Position in the function, set by CIRFunction::Renumber.

public:
	void			Init(const char* szName = NULL);
	void			Kill(void);

	char*			GetName(void);

	SIRInstruction*	AddInstruction(uint16 uiOpcode, SIRType sType, SIRReference sResult, SIRReference sArgument0, SIRReference sArgument1);
	SIRPhi*			AddPhi(SIRType sType, SIRReference sResult);
	void			AddPhiArgument(SIRPhi* psPhi, CIRBlock* pcBlock, SIRReference sValue);
	SIRPhiArgument*	FindPhiArgument(SIRPhi* psPhi, CIRBlock* pcBlock);

	void			Jump(CIRBlock* pcTarget);
	void			Branch(SIRReference sCondition, CIRBlock* pcThen, CIRBlock* pcElse);
	void			Return(SIRType sType, SIRReference sValue);
	void			Return(void);
	void			Unreachable(void);
	bool			HasJump(void);

	size			NumSuccessors(void);
	CIRBlock*		GetSuccessor(size uiIndex);
	bool			HasPredecessor(CIRBlock* pcBlock);
	bool			Defines(SIRReference sValue);

protected:
	void			SetJump(EIRJumpKind eKind, SIRType sType, SIRReference sArgument, CIRBlock* pcThen, CIRBlock* pcElse);
};


#endif // __I_R_BLOCK_H__
