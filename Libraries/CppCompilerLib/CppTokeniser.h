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
#ifndef __C_P_P_TOKENISER_H__
#define __C_P_P_TOKENISER_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/MemoryStackExtended.h"
#include "ASCIIIndex.h"
#include "CppTokens.h"
#include "CppToken.h"
#include "PreprocessedTokens.h"
#include "CPPErrors.h"
#include "LiteralHelper.h"


typedef CArrayTemplatePtr<CCPPToken>	CCTArrayTokens;


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
	CCPPErrors				mcErrors;
	int						miSource;			//The preprocessed token being tokenised.

	int64					mlliOverride;
	int64					mlliFinal;
	int64					mlliFunc;				//__func__, __FUNCTION__, __PRETTY_FUNCTION__, __FUNCSIG__ and __FUNCDNAME__: the name of the
	int64					mlliFunction;			//function they are written in (CCPPUnit::FunctionNameText).
	int64					mlliPrettyFunction;
	int64					mlliFuncSig;
	int64					mlliFuncDName;
	int64					mlliMemcpy;				//memcpy and __builtin_memcpy, which the compiler may write as a copy (CCPPUnit::IsInlineMemcpy).
	int64					mlliBuiltinMemcpy;

	void				Init(CMallocator* pcTokens = NULL);
	void				Kill(void);
	bool				Tokenise(CPreprocessedTokens* pcSource);

	size				NumTokens(void) const;
	CCPPToken*			Get(size iIndex);
	SPreprocessedToken*	GetSource(CCPPToken* pcToken);
	int64				GetIdentifierID(char* szName);
	char*				GetIdentifier(int64 lliID, int* piLength = NULL);
	char*				GetReservedWord(ECppTokens eToken);
	void				Print(CChars* psz);
	CCPPErrors*			GetErrors(void);

protected:
	void				Clear(void);
	bool				Token(SPreprocessedToken* psToken);
	bool				Identifier(SPreprocessedToken* psToken);
	bool				Punctuator(SPreprocessedToken* psToken);
	bool				Literal(SPreprocessedToken* psToken);
	bool				FollowsNumber(SPreprocessedToken* psToken);
	CCPPToken*			Number(SPreprocessedToken* psToken, size* puiLength);
	CCPPToken*			Integer(SNumberLiteral* psLiteral);
	CCPPToken*			Floating(SPreprocessedToken* psToken, SNumberLiteral* psLiteral);
	CCPPToken*			Character(SPreprocessedToken* psToken, size uiLength);
	CCPPToken*			String(SPreprocessedToken* psToken, size uiLength);
	void				AddToken(CCPPToken* pcToken);
	int64				GetIdentifierID(const char* szName, size uiLength);
	bool				Error(SPreprocessedToken* psToken, const char* szError);
	bool				SharedError(SPreprocessedToken* psToken, const char* szError);
};


#endif // !__C_P_P_TOKENISER_H__

