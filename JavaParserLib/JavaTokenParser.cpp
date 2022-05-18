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
void CJavaTokenParser::Init(CJavaTokenDefinitions* pcDefinitions, CJavaTokenMemory* pcTokens, char* szFilename, char* szText, int iTextLen)
{
	mpcTokens = pcTokens;
	mcParser.Init(szText, iTextLen);
	mpcStart = NULL;
	mszFileName.Init(szFilename);

	mpcDefinitions = pcDefinitions;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::Init(CJavaTokenDefinitions* pcDefinitions, CJavaTokenMemory* pcTokens, char* szFilename, char* szText)
{
	int		iTextLen;

	iTextLen = strlen(szText);
	Init(pcDefinitions, pcTokens, szFilename, szText, iTextLen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::Kill(void)
{
	mpcStart = NULL;

	mcParser.Kill();
	mpcTokens = NULL;

	mszFileName.Kill();

	mpcDefinitions = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CJavaTokenParser::Parse(void)
{	
	CJavaToken*			pcCurrent;
	CJavaToken*			pcPrevious;
	CJavaTokenNull		pcNull;
	CJavaTokenBoolean	pcBoolean;

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
		ContinueOnTrueReturnOnError(ParseScope(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseBoolean(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseNull(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseIdentifier(&pcCurrent));
		ContinueOnTrueReturnOnError(ParseAnnotation(&pcCurrent));
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
	CJavaToken*			pcLastStartToken;
	int					iBlockChange;
	BOOL				bAppendNewLine;
	BOOL				bMethodDeclaration;
	BOOL				bAnnotation;
	BOOL				bLastAnnotation;

	iBlockDepth = 0;

	pcLastStartToken = NULL;
	bLastAnnotation = FALSE;
	pcStartToken = GetFirstToken();

	for (;;)
	{
		pcEndToken = GetLineEndToken(pcStartToken);
		iBlockChange = ChangeDepth(pcStartToken, pcEndToken);

		if (iBlockChange < 0)
		{
			iBlockDepth += iBlockChange;
		}

		bMethodDeclaration = IsMethodDeclaration(pcStartToken, pcEndToken);
		bAnnotation = IsAnnotation(pcStartToken, pcEndToken);
		bAppendNewLine = PrintNewLine(pcStartToken, pcLastStartToken);
		if ((bAppendNewLine || bMethodDeclaration) && !bLastAnnotation)
		{
			pszDest->AppendNewLine();
		}
		bLastAnnotation = bAnnotation;

		pszDest->Append('\t', iBlockDepth);

		PrintLine(pszDest, pcStartToken, pcEndToken);

		if (iBlockChange >= 0)
		{
			iBlockDepth += iBlockChange;
		}

		pcLastStartToken = pcStartToken;
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
BOOL CJavaTokenParser::PrintNewLine(CJavaToken* pcStartToken, CJavaToken* pcLastStartToken)
{
	CJavaTokenKeyword*		pcStartKeyWord;
	CJavaTokenKeyword*		pcPreviousStartKeyWord;

	if (pcLastStartToken != NULL)
	{
		if (pcLastStartToken->IsKeyword())
		{
			pcPreviousStartKeyWord = (CJavaTokenKeyword*)pcLastStartToken;

			if (pcStartToken->IsKeyword())
			{
				pcStartKeyWord = (CJavaTokenKeyword*)pcStartToken;
				if (pcStartKeyWord->Is(JK_import) && pcPreviousStartKeyWord->Is(JK_package))
				{
					return TRUE;
				}
				else if (pcPreviousStartKeyWord->Is(JK_import) && !pcStartKeyWord->Is(JK_import))
				{
					return TRUE;
				}
			}
			else if (pcPreviousStartKeyWord->Is(JK_import))
			{
				return TRUE;
			}
		}

		if (pcStartToken->IsKeyword())
		{
			pcStartKeyWord = (CJavaTokenKeyword*)pcStartToken;
			if (pcStartKeyWord->Is(JK_if) || pcStartKeyWord->Is(JK_for))
			{
				if (!pcLastStartToken->IsSeparator())
				{
					return TRUE;
				}
			}
		}

		if (pcStartToken->IsAnnotation())
		{
			return TRUE;
		}
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaTokenParser::IsMethodDeclaration(CJavaToken* pcStartToken, CJavaToken* pcEndToken)
{
	CJavaToken*				pcToken;
	CJavaTokenSeparator*	pcSeparator;
	CJavaTokenKeyword*		pcKeyword;
	BOOL					bDone;
	int						iLeftPosition;
	int						iRightPosition;
	int						iPosition;
	int						iLeftCount;
	int						iRightCount;
	int						iModifierCount;

	bDone = FALSE;
	pcToken = pcStartToken;
	if (pcToken == pcEndToken)
	{
		return FALSE;
	}

	iPosition = 0;
	iLeftCount = 0;
	iRightCount = 0;
	iModifierCount = 0;
	do
	{
		if (pcToken->IsSeparator())
		{
			pcSeparator = (CJavaTokenSeparator*)pcToken;
			if (pcSeparator->Is(JS_RoundBracketLeft))
			{
				iLeftPosition = iPosition;
				iLeftCount++;
			}
			else if (pcSeparator->Is(JS_RoundBracketRight))
			{

				iRightPosition = iPosition;
				iRightCount++;
			}
		}

		if (pcToken->IsKeyword())
		{
			pcKeyword = (CJavaTokenKeyword*)pcToken;
			if (pcKeyword->Is(JK_private) || pcKeyword->Is(JK_protected) || pcKeyword->Is(JK_public))
			{
				iModifierCount++;
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

	if ((iLeftCount == 1) && (iRightCount == 1) && (iModifierCount > 0))
	{
		return TRUE;
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CJavaTokenParser::IsAnnotation(CJavaToken* pcStartToken, CJavaToken* pcEndToken)
{
	CJavaToken*		pcToken;
	BOOL			bDone;

	bDone = FALSE;
	pcToken = pcStartToken;
	bDone = (pcToken == pcEndToken);

	do
	{
		if (pcToken->IsAnnotation())
		{
			return TRUE;
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

	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CJavaTokenParser::ChangeDepth(CJavaToken* pcStartToken, CJavaToken* pcEndToken)
{
	CJavaToken*				pcToken;
	CJavaTokenSeparator*	pcSeparator;
	BOOL					bDone;
	int						iBlockDepth;

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
			pcSeparator = (CJavaTokenSeparator*)pcToken;
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
	CJavaToken*		pcToken;
	CJavaToken*		pcNextToken;
	BOOL			bDone;

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
	CJavaToken*				pcToken;
	CJavaTokenSeparator*	pcSeparator;
	CJavaTokenKeyword*		pcKeyWord;
	BOOL					bInFor;
	BOOL					bLineEnder;
	CJavaToken*				pcPrevious;
	int						iRoundDepth;
	BOOL					bCloseCurly;

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
					pcKeyWord = (CJavaTokenKeyword*)pcToken;
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
			pcSeparator = (CJavaTokenSeparator*)pcToken;
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
			pcKeyWord = (CJavaTokenKeyword*)pcToken;
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
		else if (pcToken->IsAnnotation())
		{
			bLineEnder = TRUE;
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
	BOOL					bCharThingLeft;
	BOOL					bCharThingRight;
	CJavaTokenSeparator*	pcSeparator;
	CCJavaTokenAmbiguous*	pcAmbiguous;
	CJavaTokenKeyword*		pcKeyword;
	CJavaTokenOperator*		pcOperator;
	BOOL					bMethodLeft;
	BOOL					bMethodRight;

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

	if (pcLeft->IsOperator() || pcRight->IsOperator())
	{
		bMethodLeft = FALSE;
		bMethodRight = FALSE;
		if (pcLeft->IsOperator())
		{
			pcOperator = (CJavaTokenOperator*)pcLeft;
			if (pcOperator->Is(JO_MethodReference))
			{
				bMethodLeft = TRUE;
			}
		}
		if (pcRight->IsOperator())
		{
			pcOperator = (CJavaTokenOperator*)pcRight;
			if (pcOperator->Is(JO_MethodReference))
			{
				bMethodRight = TRUE;
			}
		}

		if (!bMethodLeft && !bMethodRight)
		{
			return TRUE;
		}
	}

	if (pcLeft->IsSeparator())
	{
		pcSeparator = (CJavaTokenSeparator*)pcLeft;
		if (pcSeparator->Is(JS_Comma))
		{
			return TRUE;
		}

		if (pcSeparator->Is(JS_SquareBracketRight))
		{
			if (bCharThingRight)
			{
				return TRUE;
			}
		}
	}

	if (pcLeft->IsAmbiguous())
	{
		pcAmbiguous = (CCJavaTokenAmbiguous*)pcLeft;
		if (pcAmbiguous->Is(JA_QuestionMark))
		{
			return TRUE;
		}

		if (pcAmbiguous->Is(JA_AngleBracketRight))
		{
			if (bCharThingRight)
			{
				return TRUE;
			}
		}
	}

	if (pcRight->IsAmbiguous())
	{
		pcAmbiguous = (CCJavaTokenAmbiguous*)pcRight;
		if (pcAmbiguous->Is(JA_QuestionMark))
		{
			if (pcLeft->IsIdentifier())
			{
				return TRUE;
			}

			if (pcLeft->IsSeparator())
			{
				pcSeparator = (CJavaTokenSeparator*)pcLeft;
				if (pcSeparator->Is(JS_SquareBracketRight) || pcSeparator->Is(JS_RoundBracketRight))
				{
					return TRUE;
				}
			}
		}
	}

	if (pcLeft->IsKeyword())
	{
		pcKeyword = (CJavaTokenKeyword*)pcLeft;
		if (pcKeyword->Is(JK_if) || pcKeyword->Is(JK_for))
		{
			return TRUE;
		}
	}

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
		*ppcCurrent = mpcTokens->CreateComment(szText, iLength);
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
	EJavaTokenKeyword				eKeyword;
	CJavaTokenKeywordDefinition*	pcKeyword;
	TRISTATE						tResult;

	tResult = mcParser.GetEnumeratorIdentifier<CJavaTokenKeywordDefinition>(mpcDefinitions->GetKeywords(), (int*)&eKeyword, FALSE);
	if (tResult == TRITRUE)
	{
		pcKeyword = mpcDefinitions->GetKeyword(eKeyword);
		*ppcCurrent = mpcTokens->CreateKeyword(pcKeyword);
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
	TRISTATE							tResult;
	ECJavaTokenAmbiguous				eAmbiguous;
	CCJavaTokenAmbiguousDefinition*		pcAmbiguous;

	tResult = mcParser.GetEnumeratorSequence<CCJavaTokenAmbiguousDefinition>(mpcDefinitions->GetAmbiguous(), (int*)&eAmbiguous, FALSE);
	if (tResult == TRITRUE)
	{
		pcAmbiguous = mpcDefinitions->GetAmbiguous(eAmbiguous);
		*ppcCurrent = mpcTokens->CreateAmbiguous(pcAmbiguous);
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
	TRISTATE						tResult;
	EJavaTokenOperator				eOperator;
	CJavaTokenOperatorDefinition*	pcOperator;

	tResult = mcParser.GetEnumeratorSequence<CJavaTokenOperatorDefinition>(mpcDefinitions->GetOperators(), (int*)&eOperator, FALSE);
	if (tResult == TRITRUE)
	{
		pcOperator = mpcDefinitions->GetOperator(eOperator);
		*ppcCurrent = mpcTokens->CreateOperator(pcOperator);
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
	TRISTATE						tResult;
	EJavaTokenSeparator				eSeparator;
	CJavaTokenSeparatorDefinition*	pcSeparator;

	tResult = mcParser.GetEnumeratorSequence<CJavaTokenSeparatorDefinition>(mpcDefinitions->GetSeparators(), (int*)&eSeparator, FALSE);
	if (tResult == TRITRUE)
	{
		pcSeparator = mpcDefinitions->GetSeparator(eSeparator);
		*ppcCurrent = mpcTokens->CreateSeparator(pcSeparator);
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
TRISTATE CJavaTokenParser::ParseScope(CJavaToken** ppcCurrent)
{
	TRISTATE					tResult;
	EJavaTokenScope				eScope;
	CJavaTokenScopeDefinition*	pcScope;

	tResult = mcParser.GetEnumeratorSequence<CJavaTokenScopeDefinition>(mpcDefinitions->GetScopes(), (int*)&eScope, FALSE);
	if (tResult == TRITRUE)
	{
		pcScope = mpcDefinitions->GetScope(eScope);
		*ppcCurrent = mpcTokens->CreateScope(pcScope);
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
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("true", FALSE);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mpcTokens->CreateBoolean(TRUE);
		return TRITRUE;;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}

	tResult = mcParser.GetExactIdentifier("false", FALSE);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mpcTokens->CreateBoolean(FALSE);
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
		*ppcCurrent = mpcTokens->CreateIdentifier(szText, iLength);
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
TRISTATE CJavaTokenParser::ParseAnnotation(CJavaToken** ppcCurrent)
{
	TRISTATE		tResult;
	char			szText[4 KB];
	int				iLength;
	STextPosition	sPosition;

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacter('@', FALSE);
	if (tResult == TRITRUE)
	{
		mcParser.GetPosition(&sPosition);
		tResult = mcParser.GetIdentifier(szText, &iLength, FALSE, FALSE);
		if (tResult == TRITRUE)
		{
			*ppcCurrent = mpcTokens->CreateAnnotation(szText, iLength, &sPosition);
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
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}
	else
	{
		mcParser.PopPosition();
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
			*ppcCurrent = mpcTokens->CreateInteger((int64)ulli);
			return TRITRUE;;
		}
		else if (iSuffix == INTEGER_SUFFIX_NONE)
		{
			*ppcCurrent = mpcTokens->CreateInteger((int32)ulli);
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
			*ppcCurrent = mpcTokens->CreateFloat((float64)ldf);
			return TRITRUE;;
		}
		else if (iSuffix == FLOAT_SUFFIX_F)
		{
			*ppcCurrent = mpcTokens->CreateFloat((float32)ldf);
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
			*ppcCurrent = mpcTokens->CreateCharacter((char)c);
			return TRITRUE;;
		}
		else if (iWidth = 2)
		{
			*ppcCurrent = mpcTokens->CreateCharacter((char16)c);
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
			*ppcCurrent = mpcTokens->CreateString(sz, iLength);
			return TRITRUE;;
		}
		else if (iWidth == 2)
		{
			*ppcCurrent = mpcTokens->CreateString((char16*)sz, iLength);
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
	TRISTATE	tResult;

	tResult = mcParser.GetExactIdentifier("null", FALSE);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mpcTokens->CreateNull();
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

