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
#ifndef __PREPROCESSOR_TOKENISER_H__
#define __PREPROCESSOR_TOKENISER_H__
#include "BaseLib/Define.h"
#include "CFile.h"
#include "DirectiveMap.h"
#include "PreprocessorParser.h"
#include "PPDirective.h"

#define TOKENS_CHUNK_SIZE_TOKENISER	32


class CPPConditional;
class CPreprocessorTokeniser
{
public:
	CDirectiveMap			mcDirectives;
	CPreprocessorParser		mcParser;
	CPPConditional*			mpcPrev;

	void 					Init(void);
	void 					Kill(void);

	BOOL					Tokenise(CArrayCBlockSet* pacBlockSets, char* szPos, int iLength, BOOL bAllowAnnotations);
	BOOL					Tokenise(CPPTokenHolder* pcTokenHolders, CPPTokens* pcTokens, char* szPos, int iLength, BOOL bAllowAnnotations, int iBlock, int iIndex);
	void					TokeniseDefine(CPPTokenHolder* pcHolder, char* sz, CPPTokens* pcTokens);
	EPreprocessorDirective	GetDirective(void);
	CPPDirective*			TokeniseDirective(CPPTokens* pcTokens, EPreprocessorDirective eDirective, int iBlock, int iIndex);
	CPPLine*				Line(CPPTokens* pcTokens);

	CPPDirective*			TokeniseHashDefine(CPPTokens* pcTokens);
	CPPDirective*			TokeniseHashUndef(CPPTokens* pcTokens);
	CPPDirective*			TokeniseHashInclude(CPPTokens* pcTokens);
	CPPDirective*			TokeniseHashEndif(CPPTokens* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashIfndef(CPPTokens* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashIfdef(CPPTokens* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashElse(CPPTokens* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashIf(CPPTokens* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashElif(CPPTokens* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashError(CPPTokens* pcTokens);
	CPPDirective*			TokeniseHashPragma(CPPTokens* pcTokens);

	CPPToken*				AddToken(CPPToken* pcToken, CArrayPPTokenPtrs* pcTokens);
	void					NextConditional(CPPConditional* pcCurrent);
};


#endif // !__PREPROCESSOR_TOKENISER_H__

