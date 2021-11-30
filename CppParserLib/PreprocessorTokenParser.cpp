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
#include <ctype.h>
#include <math.h>
#include "PreprocessorTokenParser.h"
#include "PPText.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SPPHolderMark::Init(CPPAbstractHolder* pcHolder, CPPToken* pcCurrentToken, int iTokenIndex)
{
	this->pcHolder = pcHolder;
	this->pcCurrentToken = pcCurrentToken;
	this->iTokenIndex = iTokenIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SPPHolderMark::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SPPTPPosition::Init(CArrayPPHolderMark* pacPPHolders)
{
	acPPHolders.Init();
	acPPHolders.Copy(pacPPHolders);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SPPTPPosition::Kill(void)
{
	acPPHolders.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::Init(void)
{
	macPPHolderMark.Init();
	mpsCurrent = NULL;
	macStack.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::Init(CPPAbstractHolder* pcLine)
{
	macPPHolderMark.Init();
	mpsCurrent = NULL;
	MarkDown(pcLine);
	macStack.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::Kill(void)
{
	int				i;
	SPPTPPosition*	psPos;
	SPPHolderMark*	psMark;

	for (i = 0; i < macPPHolderMark.NumElements(); i++)
	{
		psMark = macPPHolderMark.Get(i);
		psMark->Kill();
	}
	macPPHolderMark.Kill();

	for (i = 0; i < macStack.NumElements(); i++)
	{
		psPos = macStack.Get(i);
		psPos->Kill();
	}
	macStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::Bump(void)
{
	CPPAbstractHolder*		pcHolder;

	if (mpsCurrent->pcCurrentToken != NULL)
	{
		if (mpsCurrent->pcCurrentToken->IsAbstractHolder())
		{
			pcHolder = (CPPAbstractHolder*)mpsCurrent->pcCurrentToken;
			MarkDown(pcHolder);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::MarkDown(CPPAbstractHolder* pcHolder)
{
	CPPToken**		ppcToken;

	mpsCurrent = macPPHolderMark.Add();
	mpsCurrent->Init(pcHolder, NULL, 0);

	ppcToken = pcHolder->GetTokens()->mcArray.SafeGet(0);
	if (ppcToken)
	{
		AssignCurrent(*ppcToken, 0);
		Bump();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::MarkUp(void)
{
	SPPHolderMark*	psMark;

	if (macPPHolderMark.NumElements() > 1)
	{
		psMark = macPPHolderMark.Tail();
		psMark->Kill();
		macPPHolderMark.RemoveTail();

		mpsCurrent = macPPHolderMark.Tail();
		NextToken();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetIdentifier(CExternalString* pcIdentifier)
{
	CPPText*	pcText;

	if (!mpsCurrent->pcCurrentToken)
	{
		return FALSE;
	}

	SkipWhiteSpace();

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_Identifier)
		{
			pcIdentifier->msz = pcText->mcText.msz;
			pcIdentifier->miLen = pcText->mcText.miLen;
			NextToken();
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetNumber(CExternalString* pcIdentifier)
{
	CPPText*	pcText;

	if (!mpsCurrent->pcCurrentToken)
	{
		return FALSE;
	}

	SkipWhiteSpace();

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_Number)
		{
			pcIdentifier->msz = pcText->mcText.msz;
			pcIdentifier->miLen = pcText->mcText.miLen;
			NextToken();
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetExactDecorator(char c, BOOL bSkipWhiteSpace)
{
	CPPText*	pcText;

	if (!mpsCurrent->pcCurrentToken)
	{
		return FALSE;
	}

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
		if (!mpsCurrent->pcCurrentToken)
		{
			return FALSE;
		}
	}

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_Decorator)
		{
			if (pcText->mcText.msz[0] == c)
			{
				//Remember that it's guaranteed that a decorator is one char long.
				NextToken();
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetExactDecorator(char* sz, BOOL bSkipWhiteSpace)
{
	CPPText*	pcText;
	int			iExpectedLength;
	int			iTokenLength;
	int			i;
	char		c;
	int			iDone;

	if (!mpsCurrent->pcCurrentToken)
	{
		return FALSE;
	}

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
		if (mpsCurrent->pcCurrentToken == NULL)
		{
			PassPosition();
			return FALSE;
		}
	}

	iDone = 0;
	iExpectedLength = strlen(sz);
	while (iDone < iExpectedLength)
	{
		if (mpsCurrent->pcCurrentToken->IsText())
		{
			pcText = (CPPText*)mpsCurrent->pcCurrentToken;
			if (pcText->meType == PPT_Decorator)
			{
				iTokenLength = pcText->Length();
				for (i = 0; i < iExpectedLength && i < iTokenLength; i++)
				{
					c = sz[i];
					if (pcText->mcText.msz[0] != c)
					{
						PopPosition();
						return FALSE;
					}
				}
				iDone += iTokenLength;
				NextToken();
			}
			else
			{
				PopPosition();
				return FALSE;
			}
		}
		else
		{
			PopPosition();
			return FALSE;
		}
	}
	PassPosition();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetExactIdentifier(char* szIdentifier, BOOL bCaseSensitive, BOOL bSkipWhiteSpace)
{
	CPPText*	pcText;
	int			i;
	int			iLength;
	char		c1;
	char		c2;
	char		c;

	if (!mpsCurrent->pcCurrentToken)
	{
		return FALSE;
	}

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
		if (!mpsCurrent->pcCurrentToken)
		{
			PassPosition();
			return FALSE;
		}
	}

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		iLength = (int)strlen(szIdentifier);
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_Identifier)
		{
			if (pcText->mcText.miLen != iLength)
			{
				PopPosition();
				return FALSE;
			}

			if (bCaseSensitive)
			{
				for (i = 0; i < iLength; i++)
				{
					c = szIdentifier[i];
					if (pcText->mcText.msz[i] != c)
					{
						PopPosition();
						return FALSE;
					}
				}
			}
			else
			{
				for (i = 0; i < iLength; i++)
				{
					c1 = toupper(pcText->mcText.msz[i]);
					c2 = toupper(szIdentifier[i]);
					if (c1 != c2)
					{
						PopPosition();
						return FALSE;
					}
				}
			}

			NextToken();
			PassPosition();
			return TRUE;
		}
	}
	PopPosition();
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetQuotedCharacterSequence(char cOpen, char cClose, CExternalString* pcSequence)
{
	BOOL		bResult;
	char*		szStart;
	CPPText*	pcText;
	int			i;

	PushPosition();

	SkipWhiteSpace();
	bResult = GetExactDecorator(cOpen);
	pcSequence->miLen = 0;
	szStart = NULL;
	if (bResult)
	{
		for (i = 0;; i++)
		{
			//Caution, this is dangerous because it may be a CPPTextWithSource.  Do not talk directly to the classes members.
			if (mpsCurrent->pcCurrentToken->IsText())
			{
				pcText = (CPPText*)mpsCurrent->pcCurrentToken;
				if (i == 0)
				{
					szStart = pcText->mcText.msz;
				}
				pcSequence->miLen += pcText->mcText.miLen;
				NextToken();
			}
			else
			{
				PopPosition();
				return FALSE;
			}

			bResult = GetExactDecorator(cClose, FALSE);
			if (bResult)
			{
				pcSequence->msz = szStart;
				//pcSequence->miLen is co-incidentally correct because the size of the decorator is always 1 and the end not inclusive must be moved on by 1.  Sweet.
				PassPosition();
				return TRUE;
			}
		}
	}

	PopPosition();
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetStringDoubleQuoted(CExternalString* pcString)
{
	CPPText*	pcText;

	if (!mpsCurrent->pcCurrentToken)
	{
		return FALSE;
	}

	SkipWhiteSpace();
	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_DoubleQuoted)
		{
			pcString->msz = pcText->mcText.msz+1;
			pcString->miLen = pcText->mcText.miLen-2;
			NextToken();
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetStringSingleQuoted(CExternalString* pcString, BOOL bSkipWhiteSpace)
{
	CPPText*	pcText;

	if (!mpsCurrent->pcCurrentToken)
	{
		return FALSE;
	}

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
		if (!mpsCurrent->pcCurrentToken)
		{
			return FALSE;
		}
	}

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_SingleQuoted)
		{
			pcString->msz = pcText->mcText.msz+1;
			pcString->miLen = pcText->mcText.miLen-2;
			NextToken();
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::SkipWhiteSpace(void)
{
	for (;;)
	{
		if ((!mpsCurrent->pcCurrentToken) || (!mpsCurrent->pcCurrentToken->IsWhiteSpace()))
		{
			return;
		}

		//In theory there should never be whitespace followed by white space.
		//In practice it's possible that the preprocessor could have inserted a holder with white space after white space.
		NextToken();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetFloat(long double* pf)
{
	CPPText*				pcText;
	char					c;
	uint64	ulliLeft;
	uint64	ulliRight;
	int						iSign;
	BOOL					bResult;
	long double				fLeft;
	long double				fRight;
	long double				fTemp;
	int						iNumDecimals;

	PushPosition();

	SkipWhiteSpace();
	iSign = 1;

	if (!mpsCurrent->pcCurrentToken)
	{
		return FALSE;
	}

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_Decorator)
		{
			c = pcText->mcText.msz[0];
			if ((c == '+') || (c == '-'))
			{
				if (c == '-')
				{
					iSign = -1;
				}
			}
			else
			{
				PopPosition();
				return FALSE;
			}

			NextToken();
		}
	}

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		bResult = GetDigits(&ulliLeft, &iNumDecimals);
		fLeft = ((long double)ulliLeft) * iSign;

		if (bResult)
		{
			bResult = GetExactDecorator('.', FALSE);
			if (bResult)
			{
				bResult = GetDigits(&ulliRight, &iNumDecimals);
				if (bResult)
				{
					fRight = (long double)ulliRight;
					fTemp = pow(10.0L, -iNumDecimals);
					fRight *= fTemp;

					if (fLeft >= 0)
					{
						*pf = fLeft + fRight;
					}
					else
					{
						*pf = fLeft - fRight;
					}
				}
				PassPosition();
				return TRUE;
			}
			else
			{
				PopPosition();
				return FALSE;
			}
		}
		else
		{
			PopPosition();
			return FALSE;
		}
	}
	else
	{
		PopPosition();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetInteger(uint64* pulli, BOOL bSkipWhiteSpace, BOOL bTestSign)
{	
	CPPText*				pcText;
	char					c;
	uint64	ulli;
	int						iSign;
	BOOL					bResult;
	int						iNumDecimals;

	PushPosition();

	if (bSkipWhiteSpace)
	{
		SkipWhiteSpace();
		if (!mpsCurrent->pcCurrentToken)
		{
			return FALSE;
		}
	}

	if (!mpsCurrent->pcCurrentToken)
	{
		return FALSE;
	}

	iSign = 1;
	if (bTestSign)
	{
		if (mpsCurrent->pcCurrentToken->IsText())
		{
			pcText = (CPPText*)mpsCurrent->pcCurrentToken;
			if (pcText->meType == PPT_Decorator)
			{
				c = pcText->mcText.msz[0];
				if ((c == '+') || (c == '-'))
				{
					if (c == '-')
					{
						iSign = -1;
					}
				}
				else
				{
					PopPosition();
					return FALSE;
				}

				NextToken();
			}
		}
	}

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		bResult = GetDigits(&ulli, &iNumDecimals);

		if (bResult)
		{
			bResult = GetExactDecorator('.', FALSE);
			if (!bResult)
			{
				*pulli = ulli;
				PassPosition();
				return TRUE;
			}
			else
			{
				PopPosition();
				return FALSE;
			}
		}
		else
		{
			PopPosition();
			return FALSE;
		}
	}
	else
	{
		PopPosition();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetDigits(uint64* pulli, int* piNumDecimals)
{
	uint64	ulli;
	int						i;
	CPPText*				pcText;
	int						iTemp;

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;

		if (pcText->meType == PPT_Number)
		{
			ulli = 0;
			for (i = 0; i < pcText->mcText.miLen; i++)
			{
				iTemp = pcText->mcText.msz[i] - '0';
				ulli *= 10;
				ulli += iTemp;
			}

			*pulli = ulli;
			*piNumDecimals = pcText->mcText.miLen;
			NextToken();
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetHexadecimal(uint64* pulli, int* piNumDigits)
{
	uint64	iNum;
	BOOL					bReturn;
	int						i;
	CPPText*				pcText;
	int						j;
	char					c;

	PushPosition();
	SkipWhiteSpace();

	bReturn = GetExactIdentifier("0", TRUE, FALSE);
	if (!bReturn)
	{
		PopPosition();
		return FALSE;
	}

	bReturn = GetExactIdentifier("X", FALSE, FALSE);
	if (!bReturn)
	{
		PopPosition();
		return FALSE;
	}

	*pulli = 0;
	iNum = 0;

	for (i = 0;; i++)
	{
		if (!mpsCurrent->pcCurrentToken)
		{
			if (i == 0)
			{
				PopPosition();
				return FALSE;
			}
			else
			{
				*pulli = iNum;
				PassPosition();
				return TRUE;
			}
		}

		if (mpsCurrent->pcCurrentToken->IsText())
		{
			pcText = (CPPText*)mpsCurrent->pcCurrentToken;
			if ((pcText->meType == PPT_Identifier) || (pcText->meType == PPT_Number))
			{
				for (j = 0; j < pcText->mcText.miLen; j++)
				{
					iNum *= 16;

					c = pcText->mcText.msz[i];
					if ((c >= '0') && (c <= '9'))
					{
						iNum += (c - '0');
					}
					else if ((c >= 'a') && (c <= 'f'))
					{
						iNum += (c - 'a' + 10);
					}
					else if ((c >= 'A') && (c <= 'F'))
					{
						iNum += (c - 'A' + 10);
					}
					else
					{
						PopPosition();
						return FALSE;
					}
				}
				NextToken();
			}
			else
			{
				if (i == 0)
				{
					PopPosition();
					return FALSE;
				}
				else
				{
					*pulli = iNum;
					PassPosition();
					return TRUE;
				}
			}
		}
		else
		{
			PopPosition();
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::GetOctal(uint64* pulli, int* piNumDigits)
{
	uint64	iNum;
	BOOL					bReturn;
	int						i;
	CExternalString			cString;
	char					c;

	PushPosition();
	SkipWhiteSpace();

	bReturn = GetNumber(&cString);
	if (!bReturn)
	{
		PopPosition();
		return FALSE;
	}

	*pulli = 0;
	iNum = 0;

	c = cString.msz[0];
	if (c != '0')
	{
		PopPosition();
		return FALSE;
	}

	for (i = 1; i < cString.miLen; i++)
	{
		iNum *= 8;

		c = cString.msz[i];
		if (c >= '8')
		{
			PopPosition();
			return FALSE;
		}
		iNum += (c - '0');
	}

	*pulli = iNum;
	PassPosition();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::NextToken(void)
{
	CPPAbstractHolder*	pcHolder;
	int					iIndex;

	if (mpsCurrent->pcCurrentToken)
	{
		pcHolder = mpsCurrent->pcHolder;
		iIndex = mpsCurrent->iTokenIndex+1;
		if (iIndex < pcHolder->TokenLength())
		{
			AssignCurrent(pcHolder->Get(iIndex), iIndex);
			Bump();
		}
		else
		{
			AssignCurrent(NULL, -1);
			MarkUp();
		}
		return TRUE;
	}
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::NextToken(int iNumToSkip)
{
	int		i;

	for (i = 0; i < iNumToSkip; i++)
	{
		NextToken();
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::PushPosition(void)
{
	SPPTPPosition*	psPos;

	psPos = macStack.Add();
	psPos->Init(&macPPHolderMark);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::PopPosition(void)
{
	SPPTPPosition*	psPos;

	psPos = macStack.Tail();
	macPPHolderMark.Copy(&psPos->acPPHolders);

	mpsCurrent = macPPHolderMark.Tail();
	psPos->Kill();
	macStack.Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::PassPosition(void)
{
	SPPTPPosition*	psPos;

	psPos = macStack.Tail();
	psPos->Kill();
	macStack.Pop();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPreprocessorTokenParser::Line(void)
{
	if (HasToken())
	{
		return mpsCurrent->pcCurrentToken->Line();
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPreprocessorTokenParser::Column(void)
{
	if (HasToken())
	{
		return mpsCurrent->pcCurrentToken->Column();
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::AppendRemaining(CChars* psz)
{
	SPPHolderMark*	psMark;
	CPPAbstractHolder*		pcHolder;

	psMark = macPPHolderMark.Get(0);
	pcHolder = psMark->pcHolder;
	AppendRemaining(psz, pcHolder, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::AppendRemaining(CChars* psz, CPPAbstractHolder* pcHolder, BOOL bAppending)
{
	int			i;
	CPPToken*	pcToken;
	CPPAbstractHolder*	pcChild;

	for (i = 0; i < pcHolder->TokenLength(); i++)
	{
		pcToken = pcHolder->Get(i);
		if (pcToken->IsAbstractHolder())
		{
			pcChild = (CPPAbstractHolder*)pcToken;
			if (AppendRemaining(psz, pcChild, bAppending))
			{
				bAppending = TRUE;
			}
		}
		else
		{
			if (mpsCurrent->pcCurrentToken == pcToken)
			{
				bAppending = TRUE;
			}

			if (bAppending)
			{
				pcToken->Print(psz);
			}
		}
	}
	return bAppending;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::HasToken(void)
{
	return mpsCurrent->pcCurrentToken != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::HasTokens(void)
{
	SPPHolderMark*			psMark;
	CPPAbstractHolder*		pcHolder;

	psMark = macPPHolderMark.Get(0);
	pcHolder = psMark->pcHolder;
	return !pcHolder->IsEmpty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessorTokenParser::GetToken(void)
{
	return mpsCurrent->pcCurrentToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorTokenParser::ToDecortatorString(char* szDest, int iMaxLen, int* piLen)
{
	CPPText*	pcText;
	int			i;

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_Decorator)
		{
			PushPosition();
			for (i = 0; i < iMaxLen; i++)
			{
				szDest[i] = pcText->mcText.msz[0];
				NextToken();

				if (!mpsCurrent->pcCurrentToken)
				{
					break;
				}

				if (mpsCurrent->pcCurrentToken->IsText())
				{
					pcText = (CPPText*)mpsCurrent->pcCurrentToken;
					if (pcText->meType == PPT_Decorator)
					{
						continue;
					}
				}
				break;
			}

			PopPosition();
			*piLen = i+1;
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::AssignCurrent(CPPToken* pcToken, int iTokenIndex)
{
	mpsCurrent->pcCurrentToken = pcToken;
	mpsCurrent->iTokenIndex = iTokenIndex;

	int						iIndex;
	CPPToken*				pcTest;
	int						i;
	CPPAbstractHolder*		pcHolder;

	if ((pcToken != NULL) && (pcToken->IsAbstractHolder()))
	{
		return;
	}

	pcHolder = mpsCurrent->pcHolder;
	iIndex = -1;
	for (i = 0; i < pcHolder->TokenLength(); i++)
	{
		pcTest = pcHolder->Get(i);
		if (pcTest == pcToken)
		{
			iIndex = i;
			break;
		}
	}

	if (iIndex != iTokenIndex)
	{
		CChars::Dump("Oh fuck.\n");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::Dump(void)
{
	CChars	sz;

	sz.Init();
	AppendRemaining(&sz);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::DumpAll(void)
{
	CChars			sz;
	SPPHolderMark*	psMark;

	sz.Init();
	psMark = macPPHolderMark.Get(0);
	psMark->pcHolder->Print(&sz);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

