/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __C_P_P_SYNTAX_MEMORY_H__
#define __C_P_P_SYNTAX_MEMORY_H__
#include <new>
#include "BaseLib/MemoryStackExtended.h"
#include "CPPSyntax.h"


class CCPPSyntaxAccessSpecifier;
class CCPPSyntaxAliasDeclaration;
class CCPPSyntaxAlignasSpecifier;
class CCPPSyntaxArraySuffix;
class CCPPSyntaxAttribute;
class CCPPSyntaxBaseSpecifier;
class CCPPSyntaxBinaryExpression;
class CCPPSyntaxBreakStatement;
class CCPPSyntaxCallExpression;
class CCPPSyntaxCaseStatement;
class CCPPSyntaxCastExpression;
class CCPPSyntaxCatchHandler;
class CCPPSyntaxClassSpecifier;
class CCPPSyntaxCompoundStatement;
class CCPPSyntaxConditionalExpression;
class CCPPSyntaxConstructExpression;
class CCPPSyntaxContinueStatement;
class CCPPSyntaxDeclSpecifiers;
class CCPPSyntaxDeclarationStatement;
class CCPPSyntaxDeclarator;
class CCPPSyntaxDecltypeSpecifier;
class CCPPSyntaxDeleteExpression;
class CCPPSyntaxDoStatement;
class CCPPSyntaxElaboratedTypeSpecifier;
class CCPPSyntaxEmptyDeclaration;
class CCPPSyntaxEnumSpecifier;
class CCPPSyntaxEnumerator;
class CCPPSyntaxExceptionSpecification;
class CCPPSyntaxExpressionStatement;
class CCPPSyntaxExtensionDeclaration;
class CCPPSyntaxForStatement;
class CCPPSyntaxFunctionDefinition;
class CCPPSyntaxFunctionSuffix;
class CCPPSyntaxGotoStatement;
class CCPPSyntaxIdExpression;
class CCPPSyntaxIfStatement;
class CCPPSyntaxInitDeclarator;
class CCPPSyntaxInitializerList;
class CCPPSyntaxKeywordSpecifier;
class CCPPSyntaxLabelStatement;
class CCPPSyntaxLinkageSpecification;
class CCPPSyntaxLiteralExpression;
class CCPPSyntaxMemberExpression;
class CCPPSyntaxMemberInitializer;
class CCPPSyntaxName;
class CCPPSyntaxNameComponent;
class CCPPSyntaxNamedCastExpression;
class CCPPSyntaxNamedTypeSpecifier;
class CCPPSyntaxNamespace;
class CCPPSyntaxNewExpression;
class CCPPSyntaxNoexceptExpression;
class CCPPSyntaxPackExpansionExpression;
class CCPPSyntaxParameter;
class CCPPSyntaxParenthesisedExpression;
class CCPPSyntaxParenthesisedInitializer;
class CCPPSyntaxPointerOperator;
class CCPPSyntaxRangeForStatement;
class CCPPSyntaxReturnStatement;
class CCPPSyntaxSimpleDeclaration;
class CCPPSyntaxSizeofExpression;
class CCPPSyntaxSkippedDeclaration;
class CCPPSyntaxSkippedStatement;
class CCPPSyntaxStaticAssert;
class CCPPSyntaxStringExpression;
class CCPPSyntaxSubscriptExpression;
class CCPPSyntaxSwitchStatement;
class CCPPSyntaxTemplateDeclaration;
class CCPPSyntaxThisExpression;
class CCPPSyntaxThrowExpression;
class CCPPSyntaxTranslationUnit;
class CCPPSyntaxTryStatement;
class CCPPSyntaxTypeId;
class CCPPSyntaxTypeParameter;
class CCPPSyntaxTypeTraitExpression;
class CCPPSyntaxTypeidExpression;
class CCPPSyntaxUnaryExpression;
class CCPPSyntaxUsingDeclaration;
class CCPPSyntaxUsingDirective;
class CCPPSyntaxWhileStatement;


//Nodes are allocated together and freed together when the tree is killed.
class CCPPSyntaxMemory
{
protected:
	CMemoryStackExtended	mcStack;
	CArraySyntax			mapcSyntaxes;
	CCPPSyntaxTree*			mpcTree;

public:
	void							Init(CCPPSyntaxTree* pcTree);
	void							Kill(void);
	size							NumSyntaxes(void);

	CCPPSyntaxTranslationUnit*		CreateTranslationUnit(CCPPSyntax* pcParent);
	CCPPSyntaxNamespace*			CreateNamespace(CCPPSyntax* pcParent);
	CCPPSyntaxLinkageSpecification*	CreateLinkageSpecification(CCPPSyntax* pcParent);
	CCPPSyntaxSimpleDeclaration*	CreateSimpleDeclaration(CCPPSyntax* pcParent);
	CCPPSyntaxFunctionDefinition*	CreateFunctionDefinition(CCPPSyntax* pcParent);
	CCPPSyntaxInitDeclarator*		CreateInitDeclarator(CCPPSyntax* pcParent);
	CCPPSyntaxTemplateDeclaration*	CreateTemplateDeclaration(CCPPSyntax* pcParent);
	CCPPSyntaxTypeParameter*		CreateTypeParameter(CCPPSyntax* pcParent);
	CCPPSyntaxParameter*			CreateParameter(CCPPSyntax* pcParent);
	CCPPSyntaxUsingDirective*		CreateUsingDirective(CCPPSyntax* pcParent);
	CCPPSyntaxUsingDeclaration*		CreateUsingDeclaration(CCPPSyntax* pcParent);
	CCPPSyntaxAliasDeclaration*		CreateAliasDeclaration(CCPPSyntax* pcParent);
	CCPPSyntaxStaticAssert*			CreateStaticAssert(CCPPSyntax* pcParent);
	CCPPSyntaxExtensionDeclaration*	CreateExtensionDeclaration(CCPPSyntax* pcParent);
	CCPPSyntaxEmptyDeclaration*		CreateEmptyDeclaration(CCPPSyntax* pcParent);
	CCPPSyntaxAccessSpecifier*		CreateAccessSpecifier(CCPPSyntax* pcParent);
	CCPPSyntaxSkippedDeclaration*	CreateSkippedDeclaration(CCPPSyntax* pcParent);
	CCPPSyntaxEnumerator*			CreateEnumerator(CCPPSyntax* pcParent);
	CCPPSyntaxBaseSpecifier*		CreateBaseSpecifier(CCPPSyntax* pcParent);
	CCPPSyntaxMemberInitializer*	CreateMemberInitializer(CCPPSyntax* pcParent);
	CCPPSyntaxAttribute*			CreateAttribute(CCPPSyntax* pcParent);
	CCPPSyntaxDeclSpecifiers*		CreateDeclSpecifiers(CCPPSyntax* pcParent);
	CCPPSyntaxKeywordSpecifier*		CreateKeywordSpecifier(CCPPSyntax* pcParent);
	CCPPSyntaxAlignasSpecifier*		CreateAlignasSpecifier(CCPPSyntax* pcParent);
	CCPPSyntaxNamedTypeSpecifier*	CreateNamedTypeSpecifier(CCPPSyntax* pcParent);
	CCPPSyntaxClassSpecifier*		CreateClassSpecifier(CCPPSyntax* pcParent);
	CCPPSyntaxEnumSpecifier*		CreateEnumSpecifier(CCPPSyntax* pcParent);
	CCPPSyntaxElaboratedTypeSpecifier*	CreateElaboratedTypeSpecifier(CCPPSyntax* pcParent);
	CCPPSyntaxDecltypeSpecifier*	CreateDecltypeSpecifier(CCPPSyntax* pcParent);
	CCPPSyntaxDeclarator*			CreateDeclarator(CCPPSyntax* pcParent);
	CCPPSyntaxPointerOperator*		CreatePointerOperator(CCPPSyntax* pcParent);
	CCPPSyntaxFunctionSuffix*		CreateFunctionSuffix(CCPPSyntax* pcParent);
	CCPPSyntaxArraySuffix*			CreateArraySuffix(CCPPSyntax* pcParent);
	CCPPSyntaxExceptionSpecification*	CreateExceptionSpecification(CCPPSyntax* pcParent);
	CCPPSyntaxTypeId*				CreateTypeId(CCPPSyntax* pcParent);
	CCPPSyntaxName*					CreateName(CCPPSyntax* pcParent);
	CCPPSyntaxNameComponent*		CreateNameComponent(CCPPSyntax* pcParent);
	CCPPSyntaxCompoundStatement*	CreateCompoundStatement(CCPPSyntax* pcParent);
	CCPPSyntaxDeclarationStatement*	CreateDeclarationStatement(CCPPSyntax* pcParent);
	CCPPSyntaxExpressionStatement*	CreateExpressionStatement(CCPPSyntax* pcParent);
	CCPPSyntaxIfStatement*			CreateIfStatement(CCPPSyntax* pcParent);
	CCPPSyntaxSwitchStatement*		CreateSwitchStatement(CCPPSyntax* pcParent);
	CCPPSyntaxCaseStatement*		CreateCaseStatement(CCPPSyntax* pcParent);
	CCPPSyntaxLabelStatement*		CreateLabelStatement(CCPPSyntax* pcParent);
	CCPPSyntaxWhileStatement*		CreateWhileStatement(CCPPSyntax* pcParent);
	CCPPSyntaxDoStatement*			CreateDoStatement(CCPPSyntax* pcParent);
	CCPPSyntaxForStatement*			CreateForStatement(CCPPSyntax* pcParent);
	CCPPSyntaxRangeForStatement*	CreateRangeForStatement(CCPPSyntax* pcParent);
	CCPPSyntaxBreakStatement*		CreateBreakStatement(CCPPSyntax* pcParent);
	CCPPSyntaxContinueStatement*	CreateContinueStatement(CCPPSyntax* pcParent);
	CCPPSyntaxReturnStatement*		CreateReturnStatement(CCPPSyntax* pcParent);
	CCPPSyntaxGotoStatement*		CreateGotoStatement(CCPPSyntax* pcParent);
	CCPPSyntaxTryStatement*			CreateTryStatement(CCPPSyntax* pcParent);
	CCPPSyntaxCatchHandler*			CreateCatchHandler(CCPPSyntax* pcParent);
	CCPPSyntaxSkippedStatement*		CreateSkippedStatement(CCPPSyntax* pcParent);
	CCPPSyntaxLiteralExpression*	CreateLiteralExpression(CCPPSyntax* pcParent);
	CCPPSyntaxStringExpression*		CreateStringExpression(CCPPSyntax* pcParent);
	CCPPSyntaxIdExpression*			CreateIdExpression(CCPPSyntax* pcParent);
	CCPPSyntaxThisExpression*		CreateThisExpression(CCPPSyntax* pcParent);
	CCPPSyntaxParenthesisedExpression*	CreateParenthesisedExpression(CCPPSyntax* pcParent);
	CCPPSyntaxUnaryExpression*		CreateUnaryExpression(CCPPSyntax* pcParent);
	CCPPSyntaxBinaryExpression*		CreateBinaryExpression(CCPPSyntax* pcParent);
	CCPPSyntaxConditionalExpression*	CreateConditionalExpression(CCPPSyntax* pcParent);
	CCPPSyntaxCallExpression*		CreateCallExpression(CCPPSyntax* pcParent);
	CCPPSyntaxSubscriptExpression*	CreateSubscriptExpression(CCPPSyntax* pcParent);
	CCPPSyntaxMemberExpression*		CreateMemberExpression(CCPPSyntax* pcParent);
	CCPPSyntaxCastExpression*		CreateCastExpression(CCPPSyntax* pcParent);
	CCPPSyntaxConstructExpression*	CreateConstructExpression(CCPPSyntax* pcParent);
	CCPPSyntaxNamedCastExpression*	CreateNamedCastExpression(CCPPSyntax* pcParent);
	CCPPSyntaxSizeofExpression*		CreateSizeofExpression(CCPPSyntax* pcParent);
	CCPPSyntaxTypeidExpression*		CreateTypeidExpression(CCPPSyntax* pcParent);
	CCPPSyntaxNewExpression*		CreateNewExpression(CCPPSyntax* pcParent);
	CCPPSyntaxDeleteExpression*		CreateDeleteExpression(CCPPSyntax* pcParent);
	CCPPSyntaxThrowExpression*		CreateThrowExpression(CCPPSyntax* pcParent);
	CCPPSyntaxInitializerList*		CreateInitializerList(CCPPSyntax* pcParent);
	CCPPSyntaxParenthesisedInitializer*	CreateParenthesisedInitializer(CCPPSyntax* pcParent);
	CCPPSyntaxTypeTraitExpression*	CreateTypeTraitExpression(CCPPSyntax* pcParent);
	CCPPSyntaxPackExpansionExpression*	CreatePackExpansionExpression(CCPPSyntax* pcParent);
	CCPPSyntaxNoexceptExpression*	CreateNoexceptExpression(CCPPSyntax* pcParent);

protected:
	template<class M>	M*			Create(CCPPSyntax* pcParent);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CCPPSyntaxMemory::Create(CCPPSyntax* pcParent)
{
	M*	pcSyntax;

	pcSyntax = (M*)mcStack.Add(sizeof(M));
	new(pcSyntax) M;
	pcSyntax->Init(mpcTree, pcParent);
	mapcSyntaxes.Add(pcSyntax);
	return pcSyntax;
}


#endif // __C_P_P_SYNTAX_MEMORY_H__

