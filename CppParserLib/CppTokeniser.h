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
#ifndef __C_P_P_TOKENISER_H__
#define __C_P_P_TOKENISER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/MemoryStackExtended.h"
#include "ASCIIIndex.h"
#include "CppTokens.h"
#include "CppToken.h"
#include "PreprocessedTokens.h"


typedef CArrayTemplate<CCPPToken*>	CCTArrayTokens;


//Turns preprocessed tokens into C++ tokens: keywords and punctuators, identifiers (each name is given an ID) and
//literals with their values.  [lex.phases] 7
class CCPPTokeniser
{
public:
	CASCIIIndex				mcReservedWords;
	CASCIIIndex				mcIdentifiers;
	CCTArrayTokens			mcTokens;
	CMemoryStackExtended	mcStack;
	CPreprocessedTokens*	mpcSource;
	bool					mbError;

	int64					mlliOverride;
	int64					mlliFinal;

	void 				Init(void);
	void 				Kill(void);
	bool 				Tokenise(CPreprocessedTokens* pcSource);

	size				NumTokens(void);
	CCPPToken*			Get(size iIndex);
	SPreprocessedToken*	GetSource(CCPPToken* pcToken);
	int64				GetIdentifierID(char* szName);
	char*				GetIdentifier(int64 lliID, int* piLength = NULL);
	char*				GetReservedWord(ECppTokens eToken);
	void				Print(CChars* psz);

protected:
	void				Clear(void);
	bool				Token(SPreprocessedToken* psToken, int iSource);
	bool				Identifier(SPreprocessedToken* psToken, int iSource);
	bool				Punctuator(SPreprocessedToken* psToken, int iSource);
	bool				Number(SPreprocessedToken* psToken, int iSource);
	bool				Integer(SPreprocessedToken* psToken, int iSource, char* szDigits, int iBase, char* szSuffix);
	bool				Floating(SPreprocessedToken* psToken, int iSource, char* szNumber);
	bool				Character(SPreprocessedToken* psToken, int iSource);
	bool				String(SPreprocessedToken* psToken, int iSource);
	bool				Escape(const char** psz, const char* szEnd, uint32* puiValue);
	void				AddToken(CCPPToken* pcToken, int iSource);
	bool				Error(SPreprocessedToken* psToken, char* szError);
};


#endif // !__C_P_P_TOKENISER_H__

