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
	iExpectedLength = (int)strlen(sz);
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
	size		i;
	size		iLength;
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
		iLength = strlen(szIdentifier);
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
CPPToken* CPreprocessorTokenParser::GetToken(void)
{
	return mpsCurrent->pcCurrentToken;
}


//////////////////////////////////////////////////////////////////////////
//
// The current token is pcToken, at iTokenIndex in the current holder.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorTokenParser::AssignCurrent(CPPToken* pcToken, int iTokenIndex)
{
	mpsCurrent->pcCurrentToken = pcToken;
	mpsCurrent->iTokenIndex = iTokenIndex;
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

