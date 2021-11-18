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
#include "ASCIITree.h"
#include "PreprocessorParser.h"
#include "PPDirective.h"

#define TOKENS_CHUNK_SIZE_TOKENISER	32


class CPPConditional;
class CPreprocessorTokeniser
{
public:
	CASCIITree				mcDirectives;
	CPreprocessorParser		mcParser;
	CPPConditional*			mpcPrev;

	void 					Init(void);
	void 					Kill(void);

	BOOL					Tokenise(CArrayCBlockSet* pacBlockSets, char* szPos, int iLength, BOOL bAllowAnnotations);
	BOOL					Tokenise(CPPTokenHolder* pcTokens, CMemoryStackExtended* pcStackGetIndex, char* szPos, int iLength, BOOL bAllowAnnotations, int iBlock, int iIndex);
	void					TokeniseDefine(CPPTokenHolder* pcHolder, char* sz, CMemoryStackExtended* pcStackGetIndex);
	EPreprocessorDirective	GetDirective(void);
	CPPDirective*			TokeniseDirective(CMemoryStackExtended* mpcStackGetIndex, EPreprocessorDirective eDirective, int iBlock, int iIndex);
	CPPLine*				Line(CMemoryStackExtended* mpcStackGetIndex);

	CPPDirective*			TokeniseHashDefine(CMemoryStackExtended* mpcStackGetIndex);
	CPPDirective*			TokeniseHashUndef(CMemoryStackExtended* mpcStackGetIndex);
	CPPDirective*			TokeniseHashInclude(CMemoryStackExtended* mpcStackGetIndex);
	CPPDirective*			TokeniseHashEndif(CMemoryStackExtended* mpcStackGetIndex, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashIfndef(CMemoryStackExtended* mpcStackGetIndex, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashIfdef(CMemoryStackExtended* mpcStackGetIndex, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashElse(CMemoryStackExtended* mpcStackGetIndex, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashIf(CMemoryStackExtended* mpcStackGetIndex, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashElif(CMemoryStackExtended* mpcStackGetIndex, int iBlock, int iIndex);
	CPPDirective*			TokeniseHashError(CMemoryStackExtended* mpcStackGetIndex);
	CPPDirective*			TokeniseHashPragma(CMemoryStackExtended* mpcStackGetIndex);

	CPPToken*				AddToken(CPPToken* pcToken, CArrayPPTokens* pcTokens);
	void					NextConditional(CPPConditional* pcCurrent);
};


#endif // !__PREPROCESSOR_TOKENISER_H__

