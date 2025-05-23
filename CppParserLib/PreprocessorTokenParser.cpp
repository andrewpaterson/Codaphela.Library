/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
void SPPHolderMark::Init(CPPTokenListHolder* pcHolder, CPPToken* pcCurrentToken, int iTokenIndex)
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
void CPreprocessorTokenParser::Init(CPPTokenListHolder* pcLine)
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
	size			i;
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
	CPPTokenListHolder*		pcHolder;

	if (mpsCurrent->pcCurrentToken != NULL)
	{
		if (mpsCurrent->pcCurrentToken->IsTokenListHolder())
		{
			pcHolder = (CPPTokenListHolder*)mpsCurrent->pcCurrentToken;
			MarkDown(pcHolder);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::MarkDown(CPPTokenListHolder* pcHolder)
{
	CPPToken*		pcToken;

	mpsCurrent = macPPHolderMark.Add();
	mpsCurrent->Init(pcHolder, NULL, 0);

	pcToken = pcHolder->GetTokenList()->Get(0);
	if (pcToken)
	{
		AssignCurrent(pcToken, 0);
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
bool CPreprocessorTokenParser::GetIdentifier(CExternalString* pcIdentifier)
{
	CPPText*	pcText;

	if (!mpsCurrent->pcCurrentToken)
	{
		return false;
	}

	SkipWhitespace();

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_Identifier)
		{
			pcIdentifier->msz = pcText->mcText.msz;
			pcIdentifier->miLen = pcText->mcText.miLen;
			NextToken();
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetNumber(CExternalString* pcIdentifier)
{
	CPPText*	pcText;

	if (!mpsCurrent->pcCurrentToken)
	{
		return false;
	}

	SkipWhitespace();

	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_Number)
		{
			pcIdentifier->msz = pcText->mcText.msz;
			pcIdentifier->miLen = pcText->mcText.miLen;
			NextToken();
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetExactDecorator(char c, bool bSkipWhitespace)
{
	CPPText*	pcText;

	if (!mpsCurrent->pcCurrentToken)
	{
		return false;
	}

	if (bSkipWhitespace)
	{
		SkipWhitespace();
		if (!mpsCurrent->pcCurrentToken)
		{
			return false;
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
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetExactDecorator(char* sz, bool bSkipWhitespace)
{
	CPPText*	pcText;
	int			iExpectedLength;
	int			iTokenLength;
	int			i;
	char		c;
	int			iDone;

	if (!mpsCurrent->pcCurrentToken)
	{
		return false;
	}

	PushPosition();

	if (bSkipWhitespace)
	{
		SkipWhitespace();
		if (mpsCurrent->pcCurrentToken == NULL)
		{
			PassPosition();
			return false;
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
						return false;
					}
				}
				iDone += iTokenLength;
				NextToken();
			}
			else
			{
				PopPosition();
				return false;
			}
		}
		else
		{
			PopPosition();
			return false;
		}
	}
	PassPosition();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetExactIdentifier(char* szIdentifier, bool bCaseSensitive, bool bSkipWhitespace)
{
	CPPText*	pcText;
	int			i;
	int			iLength;
	char		c1;
	char		c2;
	char		c;

	if (!mpsCurrent->pcCurrentToken)
	{
		return false;
	}

	PushPosition();

	if (bSkipWhitespace)
	{
		SkipWhitespace();
		if (!mpsCurrent->pcCurrentToken)
		{
			PassPosition();
			return false;
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
				return false;
			}

			if (bCaseSensitive)
			{
				for (i = 0; i < iLength; i++)
				{
					c = szIdentifier[i];
					if (pcText->mcText.msz[i] != c)
					{
						PopPosition();
						return false;
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
						return false;
					}
				}
			}

			NextToken();
			PassPosition();
			return true;
		}
	}
	PopPosition();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetQuotedCharacterSequence(char cOpen, char cClose, CExternalString* pcSequence)
{
	bool		bResult;
	char*		szStart;
	CPPText*	pcText;
	int			i;

	PushPosition();

	SkipWhitespace();
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
				return false;
			}

			bResult = GetExactDecorator(cClose, false);
			if (bResult)
			{
				pcSequence->msz = szStart;
				//pcSequence->miLen is co-incidentally correct because the size of the decorator is always 1 and the end not inclusive must be moved on by 1.  Sweet.
				PassPosition();
				return true;
			}
		}
	}

	PopPosition();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetStringDoubleQuoted(CExternalString* pcString)
{
	CPPText*	pcText;

	if (!mpsCurrent->pcCurrentToken)
	{
		return false;
	}

	SkipWhitespace();
	if (mpsCurrent->pcCurrentToken->IsText())
	{
		pcText = (CPPText*)mpsCurrent->pcCurrentToken;
		if (pcText->meType == PPT_DoubleQuoted)
		{
			pcString->msz = pcText->mcText.msz+1;
			pcString->miLen = pcText->mcText.miLen-2;
			NextToken();
			return true;
		}
		return false;
	}
	return false;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetStringSingleQuoted(CExternalString* pcString, bool bSkipWhitespace)
{
	CPPText*	pcText;

	if (!mpsCurrent->pcCurrentToken)
	{
		return false;
	}

	if (bSkipWhitespace)
	{
		SkipWhitespace();
		if (!mpsCurrent->pcCurrentToken)
		{
			return false;
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
			return true;
		}
		return false;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::SkipWhitespace(void)
{
	for (;;)
	{
		if ((!mpsCurrent->pcCurrentToken) || (!mpsCurrent->pcCurrentToken->IsWhitespace()))
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
bool CPreprocessorTokenParser::GetFloat(long double* pf)
{
	CPPText*		pcText;
	char			c;
	uint64			ulliLeft;
	uint64			ulliRight;
	int				iSign;
	bool			bResult;
	long double		fLeft;
	long double		fRight;
	long double		fTemp;
	int				iNumDecimals;

	PushPosition();

	SkipWhitespace();
	iSign = 1;

	if (!mpsCurrent->pcCurrentToken)
	{
		return false;
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
				return false;
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
			bResult = GetExactDecorator('.', false);
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
				return true;
			}
			else
			{
				PopPosition();
				return false;
			}
		}
		else
		{
			PopPosition();
			return false;
		}
	}
	else
	{
		PopPosition();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetInteger(uint64* pulli, bool bSkipWhitespace, bool bTestSign)
{	
	CPPText*	pcText;
	char		c;
	uint64		ulli;
	int			iSign;
	bool		bResult;
	int			iNumDecimals;

	PushPosition();

	if (bSkipWhitespace)
	{
		SkipWhitespace();
		if (!mpsCurrent->pcCurrentToken)
		{
			return false;
		}
	}

	if (!mpsCurrent->pcCurrentToken)
	{
		return false;
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
					return false;
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
			bResult = GetExactDecorator('.', false);
			if (!bResult)
			{
				*pulli = ulli;
				PassPosition();
				return true;
			}
			else
			{
				PopPosition();
				return false;
			}
		}
		else
		{
			PopPosition();
			return false;
		}
	}
	else
	{
		PopPosition();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetDigits(uint64* pulli, int* piNumDecimals)
{
	uint64		ulli;
	size		i;
	CPPText*	pcText;
	uint64		iTemp;

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
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetHexadecimal(uint64* pulli, int* piNumDigits)
{
	uint64	iNum;
	bool					bReturn;
	size					i;
	CPPText*				pcText;
	size					j;
	char					c;

	PushPosition();
	SkipWhitespace();

	bReturn = GetExactIdentifier("0", true, false);
	if (!bReturn)
	{
		PopPosition();
		return false;
	}

	bReturn = GetExactIdentifier("X", false, false);
	if (!bReturn)
	{
		PopPosition();
		return false;
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
				return false;
			}
			else
			{
				*pulli = iNum;
				PassPosition();
				return true;
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
						return false;
					}
				}
				NextToken();
			}
			else
			{
				if (i == 0)
				{
					PopPosition();
					return false;
				}
				else
				{
					*pulli = iNum;
					PassPosition();
					return true;
				}
			}
		}
		else
		{
			PopPosition();
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::GetOctal(uint64* pulli, int* piNumDigits)
{
	uint64				iNum;
	bool				bReturn;
	size				i;
	CExternalString		cString;
	char				c;

	PushPosition();
	SkipWhitespace();

	bReturn = GetNumber(&cString);
	if (!bReturn)
	{
		PopPosition();
		return false;
	}

	*pulli = 0;
	iNum = 0;

	c = cString.msz[0];
	if (c != '0')
	{
		PopPosition();
		return false;
	}

	for (i = 1; i < cString.miLen; i++)
	{
		iNum *= 8;

		c = cString.msz[i];
		if (c >= '8')
		{
			PopPosition();
			return false;
		}
		iNum += (c - '0');
	}

	*pulli = iNum;
	PassPosition();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::NextToken(void)
{
	CPPTokenListHolder*	pcHolder;
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
		return true;
	}
	return false;
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
	CPPTokenListHolder*		pcHolder;

	psMark = macPPHolderMark.Get(0);
	pcHolder = psMark->pcHolder;
	AppendRemaining(psz, pcHolder, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::AppendRemaining(CChars* psz, CPPTokenListHolder* pcHolder, bool bAppending)
{
	int			i;
	CPPToken*	pcToken;
	CPPTokenListHolder*	pcChild;

	for (i = 0; i < pcHolder->TokenLength(); i++)
	{
		pcToken = pcHolder->Get(i);
		if (pcToken->IsTokenListHolder())
		{
			pcChild = (CPPTokenListHolder*)pcToken;
			if (AppendRemaining(psz, pcChild, bAppending))
			{
				bAppending = true;
			}
		}
		else
		{
			if (mpsCurrent->pcCurrentToken == pcToken)
			{
				bAppending = true;
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
bool CPreprocessorTokenParser::HasToken(void)
{
	return mpsCurrent->pcCurrentToken != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorTokenParser::HasTokens(void)
{
	SPPHolderMark*			psMark;
	CPPTokenListHolder*		pcHolder;

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
bool CPreprocessorTokenParser::ToDecortatorString(char* szDest, int iMaxLen, int* piLen)
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
			return true;
		}
	}
	return false;
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
	CPPTokenListHolder*		pcHolder;

	if ((pcToken != NULL) && (pcToken->IsTokenListHolder()))
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

