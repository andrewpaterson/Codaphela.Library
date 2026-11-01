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
#ifndef __I_R_65_PEEPHOLE_H__
#define __I_R_65_PEEPHOLE_H__
#include "IR65Code.h"


#define IR65_VALUE_UNKNOWN		-1		//A register holding nothing known.
#define IR65_VALUE_UNSET		-2		//A line not reached yet.


//What A, B, X and Y hold (32 bits of): an index into CIR65Peephole's values, or IR65_VALUE_.
struct SIR65Values
{
	int16	aiValues[IR65_CODE_REGISTERS];
};


typedef CArrayTemplate<SIR65Values>	CArrayIR65Values;


//What a value is: an operand a register was copied from (or stored to).
struct SIR65Value
{
	uint8	uiNames;		//A bit per register it names: IR65_CODE_ALL's four, then S, V and W.
	uint8	uiBase;			//For memory, the register the offset is from: 'W', 'V' or 'S'; else 0.
	int32	iOffset;		//For memory, the offset.
};


typedef CArrayTemplate<SIR65Value>	CArrayIR65Value;


//The last clean up of a function's finished 65C64 code (IR.txt 65C64 back end, Peephole), after the Flags and
//Widths passes have added their own code: a cpy of a value the register already holds goes (registers,
//constants, and the function's own memory, which is never volatile), a jump to a jump goes straight to where
//that one goes, a bra to a ret is a ret, a branch round a bra is the opposite branch to the bra's label, a branch
//to the next line goes and so does code nothing reaches.
class CIR65Peephole
{
protected:
	CIR65Code				mcCode;
	CArrayTemplate<uint8>	mauiReached;	//Per line: 1 if it can run.
	CArrayChars				maszValues;		//The operands registers hold, as written.
	CArrayIR65Value			masValues;		//What each of them is.
	CArrayIR65Values		masStates;		//Per line: what the registers hold as it starts.
	CArrayTemplate<int16>	maiCopied;		//Per line: the value a cpy to a register (or a store from one) copies.
	CArrayTemplate<uint8>	mauiDemand;		//Per line: the flags read after it before they are set.

public:
	void	Init(void);
	void	Kill(void);

	void	Optimise(CChars* pszCode, size uiStart);

protected:
	bool	RemoveCopies(void);
	int16	AddValue(const char* szOperand, size uiLength);
	void	ComputeCopied(void);
	void	ComputeStates(void);
	void	Transfer(size uiLine, SIR65Values* psState);
	void	Forget(SIR65Values* psState, uint8 uiNames);
	void	ForgetMemory(SIR65Values* psState, const char* szOperand, size uiLength);
	int16	Canonical(SIR65Values* psState, int16 iValue);

	size	NextInstruction(size uiLine);
	bool	Retarget(size uiLine, const char* szMnemonic, size uiTarget);
	bool	ThreadJumps(void);
	bool	ReturnDirectly(void);
	bool	InvertBranches(void);
	bool	RemoveBranchesToNext(void);
	bool	RemoveUnreached(void);
};


#endif // __I_R_65_PEEPHOLE_H__

