/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Define.h"
#include "BaseLib/DebugOutput.h"
#include "BaseLib/EnumeratorVoid.h"
#include "TokenArray.h"
#include "TokenTree.h"
#include "Tokens.h"
#include "TokenDumper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::Init(CTokenTree* pcTree, SNodeElement* psNode)
{
	mcTokens.Init(16);
	mpsNode = psNode;
	mpcTree = pcTree;
	mbFinalised = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::Kill(void)
{
	mcTokens.Kill();
	mpsNode = NULL;
	mpcTree = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::Dump(void)
{
	DumpTokens(mcTokens.GetData(), mcTokens.NumElements(), &mpcTree->mcKeywords, &mpcTree->mcIdentifiers);
	if ((mpsNode) && (mpsNode->szName) && (strlen(mpsNode->szName) != 0))
	{
		EngineOutput("[");
		EngineOutput(mpsNode->szName);
		EngineOutput("]");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::AddKeywordText(char* sz)
{
	STokenKeywordText*	psToken;
	int					iID;

	iID = mpcTree->mcKeywords.Get(sz);

	psToken = NULL;
	psToken = (STokenKeywordText*)GrowBy(psToken->Size());
	psToken->Init(iID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::AddIdentifier(char* sz)
{
	STokenIdentifier*	psToken;
	int					iID;

	iID = mpcTree->mcIdentifiers.Get(sz);

	psToken = NULL;
	psToken = (STokenIdentifier*)GrowBy(psToken->Size());
	psToken->Init(iID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::AddKeywordChar(char c)
{
	STokenKeywordChar*	psToken;
	int					iSize;

	psToken = NULL;
	iSize = psToken->Size();
	psToken = (STokenKeywordChar*)GrowBy(iSize);
	memset(psToken, 0, iSize*4);
	psToken->Init(c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::AddInteger(int i)
{
	STokenInteger*	psToken;

	psToken = NULL;
	psToken = (STokenInteger*)GrowBy(psToken->Size());
	psToken->Init(i);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::AddReal(float f)
{
	STokenReal*		psToken;

	psToken = NULL;
	psToken = (STokenReal*)GrowBy(psToken->Size());
	psToken->Init(f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::AddUnknownText(char* sz)
{
	STokenUnknownText*	psToken;

	psToken = NULL;
	psToken = (STokenUnknownText*)GrowBy(psToken->Size(sz));
	psToken->Init(sz);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::AddUnknownChar(char c)
{
	STokenUnknownChar*	psToken;
	int					iSize;

	psToken = NULL;
	iSize = psToken->Size();
	psToken = (STokenUnknownChar*)GrowBy(iSize);
	memset(psToken, 0, iSize*4);
	psToken->Init(c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::AddNode(void)
{
	STokenSingle*	psToken;

	psToken = NULL;
	psToken = (STokenSingle*)GrowBy(psToken->Size());
	psToken->Init(ET_Node);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTokenArray::GetNodeIndex(int iNum)
{
	int*	pi;
	int		iCount;
	int		iIndex;

	pi = mcTokens.Get(0);
	iCount = 0;
	while(pi)
	{
		if (*pi == ET_Node)
		{
			if (iCount == iNum)
			{
				iIndex = mcTokens.GetIndex(pi);
				return iIndex;
			}
			iCount++;
		}
		pi = GetNextToken(pi);
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::RemoveNode(int iNum)
{
	int		iIndex;

	iIndex = GetNodeIndex(iNum);
	if (iIndex != -1)
	{
		mcTokens.RemoveAt(iIndex);
		return;
	}
	gcLogger.Error("Couldn't remove node.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokenArray::ReplaceNode(int iNum, CTokenArray* pcReplacement)
{
	int		iIndex;

	iIndex = GetNodeIndex(iNum);
	if (iIndex != -1)
	{
		mcTokens.RemoveAt(iIndex);
		mcTokens.InsertArrayAt(&pcReplacement->mcTokens, iIndex);
		return;
	}
	gcLogger.Error("Couldn't replace node.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int* CTokenArray::GrowBy(int iNum)
{
	int	iOldLength;

	iOldLength = mcTokens.GrowByNumElements(iNum);
	return mcTokens.Get(iOldLength);
}

