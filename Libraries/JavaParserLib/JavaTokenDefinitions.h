/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __JAVA_TOKEN_DEFINITIONS_H__
#define __JAVA_TOKEN_DEFINITIONS_H__
#include "BaseLib/TextParser.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePrimitive.h"
#include "BaseLib/Logger.h"
#include "JavaTokenMemory.h"
#include "JavaTokenKeyword.h"


//Token definitions indexed by their token enum, with their names alongside for CTextParser::GetLongestIdentifier and GetLongestCharacterSequence.
template<class M>
class CJavaTokenDefinitionTable
{
protected:
	CArrayTemplate<M>						macDefinitions;
	CArrayTemplatePrimitive<const char*>	maszNames;

public:
	void				Init(void);
	void				Kill(void);

	M*					Add(size uiIndex, const char* szName);
	M*					Get(size uiIndex);
	const char* const*	GetNames(void);
	size				NumElements(void);
};


typedef CJavaTokenDefinitionTable<CJavaTokenKeywordDefinition>		CKeywordDefinitions;
typedef CJavaTokenDefinitionTable<CJavaTokenSeparatorDefinition>	CSeparatorDefinitions;
typedef CJavaTokenDefinitionTable<CJavaTokenOperatorDefinition>		COperatorDefinitions;
typedef CJavaTokenDefinitionTable<CCJavaTokenAmbiguousDefinition>	CAmbiguousDefinitions;
typedef CJavaTokenDefinitionTable<CJavaTokenScopeDefinition>		CScopeDefinitions;


class CJavaTokenDefinitions
{
protected:
	CKeywordDefinitions		mcKeywords;
	COperatorDefinitions	mcOperators;
	CSeparatorDefinitions	mcSeparators;
	CAmbiguousDefinitions	mcAmbiguous;
	CScopeDefinitions		mcScopes;

public:
	void 						Init(void);
	void 						Kill(void);

	CKeywordDefinitions*		GetKeywords(void);
	CSeparatorDefinitions*		GetSeparators(void);
	COperatorDefinitions*		GetOperators(void);
	CAmbiguousDefinitions*		GetAmbiguous(void);
	CScopeDefinitions*			GetScopes(void);

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

};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CJavaTokenDefinitionTable<M>::Init(void)
{
	macDefinitions.Init();
	maszNames.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CJavaTokenDefinitionTable<M>::Kill(void)
{
	size	i;
	size	uiNumElements;

	uiNumElements = macDefinitions.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		macDefinitions.Get(i)->Kill();
	}
	maszNames.Kill();
	macDefinitions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CJavaTokenDefinitionTable<M>::Add(size uiIndex, const char* szName)
{
	if (uiIndex != macDefinitions.NumElements())
	{
		gcLogger.Error2(__METHOD__, " Token definition [", szName, "] must be added in the order of its enum.", NULL);
		return NULL;
	}

	maszNames.Add(szName);
	return macDefinitions.Add();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CJavaTokenDefinitionTable<M>::Get(size uiIndex)
{
	return macDefinitions.SafeGet(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
const char* const* CJavaTokenDefinitionTable<M>::GetNames(void)
{
	return maszNames.GetData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CJavaTokenDefinitionTable<M>::NumElements(void)
{
	return macDefinitions.NumElements();
}


#endif // !__JAVA_TOKEN_DEFINITIONS_H__

