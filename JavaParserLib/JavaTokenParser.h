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
#ifndef __JAVA_TOKEN_PARSER_H__
#define __JAVA_TOKEN_PARSER_H__
#include "BaseLib/TextParser.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/EnumeratorTemplate.h"
#include "JavaTokenMemory.h"
#include "JavaTokenDefinitions.h"
#include "JavaTokenKeyword.h"


class CJavaTokenParser
{
protected:
	CJavaTokenMemory*		mpcTokens;
	CTextParser				mcParser;
	CChars					mszFileName;

	CJavaTokenDefinitions*	mpcDefinitions;

	CJavaToken*				mpcStart;

public:
	void			Init(CJavaTokenDefinitions* pcDefinitions, CJavaTokenMemory* pcTokens, char* szFilename, char* szText);
	void 			Init(CJavaTokenDefinitions* pcDefinitions, CJavaTokenMemory* pcTokens, char* szFilename, char* szText, int iTextLen);
	void 			Kill(void);

	TRISTATE		Parse(void);
	BOOL			Parse(BOOL bFailOnError);

	CJavaToken*		GetFirstToken(void);

	void			PrettyPrint(CChars* pszDest);
	void			TypePrint(CChars* pszDest);
	void			Dump(BOOL bIncludeType = FALSE);

protected:
	CJavaToken*		GetLineEndToken(CJavaToken* pcStartToken);
	void			PrintLine(CChars* pszDest, CJavaToken* pcStartToken, CJavaToken* pcEndToken);
	int				ChangeDepth(CJavaToken* pcStartToken, CJavaToken* pcEndToken);
	BOOL			PrintSpace(CJavaToken* pcLeft, CJavaToken* pcRight);
	BOOL			PrintNewLine(CJavaToken* pcStartToken, CJavaToken* pcLastStartToken);
	BOOL			IsMethodDeclaration(CJavaToken* pcStartToken, CJavaToken* pcEndToken);
	BOOL			IsAnnotation(CJavaToken* pcStartToken, CJavaToken* pcEndToken);

	TRISTATE		ParseComment(CJavaToken** ppcCurrent);
	TRISTATE		ParseKeyword(CJavaToken** ppcCurrent);
	TRISTATE		ParseAmbiguous(CJavaToken** ppcCurrent);
	TRISTATE		ParseOperator(CJavaToken** ppcCurrent);
	TRISTATE		ParseSeparator(CJavaToken** ppcCurrent);
	TRISTATE		ParseScope(CJavaToken** ppcCurrent);
	TRISTATE		ParseBoolean(CJavaToken** ppcCurrent);
	TRISTATE		ParseIdentifier(CJavaToken** ppcCurrent);
	TRISTATE		ParseFloat(CJavaToken** ppcCurrent);
	TRISTATE		ParseInteger(CJavaToken** ppcCurrent);
	TRISTATE		ParseCharacter(CJavaToken** ppcCurrent);
	TRISTATE		ParseString(CJavaToken** ppcCurrent);
	TRISTATE		ParseNull(CJavaToken** ppcCurrent);
	TRISTATE		ParseAnnotation(CJavaToken** ppcCurrent);
};


#endif // !__JAVA_TOKEN_PARSER_H__

