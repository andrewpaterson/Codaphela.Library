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
#ifndef __I_R_65_WIDTHS_H__
#define __I_R_65_WIDTHS_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/MapStringInt.h"


#define IR65_WIDTH_REGISTERS	4		//A, B, X and Y.


//What a line of a function's code is, for CIR65Widths.
enum EIR65WidthKind
{
	IR65WK_Other,		//Blank, or anything that isn't run.
	IR65WK_Label,
	IR65WK_Alu,			//cpy, add, sub, and, or, xor, shl, shr, cmp: op first, second.
	IR65WK_Negate,		//neg first {, second}.
	IR65WK_Push,		//psh registers, memory or a value.
	IR65WK_Pull,		//pul registers or memory.
	IR65WK_Width,		//wth and wtth.
	IR65WK_Extend,		//wthx.
	IR65WK_Multiply,	//mul: A x B -> Y.
	IR65WK_Divide,		//div: Y / A -> Y, remainder in B.
	IR65WK_Call,		//jsr.
	IR65WK_Return,		//ret and stp.
	IR65WK_Branch,		//bras, brac, brasa and braso: may fall through.
	IR65WK_Jump,		//bra.
};


//A line of a function's code, decoded.  Registers are 0 to 3 for A, B, X and Y, -1 for none (or S, V, W).
struct SIR65WidthLine
{
	uint8	uiKind;
	int8	iFirst;			//The first operand's register.
	int8	iSecond;		//The second operand's register.
	bool	bFirstRegister;	//The first operand is a register (A, B, X, Y, S, V or W): it sets the operation's width.
	bool	bReadsFirst;	//An Alu reading its first operand (all but cpy).
	bool	bWritesFirst;	//An Alu writing its first operand (all but cmp).
	bool	bShift;			//shl or shr: the second operand is read at its own width.
	uint8	uiRegisters;	//A bit per register: psh and pul lists, and wth and wtth.
	uint8	uiAddresses;	//A bit per register used in a memory operand's address.
	uint8	uiBits;			//wth, wtth and wthx.
	int32	iTarget;		//A branch or jump's label's line.
};


typedef CArrayTemplate<SIR65WidthLine>	CArrayIR65WidthLines;


//The register widths (8, 16, 32 or 64 bits; 0 for not known yet) or live bits (IR65WidthBands) of A, B, X and Y.
struct SIR65Widths
{
	uint8	auiRegisters[IR65_WIDTH_REGISTERS];
};


typedef CArrayTemplate<SIR65Widths>	CArrayIR65Widths;


//Removes width changes from a function's 65C64 code (IR.txt 65C64 back end, Widths).  The back end writes each
//instruction with A, B, X and Y 32 bits wide before and after it, so a narrow load is "wth X, 8 / cpy X, [m] /
//wthx X, 32" even when only the low byte is ever read.  A wthx whose extended bits are never read becomes a wth,
//then each register's width is only set straight before an instruction that uses the register, and only when it
//isn't already that width.  A label starts with the widths its code first needs (so a loop keeps a narrow
//register narrow), and each jump to it sets them first.  Calls and returns keep the 32 bit convention.
class CIR65Widths
{
protected:
	CArrayChars				maszLines;
	CArrayIR65WidthLines	masLines;
	CArrayIR65Widths		masWidths;		//Per line: the widths it starts with.
	CArrayIR65Widths		masLive;		//Per line: the bits read from its start before they are written (IR65WidthBands).
	CArrayIR65Widths		masEntries;		//Per line: a label's widths as the new code reaches it (0 unknown or undecided).
	CArrayTemplate<uint8>	mauiReached;	//Per line: 1 if it can run.
	CArrayTemplate<uint8>	mauiJumpedBack;	//Per line: 1 for a label a branch after it goes to.
	CArrayTemplate<uint8>	mauiArrived;	//Per line: 1 for a label the new code has reached (Arrive).
	CMapStringInt			mmLabels;		//Label name to line.
	SIR65Widths				msActual;		//The widths the new code has set.
	CChars*					mpszOut;

public:
	void	Init(void);
	void	Kill(void);

	void	Optimise(CChars* pszCode, size uiStart);

protected:
	bool	Decode(void);
	bool	DecodeLine(CChars* pszLine, SIR65WidthLine* psLine);
	void	Effect(SIR65WidthLine* psLine, SIR65Widths* psWidths, SIR65Widths* psReads, SIR65Widths* psWrites, uint8* puiUses);
	bool	FallsThrough(SIR65WidthLine* psLine);
	bool	ComputeWidths(void);
	void	ComputeLive(void);
	void	LiveOut(size uiLine, SIR65Widths* psOut);
	bool	RemoveExtensions(void);
	uint8	NextNeed(size uiFirst, int iRegister);
	void	Anticipate(size uiLabel, size uiFallThrough);
	void	DecideEntry(size uiLabel, bool bFromActual);
	void	Arrive(size uiLabel);
	void	SetWidths(SIR65Widths* psTarget, size uiNext);
	void	Write(void);
	void	WriteLine(size uiLine);
};


#endif // __I_R_65_WIDTHS_H__
