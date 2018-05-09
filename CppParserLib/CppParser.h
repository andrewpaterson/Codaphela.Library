/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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


class CCppParser
{
public:
	CAbstractSyntaxTree	mcAST;
	CTextParser			mcParser;
	CArrayChars		mszSimpleTypes;
	CArrayChars		mszSimpleModifiers;
	CArrayChars		mszAccessSpecifiers;
	CArrayChars		mszSimpleOperators;
	CArrayChars		mszReservedWords;
	char				mszScratchPad[MAX_IDENTIFIER];
	CArrayChars		maszCallStack;

	void 		Init(void);
	void 		Kill(void);
	void 		BuildAST(char* szSourze);
	BOOL		ReservedWord(void);
	CCppReturn	ClassDecl(void);
	CCppReturn	EnumDecl(void);
	CCppReturn	VariablesDecl(void);
	CCppReturn	ListDecl(void);
	CCppReturn	FunctionParametersDecl(BOOL bLooksLikeConstructor);
	CCppReturn	FunctionPointerDecl(BOOL bAllowsName);
	CCppReturn	FunctionDecl(BOOL* pbDefn);
	CCppReturn	FunctionDefn(void);
	CCppReturn	SingleVariableDeclaration(void);
	CCppReturn	DeclInitialisers(void);
	CCppReturn	DeclInitialisersContstructor(void);
	CCppReturn	ClassSpec(EClassType* pcClassType);
	CCppReturn	SimpleType(ESimpleTypes* peSpec);
	CCppReturn	SimpleModifier(ESimpleModifiers* peSpec);
	CCppReturn	Name(void);
	CCppReturn	ArrayBody(void);
	CCppReturn	ArrayOperator(void);
	CCppReturn	ArrayDecl(void);
	CCppReturn	Expression(void);
	CCppReturn	Expressable(void);
	CCppReturn	Operator(ESimpleOperators* peOp, BOOL bEmptyTypeCast);
	CCppReturn	SimpleOperator(ESimpleOperators* peOp);
	CCppReturn	Parentheses(void);
	CCppReturn	Constant(void);
	CCppReturn	DoubleConst(void);
	CCppReturn	BooleanConst(void);
	CCppReturn	CharConst(void);
	CCppReturn	StringConst(void);
	CCppReturn	IntegerConst(void);
	CCppReturn	IntegerConstType(void);
	CCppReturn	DecimalInteger(void);
	CCppReturn	Arguments(void);  // arguments are more properly thought of as the actual values or references assigned to the parameter variables
	CCppReturn	Aggregate(int* piExpressions);
	CCppReturn	TypeCast(BOOL bEmpty);
	CCppReturn	Pointers(int* piCount);
	CCppReturn	PointersOrReference(int* piPointerCount, BOOL* pbReference);
	CCppReturn	Type(ESimpleTypes* peSpec, int* piCount, BOOL bAllowDesctructor);
	CCppReturn	TypeOrFunctionPointer(BOOL bAllowsName);
	CCppReturn	ClassDefn(void);
	CCppReturn	ClassInheritance(void);
	CCppReturn	ClassBody(void);
	CCppReturn	AccessSpecifier(EAccessSpecifiers* peSpec);
	CCppReturn	ClassAccess(void);
	CCppReturn	ClosingCurlyColon(void);
	CCppReturn	Parameters(int* piCount);  //arguments are more properly thought of as the actual values or references assigned to the parameter variables
	CCppReturn	Parameter(void);
	CCppReturn	FunctionBody(void);
	CCppReturn	Statement(void);
	CCppReturn	ExpressionStatement(void);
	CCppReturn	ForStatement(void);
	CCppReturn	IfStatement(void);
	CCppReturn	DoStatement(void);
	CCppReturn	WhileStatement(void);
	CCppReturn	ReturnStatement(void);
	CCppReturn	BreakStatement(void);
	CCppReturn	ContinueStatement(void);
	CCppReturn	EndStatement(void);
	CCppReturn	BlockStatement(void);
	CCppReturn	ElseStatement(void);
	CCppReturn	SwitchStatement(void);
	CCppReturn	CaseStatement(void);
	CCppReturn	GotoStatement(void);
	CCppReturn	TypedefDecl(void);
	CCppReturn	LabelStatement(void);
	CCppReturn	BlockBody(void);
	CCppReturn	EnumDefn(void);
	CCppReturn	EnumSpec(void);
	CCppReturn	EnumBody(void);
	CCppReturn	EnumValue(void);
	CCppReturn	CompoundName(int* piCount, BOOL bAllowDesctructor);
	CCppReturn	Reference(void);
	CCppReturn	ArrayAccess(void);
	CCppReturn	TemplateDecl(void);
	CCppReturn	TemplateBody(void);
	CCppReturn	Template(void);
	CCppReturn	TemplateParameters(void);
	CCppReturn	TemplateCompilation(void);
	CCppReturn	ClassParam(void);
	CCppReturn	TypenameParam(void);
	CCppReturn	TemplateParam(void);
	CCppReturn	DeclaratorParam(void);
	CCppReturn	NamespaceDecl(void);
	CCppReturn	NamespaceDefn(void);
	CCppReturn	NamespaceBody(void);
	CCppReturn	Using(void);
	CCppReturn	EmptyDecl(void);

	void		Expected(char* szString);
	void		DebugPush(char* szMethod);
	void		DebugPop(CCppReturn bSuccessful);
};


#define PARSE_PUSH(p)	DebugPush(p)

#define PARSE_POP(p)	DebugPop(p); return p


#endif // __C_P_P_PARSER_H__

