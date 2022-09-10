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
#include "JavaTokenParser.h"
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

	CChars					mszFilename;
	CExternalString			mcText;
	CLogger*				mpcLogger;

	int						miErrors;

public:
	void				Init(CJavaSyntaxMemory* pcSyntaxes, CJavaTokenParser* pcTokenParser);
	void 				Kill(void);

	bool				Parse(void);
	bool				Parse(bool bFailOnError);

	CJavaSyntaxFile*	GetSyntaxFile(void);
	char*				GetFilename(void);

	char*				PrettyPrint(CChars* pszDest);
	void				TypePrint(CChars* pszDest);
	void				Dump(bool bIncludeType = false);

protected:
	void	PushPosition(void);
	void	PopPosition(void);
	void	PassPosition(void);

protected:
	CJavaSyntaxPackage*					ParsePackage(CJavaSyntax* pcParent);
	CJavaSyntaxImport*					ParseImport(CJavaSyntax* pcParent);
	CJavaSyntaxClass*					ParseClass(CJavaSyntax* pcParent);
	CJavaSyntaxEnum*					ParseEnum(CJavaSyntax* pcParent);
	CJavaSyntaxInterface*				ParseInterface(CJavaSyntax* pcParent);
	CJavaSyntaxType*					ParseType(CJavaSyntax* pcParent);
	CJavaSyntaxExtent*					ParseExtent(CJavaSyntax* pcParent);
	CJavaSyntaxTypeCommon*				ParseTypeCommon(CJavaSyntax* pcParent);
	CJavaSyntaxGeneric*					ParseGeneric(CJavaSyntax* pcParent);
	CJavaSyntaxClassBlock*				ParseClassBlock(CJavaSyntax* pcParent);
	CJavaSyntaxStatement*				ParseClassBlockStatement(CJavaSyntax* pcParent);
	CJavaSyntaxVariableDeclaration*		ParseClassVariable(CJavaSyntax* pcParent);
	CJavaSyntaxVariableInitialiser*		ParseVariableInitialiser(CJavaSyntax* pcParent);
	CJavaSyntaxVariableInitialiser*		ParseArrayVariableInitialiser(CJavaSyntax* pcParent);
	CJavaSyntaxParentheses*				ParseParentheses(CJavaSyntax* pcParent);
	CJavaSyntaxOperator*				ParsePrefixOperator(CJavaSyntax* pcParent);
	CJavaSyntaxOperator*				ParseOperator(CJavaSyntax* pcParent);

	CJavaSyntaxValueExpression*			ParseExpression(CJavaSyntax* pcParent);
	CJavaSyntaxArrayValueExpression*	ParseArrayExpression(CJavaSyntax* pcParent);
	CJavaSyntaxLiteral*					ParseLiteral(CJavaSyntax* pcParent);
	CJavaSyntaxMethodCall*				ParseMethodCall(CJavaSyntax* pcParent);

protected:
	CJavaModifiers			ParseModifiers(uint8 uiAllowedModifiers);
	int						ParseArrayDeclaration(void);

	bool					GetKeyword(EJavaTokenKeyword eKeyword);
	bool					GetSeparator(EJavaTokenSeparator eSeparator);
	CJavaTokenIdentifier*	GetIdentifier(void);
	bool					GetScope(EJavaTokenScope eGeneric);
	bool					GetAmbiguous(ECJavaTokenAmbiguous eAmbiguous, CCJavaTokenAmbiguous** ppcAmbiguous = NULL);
	bool					GetOperator(EJavaTokenOperator eOperator);
	CJavaTokenKeyword*		GetModifierKeyword(void);
	CJavaTokenKeyword*		GetPrimitveKeyword(void);
	CJavaTokenLiteral*		GetLiteral(void);
	CJavaTokenOperator*		GetPrefixOperator(void);
	CJavaTokenOperator*		GetOperator(void);

	bool					IsKeyword(CJavaToken* pcToken, EJavaTokenKeyword eKeyword);
	bool					IsSeparator(CJavaToken* pcToken, EJavaTokenSeparator eSeparator);
	bool					IsScope(CJavaToken* pcToken, EJavaTokenScope eGeneric);
	bool					IsAmbiguous(CJavaToken* pcToken, ECJavaTokenAmbiguous eAmbiguous);
	bool					IsOperator(CJavaToken* pcToken, EJavaTokenOperator eOperator);
	bool					IsLiteral(CJavaToken* pcToken, EJavaTokenLiteralType eLiteralType);
	bool					IsIdentifier(CJavaToken* pcToken);

	void					Next(void);
	bool					HasNext(void);
	void					SkipComments(void);

	bool					ReplaceAmbiguous(CJavaToken* pcSearch, CJavaToken* pcReplacement);
	CJavaTokenScope*		CreateScope(STextPosition* psPosition, EJavaTokenScope eScope);
	void					PrivateError(char* szError);

protected:
	template<class M>	M*	Error(char* szError);
	template<class M>	M*	Error(void);
	template<class M>	M*	Mismatch(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CJavaSyntaxParser::Error(char* szError)
{
	PrivateError(szError);
	return (M*)&mcError;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CJavaSyntaxParser::Error(void)
{
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

