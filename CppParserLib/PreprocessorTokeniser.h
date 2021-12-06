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
#include "SourceFile.h"
#include "DirectiveMap.h"
#include "PreprocessorParser.h"
#include "PPDirective.h"

#define TOKENS_CHUNK_SIZE_TOKENISER	32


class CPPConditional;
class CPreprocessorTokeniser
{
protected:
	CDirectiveMap			mcDirectives;
	CPreprocessorParser		mcParser;
	CPPConditional*			mpcPrev;

public:
	void 					Init(void);
	void 					Kill(void);

	BOOL					Tokenise(CPPBlockSetArray* pacBlockSets, CPPTokenMemory* pcTokenMemory, char* szPos, int iLength);
	BOOL					Tokenise(CPPTokenList* pcTokenHolder, CPPTokenMemory* pcTokens, char* szPos, int iLength, int iBlock, int iIndex);
	void					TokeniseDefine(CPPTokenList* pcHolder, char* sz, CPPTokenMemory* pcTokens);
	EPreprocessorDirective	GetDirective(void);
	CPPDirective*			TokeniseDirective(CPPTokenMemory* pcTokens, EPreprocessorDirective eDirective, int iBlock, int iIndex);
	CPPLine*				Line(CPPTokenMemory* pcTokens);

	CPPDirective*			TokeniseHashDefine(CPPTokenMemory* pcTokens);
	CPPDirective*			TokeniseHashUndef(CPPTokenMemory* pcTokens);
	CPPDirective*			TokeniseHashInclude(CPPTokenMemory* pcTokens);
	CPPDirective*			TokeniseHashEndif(CPPTokenMemory* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashIfndef(CPPTokenMemory* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashIfdef(CPPTokenMemory* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashElse(CPPTokenMemory* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashIf(CPPTokenMemory* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashElif(CPPTokenMemory* pcTokens, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashError(CPPTokenMemory* pcTokens);
	CPPDirective*			TokeniseHashPragma(CPPTokenMemory* pcTokens);

	CPPToken*				AddToken(CPPToken* pcToken, CArrayPPTokenPtrs* pcTokens);
	void					NextConditional(CPPConditional* pcCurrent);
};


#endif // !__PREPROCESSOR_TOKENISER_H__

