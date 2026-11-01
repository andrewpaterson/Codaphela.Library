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
	char*					mszFileName;
	CArrayPPComments*		mpacComments;

public:
	void 					Init(char* szFileName);
	void					SetComments(CArrayPPComments* pacComments);
	void 					Kill(void);

	bool					TokeniseIntoBlockSets(CPPBlockSetArray* pacBlockSets, CPPTokenMemory* pcTokenMemory, char* szPos, int iLength);
	void					TokeniseDefine(CPPTokenList* pcHolder, char* sz, CPPTokenMemory* pcTokens);
	EPreprocessorDirective	GetDirective(void);
	CPPDirective*			TokeniseDirective(CPPTokenMemory* pcTokens, EPreprocessorDirective eDirective, int iBlock, int iIndex);
	CPPLine*				Line(CPPTokenMemory* pcTokens);

	CPPDirective*			TokeniseHashInclude(CPPTokenMemory* pcTokens);
	CPPDirective*			TokeniseHashConditional(CPPTokenMemory* pcTokens, EPreprocessorDirective eDirective, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashPlain(CPPTokenMemory* pcTokens, EPreprocessorDirective eDirective);

	void					NextConditional(CPPConditional* pcCurrent);
};


#endif // !__PREPROCESSOR_TOKENISER_H__

