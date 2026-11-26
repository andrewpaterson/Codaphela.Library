/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

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


class CPreprocessorLineTokeniser
{
protected:
	CPPTokenList*			mpcTokenHolder;
	const char*				mszWhitespaceStart;
	const char*				mszIdentifierStart;
	const char*				mszDecorationStart;
	const char*				mszDoubleQuoteStart;
	const char*				mszSingleQuoteStart;
	const char*				mszHashStart;
	size					muiHashes;			//# and %: in the hashes from mszHashStart.
	const char*				mszNumberStart;
	bool					mbOnlyWhitespace;
	CPreprocessorParser*	mpcParser;
	CPPTokenMemory*			mpcTokens;
	bool					mbContainsLineContinuers;
	bool					mbContainsUniversalNames;	//The token being read has \u or \U names in it, outside a literal.
	int						miStartLine;
	int						miStartColumn;

public:
	static void		Preprocess(CPPTokenList* pcLinesTokens, CPreprocessorParser* pcParser, CPPTokenMemory* pcTokens);

public:
	void		Init(CPPTokenList* pcLinesTokens, CPreprocessorParser* pcParser, CPPTokenMemory* pcTokens);
	void		Kill(void);
	void		Preprocess(void);
	bool		PossibleComment(void);
	void		Quote(const char** pszStart, bool bInOther);
	void		EndQuoted(const char** pszStart, bool bString);
	void		IdentifierCharacter(void);
	bool		IsDigitSeparator(void) const;
	bool		IsPPNumberContinuation(void) const;
	bool		IsPPNumberStart(void) const;
	void		AddRelevantToken(void);
	CPPText*	AddText(EPreprocessorText eType, const char* szStart, const char* szEndExclusive);
	void		NullAll(void);
	bool		IsDigraphHash(void);
	void		MarkStart(void);
	void		Respell(CChars* psz, const char* szStart, const char* szEnd);
};


#endif // !__PREPROCESSOR_LINE_TOKENISER_H__

