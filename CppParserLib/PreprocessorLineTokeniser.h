/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __PREPROCESSOR_LINE_TOKENISER_H__
#define __PREPROCESSOR_LINE_TOKENISER_H__
#include "ASCIITree.h"
#include "PPToken.h"
#include "PreprocessorParser.h"
#include "BaseLib/ExternalString.h"
#include "PPLine.h"
#include "PPText.h"
#include "PPTokenMemory.h"


class CPreprocessorLineTokensier
{
protected:
	CPPTokenList*			mpcTokenHolder;
	char*					mszWhitespaceStart;
	char*					mszIdentifierStart;
	char*					mszDecorationStart;
	char*					mszDoubleQuoteStart;
	char*					mszSingleQuoteStart;
	char*					mszHashStart;
	char*					mszNumberStart;
	bool					mbOnlyWhitespace;
	CPreprocessorParser*	mpcParser;
	CPPTokenMemory*			mpcTokens;
	bool					mbContainsLineContinuers;
	bool					mbAllowEscapes;

public:
	static void		Preprocess(CPPTokenList* pcLinesTokens, CPreprocessorParser* pcParser, CPPTokenMemory* pcTokens, bool bAllowEscapes);

public:
	void		Init(CPPTokenList* pcLinesTokens, CPreprocessorParser* pcParser, CPPTokenMemory* pcTokens, bool bAllowEscapes);
	void		Kill(void);
	void		Preprocess(void);
	bool		PossibleComment(void);
	void		AddRelevantToken(void);
	CPPText* 	AddText(EPreprocessorText eType, char* szStart, char* szEndExclusive);
	void		AddDoubleQuotedToken(void);
	void		AddSingleQuotedToken(void);
	void		NullAll(void);
	void		ReplaceEscapeCodes(CChars* psz, char* szStart, char* szEnd, char cQuotes);
	void		ReplaceLineContinuers(CChars* psz, char* szStart, char* szEnd);
};


#endif // !__PREPROCESSOR_LINE_TOKENISER_H__

