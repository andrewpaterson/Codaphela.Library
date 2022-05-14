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
#ifndef __JAVA_TOKEN_DEFINITIONS_H__
#define __JAVA_TOKEN_DEFINITIONS_H__
#include "BaseLib/TextParser.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/EnumeratorTemplate.h"
#include "JavaTokenMemory.h"
#include "JavaTokenKeyword.h"


typedef CEnumeratorTemplate<CJavaTokenKeywordDefinition> CKeywordEnumerator;
typedef CEnumeratorTemplate<CJavaTokenSeparatorDefinition> CSeparatorEnumerator;
typedef CEnumeratorTemplate<CJavaTokenOperatorDefinition> COperatorEnumerator;
typedef CEnumeratorTemplate<CCJavaTokenAmbiguousDefinition> CAmbiguousEnumerator;
typedef CEnumeratorTemplate<CJavaTokenScopeDefinition> CScopeEnumerator;


class CJavaTokenDefinitions
{
protected:
	CKeywordEnumerator		mcKeywords;
	COperatorEnumerator		mcOperators;
	CSeparatorEnumerator	mcSeparators;
	CAmbiguousEnumerator	mcAmbiguous;
	CScopeEnumerator		mcScopes;

public:
	void 						Init(void);
	void 						Kill(void);

	CKeywordEnumerator*			GetKeywords(void);
	CSeparatorEnumerator*		GetSeparators(void);
	COperatorEnumerator*		GetOperators(void);
	CAmbiguousEnumerator*		GetAmbiguous(void);
	CScopeEnumerator*			GetScopes(void);

	CJavaTokenKeywordDefinition*		GetKeyword(EJavaTokenKeyword eKeyword);
	CJavaTokenOperatorDefinition*	GetOperator(EJavaTokenOperator eOperator);
	CJavaTokenSeparatorDefinition*	GetSeparator(EJavaTokenSeparator eSeparator);
	CCJavaTokenAmbiguousDefinition*	GetAmbiguous(ECJavaTokenAmbiguous eAmbiguous);
	CJavaTokenScopeDefinition*		GetScope(EJavaTokenScope eGeneric);

protected:
	void						InitKeywords(void);
	void						InitOperators(void);
	void						InitSeparators(void);
	void						InitAmbiguous(void);
	void						InitGenerics(void);

	void						AddKeywordDefinition(char* szKeyword, EJavaTokenKeyword eKeyword);
	void						AddSeparatorDefinition(char* szSeparator, EJavaTokenSeparator eSeparator);
	void						AddOperatorDefinition(EJavaTokenOperatorType eType, EJavaTokenOperator eOperator, char* szOperator);
	void						AddAmbiguousDefinition(char* szAmbiguous, ECJavaTokenAmbiguous eAmbiguous);
	void						AddGenericDefinition(char* szGeneric, EJavaTokenScope eGeneric);

	void						KillKeywords(void);
	void						KillOperators(void);
	void						KillSeparators(void);
	void						KillAmbiguous(void);
	void						KillGenerics(void);
};


#endif // !__JAVA_TOKEN_DEFINITIONS_H__

