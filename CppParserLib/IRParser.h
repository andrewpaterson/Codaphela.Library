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
#ifndef __I_R_PARSER_H__
#define __I_R_PARSER_H__
#include "CPPErrors.h"
#include "IRModule.h"


//Reads IR text (IR.txt Text form) into a module.  Parsing stops at the first error.  Types and dominance are not
//checked here: run CIRVerifier on the result.  Numbered temporaries and blocks (%3, 7:) become unnamed ones.
class CIRParser
{
protected:
	CIRModule*				mpcModule;
	CCPPErrors*				mpcErrors;
	const char*				mszFile;
	const char*				mszText;
	size					muiPosition;
	bool					mbError;
	CChars					mszName;				//The name just read.
	CChars					mszNumber;				//The number just read.
	CChars					mszResult;				//The name of the temporary being defined.

	CIRFunction*			mpcFunction;
	CIRBlock*				mpcBlock;
	CArrayTemplate<uint32>	mauiNumberedTemporaries;	//Per number: 1 + the temporary's index, 0 if not seen.
	CArrayIRBlocks			mapcNumberedBlocks;			//Per number: the block, NULL if not seen.
	CArrayTemplate<size>	mauiTemporaryUses;			//Per temporary: where it was first named.
	CArrayTemplate<size>	mauiBlockUses;				//Per block index: where it was first named.
	CArrayTemplate<uint8>	macLabelled;				//Per block index: 1 once its label is read.
	CArrayIRBlocks			mapcLabelOrder;				//The blocks in the order of their labels.
	size					muiResultPosition;			//Where the temporary being defined is named.

public:
	void	Init(CIRModule* pcModule, CCPPErrors* pcErrors, const char* szFile = NULL);
	void	Kill(void);

	bool	Parse(const char* szText);

protected:
	bool	Error(const char* szMessage);
	bool	ErrorAt(size uiPosition, const char* szMessage);
	void	SkipSpace(void);
	bool	AtEnd(void);
	char	Peek(void);
	bool	GetChar(char c);
	bool	ExpectChar(char c);
	bool	IsNameCharacter(char c);
	bool	GetName(void);
	bool	ExpectName(const char* szError);
	bool	GetKeyword(const char* szKeyword);
	bool	GetEllipsis(bool* pbFound);
	bool	GetNumber(void);
	bool	ParseInteger(const char* szNumber, uint64* pulliValue);
	bool	GetInteger(uint64* pulliValue, const char* szError);
	bool	ParseBits(SIRType sType, uint64* pulliBits);
	bool	GetString(CArrayTemplate<uint8>* pacBytes);

	bool	MatchType(const char* szName, SIRType* psType, bool bAllowVoid);
	bool	ParseType(SIRType* psType, bool bAllowVoid);
	bool	ParseAttributes(uint32* puiAttributes, uint32* puiAlign);
	bool	ParseNewSymbol(uint32* puiSymbol);

	bool	ParseSpace(void);
	bool	ParseGlobal(void);
	bool	ParseRestrict(void);
	bool	ParseGlobalItem(CIRGlobal* pcGlobal);
	bool	ParseFunction(void);
	bool	ParseBody(void);
	bool	ParseStatement(void);
	bool	ParseOperation(uint16 uiOpcode, SIROpcode* psOpcode, bool bResult);
	bool	ParsePhi(void);
	bool	ParseCall(bool bResult);
	bool	ParseJump(void);

	SIRType	OperandType(uint8 uiKind, SIRType sType);
	bool	ParseValue(SIRType sType, SIRReference* psReference);
	bool	ParseBlockName(CIRBlock** ppcBlock);
	SIRReference	Temporary(const char* szName, size uiPosition);
	CIRBlock*	Block(const char* szName, size uiPosition);
	bool	DefineTemporary(SIRType sType, SIRReference* psReference);
	bool	IsNumber(const char* szName, uint32* puiNumber);
	void	SetPosition(CArrayTemplate<size>* pauiPositions, size uiIndex, size uiPosition);
	bool	FinishFunction(void);
};


#endif // __I_R_PARSER_H__
