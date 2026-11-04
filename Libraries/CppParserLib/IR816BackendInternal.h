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
#ifndef __IR816_BACKEND_INTERNAL_H__
#define __IR816_BACKEND_INTERNAL_H__
#include "IR816Backend.h"


//What the W65C816 back end's files (IR816Backend.cpp and the others IR816*.cpp) share: definitions, the
//assembly text's decoded lines, and the functions one uses of another's.


#define IR816_KEEP_IN_A	" ;A"		//Ends a store Peephole may leave the value in A for (Store).
#define IR816_ROUND		" ;R"		//Ends a bra round a loop (JumpTo), for PeepholeBranches.
#define IR816_SHIFT_STEPS	7			//The one bit steps a variable shift jumps into, after its byte and word moves.


//Every runtime routine's name starts with IR816_RUNTIME_PREFIX, IR816_RUNTIME_PREFIX_LENGTH characters; then the
//operation and the bits (R816_UDiv32), or the name of an assembly routine (R816_BcdAdd).  TableDivide's tables are
//IR816_SIGNED_TABLE or IR816_UNSIGNED_TABLE and the divisor (R816_UTable10), the divisor IR816_TABLE_LENGTH in.
#define IR816_RUNTIME_PREFIX		"R816_"
#define IR816_RUNTIME_PREFIX_LENGTH	5
#define IR816_SIGNED_TABLE			"R816_STable"
#define IR816_UNSIGNED_TABLE		"R816_UTable"
#define IR816_TABLE_LENGTH			11
#define IR816_BITS_TABLE			"R816_Bits"		//R816_Bits16, R816_Bits32 and R816_Bits64: 1 << k for k from 0 to 63 (0 past the width).
#define IR816_BITS_TABLE_LENGTH		9
#define IR816_FRAME_OPERANDS		".dp"	//A soft float frame operand entry's label suffix (FrameOperandCall).
#define IR816_DIRECT_PAGE			256		//The bytes [D + n] reaches (IR816FindReads).
#define IR816_SCRATCH_FIRST			3		//The caller's bytes the scratch runtime routines may read (D + 5 to D + 22, with
#define IR816_SCRATCH_END			24		//a byte either side).


//The W65C816's mnemonics, as the text passes (Peephole ...) decode them (IR816Decode); gasIR816Mnemonics is in this
//order, alphabetical.
enum EIR816Mnemonic
{
	IR816M_Adc,
	IR816M_And,
	IR816M_Asl,
	IR816M_Bcc,
	IR816M_Bcs,
	IR816M_Beq,
	IR816M_Bit,
	IR816M_Bmi,
	IR816M_Bne,
	IR816M_Bpl,
	IR816M_Bra,
	IR816M_Brk,
	IR816M_Brl,
	IR816M_Bvc,
	IR816M_Bvs,
	IR816M_Clc,
	IR816M_Cld,
	IR816M_Cli,
	IR816M_Clv,
	IR816M_Cmp,
	IR816M_Cop,
	IR816M_Cpx,
	IR816M_Cpy,
	IR816M_Dec,
	IR816M_Dex,
	IR816M_Dey,
	IR816M_Eor,
	IR816M_Inc,
	IR816M_Inx,
	IR816M_Iny,
	IR816M_Jml,
	IR816M_Jmp,
	IR816M_Jsl,
	IR816M_Jsr,
	IR816M_Lda,
	IR816M_Ldx,
	IR816M_Ldy,
	IR816M_Lsr,
	IR816M_Mvn,
	IR816M_Mvp,
	IR816M_Nop,
	IR816M_Ora,
	IR816M_Pea,
	IR816M_Pei,
	IR816M_Per,
	IR816M_Pha,
	IR816M_Phb,
	IR816M_Phd,
	IR816M_Phk,
	IR816M_Php,
	IR816M_Phx,
	IR816M_Phy,
	IR816M_Pla,
	IR816M_Plb,
	IR816M_Pld,
	IR816M_Plp,
	IR816M_Plx,
	IR816M_Ply,
	IR816M_Rep,
	IR816M_Rol,
	IR816M_Ror,
	IR816M_Rti,
	IR816M_Rtl,
	IR816M_Rts,
	IR816M_Sbc,
	IR816M_Sec,
	IR816M_Sed,
	IR816M_Sei,
	IR816M_Sep,
	IR816M_Sta,
	IR816M_Stp,
	IR816M_Stx,
	IR816M_Sty,
	IR816M_Stz,
	IR816M_Tax,
	IR816M_Tay,
	IR816M_Tcd,
	IR816M_Tcs,
	IR816M_Tdc,
	IR816M_Trb,
	IR816M_Tsb,
	IR816M_Tsc,
	IR816M_Tsx,
	IR816M_Txa,
	IR816M_Txs,
	IR816M_Txy,
	IR816M_Tya,
	IR816M_Tyx,
	IR816M_Wai,
	IR816M_Wdm,
	IR816M_Xba,
	IR816M_Xce,
	IR816M_Unknown,		//Anything else (IR816UnknownFlags).
};


//What the text passes know of a mnemonic, a bit each.  The groups are the ones the passes' rules are written in; a
//rule only relies on the instructions it names, so a group holds those its rules were taught (the passes see only
//code the back end writes), not always every instruction with the property.
#define IR816MF_CARRY			0x00000001	//clc, sec: change C only.
#define IR816MF_LOAD_INDEX		0x00000002	//ldx, ldy.
#define IR816MF_STEP_INDEX		0x00000004	//inx, iny, dex, dey.
#define IR816MF_STORE			0x00000008	//sta, stx, sty, stz: write their operand, change no register or flag.
#define IR816MF_LOGIC			0x00000010	//and, ora, eor: a bit of A depends only on that bit of A and the operand.
#define IR816MF_COMPARE			0x00000020	//cmp, cpx, cpy.
#define IR816MF_SHIFT_STEP		0x00000040	//asl, lsr, rol, ror, inc, dec: A or memory, as the operand ("A") says.
#define IR816MF_LOADS_A			0x00000080	//lda, pla, txa, tya, tdc, tsc: set all of A (and N and Z) without reading it.
#define IR816MF_ZN_FROM_A		0x00000100	//N and Z are all of A after it: lda, pla, txa, tya, tdc, and, ora, eor, adc, sbc (not tsc).
#define IR816MF_READS_A			0x00000200	//Reads A (as a value: a shift or step of A is the operand's).
#define IR816MF_A_ALU			0x00000400	//lda, sta, and, ora, eor, adc, sbc, cmp, bit: A and an operand.
#define IR816MF_Y_ALU			0x00000800	//ldy, sty, tay, tya, iny, dey, cpy: Y (and A) and an operand.
#define IR816MF_COPIES_A		0x00001000	//tax, tay, pha: copy A, changing neither it nor C.
#define IR816MF_BRANCH			0x00002000	//bra, brl and the conditional branches: to a label (IR816BranchLine).
#define IR816MF_BRANCH_ZN		0x00004000	//beq, bne, bmi, bpl: read Z or N.
#define IR816MF_BRANCH_C		0x00008000	//bcc, bcs: read C.
#define IR816MF_ENDS_FLOW		0x00010000	//bra, brl, jml, jmp, rtl, rts, stp: the next line isn't run after it (IR816Unreachable).
#define IR816MF_LEAVES			0x00020000	//Every b, j and rt mnemonic (bit and brk too, conservatively): may leave straight line code.
#define IR816MF_WIDTH			0x00040000	//sep, rep.
#define IR816MF_SETS_D			0x00080000	//pld, tcd: the direct page moves.
#define IR816MF_BLOCK_MOVE		0x00100000	//mvn, mvp: A, X, Y and memory.
#define IR816MF_CALL			0x00200000	//jsl, jsr.
#define IR816MF_INTERRUPT		0x00400000	//brk, cop.
#define IR816MF_READS_X			0x00800000	//txa, txy, stx, inx, dex, cpx, phx, txs, mvn, mvp (an X indexed operand is the operand's).
#define IR816MF_WRITES_X		0x01000000	//tax, tyx, ldx, plx: set all of X (tsx too, but only IR816UsesXOrY takes it).
#define IR816MF_USES_Y			0x02000000	//Reads or writes Y; a call (jsl, jsr) may change it.
#define IR816MF_WRITES_Y		0x04000000	//tay, txy, ldy, ply, iny, dey, mvn, mvp.
#define IR816MF_PUSH_REGISTER	0x08000000	//pha, phx, phy.
#define IR816MF_PUSH_WORD		0x10000000	//pea, pei, phk: a constant, a direct page word or K.
#define IR816MF_SETS_CARRY		0x20000000	//clc, sec, cmp, cpx, cpy, asl, lsr, rol, ror, adc, sbc, xce (not plp, sep and rep).
#define IR816MF_READS_CARRY		0x40000000	//adc, sbc, rol, ror, xce, php, bcc, bcs.


//An instruction line ("\t\tmnemonic operand"), decoded by IR816Decode.
struct SIR816Line
{
	char		szMnemonic[8];
	uint16		uiMnemonic;		//EIR816Mnemonic.
	uint32		uiFlags;		//Its IR816MF_ bits.
	const char*	szOperand;
	size		uiOperand;		//To the end of the line, less a IR816_KEEP_IN_A mark.
	bool		bMarked;		//The line ends with IR816_KEEP_IN_A (Store).
	bool		bDirect;		//The operand is a direct page word, [D + iOffset] (IR816IsDirect).
	int32		iOffset;
};


//What a register holds (PeepholeRegisters): up to IR816_HELD direct page words ([D + n], aiOffset n) or constants
//(aiOffset -1), each "" when not known.  A register stored to a word holds both the word and what it held.
#define IR816_HELD			2
#define IR816_HELD_LENGTH	32


struct SIR816Held
{
	char	aszOperand[IR816_HELD][IR816_HELD_LENGTH];
	int32	aiOffset[IR816_HELD];
};


//The back end's settings (IR816Backend.cpp: SetRegisterArguments, SetSoftFloatInFrame).
extern bool	gbIR816RegisterArguments;
extern bool	gbIR816SoftFloatInFrame;


//IR816Backend.cpp: the register rule: which arguments go in A, X and Y, and a register function's label suffix.
int			IR816AssignRegisters(CArrayIR816Arguments* pasArguments);
void		IR816AppendRegisterSuffix(CChars* psz, CArrayIR816Arguments* pasArguments);

//IR816Frames.cpp: a frame's entry and exit, as Prologue and Epilogue write them.
void		IR816AppendFrameEntry(CChars* psz, int32 iFrameBytes);
void		IR816AppendRegisterFrameEntry(CChars* psz, int32 iRegisterWords, int32 iFrameBytes);
void		IR816AppendFrameExit(CChars* psz, int32 iFrameBytes);

//IR816Lines.cpp: assembly lines decoded, and what A, X and Y are known to hold.
bool		IR816IsInstruction(const char* szLine);
void		IR816Decode(const char* szLine, SIR816Line* psLine);
bool		IR816OperandIs(SIR816Line* psLine, const char* szOperand, size uiOperand);
bool		IR816LeavesA(SIR816Line* psLine);
const char*	IR816InvertedBranch(uint16 uiMnemonic);
bool		IR816IsDecimal(const char* szOperand);
const char*	IR816NextLine(const char* szLine);
const char*	IR816LineEnd(const char* szLine);
bool		IR816ZNReadAfter(const char* szLine);
void		IR816ForgetHeld(SIR816Held* asHeld, int iFirst, int iLast);
bool		IR816Holds(SIR816Held* asHeld, int iRegister, SIR816Line* psLine);
void		IR816UpdateHeld(SIR816Held* asHeld, SIR816Line* psLine);

//IR816DeadStores.cpp: stores nothing reads, and what the code reads of the frame.
bool		IR816MentionsNear(const char* szOperand, size uiOperand, int32 iOffset);
bool		IR816DeadStore(const char* szLine, const char* szWord, size uiWord);
bool		IR816MovesS(const char* szLine);
bool		IR816FindReads(const char* szLine, uint8* abRead);

//IR816Runtime.cpp: how a runtime routine takes its operands.
bool		IR816IndexedRoutine(const char* szOperand, size uiOperand);
bool		IR816ScratchRoutine(const char* szName);


#endif // __IR816_BACKEND_INTERNAL_H__
