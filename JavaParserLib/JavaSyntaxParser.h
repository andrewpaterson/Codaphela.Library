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
#ifndef __JAVA_SYNTAX_PARSER_H__
#define __JAVA_SYNTAX_PARSER_H__
#include "JavaSyntaxMemory.h"
#include "JavaTokenDefinitions.h"
#include "JavaSyntaxTree.h"


class CJavaSyntaxParser
{
protected:
	CJavaSyntaxMemory		mcSyntaxes;

	CJavaTokenDefinitions*	mpcDefinitions;

	CJavaToken*				mpcFirstToken;
	CJavaSyntaxTree			mcSyntaxTree;

public:
	void			Init(CJavaTokenDefinitions* pcDefinitions, char* szFilename, CJavaToken* pcFirstToken);
	void 			Kill(void);

	TRISTATE		Parse(void);
	BOOL			Parse(BOOL bFailOnError);

	void			PrettyPrint(CChars* pszDest);
	void			TypePrint(CChars* pszDest);
	void			Dump(BOOL bIncludeType = FALSE);

protected:
	BOOL			IsKeyword(CJavaToken* pcToken, EJavaKeyword eKeyword);
	BOOL			IsSeparator(CJavaToken* pcToken, EJavaSeparator eSeparator);
	BOOL			IsGeneric(CJavaToken* pcToken, EJavaGeneric eGeneric);
	BOOL			IsAmbiguous(CJavaToken* pcToken, EJavaAmbiguous eAmbiguous);
	BOOL			IsOperator(CJavaToken* pcToken, EJavaOperator eOperator);
	BOOL			IsLiteral(CJavaToken* pcToken, EJavaLiteralType eLiteralType);

	CJavaToken*		SkipComments(CJavaToken* pcToken);
};


#endif // !__JAVA_SYNTAX_PARSER_H__

