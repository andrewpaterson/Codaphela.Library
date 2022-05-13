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
#include "BaseLib/TextParser.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/NewLine.h"
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
void CJavaTokenDefinitions::AddKeywordDefinition(char* szKeyword, EJavaKeyword eKeyword)
{
	CJavaKeywordDefinition	cDefinition;

	cDefinition.Init(eKeyword, szKeyword);
	mcKeywords.Add(szKeyword, &cDefinition, eKeyword);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::AddSeparatorDefinition(char* szSeparator, EJavaSeparator eSeparator)
{
	CJavaSeparatorDefinition	cDefinition;

	cDefinition.Init(eSeparator, szSeparator);
	mcSeparators.Add(szSeparator, &cDefinition, eSeparator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::AddOperatorDefinition(EJavaOperatorType eType, EJavaOperator eOperator, char* szOperator)
{
	CJavaOperatorDefinition	cDefinition;

	cDefinition.Init(eType, eOperator, szOperator);
	mcOperators.Add(szOperator, &cDefinition, eOperator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::AddGenericDefinition(char* szGeneric, EJavaScope eGeneric)
{
	CJavaScopeDefinition	cDefinition;

	cDefinition.Init(eGeneric, szGeneric);
	mcScopes.Add(szGeneric, &cDefinition, eGeneric);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::AddAmbiguousDefinition(char* szAmbiguous, EJavaAmbiguous eAmbiguous)
{
	CJavaAmbiguousDefinition	cDefinition;

	cDefinition.Init(eAmbiguous, szAmbiguous);
	mcAmbiguous.Add(szAmbiguous, &cDefinition, eAmbiguous);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::Kill(void)
{
	KillAmbiguous();

	KillKeywords();
	KillOperators();
	KillSeparators();
	KillGenerics();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::KillKeywords(void)
{
	char* szName;
	SEnumeratorIterator		sIterator;
	int						iID;
	CJavaKeywordDefinition* pcKeyword;

	mcKeywords.StartIteration(&sIterator, &szName, &iID, &pcKeyword);
	while (sIterator.bValid)
	{
		pcKeyword->Kill();
		mcKeywords.Iterate(&sIterator, &szName, &iID, &pcKeyword);
	}

	mcKeywords.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::KillSeparators(void)
{
	char* szName;
	SEnumeratorIterator		sIterator;
	int						iID;
	CJavaSeparatorDefinition* pcSeparator;

	mcSeparators.StartIteration(&sIterator, &szName, &iID, &pcSeparator);
	while (sIterator.bValid)
	{
		pcSeparator->Kill();
		mcSeparators.Iterate(&sIterator, &szName, &iID, &pcSeparator);
	}

	mcSeparators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::KillOperators(void)
{
	char* szName;
	SEnumeratorIterator			sIterator;
	int							iID;
	CJavaOperatorDefinition* pcOperator;

	mcOperators.StartIteration(&sIterator, &szName, &iID, &pcOperator);
	while (sIterator.bValid)
	{
		pcOperator->Kill();
		mcOperators.Iterate(&sIterator, &szName, &iID, &pcOperator);
	}

	mcOperators.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::KillAmbiguous(void)
{
	char* szName;
	SEnumeratorIterator			sIterator;
	int							iID;
	CJavaAmbiguousDefinition* pcAmbiguous;

	mcAmbiguous.StartIteration(&sIterator, &szName, &iID, &pcAmbiguous);
	while (sIterator.bValid)
	{
		pcAmbiguous->Kill();
		mcAmbiguous.Iterate(&sIterator, &szName, &iID, &pcAmbiguous);
	}

	mcAmbiguous.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenDefinitions::KillGenerics(void)
{
	char* szName;
	SEnumeratorIterator			sIterator;
	int							iID;
	CJavaScopeDefinition* pcGeneric;

	mcScopes.StartIteration(&sIterator, &szName, &iID, &pcGeneric);
	while (sIterator.bValid)
	{
		pcGeneric->Kill();
		mcScopes.Iterate(&sIterator, &szName, &iID, &pcGeneric);
	}

	mcScopes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CKeywordEnumerator* CJavaTokenDefinitions::GetKeywords(void)
{
	return &mcKeywords;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSeparatorEnumerator* CJavaTokenDefinitions::GetSeparators(void)
{
	return &mcSeparators;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
COperatorEnumerator* CJavaTokenDefinitions::GetOperators(void)
{
	return &mcOperators;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAmbiguousEnumerator* CJavaTokenDefinitions::GetAmbiguous(void)
{
	return &mcAmbiguous;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CScopeEnumerator* CJavaTokenDefinitions::GetScopes(void)
{
	return &mcScopes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaKeywordDefinition* CJavaTokenDefinitions::GetKeyword(EJavaKeyword eKeyword)
{
	CJavaKeywordDefinition* pcKeyword;

	mcKeywords.GetWithID(eKeyword, &pcKeyword, NULL);

	return pcKeyword;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaOperatorDefinition* CJavaTokenDefinitions::GetOperator(EJavaOperator eOperator)
{
	CJavaOperatorDefinition* pcOperator;

	mcOperators.GetWithID(eOperator, &pcOperator, NULL);

	return pcOperator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaSeparatorDefinition* CJavaTokenDefinitions::GetSeparator(EJavaSeparator eSeparator)
{
	CJavaSeparatorDefinition* pcSeparator;

	mcSeparators.GetWithID(eSeparator, &pcSeparator, NULL);

	return pcSeparator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaAmbiguousDefinition* CJavaTokenDefinitions::GetAmbiguous(EJavaAmbiguous eAmbiguous)
{
	CJavaAmbiguousDefinition* pcAmbiguous;

	mcAmbiguous.GetWithID(eAmbiguous, &pcAmbiguous, NULL);

	return pcAmbiguous;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaScopeDefinition* CJavaTokenDefinitions::GetScope(EJavaScope eGeneric)
{
	CJavaScopeDefinition* pcGeneric;

	mcScopes.GetWithID(eGeneric, &pcGeneric, NULL);

	return pcGeneric;
}

