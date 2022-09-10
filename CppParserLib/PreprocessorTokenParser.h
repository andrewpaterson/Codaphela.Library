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
#ifndef __PREPROCESSOR_TOKEN_PARSER_H__
#define __PREPROCESSOR_TOKEN_PARSER_H__
#include "BaseLib/Define.h"
#include "PPLine.h"
#include "BaseLib/ExternalString.h"


struct SPPHolderMark
{
	CPPToken*		pcCurrentToken;
	int				iTokenIndex;
	CPPTokenListHolder*		pcHolder;

	void Init(CPPTokenListHolder* pcHolder, CPPToken* pcCurrentToken, int iTokenIndex);
	void Kill(void);
};


typedef CArrayTemplate<SPPHolderMark>		CArrayPPHolderMark;


struct SPPTPPosition
{
	CArrayPPHolderMark	acPPHolders;

	void Init(CArrayPPHolderMark* pacPPHolders);
	void Kill(void);
};


typedef CArrayTemplate<SPPTPPosition>	CArrayPPTPPositions;


class CPreprocessorTokenParser
{
public:
	CArrayPPTPPositions		macStack;
	CArrayPPHolderMark		macPPHolderMark;
	SPPHolderMark*			mpsCurrent;

	void 		Init(void);
	void 		Init(CPPTokenListHolder* pcLine);
	void 		Kill(void);
	void		Bump(void);
	void		MarkDown(CPPTokenListHolder* pcHolder);
	void		MarkUp(void);
	bool 		GetIdentifier(CExternalString* pcIdentifier);
	bool		GetNumber(CExternalString* pcIdentifier);
	bool 		GetExactDecorator(char c, bool bSkipWhiteSpace = true);
	bool		GetExactDecorator(char* sz, bool bSkipWhiteSpace = true);
	bool 		GetExactIdentifier(char* szIdentifier, bool bCaseSensitive, bool bSkipWhiteSpace);
	void 		SkipWhiteSpace(void);
	bool 		NextToken(void);
	void		NextToken(int iNumToSkip);
	bool 		GetQuotedCharacterSequence(char cOpen, char cClosed, CExternalString* pcSequence);
	bool 		GetStringDoubleQuoted(CExternalString* pcString);
	bool 		GetStringSingleQuoted(CExternalString* pcString, bool bSkipWhiteSpace);
	bool		GetInteger(uint64* pulli, bool bSkipWhiteSpace, bool bTestSign);
	bool		GetFloat(long double* pf);
	void 		PushPosition(void);
	void 		PopPosition(void);
	void 		PassPosition(void);
	int			Line(void);
	int			Column(void);
	void		AppendRemaining(CChars* psz);
	bool		AppendRemaining(CChars* psz, CPPTokenListHolder* pcHolder, bool bAppending);
	bool		GetDigits(uint64* pulli, int* piNumDecimals);
	bool		GetHexadecimal(uint64* pulli, int* piNumDigits);
	bool		GetOctal(uint64* pulli, int* piNumDigits);
	bool		HasToken(void);
	CPPToken*	GetToken(void);
	bool		HasTokens(void);
	bool		ToDecortatorString(char* szDest, int iMaxLen, int* piLen);
	void		AssignCurrent(CPPToken* pcToken, int iTokenIndex);
	void		Dump(void);
	void		DumpAll(void);
};


#endif // !__PREPROCESSOR_TOKEN_PARSER_H__

