/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ASM65816_ASSEMBLER_H__
#define __ASM65816_ASSEMBLER_H__
#include "Asm65BaseAssembler.h"


#define ASM65816_MAX_BYTES		5
#define ASM65816_MAX_RELOCATIONS	2
#define ASM65816_WIDTH_UNKNOWN	0


//How an instruction's operand is addressed: the W65C816S's addressing modes as written
//in Assembler65816Syntax.txt.
enum EAsm65816Mode
{
	A65816M_Implied,
	A65816M_Accumulator,			//asl A
	A65816M_ImmediateM,			//lda 5: 8 or 16 bits by m
	A65816M_ImmediateX,			//ldx 5: 8 or 16 bits by x
	A65816M_Immediate8,			//rep, sep, brk, cop and wdm
	A65816M_Immediate16,			//pea
	A65816M_Absolute,				//[value]
	A65816M_AbsoluteX,			//[value + X]
	A65816M_AbsoluteY,			//[value + Y]
	A65816M_Long,					//[value]:24
	A65816M_LongX,				//[value + X]:24
	A65816M_Direct,				//[D + value]
	A65816M_DirectX,				//[D + value + X]
	A65816M_DirectY,				//[D + value + Y]
	A65816M_DirectIndirect,		//[[D + value]]
	A65816M_DirectIndirectX,		//[[D + value + X]]
	A65816M_DirectIndirectY,		//[[D + value] + Y]
	A65816M_DirectIndirectLong,	//[[D + value]:24]
	A65816M_DirectIndirectLongY,	//[[D + value]:24 + Y]
	A65816M_Stack,				//[S + value]
	A65816M_StackIndirectY,		//[[S + value] + Y]
	A65816M_Relative,				//bra Label
	A65816M_RelativeLong,			//brl Label, per Label
	A65816M_JumpAbsolute,			//jmp Label, jsr Label
	A65816M_JumpLong,				//jml Label, jsl Label
	A65816M_JumpIndirect,			//jmp [Vector]
	A65816M_JumpIndirectX,		//jmp [Table + X], jsr [Table + X]
	A65816M_JumpIndirectLong,		//jml [Vector]:24
	A65816M_BlockMove,			//mvn Source >> 16, Destination >> 16
	A65816M_NumModes,
};


enum EAsm65816Base
{
	A65816B_Value,
	A65816B_Direct,
	A65816B_Stack,
};


//An operand as written: A, a value or memory in square brackets.
struct SAsm65816Operand
{
	bool			bAccumulator;
	bool			bMemory;
	SAsm65Value		sValue;
	uint8			uiValueBits;		//A :8, :16 or :24 suffix on a value, else 0.
	uint8			eBase;				//EAsm65816Base inside the (inner) brackets.
	bool			bPointer;			//[[...]]: the address is read from memory.
	bool			bLongPointer;		//[[...]:24 ...]
	char			cInnerIndex;		//X inside the inner brackets, or 0.
	char			cIndex;				//X or Y after the inner brackets (or with no pointer), or 0.
	bool			bLong;				//[...]:24
	SAsm65Token*	psToken;
};


//A field the linker fills in: uiBytes at byte uiOffset of the instruction.
struct SAsm65816Relocation
{
	SAsm65Value		sValue;
	uint8			uiOffset;
	uint8			uiBytes;
	bool			bSigned;
	bool			bRelative;
	SAsm65Token*	psToken;
};


struct SAsm65816Instruction
{
	uint8				auiBytes[ASM65816_MAX_BYTES];
	uint8				uiBytes;
	SAsm65816Relocation	asRelocations[ASM65816_MAX_RELOCATIONS];
	uint8				uiRelocations;
};


//Assembles W65C816S source text (see Assembler65816Syntax.txt).  The passes, labels,
//expressions, directives, sections and objects are CAsm65BaseAssembler's; the instructions,
//operands and the tracking of the m and x widths are its own.
class CAsm65816Assembler : public CAsm65BaseAssembler
{
protected:
	uint8	muiWidthA;		//8, 16 or ASM65816_WIDTH_UNKNOWN.
	uint8	muiWidthX;
	bool	mbRelax;		//.relax: a branch to a label too far away becomes a branch round a brl.

public:
	void			Init(void);

protected:
	bool			Instruction(SAsm65Token* psMnemonic) override;
	bool			IsRegisterName(SAsm65Token* psToken) override;
	uint32			InstructionAlignment(void) override;
	bool			ProcessorDirective(SAsm65Token* psDirective, bool* pbResult) override;
	void			SectionStarted(void) override;

	//Asm65816Assembler.cpp: operands, widths and writing instructions.
	bool			Operand65816(SAsm65816Operand* psOperand);
	bool			Inner65816(SAsm65816Operand* psOperand);
	bool			Index65816(char* pcIndex, bool bAllowY);
	bool			Bits65816(uint8* puiBits, bool bAllow24);
	bool			Width(void);
	bool			StatusBits(uint8* puiBits);
	void			ChangeWidths(bool bSet, uint8 uiBits);
	void			Emit65816(SAsm65816Instruction* psInstruction);
	void			AddByte(SAsm65816Instruction* psInstruction, uint8 uiByte);
	bool			AddField(SAsm65816Instruction* psInstruction, SAsm65Value* psValue, uint8 uiBytes, bool bSigned, uint8 uiForm, SAsm65Token* psToken);
	bool			AddRelative(SAsm65816Instruction* psInstruction, SAsm65Value* psValue, uint8 uiBytes, uint8 uiLength, SAsm65Token* psToken);

	//Asm65816Instructions.cpp: choosing the opcode and writing its operand.
	bool			ChooseMode(int iMnemonic, SAsm65816Operand* psOperand, SAsm65Token* psMnemonic, uint8* puiMode);
	bool			IsLongBranch(SAsm65Value* psTarget);
	bool			AbsoluteOrLong(int iMnemonic, SAsm65816Operand* psOperand, uint8 uiAbsolute, uint8 uiLong, uint8* puiMode);
	bool			Encode(int iMnemonic, uint8 uiMode, SAsm65816Operand* psOperand, SAsm65816Operand* psSecond, SAsm65Token* psMnemonic);
	bool			Immediate(SAsm65816Instruction* psInstruction, SAsm65816Operand* psOperand, uint8 uiWidth, const char* szRegister);
	bool			DirectOffset(SAsm65816Instruction* psInstruction, SAsm65816Operand* psOperand);
};


//The opcode table, for tests: the mnemonic and EAsm65816Mode of each opcode.
const char*	Asm65816Mnemonic(uint8 uiOpcode, uint8* puiMode);


#endif // __ASM65816_ASSEMBLER_H__

