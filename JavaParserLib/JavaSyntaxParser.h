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
#include "BaseLib/Logger.h"
#include "JavaSyntaxMemory.h"
#include "JavaTokenDefinitions.h"
#include "JavaSyntaxTree.h"
#include "JavaSyntaxError.h"
#include "JavaSyntaxMismatch.h"


class CJavaSyntaxParser
{
protected:
	CJavaSyntaxMemory*		mpcSyntaxes;
	CJavaTokenMemory*		mpcTokens;

	CJavaTokenDefinitions*	mpcDefinitions;

	CJavaToken*				mpcFirstToken;
	CJavaToken*				mpcCurrentToken;
	CJavaTokenPtrArray		mapcPositions;

	CJavaSyntaxTree			mcSyntaxTree;

	CJavaSyntaxError		mcError;
	CJavaSyntaxMismatch		mcMismatch;

	CLogger*				mpcLogger;

public:
	void	Init(CLogger* pcLogger, CJavaSyntaxMemory* pcSyntaxes, CJavaTokenDefinitions* pcDefinitions, CJavaTokenMemory* pcTokens, char* szFilename, CJavaToken* pcFirstToken);
	void 	Kill(void);

	BOOL	Parse(void);
	BOOL	Parse(BOOL bFailOnError);

	void	PrettyPrint(CChars* pszDest);
	void	TypePrint(CChars* pszDest);
	void	Dump(BOOL bIncludeType = FALSE);

protected:
	void						PushPosition(void);
	void						PopPosition(void);
	void						PassPosition(void);
	
	CJavaSyntaxPackage*			ParsePackage(void);
	CJavaSyntaxImport*			ParseImport(void);
	CJavaSyntaxClass*			ParseClass(void);
	CJavaSyntaxEnum*			ParseEnum(void);
	CJavaSyntaxInterface*		ParseInterface(void);
	CJavaSyntaxClassGeneric*	ParseClassGeneric(void);

	void						ParseClassModifier(BOOL* pbPublic, BOOL* pbAbstract, BOOL* pbFinal);

	BOOL						GetKeyword(EJavaKeyword eKeyword);
	BOOL						GetSeparator(EJavaSeparator eSeparator);
	CJavaIdentifier*			GetIdentifier(void);
	BOOL						GetScope(EJavaScope eGeneric);
	BOOL						GetAmbiguous(EJavaAmbiguous eAmbiguous, CJavaAmbiguous** ppcAmbiguous = NULL);
	BOOL						GetOperator(EJavaOperator eOperator);

	BOOL						IsKeyword(CJavaToken* pcToken, EJavaKeyword eKeyword);
	BOOL						IsSeparator(CJavaToken* pcToken, EJavaSeparator eSeparator);
	BOOL						IsScope(CJavaToken* pcToken, EJavaScope eGeneric);
	BOOL						IsAmbiguous(CJavaToken* pcToken, EJavaAmbiguous eAmbiguous);
	BOOL						IsOperator(CJavaToken* pcToken, EJavaOperator eOperator);
	BOOL						IsLiteral(CJavaToken* pcToken, EJavaLiteralType eLiteralType);
	BOOL						IsIdentifier(CJavaToken* pcToken);

	void						Next(void);
	BOOL						HasNext(void);
	void						SkipComments(void);

	void						ReplaceAmbiguous(CJavaToken* pcCurrent, CJavaToken* pcReplacement);

protected:
	template<class M>	M*	Error(char* szError);
	template<class M>	M*	Mismatch(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CJavaSyntaxParser::Error(char* szError)
{
	mpcLogger->Error(szError);
	PassPosition();
	return (M*)&mcError;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CJavaSyntaxParser::Mismatch(void)
{
	PopPosition();
	return (M*)&mcMismatch;
}


#endif // !__JAVA_SYNTAX_PARSER_H__

