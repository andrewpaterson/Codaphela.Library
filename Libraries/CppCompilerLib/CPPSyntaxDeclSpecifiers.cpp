/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "CPPSyntaxes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::Init(CCPPSyntaxTree* pcTree, CCPPSyntax* pcParent)
{
	CCPPSyntax::Init(pcTree, pcParent);
	macpSpecifiers.Init(NodeArrayMallocator());
	muiFlags = 0;
	muiTypeKeywords = 0;
	miLongs = 0;
	mpcType = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::Kill(void)
{
	macpSpecifiers.Kill();
	CCPPSyntax::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// How many simple type keywords the specifiers name their type with: one
// for int, two for unsigned int and three for unsigned long long.
//
//////////////////////////////////////////////////////////////////////////
size CCPPSyntaxDeclSpecifiers::NumTypeKeywords(void) const
{
	uint32	uiKeywords;
	size	uiCount;

	uiCount = (size)miLongs;
	for (uiKeywords = muiTypeKeywords; uiKeywords != 0; uiKeywords &= uiKeywords - 1)
	{
		uiCount++;
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the specifiers define the type they name: a class with its
// members, or an enum with its enumerators.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxDeclSpecifiers::DefinesType(void)
{
	if (!mpcType)
	{
		return false;
	}
	if (mpcType->GetKind() == CPPS_EnumSpecifier)
	{
		return ((CCPPSyntaxEnumSpecifier*)mpcType)->mbBody;
	}
	return mpcType->GetKind() == CPPS_ClassSpecifier;
}


//////////////////////////////////////////////////////////////////////////
//
// A simple type keyword counted in the specifiers: what the type the
// keywords name is worked out from (CCPPDeclarators).
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::CountTypeKeyword(ECppTokens eToken)
{
	switch (eToken)
	{
	case CT_void:		muiTypeKeywords |= CPPTK_VOID;		break;
	case CT_bool:		muiTypeKeywords |= CPPTK_BOOL;		break;
	case CT_char:		muiTypeKeywords |= CPPTK_CHAR;		break;
	case CT_wchar_t:	muiTypeKeywords |= CPPTK_WCHAR_T;	break;
	case CT_char16_t:	muiTypeKeywords |= CPPTK_CHAR16_T;	break;
	case CT_char32_t:	muiTypeKeywords |= CPPTK_CHAR32_T;	break;
	case CT_short:		muiTypeKeywords |= CPPTK_SHORT;		break;
	case CT_int:		muiTypeKeywords |= CPPTK_INT;		break;
	case CT_long:		miLongs++;							break;
	case CT_signed:
	case CT___signed__:	muiTypeKeywords |= CPPTK_SIGNED;	break;
	case CT_unsigned:	muiTypeKeywords |= CPPTK_UNSIGNED;	break;
	case CT_float:		muiTypeKeywords |= CPPTK_FLOAT;		break;
	case CT_double:		muiTypeKeywords |= CPPTK_DOUBLE;	break;
	case CT_auto:		muiTypeKeywords |= CPPTK_AUTO;		break;
	case CT___int8:		muiTypeKeywords |= CPPTK_INT8;		break;
	case CT___int16:	muiTypeKeywords |= CPPTK_INT16;		break;
	case CT___int32:	muiTypeKeywords |= CPPTK_INT32;		break;
	case CT___int64:	muiTypeKeywords |= CPPTK_INT64;		break;
	default:													break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::PrettyPrint(CChars* pszDest, int iDepth)
{
	PrintList(pszDest, &macpSpecifiers, " ", iDepth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPSyntaxDeclSpecifiers::VisitFields(CCPPSyntaxFields* pcFields)
{
	pcFields->Children(&macpSpecifiers);
	pcFields->Integer(&muiFlags);
	pcFields->Integer(&muiTypeKeywords);
	pcFields->Integer(&miLongs);
	pcFields->Child(&mpcType);
}


//////////////////////////////////////////////////////////////////////////
//
// The declarator the type the specifiers name was made by, when they name a
// typedef (or an alias) whose declarator is more than its name: typedef int
// Row[3]; and typedef int Binary(int, int);, through typedefs of the
// typedef.  *ppcSpecifiers are the specifiers that declarator stands after.
// NULL if they name no such type.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxDeclarator* CCPPSyntaxDeclSpecifiers::TypedefDeclarator(CCPPSyntaxDeclSpecifiers** ppcSpecifiers)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSymbol*					pcSymbol;
	CCPPSyntax*					pcDeclaration;
	CCPPSyntaxDeclarator*		pcDeclarator;
	CCPPSyntaxTypeId*			pcType;

	pcSpecifiers = this;
	for (;;)
	{
		if (!pcSpecifiers || !pcSpecifiers->mpcType || (pcSpecifiers->mpcType->GetKind() != CPPS_NamedTypeSpecifier))
		{
			return NULL;
		}
		pcSymbol = ((CCPPSyntaxNamedTypeSpecifier*)pcSpecifiers->mpcType)->mpcName->mpcSymbol;
		pcDeclaration = (pcSymbol && (pcSymbol->meKind == CPPSK_Typedef)) ? pcSymbol->mpcDeclaration : NULL;
		if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator) && pcDeclaration->mpcParent && (pcDeclaration->mpcParent->GetKind() == CPPS_SimpleDeclaration))
		{
			pcSpecifiers = ((CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcParent)->mpcSpecifiers;
			pcDeclarator = ((CCPPSyntaxInitDeclarator*)pcDeclaration)->mpcDeclarator;
		}
		else if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_AliasDeclaration))
		{
			pcType = ((CCPPSyntaxAliasDeclaration*)pcDeclaration)->mpcType;
			pcSpecifiers = pcType->mpcSpecifiers;
			pcDeclarator = pcType->mpcDeclarator;
		}
		else
		{
			return NULL;
		}
		if (pcDeclarator && (pcDeclarator->mpcNested || (pcDeclarator->macpPrefix.NumElements() != 0) || (pcDeclarator->macpSuffixes.NumElements() != 0)))
		{
			*ppcSpecifiers = pcSpecifiers;
			return pcDeclarator;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the specifiers name a function's type, and the specifiers and the
// declarator it was written with: typedef int Binary(int, int); or using
// Binary = int(int, int);.  A function declared by the type's name (Binary
// Add;) takes its parameters and what it returns from them.  [dcl.fct] 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxDeclSpecifiers::NamesFunctionType(CCPPSyntaxDeclSpecifiers** ppcSpecifiers, CCPPSyntaxDeclarator** ppcDeclarator)
{
	CCPPSyntaxDeclarator*	pcDeclarator;

	pcDeclarator = TypedefDeclarator(ppcSpecifiers);
	if (!pcDeclarator || pcDeclarator->mpcNested || (pcDeclarator->macpPrefix.NumElements() != 0) || (pcDeclarator->macpSuffixes.NumElements() != 1) || !pcDeclarator->GetFunctionSuffix())
	{
		return false;
	}
	*ppcDeclarator = pcDeclarator;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the specifiers name an array's type or a function's, typedef int
// Row[3]; or typedef int Binary(int, int);: what no function returns.
// [dcl.fct] 8
//
//////////////////////////////////////////////////////////////////////////
bool CCPPSyntaxDeclSpecifiers::NamesArrayOrFunctionType(void)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxDeclarator*		pcDeclarator;

	pcDeclarator = TypedefDeclarator(&pcSpecifiers);
	return pcDeclarator && !pcDeclarator->mpcNested && (pcDeclarator->NumPointers() == 0) && (pcDeclarator->macpSuffixes.NumElements() != 0);
}
