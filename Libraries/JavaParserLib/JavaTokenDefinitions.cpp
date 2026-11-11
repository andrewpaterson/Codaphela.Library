/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/TextParser.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/Newline.h"
#include "JavaTokenParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::Init(void)
{
	InitKeywords();
	InitOperators();
	InitSeparators();
	InitGenerics();

	InitAmbiguous();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::InitKeywords(void)
{
	mcKeywords.Init();

	AddKeywordDefinition("abstract", JK_abstract);
	AddKeywordDefinition("assert", JK_assert);
	AddKeywordDefinition("boolean", JK_boolean);
	AddKeywordDefinition("break", JK_break);
	AddKeywordDefinition("byte", JK_byte);
	AddKeywordDefinition("case", JK_case);
	AddKeywordDefinition("catch", JK_catch);
	AddKeywordDefinition("char", JK_char);
	AddKeywordDefinition("class", JK_class);
	AddKeywordDefinition("const", JK_const);
	AddKeywordDefinition("continue", JK_continue);
	AddKeywordDefinition("default", JK_default);
	AddKeywordDefinition("do", JK_do);
	AddKeywordDefinition("double", JK_double);
	AddKeywordDefinition("else", JK_else);
	AddKeywordDefinition("enum", JK_enum);
	AddKeywordDefinition("extends", JK_extends);
	AddKeywordDefinition("final", JK_final);
	AddKeywordDefinition("finally", JK_finally);
	AddKeywordDefinition("float", JK_float);
	AddKeywordDefinition("for", JK_for);
	AddKeywordDefinition("goto", JK_goto);
	AddKeywordDefinition("if", JK_if);
	AddKeywordDefinition("implements", JK_implements);
	AddKeywordDefinition("import", JK_import);
	AddKeywordDefinition("instanceof", JK_instanceof);
	AddKeywordDefinition("int", JK_int);
	AddKeywordDefinition("interface", JK_interface);
	AddKeywordDefinition("long", JK_long);
	AddKeywordDefinition("native", JK_native);
	AddKeywordDefinition("new", JK_new);
	AddKeywordDefinition("package", JK_package);
	AddKeywordDefinition("private", JK_private);
	AddKeywordDefinition("protected", JK_protected);
	AddKeywordDefinition("public", JK_public);
	AddKeywordDefinition("return", JK_return);
	AddKeywordDefinition("short", JK_short);
	AddKeywordDefinition("static", JK_static);
	AddKeywordDefinition("strictfp", JK_strictfp);
	AddKeywordDefinition("super", JK_super);
	AddKeywordDefinition("switch", JK_switch);
	AddKeywordDefinition("synchronized", JK_synchronized);
	AddKeywordDefinition("this", JK_this);
	AddKeywordDefinition("throw", JK_throw);
	AddKeywordDefinition("throws", JK_throws);
	AddKeywordDefinition("transient", JK_transient);
	AddKeywordDefinition("try", JK_try);
	AddKeywordDefinition("void", JK_void);
	AddKeywordDefinition("volatile", JK_volatile);
	AddKeywordDefinition("while", JK_while);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::InitOperators(void)
{
	mcOperators.Init();

	AddOperatorDefinition(JOT_Arithmetic, JO_Plus, "+");
	AddOperatorDefinition(JOT_Arithmetic, JO_Minus, "-");
	AddOperatorDefinition(JOT_Arithmetic, JO_Divide, "/");
	AddOperatorDefinition(JOT_Arithmetic, JO_Multipy, "*");
	AddOperatorDefinition(JOT_Arithmetic, JO_Modulus, "%");

	AddOperatorDefinition(JOT_Unary, JO_Increment, "++");
	AddOperatorDefinition(JOT_Unary, JO_Decrement, "--");

	AddOperatorDefinition(JOT_Assignment, JO_Assign, "=");
	AddOperatorDefinition(JOT_Assignment, JO_PlusAssign, "+=");
	AddOperatorDefinition(JOT_Assignment, JO_MinusAssign, "-=");
	AddOperatorDefinition(JOT_Assignment, JO_MultiplyAssign, "*=");
	AddOperatorDefinition(JOT_Assignment, JO_DivideAssign, "/=");
	AddOperatorDefinition(JOT_Assignment, JO_ModulusAssign, "%=");
	AddOperatorDefinition(JOT_Assignment, JO_ExclusiveOrAssign, "^=");
	AddOperatorDefinition(JOT_Assignment, JO_LeftShiftAssign, "<<=");
	AddOperatorDefinition(JOT_Assignment, JO_RightShiftAssign, ">>=");
	AddOperatorDefinition(JOT_Assignment, JO_RightShiftZeroAssign, ">>>=");

	AddOperatorDefinition(JOT_Relational, JO_Equal, "==");
	AddOperatorDefinition(JOT_Relational, JO_NotEqual, "!=");
	AddOperatorDefinition(JOT_Relational, JO_LessThan, "<");
	AddOperatorDefinition(JOT_Relational, JO_GreaterThan, ">");
	AddOperatorDefinition(JOT_Relational, JO_LessThanOrEqual, "<=");
	AddOperatorDefinition(JOT_Relational, JO_GreaterThanOrEqual, ">=");

	AddOperatorDefinition(JOT_Logical, JO_LogicalNegate, "!");
	AddOperatorDefinition(JOT_Logical, JO_LogicalAnd, "&&");
	AddOperatorDefinition(JOT_Logical, JO_LogicalOr, "||");

	AddOperatorDefinition(JOT_Ternary, JO_TernaryCondition, "?");
	AddOperatorDefinition(JOT_Ternary, JO_TernaryStatement, ":");

	AddOperatorDefinition(JOT_Bitwise, JO_BitwiseAnd, "&");
	AddOperatorDefinition(JOT_Bitwise, JO_BitwiseOr, "|");
	AddOperatorDefinition(JOT_Bitwise, JO_BitwiseExclusiveOr, "^");
	AddOperatorDefinition(JOT_Bitwise, JO_BitwiseNot, "~");
	AddOperatorDefinition(JOT_Bitwise, JO_LeftShift, "<<");
	AddOperatorDefinition(JOT_Bitwise, JO_RightShift, ">>");
	AddOperatorDefinition(JOT_Bitwise, JO_RightShiftZero, ">>>");

	AddOperatorDefinition(JOT_Method, JO_MethodReference, "::");
	AddOperatorDefinition(JOT_Method, JO_LambdaOperator, "->");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::InitSeparators(void)
{
	mcSeparators.Init();

	AddSeparatorDefinition(";", JS_Semicolon);
	AddSeparatorDefinition(",", JS_Comma);
	AddSeparatorDefinition(".", JS_Dot);
	AddSeparatorDefinition("(", JS_RoundBracketLeft);
	AddSeparatorDefinition(")", JS_RoundBracketRight);
	AddSeparatorDefinition("{", JS_CurlyBracketLeft);
	AddSeparatorDefinition("}", JS_CurlyBracketRight);
	AddSeparatorDefinition("[", JS_SquareBracketLeft);
	AddSeparatorDefinition("]", JS_SquareBracketRight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::InitAmbiguous(void)
{
	mcAmbiguous.Init();

	AddAmbiguousDefinition("<", JA_AngleBracketLeft);
	AddAmbiguousDefinition(">", JA_AngleBracketRight);
	AddAmbiguousDefinition("?", JA_QuestionMark);
	AddAmbiguousDefinition("*", JA_Asterisk);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::InitGenerics(void)
{
	mcScopes.Init();

	AddGenericDefinition("<", JG_AngleBracketLeft);
	AddGenericDefinition(">", JG_AngleBracketRight);
	AddGenericDefinition("?", JG_QuestionMark);
	AddGenericDefinition("*", JG_Asterisk);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::AddKeywordDefinition(char* szKeyword, EJavaTokenKeyword eKeyword)
{
	CJavaTokenKeywordDefinition*	pcDefinition;

	pcDefinition = mcKeywords.Add(eKeyword, szKeyword);
	if (pcDefinition)
	{
		pcDefinition->Init(eKeyword, szKeyword);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::AddSeparatorDefinition(char* szSeparator, EJavaTokenSeparator eSeparator)
{
	CJavaTokenSeparatorDefinition*	pcDefinition;

	pcDefinition = mcSeparators.Add(eSeparator, szSeparator);
	if (pcDefinition)
	{
		pcDefinition->Init(eSeparator, szSeparator);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::AddOperatorDefinition(EJavaTokenOperatorType eType, EJavaTokenOperator eOperator, char* szOperator)
{
	CJavaTokenOperatorDefinition*	pcDefinition;

	pcDefinition = mcOperators.Add(eOperator, szOperator);
	if (pcDefinition)
	{
		pcDefinition->Init(eType, eOperator, szOperator);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::AddGenericDefinition(char* szGeneric, EJavaTokenScope eGeneric)
{
	CJavaTokenScopeDefinition*	pcDefinition;

	pcDefinition = mcScopes.Add(eGeneric, szGeneric);
	if (pcDefinition)
	{
		pcDefinition->Init(eGeneric, szGeneric);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::AddAmbiguousDefinition(char* szAmbiguous, ECJavaTokenAmbiguous eAmbiguous)
{
	CCJavaTokenAmbiguousDefinition*	pcDefinition;

	pcDefinition = mcAmbiguous.Add(eAmbiguous, szAmbiguous);
	if (pcDefinition)
	{
		pcDefinition->Init(eAmbiguous, szAmbiguous);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::Kill(void)
{
	mcAmbiguous.Kill();

	mcKeywords.Kill();
	mcOperators.Kill();
	mcSeparators.Kill();
	mcScopes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKeywordDefinitions* CJavaTokenDefinitions::GetKeywords(void)
{
	return &mcKeywords;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSeparatorDefinitions* CJavaTokenDefinitions::GetSeparators(void)
{
	return &mcSeparators;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
COperatorDefinitions* CJavaTokenDefinitions::GetOperators(void)
{
	return &mcOperators;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAmbiguousDefinitions* CJavaTokenDefinitions::GetAmbiguous(void)
{
	return &mcAmbiguous;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CScopeDefinitions* CJavaTokenDefinitions::GetScopes(void)
{
	return &mcScopes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenKeywordDefinition* CJavaTokenDefinitions::GetKeyword(EJavaTokenKeyword eKeyword)
{
	return mcKeywords.Get(eKeyword);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenOperatorDefinition* CJavaTokenDefinitions::GetOperator(EJavaTokenOperator eOperator)
{
	return mcOperators.Get(eOperator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenSeparatorDefinition* CJavaTokenDefinitions::GetSeparator(EJavaTokenSeparator eSeparator)
{
	return mcSeparators.Get(eSeparator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCJavaTokenAmbiguousDefinition* CJavaTokenDefinitions::GetAmbiguous(ECJavaTokenAmbiguous eAmbiguous)
{
	return mcAmbiguous.Get(eAmbiguous);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenScopeDefinition* CJavaTokenDefinitions::GetScope(EJavaTokenScope eGeneric)
{
	return mcScopes.Get(eGeneric);
}

