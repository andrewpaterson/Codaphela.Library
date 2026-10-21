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
#ifndef __C_P_P_SYNTAX_KIND_H__
#define __C_P_P_SYNTAX_KIND_H__


enum ECPPSyntax
{
	CPPS_TranslationUnit,
	CPPS_Namespace,
	CPPS_LinkageSpecification,
	CPPS_SimpleDeclaration,
	CPPS_FunctionDefinition,
	CPPS_InitDeclarator,
	CPPS_TemplateDeclaration,
	CPPS_TypeParameter,
	CPPS_Parameter,
	CPPS_UsingDirective,
	CPPS_UsingDeclaration,
	CPPS_AliasDeclaration,
	CPPS_StaticAssert,
	CPPS_ExtensionDeclaration,
	CPPS_EmptyDeclaration,
	CPPS_AccessSpecifier,
	CPPS_SkippedDeclaration,
	CPPS_Enumerator,
	CPPS_BaseSpecifier,
	CPPS_MemberInitializer,
	CPPS_Attribute,
	CPPS_DeclSpecifiers,
	CPPS_KeywordSpecifier,
	CPPS_AlignasSpecifier,
	CPPS_NamedTypeSpecifier,
	CPPS_ClassSpecifier,
	CPPS_EnumSpecifier,
	CPPS_ElaboratedTypeSpecifier,
	CPPS_DecltypeSpecifier,
	CPPS_Declarator,
	CPPS_PointerOperator,
	CPPS_FunctionSuffix,
	CPPS_ArraySuffix,
	CPPS_ExceptionSpecification,
	CPPS_TypeId,
	CPPS_Name,
	CPPS_NameComponent,
	CPPS_CompoundStatement,
	CPPS_DeclarationStatement,
	CPPS_ExpressionStatement,
	CPPS_IfStatement,
	CPPS_SwitchStatement,
	CPPS_CaseStatement,
	CPPS_LabelStatement,
	CPPS_WhileStatement,
	CPPS_DoStatement,
	CPPS_ForStatement,
	CPPS_RangeForStatement,
	CPPS_BreakStatement,
	CPPS_ContinueStatement,
	CPPS_ReturnStatement,
	CPPS_GotoStatement,
	CPPS_TryStatement,
	CPPS_CatchHandler,
	CPPS_SkippedStatement,
	CPPS_LiteralExpression,
	CPPS_StringExpression,
	CPPS_IdExpression,
	CPPS_ThisExpression,
	CPPS_ParenthesisedExpression,
	CPPS_UnaryExpression,
	CPPS_BinaryExpression,
	CPPS_ConditionalExpression,
	CPPS_CallExpression,
	CPPS_SubscriptExpression,
	CPPS_MemberExpression,
	CPPS_CastExpression,
	CPPS_ConstructExpression,
	CPPS_NamedCastExpression,
	CPPS_SizeofExpression,
	CPPS_TypeidExpression,
	CPPS_NewExpression,
	CPPS_DeleteExpression,
	CPPS_ThrowExpression,
	CPPS_InitializerList,
	CPPS_ParenthesisedInitializer,
	CPPS_TypeTraitExpression,
	CPPS_PackExpansionExpression,
	CPPS_NoexceptExpression,
};


#endif // __C_P_P_SYNTAX_KIND_H__

