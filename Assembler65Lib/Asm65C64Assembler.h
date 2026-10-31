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
#ifndef __ASM65C64_ASSEMBLER_H__
#define __ASM65C64_ASSEMBLER_H__
#include "MicroprocessorLib/65C64Decoded.h"
#include "Asm65BaseAssembler.h"


#define ASM65C64_MAX_WORDS		5
#define ASM65C64_MAX_INSTRUCTION_RELOCATIONS	2


enum EAsm65C64OperandKind
{
	AO65C64_None,
	AO65C64_Register,
	AO65C64_Constant,
	AO65C64_Memory,
};


//An address as written: base {+ inner}, [base {+ inner}] {+ post}.  The base is a
//register or a value; eBase is R65_Zero when it is a value.
struct SAsm65C64Address
{
	bool			bIndirect;
	E65Register		eBase;
	SAsm65Value		sValue;
	uint8			uiAddressBits;
	E65Register		eInner;
	E65Register		ePost;
	SAsm65Token*	psToken;
};


//An instruction operand: a register, a constant or memory ([address]).
struct SAsm65C64Operand
{
	EAsm65C64OperandKind	eKind;
	E65Register			eRegister;
	SAsm65Value			sValue;
	uint8				uiWidthBytes;
	bool				bSigned;
	SAsm65C64Address		sAddress;
	SAsm65Token*		psToken;
};


//An address as an operand access mode.  eFirst is the addressing register of the single
//register modes or the first register of the two register modes.
struct SAsm65C64Memory
{
	uint8			uiMode;
	E65Register		eFirst;
	E65Register		eSecond;
	uint8			uiAddressWords;
	uint32			uiAddress;
	SAsm65Value*	psValue;
	SAsm65Token*	psToken;
};


//An operand field the linker fills in: uiBytes starting at word uiWord.
struct SAsm65C64InstructionRelocation
{
	SAsm65Value		sValue;
	uint8			uiWord;
	uint8			uiBytes;
	bool			bSigned;
	bool			bRelative;
	SAsm65Token*	psToken;
};


//An encoded instruction: the opcode then its operand words.
struct SAsm65C64Instruction
{
	uint16						auiWords[ASM65C64_MAX_WORDS];
	uint8						uiWords;
	SAsm65C64InstructionRelocation	asRelocations[ASM65C64_MAX_INSTRUCTION_RELOCATIONS];
	uint8						uiRelocations;
};



//Assembles 65C64 source text (see Assembler65C64Syntax.txt).  The passes, labels,
//expressions, directives, sections and objects are CAsm65BaseAssembler's; the
//instructions, registers and operands are its own.
class CAsm65C64Assembler : public CAsm65BaseAssembler
{
protected:
	bool	mbMemoryWidth;

public:
	void			Init(void);

protected:
	bool			Instruction(SAsm65Token* psMnemonic) override;
	bool			IsRegisterName(SAsm65Token* psToken) override;
	uint32			InstructionAlignment(void) override;

	//Asm65C64Assembler.cpp: writing instructions.
	void			Emit(SAsm65C64Instruction* psInstruction);

	//Asm65C64Operands.cpp: registers, flags and operands.
	bool			IsRegister(SAsm65Token* psToken, E65Register* peRegister);
	bool			Register(E65Register* peRegister, const char* szWhat);
	bool			FixedRegister(E65Register eRegister);
	bool			Flag(uint8* puiFlag, bool bAllowConstant, const char* szWhat);
	bool			Direction(bool* pbDecrement);
	bool			BracketedList(void);
	bool			StopStyle(bool* pbAllowStop, uint8* puiStopStyle);
	bool			Suffixes(uint8* puiWidthBytes, bool* pbSigned, uint8* puiAddressBits);
	bool			Address(SAsm65C64Address* psAddress);
	bool			AddressInner(SAsm65C64Address* psAddress);
	bool			Operand(SAsm65C64Operand* psOperand);
	bool			Memory(SAsm65C64Address* psAddress, SAsm65C64Memory* psMemory);

	//Asm65C64Instructions.cpp: the instructions.
	bool			ArithmeticLogic(uint16 uiCode);
	bool			ArithmeticLogicMemory(uint16 uiCode, SAsm65C64Operand* psFirst, SAsm65C64Operand* psSecond, SAsm65C64Instruction* psInstruction);
	bool			Shift(bool bLeft);
	bool			ShiftMemory(uint16 uiExtend, SAsm65C64Operand* psFirst, SAsm65C64Operand* psSecond, SAsm65C64Instruction* psInstruction);
	bool			Jump(bool bPushC, bool bRelative);
	bool			JumpOptions(E65Register* peStack, bool* pbPushP);
	bool			Return(bool bRti);
	bool			Push(void);
	bool			Pull(void);
	bool			PushPullRegisters(bool bPush);
	bool			PushPullMemory(bool bPush, SAsm65C64Operand* psOperand, E65Register eStack);
	bool			StackRegister(E65Register* peStack);
	bool			ProgramState(bool bPush, bool bC, bool bP, E65Register eStack);
	bool			RegisterList(uint8* puiMask, bool* pbP, bool* pbC);
	bool			StateBits(uint16 uiSub);
	bool			NameList(const char** aszNames, int iNumNames, const char* szWhat, uint16* puiMask);
	bool			CloseGroup(SAsm65Token* psOpen, int iCount);
	bool			Width(bool bTwo);
	bool			Break(void);
	bool			Stop(void);
	bool			GetWidth(void);
	bool			ExtendWidth(void);
	bool			Branch(uint16 uiCondition);
	bool			BranchEqual(uint16 uiSub);
	bool			CopyExtend(void);
	bool			BranchAlways(void);
	bool			NoOperation(void);
	bool			MultiplyDivide(uint16 uiSub);
	bool			NegateAbsolute(uint16 uiSub);
	bool			Find(bool bSet, uint16 uiUnit);
	bool			Subcopy(bool bBits);
	bool			SubcopyPair(E65Register* peRegister, uint16* puiIndexField, bool bBits);
	bool			BitToStatus(void);
	bool			StatusToBit(void);
	bool			StatusToStatus(void);
	bool			BitIndex(uint16* puiStyle, uint16* puiWord, bool bForceWord);
	bool			Block(uint16 uiSub);
	bool			Vector(uint16 uiSub);

	bool			RelativeBytes(SAsm65Value* psTarget, SAsm65Token* psToken, uint8* puiBytes, int64* piOffset);
	bool			ChooseBytes(SAsm65Value* psValue, bool bSigned, uint8 uiExplicitBytes, const uint8* auiBytes, int iNumBytes, uint8 uiFixedWords, SAsm65Token* psToken, uint8* puiBytes);
	bool			MemoryOperand(SAsm65C64Operand* psOperand, SAsm65C64Memory* psMemory, uint8 uiFixedWords);
	bool			SingleRegisterField(SAsm65C64Memory* psMemory, const E65Register* aeList, int iListSize, SAsm65Token* psToken, uint16* puiField);
	bool			TwoRegisterFields(SAsm65C64Memory* psMemory, const E65Register* aeSecondList, int iSecondListSize, SAsm65Token* psToken, uint16* puiFirst, uint16* puiSecond);
	void			AddValueWords(SAsm65C64Instruction* psInstruction, uint64 uiValue, uint8 uiBytes);
	void			AddAddressWords(SAsm65C64Instruction* psInstruction, SAsm65C64Memory* psMemory);
	bool			AddRelocatableWords(SAsm65C64Instruction* psInstruction, SAsm65Value* psValue, uint64 uiValue, uint8 uiBytes, bool bSigned, bool bRelative, SAsm65Token* psToken);
};


#endif // __ASM65C64_ASSEMBLER_H__

