/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2021 Andrew Paterson

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
#ifndef __PP_DEFINE_H__
#define __PP_DEFINE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ExternalString.h"
#include "PPLine.h"
#include "PreprocessedTokens.h"


#define DEFINE_FLAGS_BRACKETED	0x01
#define DEFINE_FLAGS_SPECIAL	0x02
#define DEFINE_FLAGS_IN_MAP		0x04
#define DEFINE_FLAGS_VARIADIC	0x08
#define DEFINE_FLAGS_KILLED		0x80


class CDefineMap;
class CDefine
{
protected:
	CPPLine			mcReplacement;
	CArrayChars		maszArguments;
	int64			mlliID;			//If a token is undef'd and redef'd its mlliID changes.
	CDefineMap*		mpcDefineMap;
	uint32			miFlags;
	CChars			mszName;
	CSourceFile*	mpcFile;		//Where it was #defined, NULL for predefined and configuration macros.
	CArrayPreprocessedTokens	macBody;		//The replacement as preprocessed tokens, made when first expanded.
	bool			mbBodyConverted;

public:
	void 			Init(CExternalString* pcName, int64 uiID, CDefineMap* pcDefineMap);
	void 			Kill(void);
	void 			AddArgument(CExternalString* pcName);
	void			SetVariadic(void);
	void 			AddReplacementToken(CPPToken* pcToken);
	bool 			Equals(CDefine* pcOther);
	char*			GetName(void);
	int64			GetID(void);
	bool			IsBracketed(void);
	bool			IsVariadic(void);
	bool			IsSpecial(void);
	bool			IsInMap(void);
	void			SetSpecial(bool b);
	void			SetBracketed(bool b);
	CArrayChars*	GetArguments(void);
	CPPLine*		GetReplacement(void);
	void			SetFile(CSourceFile* pcFile);
	CSourceFile*	GetFile(void);
	bool			IsBodyConverted(void);
	CArrayPreprocessedTokens*	GetBody(void);
	void			SetBodyConverted(void);
	void			Dump(void);
};


#endif // !__PP_DEFINE_H__

