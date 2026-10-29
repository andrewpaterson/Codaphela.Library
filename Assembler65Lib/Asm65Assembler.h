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
#ifndef __ASM65_ASSEMBLER_H__
#define __ASM65_ASSEMBLER_H__
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/ArraySize.h"
#include "BaseLib/ArrayUint32.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/IntelHex.h"
#include "MicroprocessorLib/65C64Decoded.h"
#include "Asm65Tokeniser.h"
#include "Asm65Symbols.h"
#include "Asm65Output.h"
#include "Asm65Errors.h"
#include "Asm65Object.h"


#define ASM65_MAX_PASSES	16
#define ASM65_MAX_WORDS		5
#define ASM65_MAX_INSTRUCTION_RELOCATIONS	2


//A value from an expression.  An unresolved value uses a symbol that has no value yet.
//bLabel is set when the expression uses a symbol (it is an address, not an offset).
//C (the program counter) is never a value: it counts 0 in iValue and iProgramCounter
//says how many times it was added, so C - 2 is an offset of -2 from C.
//In an object a label is an offset in its section: iTerms counts how many times a
//section address (iSection, or the external named by uiTermToken) was added.  Only
//values with iTerms of 0 (a constant) or 1 (an address) can be assembled.
struct SAsm65Value
{
	int64	iValue;
	bool	bResolved;
	bool	bLabel;
	int		iProgramCounter;
	int		iTerms;
	int32	iSection;
	size	uiTermToken;
	uint8	uiForm;		//EAsm65RelocationForm: a section address >> 16 or & $FFFF.
};


enum EAsm65OperandKind
{
	AO65_None,
	AO65_Register,
	AO65_Constant,
	AO65_Memory,
};


//An address as written: base {+ inner}, [base {+ inner}] {+ post}.  The base is a
//register or a value; eBase is R65_Zero when it is a value.
struct SAsm65Address
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
struct SAsm65Operand
{
	EAsm65OperandKind	eKind;
	E65Register			eRegister;
	SAsm65Value			sValue;
	uint8				uiWidthBytes;
	bool				bSigned;
	SAsm65Address		sAddress;
	SAsm65Token*		psToken;
};


//An address as an operand access mode.  eFirst is the addressing register of the single
//register modes or the first register of the two register modes.
struct SAsm65Memory
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
struct SAsm65InstructionRelocation
{
	SAsm65Value		sValue;
	uint8			uiWord;
	uint8			uiBytes;
	bool			bSigned;
	bool			bRelative;
	SAsm65Token*	psToken;
};


//An encoded instruction: the opcode then its operand words.
struct SAsm65Instruction
{
	uint16						auiWords[ASM65_MAX_WORDS];
	uint8						uiWords;
	SAsm65InstructionRelocation	asRelocations[ASM65_MAX_INSTRUCTION_RELOCATIONS];
	uint8						uiRelocations;
};



//Assembles 65C64 source text (see Assembler65Syntax.txt).  Passes repeat until every
//label has settled.  An instruction never gets shorter in a later pass so the passes
//always settle.  Errors are only reported by the final pass.
//
//Assemble writes bytes at absolute addresses (.org).  AssembleObject writes sections,
//symbols and relocations into a CAsm65Object for CAsm65Linker.
class CAsm65Assembler
{
protected:
	CAsm65Errors		mcErrors;
	CAsm65Errors		mcWarnings;
	CAsm65Tokeniser		mcTokeniser;
	CAsm65Symbols		mcSymbols;
	CAsm65Output		mcOutput;
	CArrayUint8			mcMinimumWords;
	CArraySize			mcPendingLabels;

	size				muiToken;
	uint32				muiAddress;
	uint32				muiStatementAddress;
	uint32				muiPass;
	int64				miLabelShift;
	bool				mbFinal;
	bool				mbChanged;
	bool				mbStatementError;
	bool				mbRelative;
	bool				mbMemoryWidth;
	size				muiInstruction;
	uint32				muiPasses;

	bool				mbObject;
	CAsm65Object*		mpcObject;
	uint32				muiSection;
	CArrayUint32		mcSectionAddresses;
	CArraySize			mcUndefined;
	CArraySize			mcAssociations;
	int					miExpressionDepth;
	uint32				muiStatementTerms;
	uint32				muiStatementTermsUsed;
	SAsm65Token*		mpsTermToken;

	CChars							mszDirectory;
	CArrayChars						maszHexFiles;
	CArrayTemplatePtr<CIntelHex>	mapcHexFiles;

public:
	void			Init(void);
	void			Kill(void);

	bool			Assemble(const char* szSource);
	bool			AssembleObject(const char* szSource, CAsm65Object* pcObject);
	void			SetDirectory(const char* szDirectory);

	CAsm65Output*	GetOutput(void);
	char*			GetErrors(void);
	size			NumErrors(void);
	char*			GetWarnings(void);
	size			NumWarnings(void);
	bool			GetSymbol(const char* szName, int64* piValue);
	uint32			NumPasses(void);

protected:
	//Provided by each processor's assembler (CAsm816Assembler for the W65C816).
	virtual bool	Instruction(SAsm65Token* psMnemonic);
	virtual bool	IsRegisterName(SAsm65Token* psToken);
	virtual uint32	InstructionAlignment(void);
	virtual bool	ProcessorDirective(SAsm65Token* psDirective, bool* pbResult);
	virtual void	SectionStarted(void);

	//Asm65Assembler.cpp: passes, statements, labels and directives.
	void			Pass(void);
	void			Statement(void);
	void			BindLabels(void);
	void			DefineSymbol(SAsm65Token* psName, int64 iValue, bool bResolved, int32 iSection, size uiTermToken);
	size			TokenIndex(SAsm65Token* psToken);
	void			AlignTo(uint32 uiAlignment);
	void			WriteByte(uint8 uiByte);
	bool			Origin(void);
	bool			Align(void);
	bool			Data(uint8 uiBytes);
	bool			Constant(SAsm65Token* psName);
	bool			Space(void);
	bool			IncludeHex(void);
	CIntelHex*		GetHexFile(SAsm65Token* psName);
	bool			Section(void);
	bool			SectionOptions(SAsm65Token* psName, SAsm65Token** ppsGroup, SAsm65Token** ppsAssociated, bool* pbKeep, bool* pbResident, bool* pbAt, uint32* puiAt);
	void			Associate(void);
	bool			SymbolBinding(SAsm65Token* psDirective);
	void			SwitchSection(uint32 uiSection);
	bool			InBss(SAsm65Token* psToken);
	void			MarkExternals(void);
	void			MakeExternal(SAsm65Symbol* psSymbol, size uiToken);
	void			BuildObjectSymbols(void);
	bool			IsRelocatable(SAsm65Value* psValue);
	bool			IsLocalTarget(SAsm65Value* psValue);
	bool			TargetIsRelocatable(SAsm65Value* psTarget);
	void			UseTerm(void);
	void			RecordRelocation(SAsm65Value* psValue, uint32 uiOffset, uint8 uiBytes, bool bSigned, bool bRelative, uint32 uiFieldToNext, SAsm65Token* psToken);
	void			Emit(SAsm65Instruction* psInstruction);
	uint8			GetMinimumWords(void);

	SAsm65Token*	Peek(size uiOffset = 0);
	SAsm65Token*	Next(void);
	bool			Accept(EAsm65Punctuator ePunctuator);
	bool			Expect(EAsm65Punctuator ePunctuator, const char* szWhat);
	bool			ExpectEnd(void);
	bool			Error(SAsm65Token* psToken, const char* szMessage);
	bool			ErrorExpected(SAsm65Token* psToken, const char* szWhat);
	void			Warning(SAsm65Token* psToken, const char* szMessage);
	void			Report(CAsm65Errors* pcReport, SAsm65Token* psToken, const char* szMessage);

	//Asm65Expressions.cpp: expressions, registers, flags and operands.
	bool			Expression(SAsm65Value* psValue);
	bool			BitwiseOr(SAsm65Value* psValue);
	bool			BitwiseXor(SAsm65Value* psValue);
	bool			BitwiseAnd(SAsm65Value* psValue);
	bool			ShiftExpression(SAsm65Value* psValue);
	bool			Additive(SAsm65Value* psValue);
	bool			Multiplicative(SAsm65Value* psValue);
	bool			Unary(SAsm65Value* psValue);
	bool			Primary(SAsm65Value* psValue);
	bool			ExpressionValue(int64* piValue);
	bool			FormTerm(SAsm65Value* psValue, SAsm65Value* psRight, int64 iOperand, uint8 uiForm);
	bool			Combine(SAsm65Value* psValue, SAsm65Value* psRight, SAsm65Token* psOperator);
	bool			AddTerms(SAsm65Value* psValue, SAsm65Value* psRight, bool bAdd, SAsm65Token* psOperator);
	bool			SameTerm(SAsm65Value* psValue, SAsm65Value* psRight);
	bool			RelativeTarget(SAsm65Value* psValue, SAsm65Token* psToken);

	bool			IsRegister(SAsm65Token* psToken, E65Register* peRegister);
	bool			IsReservedName(SAsm65Token* psToken);
	bool			Register(E65Register* peRegister, const char* szWhat);
	bool			FixedRegister(E65Register eRegister);
	bool			Flag(uint8* puiFlag, bool bAllowConstant, const char* szWhat);
	bool			Direction(bool* pbDecrement);
	bool			BracketedList(void);
	bool			StopStyle(bool* pbAllowStop, uint8* puiStopStyle);
	bool			Suffixes(uint8* puiWidthBytes, bool* pbSigned, uint8* puiAddressBits);
	bool			Address(SAsm65Address* psAddress);
	bool			AddressInner(SAsm65Address* psAddress);
	bool			Operand(SAsm65Operand* psOperand);
	bool			Memory(SAsm65Address* psAddress, SAsm65Memory* psMemory);

	//Asm65Instructions.cpp: the instructions.
	bool			ArithmeticLogic(uint16 uiCode);
	bool			ArithmeticLogicMemory(uint16 uiCode, SAsm65Operand* psFirst, SAsm65Operand* psSecond, SAsm65Instruction* psInstruction);
	bool			Shift(bool bLeft);
	bool			ShiftMemory(uint16 uiExtend, SAsm65Operand* psFirst, SAsm65Operand* psSecond, SAsm65Instruction* psInstruction);
	bool			Jump(bool bPushC, bool bRelative);
	bool			JumpOptions(E65Register* peStack, bool* pbPushP);
	bool			Return(bool bRti);
	bool			Push(void);
	bool			Pull(void);
	bool			PushPullRegisters(bool bPush);
	bool			PushPullMemory(bool bPush, SAsm65Operand* psOperand, E65Register eStack);
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
	bool			MemoryOperand(SAsm65Operand* psOperand, SAsm65Memory* psMemory, uint8 uiFixedWords);
	bool			SingleRegisterField(SAsm65Memory* psMemory, const E65Register* aeList, int iListSize, SAsm65Token* psToken, uint16* puiField);
	bool			TwoRegisterFields(SAsm65Memory* psMemory, const E65Register* aeSecondList, int iSecondListSize, SAsm65Token* psToken, uint16* puiFirst, uint16* puiSecond);
	void			AddValueWords(SAsm65Instruction* psInstruction, uint64 uiValue, uint8 uiBytes);
	void			AddAddressWords(SAsm65Instruction* psInstruction, SAsm65Memory* psMemory);
	bool			AddRelocatableWords(SAsm65Instruction* psInstruction, SAsm65Value* psValue, uint64 uiValue, uint8 uiBytes, bool bSigned, bool bRelative, SAsm65Token* psToken);
};


#endif // __ASM65_ASSEMBLER_H__
