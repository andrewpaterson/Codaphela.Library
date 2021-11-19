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
	CPPAbstractHolder*		pcHolder;

	void Init(CPPAbstractHolder* pcHolder, CPPToken* pcCurrentToken, int iTokenIndex);
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
	void 		Init(CPPAbstractHolder* pcLine);
	void 		Kill(void);
	void		Bump(void);
	void		MarkDown(CPPAbstractHolder* pcHolder);
	void		MarkUp(void);
	BOOL 		GetIdentifier(CExternalString* pcIdentifier);
	BOOL		GetNumber(CExternalString* pcIdentifier);
	BOOL 		GetExactDecorator(char c, BOOL bSkipWhiteSpace = TRUE);
	BOOL 		GetExactIdentifier(char* szIdentifier, BOOL bCaseSensitive, BOOL bSkipWhiteSpace);
	void 		SkipWhiteSpace(void);
	void 		NextToken(void);
	void		NextToken(int iNumToSkip);
	char*		NextCharacter(void);
	BOOL 		GetQuotedCharacterSequence(char cOpen, char cClosed, CExternalString* pcSequence);
	BOOL 		GetStringDoubleQuoted(CExternalString* pcString);
	BOOL 		GetStringSingleQuoted(CExternalString* pcString, BOOL bSkipWhiteSpace);
	BOOL		GetInteger(uint64* pulli, BOOL bSkipWhiteSpace, BOOL bTestSign);
	BOOL		GetFloat(long double* pf);
	void 		PushPosition(void);
	void 		PopPosition(void);
	void 		PassPosition(void);
	int			Line(void);
	int			Column(void);
	void		AppendRemaining(CChars* psz);
	BOOL		AppendRemaining(CChars* psz, CPPAbstractHolder* pcHolder, BOOL bAppending);
	BOOL		GetDigits(uint64* pulli, int* piNumDecimals);
	BOOL		GetHexadecimal(uint64* pulli, int* piNumDigits);
	BOOL		GetOctal(uint64* pulli, int* piNumDigits);
	BOOL		HasToken(void);
	CPPToken*	GetToken(void);
	BOOL		HasTokens(void);
	BOOL		ToDecortatorString(char* szDest, int iMaxLen, int* piLen);
	void		AssignCurrent(CPPToken* pcToken, int iTokenIndex);
	void		Dump(void);
	void		DumpAll(void);
};


#endif // !__PREPROCESSOR_TOKEN_PARSER_H__

