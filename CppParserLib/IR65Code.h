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
#ifndef __I_R_65_CODE_H__
#define __I_R_65_CODE_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/MapStringInt.h"


#define IR65_CODE_REGISTERS		4		//A, B, X and Y.
#define IR65_CODE_ALL			0x0F	//A use bit per register.
#define IR65_WIDTH_CONFLICT		0xFF	//Widths that differ between the ways a line is reached.
#define IR65_WIDTH_UNKNOWN		0xFE	//A width a call (to a runtime routine) leaves, until it is set.

//The flags, as P's bits.
#define IR65_FLAG_C				0x01
#define IR65_FLAG_Z				0x02
#define IR65_FLAG_V				0x04
#define IR65_FLAG_N				0x08
#define IR65_FLAGS_ALL			0x0F


//What a line of a function's code is.
enum EIR65CodeKind
{
	IR65CK_Other,		//Blank, or anything that isn't run.
	IR65CK_Label,
	IR65CK_Alu,			//cpy, add, sub, and, or, xor, shl, shr, cmp: op first, second.
	IR65CK_Negate,		//neg first {, second}.
	IR65CK_Push,		//psh registers, memory or a value.
	IR65CK_Pull,		//pul registers or memory.
	IR65CK_Width,		//wth and wtth.
	IR65CK_Extend,		//wthx.
	IR65CK_Multiply,	//mul: A x B -> Y.
	IR65CK_Divide,		//div: Y / A -> Y, remainder in B.
	IR65CK_Call,		//jsr.
	IR65CK_Return,		//ret and stp.
	IR65CK_Branch,		//bras, brac, brasa and braso: may fall through.
	IR65CK_Jump,		//bra.
};


//A line of a function's code, decoded.  Registers are 0 to 3 for A, B, X and Y, -1 for none (or S, V, W).
struct SIR65CodeLine
{
	uint8	uiKind;
	int8	iFirst;			//The first operand's register.
	int8	iSecond;		//The second operand's register.
	bool	bFirstRegister;	//The first operand is a register (A, B, X, Y, S, V or W): it sets the operation's width.
	bool	bFirstMemory;	//The first operand is memory (an Alu or pul writing it).
	bool	bReadsFirst;	//An Alu reading its first operand (all but cpy).
	bool	bWritesFirst;	//An Alu writing its first operand (all but cmp).
	bool	bShift;			//shl or shr: the second operand is read at its own width.
	bool	bResultFlags;	//n and z are the value written (cpy, and, or, xor, add, sub, neg).
	uint8	uiRegisters;	//A bit per register: psh and pul lists, and wth and wtth.
	uint8	uiAddresses;	//A bit per register used in a memory operand's address.
	uint8	uiBits;			//wth, wtth and wthx.
	uint8	uiFlagsRead;	//IR65_FLAG_ bits a branch reads.
	uint8	uiFlagsWritten;	//IR65_FLAG_ bits the instruction sets.
	int32	iTarget;		//A branch or jump's label's line.
};


typedef CArrayTemplate<SIR65CodeLine>	CArrayIR65CodeLines;


//The register widths (8, 16, 32 or 64 bits; 0 for not known yet) or live bits (CIR65Code::Bands) of A, B, X and Y.
struct SIR65Widths
{
	uint8	auiRegisters[IR65_CODE_REGISTERS];
};


typedef CArrayTemplate<SIR65Widths>	CArrayIR65Widths;


//A function's 65C64 code as the back end wrote it (from its label on), decoded line by line for the passes over
//the text (CIR65Flags, CIR65Widths), with the widths each line runs at.  Load fails for code with an instruction
//it doesn't know, a branch out of the function, or a register used at a width that depends on the way the line was
//reached; the passes leave such code as it is.
class CIR65Code
{
protected:
	CArrayChars				maszLines;
	CArrayIR65CodeLines		masLines;
	CArrayIR65Widths		masWidths;		//Per line: the widths it starts with.
	CArrayTemplate<uint8>	mauiReached;	//Per line: 1 if it can run.
	CArrayTemplate<uint8>	mauiJumpedBack;	//Per line: 1 for a label a branch after it goes to.
	CArrayTemplate<uint8>	mauiRemoved;	//Per line: 1 for a line a pass took out (now IR65CK_Other).
	CMapStringInt			mmLabels;		//Label name to line.

public:
	void			Init(void);
	void			Kill(void);

	bool			Load(CChars* pszCode, size uiStart, bool bConflicts);

	size			NumLines(void);
	SIR65CodeLine*	GetLine(size uiLine);
	CChars*			GetText(size uiLine);
	SIR65Widths*	GetWidths(size uiLine);
	bool			IsReached(size uiLine);
	bool			IsJumpedBack(size uiLine);
	bool			FallsThrough(size uiLine);
	void			Effect(size uiLine, SIR65Widths* psWidths, SIR65Widths* psReads, SIR65Widths* psWrites, uint8* puiUses);
	void			ComputeFlagDemand(CArrayTemplate<uint8>* pauiDemand);
	void			Remove(size uiLine);
	bool			IsRemoved(size uiLine);
	bool			Replace(size uiLine, const char* szInstruction);
	void			WriteLine(CChars* pszOut, size uiLine);

	static uint8	Bands(uint8 uiBits);
	static char		RegisterName(int iRegister);
	static int		RegisterIndex(const char* sz, size uiLength);
	static bool		IsOwnMemory(const char* szOperand, size uiLength, uint8* puiBase, int32* piOffset);
	static bool		IsCopyExtendAddress(CChars* pszOperand);

protected:
	bool			Decode(void);
	bool			DecodeLine(CChars* pszLine, SIR65CodeLine* psLine);
	bool			ComputeWidths(void);
};


#endif // __I_R_65_CODE_H__
