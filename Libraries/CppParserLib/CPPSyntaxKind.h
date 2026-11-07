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


//Each kind of syntax node: N is its ECPPSyntax CPPS_N, its class CCPPSyntaxN and its name "N" (gaszSyntaxKindNames).
#define CPP_SYNTAX_KINDS(X) \
	X(TranslationUnit) \
	X(Namespace) \
	X(LinkageSpecification) \
	X(SimpleDeclaration) \
	X(FunctionDefinition) \
	X(InitDeclarator) \
	X(TemplateDeclaration) \
	X(TypeParameter) \
	X(Parameter) \
	X(UsingDirective) \
	X(UsingDeclaration) \
	X(AliasDeclaration) \
	X(StaticAssert) \
	X(ExtensionDeclaration) \
	X(EmptyDeclaration) \
	X(AccessSpecifier) \
	X(SkippedDeclaration) \
	X(Enumerator) \
	X(BaseSpecifier) \
	X(MemberInitializer) \
	X(Attribute) \
	X(DeclSpecifiers) \
	X(KeywordSpecifier) \
	X(AlignasSpecifier) \
	X(NamedTypeSpecifier) \
	X(ClassSpecifier) \
	X(EnumSpecifier) \
	X(ElaboratedTypeSpecifier) \
	X(DecltypeSpecifier) \
	X(Declarator) \
	X(PointerOperator) \
	X(FunctionSuffix) \
	X(ArraySuffix) \
	X(ExceptionSpecification) \
	X(TypeId) \
	X(Name) \
	X(NameComponent) \
	X(CompoundStatement) \
	X(DeclarationStatement) \
	X(ExpressionStatement) \
	X(IfStatement) \
	X(SwitchStatement) \
	X(CaseStatement) \
	X(LabelStatement) \
	X(AttributedStatement) \
	X(WhileStatement) \
	X(DoStatement) \
	X(ForStatement) \
	X(RangeForStatement) \
	X(BreakStatement) \
	X(ContinueStatement) \
	X(ReturnStatement) \
	X(GotoStatement) \
	X(TryStatement) \
	X(CatchHandler) \
	X(SkippedStatement) \
	X(LiteralExpression) \
	X(StringExpression) \
	X(IdExpression) \
	X(ThisExpression) \
	X(ParenthesisedExpression) \
	X(UnaryExpression) \
	X(BinaryExpression) \
	X(ConditionalExpression) \
	X(CallExpression) \
	X(SubscriptExpression) \
	X(MemberExpression) \
	X(CastExpression) \
	X(ConstructExpression) \
	X(NamedCastExpression) \
	X(SizeofExpression) \
	X(TypeidExpression) \
	X(NewExpression) \
	X(DeleteExpression) \
	X(ThrowExpression) \
	X(InitializerList) \
	X(ParenthesisedInitializer) \
	X(TypeTraitExpression) \
	X(PackExpansionExpression) \
	X(NoexceptExpression) \
	X(LazyDeclaration)


#define CPP_SYNTAX_ENUM(N)	CPPS_##N,
enum ECPPSyntax
{
	CPP_SYNTAX_KINDS(CPP_SYNTAX_ENUM)
	CPPS_NumKinds
};
#undef CPP_SYNTAX_ENUM


//A node class's kind (CCPPSyntax::GetKind), in its declaration: CPP_SYNTAX_GET_KIND(N) in class CCPPSyntaxN.
#define CPP_SYNTAX_GET_KIND(N)	ECPPSyntax GetKind(void) override { return CPPS_##N; }


#endif // __C_P_P_SYNTAX_KIND_H__

