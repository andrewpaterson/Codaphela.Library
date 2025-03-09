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
void CJavaTokenParser::Init(CLogger* pcLogger, CJavaTokenDefinitions* pcDefinitions, CJavaTokenMemory* pcTokens, char* szFilename, char* szText, int iTextLen)
{
	mpcTokens = pcTokens;
	mcParser.Init(szText, iTextLen);
	mpcStart = NULL;
	mszFilename.Init(szFilename);

	mpcDefinitions = pcDefinitions;
	mpcLogger = pcLogger;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::Init(CLogger* pcLogger, CJavaTokenDefinitions* pcDefinitions, CJavaTokenMemory* pcTokens, char* szFilename, char* szText)
{
	int		iTextLen;

	iTextLen = strlen(szText);
	Init(pcLogger, pcDefinitions, pcTokens, szFilename, szText, iTextLen);
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

	mszFilename.Kill();

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

		mcParser.SkipWhitespace();
		if (mcParser.IsOutside())
		{
			return TRITRUE;
		}

		pcPrevious = pcCurrent;
		pcCurrent = NULL;

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
bool CJavaTokenParser::Parse(bool bFailOnError)
{
	TRISTATE	tResult;

	tResult = Parse();
	if (tResult == TRITRUE)
	{
		return true;
	}
	else
	{
		CChars	szError;

		szError.Init();
		szError.Append("[");
		szError.Append(mszFilename);
		szError.Append("] Tokenising failed:");
		szError.AppendNewLine();
		mcParser.PrintPosition(&szError);
		szError.DumpKill();

		return false;
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
CLogger* CJavaTokenParser::GetLogger(void)
{
	return mpcLogger;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CJavaTokenParser::GetText(CExternalString* pcDest)
{
	mcParser.GetText(pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenMemory* CJavaTokenParser::GetTokenMemory(void)
{
	return mpcTokens;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CJavaTokenDefinitions* CJavaTokenParser::GetTokenDefinitions(void)
{
	return mpcDefinitions;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CJavaTokenParser::GetFilename(void)
{
	return mszFilename.Text();
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
	bool				bAppendNewLine;
	bool				bMethodDeclaration;
	bool				bAnnotation;
	bool				bLastAnnotation;

	iBlockDepth = 0;

	pcLastStartToken = NULL;
	bLastAnnotation = false;
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
bool CJavaTokenParser::PrintNewLine(CJavaToken* pcStartToken, CJavaToken* pcLastStartToken)
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
					return true;
				}
				else if (pcPreviousStartKeyWord->Is(JK_import) && !pcStartKeyWord->Is(JK_import))
				{
					return true;
				}
			}
			else if (pcPreviousStartKeyWord->Is(JK_import))
			{
				return true;
			}
		}

		if (pcStartToken->IsKeyword())
		{
			pcStartKeyWord = (CJavaTokenKeyword*)pcStartToken;
			if (pcStartKeyWord->Is(JK_if) || pcStartKeyWord->Is(JK_for))
			{
				if (!pcLastStartToken->IsSeparator())
				{
					return true;
				}
			}
		}

		if (pcStartToken->IsAnnotation())
		{
			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaTokenParser::IsMethodDeclaration(CJavaToken* pcStartToken, CJavaToken* pcEndToken)
{
	CJavaToken*				pcToken;
	CJavaTokenSeparator*	pcSeparator;
	CJavaTokenKeyword*		pcKeyword;
	bool					bDone;
	int						iLeftPosition;
	int						iRightPosition;
	int						iPosition;
	int						iLeftCount;
	int						iRightCount;
	int						iModifierCount;

	bDone = false;
	pcToken = pcStartToken;
	if (pcToken == pcEndToken)
	{
		return false;
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
			bDone = true;
		}
	} while (true);

	if ((iLeftCount == 1) && (iRightCount == 1) && (iModifierCount > 0))
	{
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CJavaTokenParser::IsAnnotation(CJavaToken* pcStartToken, CJavaToken* pcEndToken)
{
	CJavaToken*		pcToken;
	bool			bDone;

	bDone = false;
	pcToken = pcStartToken;
	bDone = (pcToken == pcEndToken);

	do
	{
		if (pcToken->IsAnnotation())
		{
			return true;
		}

		pcToken = pcToken->GetNext();
		if (bDone)
		{
			break;
		}

		if (pcToken == pcEndToken)
		{
			bDone = true;
		}
	} while (true);

	return false;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CJavaTokenParser::ChangeDepth(CJavaToken* pcStartToken, CJavaToken* pcEndToken)
{
	CJavaToken*				pcToken;
	CJavaTokenSeparator*	pcSeparator;
	bool					bDone;
	int						iBlockDepth;

	bDone = false;
	pcToken = pcStartToken;
	if (pcToken == pcEndToken)
	{
		bDone = true;
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
			bDone = true;
		}
	} while (true);

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
	bool			bDone;

	bDone = false;
	pcToken = pcStartToken;
	if (pcToken == pcEndToken)
	{
		bDone = true;
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
			bDone = true;
		}
	} while (true);

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
void CJavaTokenParser::Dump(bool bIncludeType)
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
	bool					bInFor;
	bool					bLineEnder;
	CJavaToken*				pcPrevious;
	int						iRoundDepth;
	bool					bCloseCurly;

	pcToken = pcStartToken;
	bInFor = false;
	bLineEnder = false;
	iRoundDepth = 0;
	bCloseCurly = false;
	pcPrevious = NULL;

	while (pcToken != NULL)
	{
		if (bLineEnder)
		{
			if (pcToken->IsKeyword())
			{
				pcKeyWord = (CJavaTokenKeyword*)pcToken;
				if (pcKeyWord->Is(JK_while))
				{
					bLineEnder = false;
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

		if (pcToken->IsSeparator())
		{
			pcSeparator = (CJavaTokenSeparator*)pcToken;
			if (pcSeparator->Is(JS_Semicolon) && !bInFor)
			{
				bLineEnder = true;
			}
			else if (pcSeparator->Is(JS_CurlyBracketLeft))
			{
				if (pcPrevious)
				{
					pcToken = pcPrevious;
				}
				bLineEnder = true;
			}
			else if (pcSeparator->Is(JS_CurlyBracketRight))
			{
				bCloseCurly = true;
				bLineEnder = true;
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
					bLineEnder = true;
				}
			}
		}
		else if (pcToken->IsKeyword())
		{
			pcKeyWord = (CJavaTokenKeyword*)pcToken;
			if (pcKeyWord->Is(JK_do))
			{
				bLineEnder = true;
			}
			else if (pcKeyWord->Is(JK_for))
			{
				bInFor = true;
				iRoundDepth = 0;
			}
		}
		else if (pcToken->IsAnnotation())
		{
			bLineEnder = true;
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
bool CJavaTokenParser::PrintSpace(CJavaToken* pcLeft, CJavaToken* pcRight)
{
	bool					bCharThingLeft;
	bool					bCharThingRight;
	CJavaTokenSeparator*	pcSeparator;
	CCJavaTokenAmbiguous*	pcAmbiguous;
	CJavaTokenKeyword*		pcKeyword;
	CJavaTokenOperator*		pcOperator;
	bool					bMethodLeft;
	bool					bMethodRight;

	if (pcRight == NULL)
	{
		return false;
	}

	bCharThingLeft = pcLeft->IsKeyword() || pcLeft->IsIdentifier() || pcLeft->IsLiteral();
	bCharThingRight = pcRight->IsKeyword() || pcRight->IsIdentifier() || pcRight->IsLiteral();

	if (pcLeft->IsOperator() || pcRight->IsOperator())
	{
		bMethodLeft = false;
		bMethodRight = false;
		if (pcLeft->IsOperator())
		{
			pcOperator = (CJavaTokenOperator*)pcLeft;
			if (pcOperator->Is(JO_MethodReference))
			{
				bMethodLeft = true;
			}
		}
		if (pcRight->IsOperator())
		{
			pcOperator = (CJavaTokenOperator*)pcRight;
			if (pcOperator->Is(JO_MethodReference))
			{
				bMethodRight = true;
			}
		}

		if (!bMethodLeft && !bMethodRight)
		{
			return true;
		}
	}

	if (pcLeft->IsSeparator())
	{
		pcSeparator = (CJavaTokenSeparator*)pcLeft;
		if (pcSeparator->Is(JS_Comma))
		{
			return true;
		}

		if (pcSeparator->Is(JS_SquareBracketRight))
		{
			if (bCharThingRight)
			{
				return true;
			}
		}
	}

	if (pcLeft->IsAmbiguous())
	{
		pcAmbiguous = (CCJavaTokenAmbiguous*)pcLeft;
		if (pcAmbiguous->Is(JA_QuestionMark))
		{
			return true;
		}

		if (pcAmbiguous->Is(JA_AngleBracketRight))
		{
			if (bCharThingRight)
			{
				return true;
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
				return true;
			}

			if (pcLeft->IsSeparator())
			{
				pcSeparator = (CJavaTokenSeparator*)pcLeft;
				if (pcSeparator->Is(JS_SquareBracketRight) || pcSeparator->Is(JS_RoundBracketRight))
				{
					return true;
				}
			}
		}
	}

	if (pcLeft->IsKeyword())
	{
		pcKeyword = (CJavaTokenKeyword*)pcLeft;
		if (pcKeyword->Is(JK_if) || pcKeyword->Is(JK_for))
		{
			return true;
		}
	}

	if (bCharThingLeft && bCharThingRight)
	{
		return true;
	}
	else
	{
		return false;
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
	STextPosition					sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetEnumeratorIdentifier<CJavaTokenKeywordDefinition>(mpcDefinitions->GetKeywords(), (size*)&eKeyword, false);
	if (tResult == TRITRUE)
	{
		pcKeyword = mpcDefinitions->GetKeyword(eKeyword);
		*ppcCurrent = mpcTokens->CreateKeyword(&sPos, pcKeyword);
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
	STextPosition						sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetEnumeratorSequence<CCJavaTokenAmbiguousDefinition>(mpcDefinitions->GetAmbiguous(), (size*)&eAmbiguous, false);
	if (tResult == TRITRUE)
	{
		pcAmbiguous = mpcDefinitions->GetAmbiguous(eAmbiguous);
		*ppcCurrent = mpcTokens->CreateAmbiguous(&sPos, pcAmbiguous);
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
	STextPosition					sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetEnumeratorSequence<CJavaTokenOperatorDefinition>(mpcDefinitions->GetOperators(), (size*)&eOperator, false);
	if (tResult == TRITRUE)
	{
		pcOperator = mpcDefinitions->GetOperator(eOperator);
		*ppcCurrent = mpcTokens->CreateOperator(&sPos, pcOperator);
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
	STextPosition					sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetEnumeratorSequence<CJavaTokenSeparatorDefinition>(mpcDefinitions->GetSeparators(), (size*)&eSeparator, false);
	if (tResult == TRITRUE)
	{
		pcSeparator = mpcDefinitions->GetSeparator(eSeparator);
		*ppcCurrent = mpcTokens->CreateSeparator(&sPos, pcSeparator);
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
TRISTATE CJavaTokenParser::ParseScope(CJavaToken** ppcCurrent)
{
	TRISTATE					tResult;
	EJavaTokenScope				eScope;
	CJavaTokenScopeDefinition*	pcScope;
	STextPosition				sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetEnumeratorSequence<CJavaTokenScopeDefinition>(mpcDefinitions->GetScopes(), (size*)&eScope, false);
	if (tResult == TRITRUE)
	{
		pcScope = mpcDefinitions->GetScope(eScope);
		*ppcCurrent = mpcTokens->CreateScope(&sPos, pcScope);
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
	TRISTATE		tResult;
	STextPosition	sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetExactIdentifier("true", false);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mpcTokens->CreateBoolean(&sPos, true);
		return TRITRUE;;
	}
	else if (tResult == TRIERROR)
	{
		return TRIERROR;
	}

	tResult = mcParser.GetExactIdentifier("false", false);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mpcTokens->CreateBoolean(&sPos, false);
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
	TRISTATE		tResult;
	char			szText[4 KB];
	size			iLength;
	STextPosition	sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetIdentifier(szText, &iLength, false, false);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mpcTokens->CreateIdentifier(&sPos, szText, iLength);
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
	size			iLength;
	STextPosition	sPos;

	mcParser.PushPosition();
	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetExactCharacter('@', false);
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetIdentifier(szText, &iLength, false, false);
		if (tResult == TRITRUE)
		{
			*ppcCurrent = mpcTokens->CreateAnnotation(&sPos, szText, iLength);
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
	uint64			ulli;
	uint16			iBase;
	uint16			iNumDigits;
	TRISTATE		tResult;
	uint16			iSuffix;
	STextPosition	sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetIntegerLiteral(&ulli, INTEGER_PREFIX_ALL, &iBase, INTEGER_SUFFIX_JAVA, &iSuffix, NUMBER_SEPARATOR_UNDERSCORE, &iNumDigits, false);
	if (tResult == TRITRUE)
	{
		if (iSuffix == INTEGER_SUFFIX_L)
		{
			*ppcCurrent = mpcTokens->CreateInteger(&sPos, (int64)ulli);
			return TRITRUE;;
		}
		else if (iSuffix == INTEGER_SUFFIX_NONE)
		{
			*ppcCurrent = mpcTokens->CreateInteger(&sPos, (int32)ulli);
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
	float96			ldf;
	uint16			iBase;
	uint16			iNumWholeDigits;
	uint16			iNumDecinalDigits;
	uint16			iNumExponentDigits;
	TRISTATE		tResult;
	uint16			iSuffix;
	uint16			iExponent;
	STextPosition	sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetFloatLiteral(&ldf, FLOAT_PREFIX_ALL, &iBase, FLOAT_SUFFIX_JAVA, &iSuffix, FLOAT_EXPONENT_ALL, &iExponent, NUMBER_SEPARATOR_UNDERSCORE, &iNumWholeDigits, &iNumDecinalDigits, &iNumExponentDigits, false);
	if (tResult == TRITRUE)
	{
		if ((iSuffix == FLOAT_SUFFIX_D) || (iSuffix == FLOAT_SUFFIX_NONE))
		{
			*ppcCurrent = mpcTokens->CreateFloat(&sPos, (float64)ldf);
			return TRITRUE;;
		}
		else if (iSuffix == FLOAT_SUFFIX_F)
		{
			*ppcCurrent = mpcTokens->CreateFloat(&sPos, (float32)ldf);
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
	TRISTATE		tResult;
	uint16			iWidth;
	uint16			c;
	STextPosition	sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetCharacterLiteral(&c, true, &iWidth, false);
	if (tResult == TRITRUE)
	{
		if (iWidth == 1)
		{
			*ppcCurrent = mpcTokens->CreateCharacter(&sPos, (char)c);
			return TRITRUE;;
		}
		else if (iWidth = 2)
		{
			*ppcCurrent = mpcTokens->CreateCharacter(&sPos, (char16)c);
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
	TRISTATE		tResult;
	uint16			iWidth;
	char			sz[4 KB];
	size			iLength;
	STextPosition	sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetStringLiteral(&sz, 4 KB, true, &iLength, &iWidth, false);
	if (tResult == TRITRUE)
	{
		if (iWidth == 1)
		{
			*ppcCurrent = mpcTokens->CreateString(&sPos, sz, iLength);
			return TRITRUE;;
		}
		else if (iWidth == 2)
		{
			*ppcCurrent = mpcTokens->CreateString(&sPos, (char16*)sz, iLength);
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
	TRISTATE		tResult;
	STextPosition	sPos;

	mcParser.GetPosition(&sPos);
	tResult = mcParser.GetExactIdentifier("null", false);
	if (tResult == TRITRUE)
	{
		*ppcCurrent = mpcTokens->CreateNull(mcParser.GetPosition(&sPos));
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

