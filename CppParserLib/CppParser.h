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
#ifndef __C_P_P_PARSER_H__
#define __C_P_P_PARSER_H__
#include "BaseLib/TextParser.h"
#include "BaseLib/ArrayChars.h"
#include "CppReturn.h"
#include "ASTSimpleSpec.h"
#include "ASTAccessSpec.h"
#include "AbstractSyntaxTree.h"
#include "SimpleOperator.h"
#include "ClassType.h"


#define MAX_IDENTIFIER	0x10000


class CCPPParser
{
public:
	CAbstractSyntaxTree		mcAST;
	CTextParser				mcParser;
	CArrayChars				mszSimpleTypes;
	CArrayChars				mszSimpleModifiers;
	CArrayChars				mszAccessSpecifiers;
	CArrayChars				mszSimpleOperators;
	CArrayChars				mszReservedWords;
	char					mszScratchPad[MAX_IDENTIFIER];
	CArrayChars				maszCallStack;

	void 		Init(void);
	void 		Kill(void);
	void 		BuildAST(char* szSourze);
	BOOL		ReservedWord(void);
	CCPPReturn	ClassDecl(void);
	CCPPReturn	EnumDecl(void);
	CCPPReturn	VariablesDecl(void);
	CCPPReturn	ListDecl(void);
	CCPPReturn	FunctionParametersDecl(BOOL bLooksLikeConstructor);
	CCPPReturn	FunctionPointerDecl(BOOL bAllowsName);
	CCPPReturn	FunctionDecl(BOOL* pbDefn);
	CCPPReturn	FunctionDefn(void);
	CCPPReturn	SingleVariableDeclaration(void);
	CCPPReturn	DeclInitialisers(void);
	CCPPReturn	DeclInitialisersContstructor(void);
	CCPPReturn	ClassSpec(EClassType* pcClassType);
	CCPPReturn	SimpleType(ESimpleTypes* peSpec);
	CCPPReturn	SimpleModifier(ESimpleModifiers* peSpec);
	CCPPReturn	Name(void);
	CCPPReturn	ArrayBody(void);
	CCPPReturn	ArrayOperator(void);
	CCPPReturn	ArrayDecl(void);
	CCPPReturn	Expression(void);
	CCPPReturn	Expressable(void);
	CCPPReturn	Operator(ESimpleOperators* peOp, BOOL bEmptyTypeCast);
	CCPPReturn	SimpleOperator(ESimpleOperators* peOp);
	CCPPReturn	Parentheses(void);
	CCPPReturn	Constant(void);
	CCPPReturn	DoubleConst(void);
	CCPPReturn	BooleanConst(void);
	CCPPReturn	CharConst(void);
	CCPPReturn	StringConst(void);
	CCPPReturn	IntegerConst(void);
	CCPPReturn	IntegerConstType(void);
	CCPPReturn	DecimalInteger(void);
	CCPPReturn	Arguments(void);  // arguments are more properly thought of as the actual values or references assigned to the parameter variables
	CCPPReturn	Aggregate(int* piExpressions);
	CCPPReturn	TypeCast(BOOL bEmpty);
	CCPPReturn	Pointers(int* piCount);
	CCPPReturn	PointersOrReference(int* piPointerCount, BOOL* pbReference);
	CCPPReturn	Type(ESimpleTypes* peSpec, int* piCount, BOOL bAllowDesctructor);
	CCPPReturn	TypeOrFunctionPointer(BOOL bAllowsName);
	CCPPReturn	ClassDefn(void);
	CCPPReturn	ClassInheritance(void);
	CCPPReturn	ClassBody(void);
	CCPPReturn	AccessSpecifier(EAccessSpecifiers* peSpec);
	CCPPReturn	ClassAccess(void);
	CCPPReturn	ClosingCurlyColon(void);
	CCPPReturn	Parameters(int* piCount);  //arguments are more properly thought of as the actual values or references assigned to the parameter variables
	CCPPReturn	Parameter(void);
	CCPPReturn	FunctionBody(void);
	CCPPReturn	Statement(void);
	CCPPReturn	ExpressionStatement(void);
	CCPPReturn	ForStatement(void);
	CCPPReturn	IfStatement(void);
	CCPPReturn	DoStatement(void);
	CCPPReturn	WhileStatement(void);
	CCPPReturn	ReturnStatement(void);
	CCPPReturn	BreakStatement(void);
	CCPPReturn	ContinueStatement(void);
	CCPPReturn	EndStatement(void);
	CCPPReturn	BlockStatement(void);
	CCPPReturn	ElseStatement(void);
	CCPPReturn	SwitchStatement(void);
	CCPPReturn	CaseStatement(void);
	CCPPReturn	GotoStatement(void);
	CCPPReturn	TypedefDecl(void);
	CCPPReturn	LabelStatement(void);
	CCPPReturn	BlockBody(void);
	CCPPReturn	EnumDefn(void);
	CCPPReturn	EnumSpec(void);
	CCPPReturn	EnumBody(void);
	CCPPReturn	EnumValue(void);
	CCPPReturn	CompoundName(int* piCount, BOOL bAllowDesctructor);
	CCPPReturn	Reference(void);
	CCPPReturn	ArrayAccess(void);
	CCPPReturn	TemplateDecl(void);
	CCPPReturn	TemplateBody(void);
	CCPPReturn	Template(void);
	CCPPReturn	TemplateParameters(void);
	CCPPReturn	TemplateCompilation(void);
	CCPPReturn	ClassParam(void);
	CCPPReturn	TypenameParam(void);
	CCPPReturn	TemplateParam(void);
	CCPPReturn	DeclaratorParam(void);
	CCPPReturn	NamespaceDecl(void);
	CCPPReturn	NamespaceDefn(void);
	CCPPReturn	NamespaceBody(void);
	CCPPReturn	Using(void);
	CCPPReturn	EmptyDecl(void);

	void		Expected(char* szString);
	void		DebugPush(char* szMethod);
	void		DebugPop(CCPPReturn bSuccessful);
};


#define PARSE_PUSH(p)	DebugPush(p)

#define PARSE_POP(p)	DebugPop(p); return p


#endif // !__C_P_P_PARSER_H__

