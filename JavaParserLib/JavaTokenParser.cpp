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
void CJavaTokenParser::Init(char* szFilename, char* szText, int iTextLen)
{
	mcTokens.Init();
	mcParser.Init(szText, iTextLen);
	mpcStart = NULL;
	mszFileName.Init(szFilename);

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
void CJavaTokenParser::Init(char* szFilename, char* szText)
{
	int		iTextLen;

	iTextLen = strlen(szText);
	Init(szFilename, szText, iTextLen);
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::InitOperators(void)
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::InitSeparators(void)
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
	AddSeparatorDefinition("]", JS_AquareBracketRight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::InitAmbiguous(void)
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
void CJavaTokenParser::InitGenerics(void)
{
	mcGenerics.Init();

	AddGenericDefinition("<", JG_AngleBracketLeft);
	AddGenericDefinition(">", JG_AngleBracketRight);
	AddGenericDefinition("?", JG_QuestionMark);
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
void CJavaTokenParser::AddSeparatorDefinition(char* szSeparator, EJavaSeparator eSeparator)
{
	CJavaSeparatorDefinition	cDefinition;

	cDefinition.Init(eSeparator, szSeparator);
	mcSeparators.Add(szSeparator, &cDefinition, eSeparator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::AddOperatorDefinition(EJavaOperatorType eType, EJavaOperator eOperator, char* szOperator)
{
	CJavaOperatorDefinition	cDefinition;

	cDefinition.Init(eType, eOperator, szOperator);
	mcOperators.Add(szOperator, &cDefinition, eOperator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::AddGenericDefinition(char* szGeneric, EJavaGeneric eGeneric)
{
	CJavaGenericDefinition	cDefinition;

	cDefinition.Init(eGeneric, szGeneric);
	mcGenerics.Add(szGeneric, &cDefinition, eGeneric);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::AddAmbiguousDefinition(char* szAmbiguous, EJavaAmbiguous eAmbiguous)
{
	CJavaAmbiguousDefinition	cDefinition;

	cDefinition.Init(eAmbiguous, szAmbiguous);
	mcAmbiguous.Add(szAmbiguous, &cDefinition, eAmbiguous);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::Kill(void)
{
	mpcStart = NULL;

	mcParser.Kill();
	mcTokens.Kill();

	mszFileName.Kill();

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
void CJavaTokenParser::KillSeparators(void)
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
void CJavaTokenParser::KillOperators(void)
{
	char* szName;
	SEnumeratorIterator			sIterator;
	int							iID;
	CJavaOperatorDefinition*	pcOperator;

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
void CJavaTokenParser::KillAmbiguous(void)
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
void CJavaTokenParser::KillGenerics(void)
{
	char* szName;
	SEnumeratorIterator			sIterator;
	int							iID;
	CJavaGenericDefinition* pcGeneric;

	mcGenerics.StartIteration(&sIterator, &szName, &iID, &pcGeneric);
	while (sIterator.bValid)
	{
		pcGeneric->Kill();
		mcGenerics.Iterate(&sIterator, &szName, &iID, &pcGeneric);
	}

	mcGenerics.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::Parse(void)
{	
	CJavaToken*		pcCurrent;
	CJavaToken*		pcPrevious;
	CJavaNull		pcNull;
	CJavaBoolean	pcBoolean;

	pcCurrent = NULL;
	pcPrevious = NULL;
	for (;;)
	{
		if (pcCurrent)
		{
			if (mpcStart == NULL)
			{
				mpcStart = pcCurrent;
			}

			if (pcPrevious)
			{
				pcPrevious->SetNext(pcCurrent);
			}
		}

		mcParser.SkipWhiteSpace(FALSE);
		if (mcParser.IsOutside())
		{
			return TRITRUE;
		}

		pcPrevious = pcCurrent;
		pcCurrent = NULL;

		ContinueOnTrueReturnOnError(ParseComment(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseKeyword(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseAmbiguous(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseOperator(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseSeparator(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseGeneric(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseBoolean(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseIdentifier(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseInteger(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseCharacter(&pcCurrent));

		return TRIERROR;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaTokenParser::Parse(BOOL bFailOnError)
{
	TRISTATE	tResult;

	tResult = Parse();
	if (tResult == TRITRUE)
	{
		return TRUE;
	}
	else
	{
		CChars	szError;

		szError.Init();
		szError.Append("[");
		szError.Append(mszFileName);
		szError.Append("] Tokenising failed:");
		szError.AppendNewLine();
		mcParser.PrintPosition(&szError);
		szError.DumpKill();

		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* CJavaTokenParser::GetFirstToken(void)
{
	return mpcStart;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::PrettyPrint(CChars* pszDest)
{
	int					iBlockDepth;
	CJavaToken*			pcStartToken;
	CJavaToken*			pcEndToken;
	int					iBlockChange;

	iBlockDepth = 0;

	pcStartToken = GetFirstToken();

	for (;;)
	{
		pcEndToken = GetLineEndToken(pcStartToken);
		iBlockChange = ChangeDepth(pcStartToken, pcEndToken);

		if (iBlockChange < 0)
		{
			iBlockDepth += iBlockChange;
		}

		pszDest->Append('\t', iBlockDepth);

		PrintLine(pszDest, pcStartToken, pcEndToken);

		if (iBlockChange >= 0)
		{
			iBlockDepth += iBlockChange;
		}

		pcStartToken = pcEndToken->GetNext();
		if (pcStartToken == NULL)
		{
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CJavaTokenParser::ChangeDepth(CJavaToken* pcStartToken, CJavaToken* pcEndToken)
{
	CJavaToken*			pcToken;
	CJavaSeparator*		pcSeparator;
	BOOL				bDone;
	int					iBlockDepth;

	bDone = FALSE;
	pcToken = pcStartToken;
	if (pcToken == pcEndToken)
	{
		bDone = TRUE;
	}

	iBlockDepth = 0;
	do
	{
		if (pcToken->IsSeparator())
		{
			pcSeparator = (CJavaSeparator*)pcToken;
			if (pcSeparator->Is(JS_CurlyBracketLeft))
			{
				iBlockDepth++;
			}
			else if (pcSeparator->Is(JS_CurlyBracketRight))
			{
				iBlockDepth--;
			}
		}

		pcToken = pcToken->GetNext();
		if (bDone)
		{
			break;
		}

		if (pcToken == pcEndToken)
		{
			bDone = TRUE;
		}
	} while (TRUE);

	return iBlockDepth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::PrintLine(CChars* pszDest, CJavaToken* pcStartToken ,CJavaToken* pcEndToken)
{
	CJavaToken*			pcToken;
	CJavaToken*			pcNextToken;
	BOOL				bDone;

	bDone = FALSE;
	pcToken = pcStartToken;
	if (pcToken == pcEndToken)
	{
		bDone = TRUE;
	}

	do
	{
		pcToken->Print(pszDest);
		pcNextToken = pcToken->GetNext();

		if (PrintSpace(pcToken, pcNextToken))
		{
			pszDest->Append(' ');
		}

		pcToken = pcNextToken;
		if (bDone)
		{
			break;
		}

		if (pcToken == pcEndToken)
		{
			bDone = TRUE;
		}
	} while (TRUE);

	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::TypePrint(CChars* pszDest)
{
	CJavaToken*		pcToken;

	pcToken = GetFirstToken();
	while (pcToken != NULL)
	{
		pszDest->Append(pcToken->GetType());
		pszDest->Append(": ");
		pcToken->Print(pszDest);
		pszDest->AppendNewLine();

		pcToken = pcToken->GetNext();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::Dump(BOOL bIncludeType)
{
	CChars	szDest;

	szDest.Init();

	if (bIncludeType)
	{
		TypePrint(&szDest);
	}
	else
	{
		PrettyPrint(&szDest);
	}

	szDest.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaToken* CJavaTokenParser::GetLineEndToken(CJavaToken* pcStartToken)
{
	CJavaToken*			pcToken;
	CJavaSeparator*		pcSeparator;
	CJavaKeyword*		pcKeyWord;
	BOOL				bInFor;
	BOOL				bLineEnder;
	CJavaToken*			pcPrevious;
	int					iRoundDepth;
	BOOL				bCloseCurly;

	pcToken = pcStartToken;
	bInFor = FALSE;
	bLineEnder = FALSE;
	iRoundDepth = 0;
	bCloseCurly = FALSE;
	pcPrevious = NULL;

	while (pcToken != NULL)
	{
		if (pcToken->IsComment())
		{
			return pcToken;
		}
		else
		{
			if (bLineEnder)
			{
				if (pcToken->IsKeyword())
				{
					pcKeyWord = (CJavaKeyword*)pcToken;
					if (pcKeyWord->Is(JK_while))
					{
						bLineEnder = FALSE;
					}
					else
					{
						return pcPrevious;
					}
				}
				else
				{
					return pcPrevious;
				}
			}
		}

		if (pcToken->IsSeparator())
		{
			pcSeparator = (CJavaSeparator*)pcToken;
			if (pcSeparator->Is(JS_Semicolon) && !bInFor)
			{
				bLineEnder = TRUE;
			}
			else if (pcSeparator->Is(JS_CurlyBracketLeft))
			{
				bLineEnder = TRUE;
			}
			else if (pcSeparator->Is(JS_CurlyBracketRight))
			{
				bCloseCurly = TRUE;
				bLineEnder = TRUE;
			}
			else if (bInFor && pcSeparator->Is(JS_RoundBracketLeft))
			{
				iRoundDepth++;
			}
			else if (bInFor && pcSeparator->Is(JS_RoundBracketRight))
			{
				iRoundDepth--;
				if (iRoundDepth == 0)
				{
					bLineEnder = TRUE;
				}
			}
		}
		else if (pcToken->IsKeyword())
		{
			pcKeyWord = (CJavaKeyword*)pcToken;
			if (pcKeyWord->Is(JK_do))
			{
				bLineEnder = TRUE;
			}
			else if (pcKeyWord->Is(JK_for))
			{
				bInFor = TRUE;
				iRoundDepth = 0;
			}
		}

		pcPrevious = pcToken;
		pcToken = pcToken->GetNext();
	}

	return pcPrevious;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaTokenParser::PrintSpace(CJavaToken* pcLeft, CJavaToken* pcRight)
{
	BOOL	bCharThingLeft;
	BOOL	bCharThingRight;

	if (pcRight == NULL)
	{
		return FALSE;
	}

	if (pcRight->IsComment())
	{
		return TRUE;
	}

	bCharThingLeft = pcLeft->IsKeyword() || pcLeft->IsIdentifier() || pcLeft->IsLiteral();
	bCharThingRight = pcRight->IsKeyword() || pcRight->IsIdentifier() || pcRight->IsLiteral();

	if (bCharThingLeft && bCharThingRight)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseComment(CJavaToken** ppcCurrent)
{
	char		szText[4 KB];
	int			iLength;
	TRISTATE	tResult;

	tResult = mcParser.GetComment(szText, &iLength, FALSE);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mcTokens.CreateComment(szText, iLength);
		return TRITRUE;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseKeyword(CJavaToken** ppcCurrent)
{
	EJavaKeyword				eKeyword;
	CJavaKeywordDefinition*		pcKeyword;
	TRISTATE					tResult;

	tResult = mcParser.GetEnumeratorIdentifier<CJavaKeywordDefinition>(&mcKeywords, (int*)&eKeyword, FALSE);
	if (tResult == TRITRUE)
	{
		mcKeywords.GetWithID(eKeyword, &pcKeyword, NULL);
		*ppcCurrent = mcTokens.CreateKeyword(pcKeyword);
		return TRITRUE;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseAmbiguous(CJavaToken** ppcCurrent)
{
	TRISTATE					tResult;
	EJavaAmbiguous				eAmbiguous;
	CJavaAmbiguousDefinition*	pcAmbiguous;

	tResult = mcParser.GetEnumeratorSequence<CJavaAmbiguousDefinition>(&mcAmbiguous, (int*)&eAmbiguous, FALSE);
	if (tResult == TRITRUE)
	{
		mcAmbiguous.GetWithID(eAmbiguous, &pcAmbiguous, NULL);
		*ppcCurrent = mcTokens.CreateAmbiguous(pcAmbiguous);
		return TRITRUE;;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseOperator(CJavaToken** ppcCurrent)
{
	TRISTATE					tResult;
	EJavaOperator				eOperator;
	CJavaOperatorDefinition*	pcOperator;

	tResult = mcParser.GetEnumeratorSequence<CJavaOperatorDefinition>(&mcOperators, (int*)&eOperator, FALSE);
	if (tResult == TRITRUE)
	{
		mcOperators.GetWithID(eOperator, &pcOperator, NULL);
		*ppcCurrent = mcTokens.CreateOperator(pcOperator);
		return TRITRUE;;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseSeparator(CJavaToken** ppcCurrent)
{
	TRISTATE					tResult;
	EJavaSeparator				eSeparator;
	CJavaSeparatorDefinition*	pcSeparator;

	tResult = mcParser.GetEnumeratorSequence<CJavaSeparatorDefinition>(&mcSeparators, (int*)&eSeparator, FALSE);
	if (tResult == TRITRUE)
	{
		mcSeparators.GetWithID(eSeparator, &pcSeparator, NULL);
		*ppcCurrent = mcTokens.CreateSeparator(pcSeparator);
		return TRITRUE;;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseGeneric(CJavaToken** ppcCurrent)
{
	TRISTATE					tResult;
	EJavaGeneric				eGeneric;
	CJavaGenericDefinition*		pcGeneric;

	tResult = mcParser.GetEnumeratorSequence<CJavaGenericDefinition>(&mcGenerics, (int*)&eGeneric, FALSE);
	if (tResult == TRITRUE)
	{
		mcGenerics.GetWithID(eGeneric, &pcGeneric, NULL);
		*ppcCurrent = mcTokens.CreateGeneric(pcGeneric);
		return TRITRUE;;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseBoolean(CJavaToken** ppcCurrent)
{
	TRISTATE					tResult;

	tResult = mcParser.GetExactIdentifier("true", FALSE);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mcTokens.CreateBoolean(TRUE);
		return TRITRUE;;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}

	tResult = mcParser.GetExactIdentifier("false", FALSE);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mcTokens.CreateBoolean(FALSE);
		return TRITRUE;;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseIdentifier(CJavaToken** ppcCurrent)
{
	TRISTATE	tResult;
	char		szText[4 KB];
	int			iLength;


	tResult = mcParser.GetIdentifier(szText, &iLength, FALSE, FALSE);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mcTokens.CreateIdentifier(szText, iLength);
		return TRITRUE;;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseInteger(CJavaToken** ppcCurrent)
{
	uint64		ulli;
	int			iBase;
	int			iNumDigits;
	TRISTATE	tResult;
	int			iSuffix;

	tResult = mcParser.GetIntegerLiteral(&ulli, INTEGER_PREFIX_ALL, &iBase, INTEGER_SUFFIX_JAVA, &iSuffix, INTEGER_SEPARATOR_UNDERSCORE, &iNumDigits, FALSE);
	if (tResult == TRITRUE)
	{
		if (iSuffix & INTEGER_SUFFIX_L)
		{
			*ppcCurrent = mcTokens.CreateInteger((int64)ulli);
			return TRITRUE;;
		}
		else
		{
			*ppcCurrent = mcTokens.CreateInteger((int32)ulli);
			return TRITRUE;;
		}
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		return TRIFALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseCharacter(CJavaToken** ppcCurrent)
{
	TRISTATE	tResult;
	int			iWidth;
	uint16		c;

	tResult = mcParser.GetCharacterLiteral(&c, TRUE, &iWidth, FALSE);
	if (tResult == TRITRUE)
	{
		if (iWidth == 1)
		{
			*ppcCurrent = mcTokens.CreateCharacter((char)c);
			return TRITRUE;;
		}
		else if (iWidth = 2)
		{
			*ppcCurrent = mcTokens.CreateInteger((char16)c);
			return TRITRUE;;
		}
		else
		{
			return TRIERROR;
		}
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		return TRIFALSE;
	}
}


