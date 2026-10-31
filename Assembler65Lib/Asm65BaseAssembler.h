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
#ifndef __ASM65_BASE_ASSEMBLER_H__
#define __ASM65_BASE_ASSEMBLER_H__
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/ArraySize.h"
#include "BaseLib/ArrayUint32.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/IntelHex.h"
#include "Asm65Tokeniser.h"
#include "Asm65Symbols.h"
#include "Asm65Output.h"
#include "Asm65Errors.h"
#include "Asm65Object.h"


#define ASM65_MAX_PASSES	16


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


//The parts of an assembler every processor shares: the passes, statements, labels,
//expressions, directives, sections and objects.  CAsm65C64Assembler (the 65C64) and
//CAsm65816Assembler (the W65C816S) provide the instructions and their operands.
//
//Passes repeat until every label has settled.  An instruction never gets shorter in a
//later pass so the passes always settle.  Errors are only reported by the final pass.
//
//Assemble writes bytes at absolute addresses (.org).  AssembleObject writes sections,
//symbols and relocations into a CAsm65Object for CAsm65Linker.
class CAsm65BaseAssembler
{
protected:
	CAsm65Errors		mcErrors;
	CAsm65Errors		mcWarnings;
	CAsm65Tokeniser		mcTokeniser;
	CAsm65Symbols		mcSymbols;
	CAsm65Output		mcOutput;
	CArrayUint8			mcMinimumLengths;
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
	//Provided by each processor's assembler.
	virtual bool	Instruction(SAsm65Token* psMnemonic) =0;
	virtual bool	IsRegisterName(SAsm65Token* psToken) =0;
	virtual uint32	InstructionAlignment(void) =0;
	virtual bool	ProcessorDirective(SAsm65Token* psDirective, bool* pbResult);
	virtual void	SectionStarted(void);

	//Asm65BaseAssembler.cpp: passes, statements, labels and directives.
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
	bool			SourceLine(SAsm65Token* psDirective);
	bool			Fingerprint(SAsm65Token* psDirective);
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
	uint8			GetMinimumLength(void);
	void			RecordLength(uint8 uiLength);

	SAsm65Token*	Peek(size uiOffset = 0);
	SAsm65Token*	Next(void);
	bool			Accept(EAsm65Punctuator ePunctuator);
	bool			Expect(EAsm65Punctuator ePunctuator, const char* szWhat);
	bool			ExpectEnd(void);
	bool			Error(SAsm65Token* psToken, const char* szMessage);
	bool			ErrorExpected(SAsm65Token* psToken, const char* szWhat);
	void			Warning(SAsm65Token* psToken, const char* szMessage);
	void			Report(CAsm65Errors* pcReport, SAsm65Token* psToken, const char* szMessage);

	//Asm65BaseExpressions.cpp: expressions.
	void			ClearValue(SAsm65Value* psValue);
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
	bool			IsReservedName(SAsm65Token* psToken);
};


#endif // __ASM65_BASE_ASSEMBLER_H__

