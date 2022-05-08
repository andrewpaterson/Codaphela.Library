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
void CJavaTokenParser::Init(CJavaTokenDefinitions* pcDefinitions, char* szFilename, char* szText, int iTextLen)
{
	mcTokens.Init();
	mcParser.Init(szText, iTextLen);
	mpcStart = NULL;
	mszFileName.Init(szFilename);

	mpcDefinitions = pcDefinitions;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::Init(CJavaTokenDefinitions* pcDefinitions, char* szFilename, char* szText)
{
	int		iTextLen;

	iTextLen = strlen(szText);
	Init(pcDefinitions, szFilename, szText, iTextLen);
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

	mpcDefinitions = NULL;
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
		ContinueOnTrueReturnOnError(ParseNull(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseIdentifier(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseFloat(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseInteger(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseCharacter(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseString(&pcCurrent));

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
				if (pcPrevious)
				{
					pcToken = pcPrevious;
				}
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

	if (pcLeft->IsOperator() || pcRight->IsOperator())
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

	tResult = mcParser.GetEnumeratorIdentifier<CJavaKeywordDefinition>(mpcDefinitions->GetKeywords(), (int*)&eKeyword, FALSE);
	if (tResult == TRITRUE)
	{
		pcKeyword = mpcDefinitions->GetKeyword(eKeyword);
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

	tResult = mcParser.GetEnumeratorSequence<CJavaAmbiguousDefinition>(mpcDefinitions->GetAmbiguous(), (int*)&eAmbiguous, FALSE);
	if (tResult == TRITRUE)
	{
		pcAmbiguous = mpcDefinitions->GetAmbiguous(eAmbiguous);
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

	tResult = mcParser.GetEnumeratorSequence<CJavaOperatorDefinition>(mpcDefinitions->GetOperators(), (int*)&eOperator, FALSE);
	if (tResult == TRITRUE)
	{
		pcOperator = mpcDefinitions->GetOperator(eOperator);
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

	tResult = mcParser.GetEnumeratorSequence<CJavaSeparatorDefinition>(mpcDefinitions->GetSeparators(), (int*)&eSeparator, FALSE);
	if (tResult == TRITRUE)
	{
		pcSeparator = mpcDefinitions->GetSeparator(eSeparator);
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

	tResult = mcParser.GetEnumeratorSequence<CJavaGenericDefinition>(mpcDefinitions->GetGenerics(), (int*)&eGeneric, FALSE);
	if (tResult == TRITRUE)
	{
		pcGeneric = mpcDefinitions->GetGeneric(eGeneric);
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

	tResult = mcParser.GetIntegerLiteral(&ulli, INTEGER_PREFIX_ALL, &iBase, INTEGER_SUFFIX_JAVA, &iSuffix, NUMBER_SEPARATOR_UNDERSCORE, &iNumDigits, FALSE);
	if (tResult == TRITRUE)
	{
		if (iSuffix == INTEGER_SUFFIX_L)
		{
			*ppcCurrent = mcTokens.CreateInteger((int64)ulli);
			return TRITRUE;;
		}
		else if (iSuffix == INTEGER_SUFFIX_NONE)
		{
			*ppcCurrent = mcTokens.CreateInteger((int32)ulli);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseFloat(CJavaToken** ppcCurrent)
{
	float96		ldf;
	int			iBase;
	int			iNumWholeDigits;
	int			iNumDecinalDigits;
	int			iNumExponentDigits;
	TRISTATE	tResult;
	int			iSuffix;
	int			iExponent;

	tResult = mcParser.GetFloatLiteral(&ldf, FLOAT_PREFIX_ALL, &iBase, FLOAT_SUFFIX_JAVA, &iSuffix, FLOAT_EXPONENT_ALL, &iExponent, NUMBER_SEPARATOR_UNDERSCORE, &iNumWholeDigits, &iNumDecinalDigits, &iNumExponentDigits, FALSE);
	if (tResult == TRITRUE)
	{
		if ((iSuffix == FLOAT_SUFFIX_D) || (iSuffix == FLOAT_SUFFIX_NONE))
		{
			*ppcCurrent = mcTokens.CreateFloat((float64)ldf);
			return TRITRUE;;
		}
		else if (iSuffix == FLOAT_SUFFIX_F)
		{
			*ppcCurrent = mcTokens.CreateFloat((float32)ldf);
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
			*ppcCurrent = mcTokens.CreateCharacter((char16)c);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseString(CJavaToken** ppcCurrent)
{
	TRISTATE	tResult;
	int			iWidth;
	char		sz[4 KB];
	int			iLength;

	tResult = mcParser.GetStringLiteral(&sz, 4 KB, TRUE, &iLength, &iWidth, FALSE);
	if (tResult == TRITRUE)
	{
		if (iWidth == 1)
		{
			*ppcCurrent = mcTokens.CreateString(sz, iLength);
			return TRITRUE;;
		}
		else if (iWidth == 2)
		{
			*ppcCurrent = mcTokens.CreateString((char16*)sz, iLength);
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::ParseNull(CJavaToken** ppcCurrent)
{
	TRISTATE					tResult;

	tResult = mcParser.GetExactIdentifier("null", FALSE);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mcTokens.CreateNull();
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

