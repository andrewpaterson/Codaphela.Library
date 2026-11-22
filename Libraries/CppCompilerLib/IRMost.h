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
#ifndef __I_R_MOST_H__
#define __I_R_MOST_H__
#include "IRFunction.h"


#define IR_MOST_GROWTH			4		//A phi that rises more often than this goes to its type's largest (then narrows).
#define IR_MOST_NARROWING		4		//The rounds that narrow what the widened phis allow.
#define IR_MOST_DEPTH			8		//The most blocks a bound from a branch is looked for above a use.


//The rules for the largest an integer value can be, read as unsigned (so how many of its low bits may be set): what
//each opcode gives from the largest its operands can be, and what a branch says of a value on one of its edges.
//What asks says how large an operand is: CIRMost, from the table it is filling.
class CIRMostRules
{
public:
	uint64					InstructionMost(CIRFunction* pcFunction, SIRInstruction* psInstruction, CIRBlock* pcBlock);
	uint64					EdgeMost(SIRReference sValue, CIRBlock* pcFrom, CIRBlock* pcTo, uint64 ulliMost);
	static uint64			FullMost(SIRType sType);

protected:
	virtual uint64			Most(SIRReference sValue) = 0;
	virtual uint64			MostAt(SIRReference sValue, CIRBlock* pcBlock);
	virtual SIRInstruction*	Definition(SIRReference sValue) = 0;
	bool					Names(SIRReference sOperand, SIRReference sValue);
	uint64					OperandMost(SIRReference sOperand, CIRBlock* pcBlock);
};


//The largest each integer temporary of a function can be, read as unsigned, and the largest the instruction that
//defines it works out before its result is truncated to its type: an operation done in a 32 bit register (64 for
//i64) on operands held zero extended.  A back end that keeps narrow values zero extended needn't mask a result
//whose raw largest fits its type.  A use below a branch on a comparison (through blocks that have only the one
//predecessor) is bounded by it, and so is a phi's value on the edge of a branch (a loop's counter below its bound:
//i < n, so i + 1 <= n).  Phis are worked to a fixed point: one that keeps rising (a counter) is widened to its
//type's largest, then each value is worked out again a few times, which only lowers them.
class CIRMost : public CIRMostRules
{
protected:
	CIRFunction*					mpcFunction;
	CArrayTemplate<uint64>			maulliMost;			//Per temporary.
	CArrayTemplate<uint64>			maulliRawMost;		//Per temporary defined by an instruction.
	CArrayTemplate<uint8>			mauiGrowth;			//Per temporary: the times a phi rose.
	CArrayTemplate<SIRInstruction*>	mapsDefinitions;	//Per temporary: the instruction that defines it, or NULL.
	CArrayTemplate<CIRBlock*>		mapcBlocks;			//Per temporary: the block of its instruction.

public:
	void	Init(void);
	void	Kill(void);

	void	Compute(CIRFunction* pcFunction);

	uint64	Most(SIRReference sValue) override;
	uint64	RawMost(SIRReference sTemporary);
	bool	FitsType(SIRReference sTemporary);

protected:
	bool			Round(bool bNarrowing);
	uint64			MostAt(SIRReference sValue, CIRBlock* pcBlock) override;
	uint64			MostOnEdge(SIRReference sValue, CIRBlock* pcFrom, CIRBlock* pcTo);
	SIRInstruction*	Definition(SIRReference sValue) override;
};


#endif // __I_R_MOST_H__
