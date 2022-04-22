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
#include "JavaTokenParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::Init(char* szText, int iTextLen)
{
	mcTokens.Init();
	mcParser.Init(szText, iTextLen);

	InitKeywords();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::Init(char* szText)
{
	int		iTextLen;

	iTextLen = strlen(szText);
	Init(szText, iTextLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::InitKeywords(void)
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
	AddKeywordDefinition("true", JK_true);
	AddKeywordDefinition("false", JK_false);
	AddKeywordDefinition("null", JK_null);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::AddKeywordDefinition(char* szKeyword, EJavaKeyword eKeyword)
{
	CJavaKeywordDefinition	cDefinition;

	cDefinition.Init(eKeyword, szKeyword);
	mcKeywords.Add(szKeyword, &cDefinition, eKeyword);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::Kill(void)
{
	mcParser.Kill();
	mcTokens.Kill();

	KillKeywords();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::KillKeywords(void)
{
	char*					szName;
	SEnumeratorIterator		sIterator;
	int						iID;
	CJavaKeywordDefinition* pcKeyword;

	mcKeywords.StartIteration(&sIterator, &szName, &iID, &pcKeyword);
	while(sIterator.bValid)
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
TRISTATE CJavaTokenParser::Parse(void)
{	
	BOOL	bResult;

	for (;;)
	{
		bResult = mcParser.SkipCPPStyleComment();
		if (bResult)
		{
			mcTokens.AddComment(NULL, 0);
		}
	}
}

