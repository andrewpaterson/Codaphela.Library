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
#ifndef __PREPROCESSED_TOKENS_H__
#define __PREPROCESSED_TOKENS_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/Chars.h"


enum EPreprocessedTokenKind
{
	PTK_Identifier,
	PTK_Number,
	PTK_Character,
	PTK_String,
	PTK_Punctuator,
	PTK_Other,

	//Only used while expanding macros.
	PTK_Placemarker,
};


//The token was preceded by whitespace.
#define PTF_LEADING_SPACE	0x01

//The token was the first on its source line.
#define PTF_START_OF_LINE	0x02

//The token names a macro that must never be expanded (it was found inside that macro's own expansion).
#define PTF_NO_EXPAND		0x04

//In a macro body: the '##' operator (as opposed to a '##' made by pasting).
#define PTF_PASTE			0x08

//In a macro body: the '#' operator.
#define PTF_STRINGIFY		0x10

//szText is in the CPreprocessedTokens' strings, not in a source file.
#define PTF_OUTPUT_TEXT		0x20


class CSourceFile;
class CDefine;


//A preprocessing token after macro expansion.  The spelling location is where its characters are: in the source,
//in a #define, or NULL for tokens the preprocessor made (by pasting, stringifying or __LINE__).
struct SPreprocessedToken
{
	const char*		szText;
	uint32			uiLength;
	uint8			uiKind;
	uint8			uiFlags;
	int16			iParameter;		//In a macro body, the parameter this token names, otherwise -1.
	CSourceFile*	pcFile;
	int32			iLine;			//0 based.
	int32			iColumn;		//0 based.
	int32			iExpansion;		//Index of the macro expansion that made this token, -1 if it came from the source text.

	void	Init(EPreprocessedTokenKind eKind, const char* szText, size uiLength, CSourceFile* pcFile, int iLine, int iColumn);
	bool	Is(char* szText);
	bool	IsPunctuator(char* szText);
	bool	IsIdentifier(void);
	bool	HasLeadingSpace(void);
	bool	IsStartOfLine(void);
	void	Append(CChars* psz);
};


typedef CArrayTemplate<SPreprocessedToken>	CArrayPreprocessedTokens;


//One expansion of a macro.  The tokens it made refer to it by index.
struct SMacroExpansion
{
	const char*		szName;			//Not NULL terminated.
	uint32			uiNameLength;
	CSourceFile*	pcFile;			//Where the macro name was (its spelling location).
	int32			iLine;
	int32			iColumn;
	int32			iParent;		//The expansion the macro name came from, -1 if it was in the source text.
};


typedef CArrayTemplate<SMacroExpansion>	CArrayMacroExpansions;


//Lines skipped by conditional compilation.
struct SInactiveRegion
{
	CSourceFile*	pcFile;
	int32			iStartLine;		//0 based, inclusive.
	int32			iEndLine;		//0 based, inclusive.
};


typedef CArrayTemplate<SInactiveRegion>	CArrayInactiveRegions;


//A line with a preprocessing directive.
struct SDirectiveLine
{
	CSourceFile*	pcFile;
	int32			iLine;			//0 based.
	bool			bActive;		//False if a conditional skipped it.
};


typedef CArrayTemplate<SDirectiveLine>	CArrayDirectiveLines;


//The output of the preprocessor for one translation unit.
class CPreprocessedTokens
{
protected:
	CArrayPreprocessedTokens	macTokens;
	CArrayMacroExpansions		macExpansions;
	CArrayInactiveRegions		macInactiveRegions;
	CArrayDirectiveLines		macDirectiveLines;
	CMemoryStackExtended		mcStrings;		//Spellings of tokens the preprocessor made.

public:
	void					Init(void);
	void					Kill(void);
	void					Clear(void);

	SPreprocessedToken*		Add(SPreprocessedToken* psToken);
	size					NumTokens(void);
	SPreprocessedToken*		Get(size iIndex);

	int						AddExpansion(const char* szName, size uiNameLength, CSourceFile* pcFile, int iLine, int iColumn, int iParent);
	size					NumExpansions(void);
	SMacroExpansion*		GetExpansion(size iIndex);

	void					AddInactiveLine(CSourceFile* pcFile, int iLine);
	size					NumInactiveRegions(void);
	SInactiveRegion*		GetInactiveRegion(size iIndex);

	void					AddDirectiveLine(CSourceFile* pcFile, int iLine, bool bActive);
	size					NumDirectiveLines(void);
	SDirectiveLine*			GetDirectiveLine(size iIndex);

	char*					AddString(const char* szText, size uiLength);

	char*					Print(CChars* psz);
	void					Dump(void);
};


bool PreprocessedTokensNeedSpace(SPreprocessedToken* psLeft, SPreprocessedToken* psRight);


#endif // __PREPROCESSED_TOKENS_H__

