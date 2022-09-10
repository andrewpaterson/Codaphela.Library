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
#include "PPTokenList.h"
#include "PPTokenListHolder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenList::Init(void)
{
	mcArray.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenList::Kill(void)
{
	CPPToken**	ppcToken;
	CPPToken*	pcToken;
	int			i;
	int			iNumTokens;

	iNumTokens = mcArray.NumElements();
	for (i = 0; i < iNumTokens; i++)
	{
		ppcToken = mcArray.Get(i);
		if (ppcToken)
		{
			pcToken = *ppcToken;
			pcToken->Unuse();
		}
	}
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPPTokenList::Add(CPPToken* pcToken)
{
	pcToken->Use();
	mcArray.Add(&pcToken);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CPPTokenList::Print(CChars* psz, bool bShowFileAndLine)
{
	CPPToken**	ppcToken;
	CPPToken*	pcToken;
	int			i;
	int			iNumTokens;

	iNumTokens = mcArray.NumElements();
	for (i = 0; i < iNumTokens; i++)
	{
		ppcToken = mcArray.Get(i);
		if (ppcToken)
		{
			pcToken = *mcArray.Get(i);

			if (bShowFileAndLine)
			{
				CChars		sz;

				if (pcToken->NeedsNewLine())
				{
					psz->Append("/* ");
					psz->LeftAlign(pcToken->ShortFileName(), ' ', 20);

					sz.Init();
					sz.Append(pcToken->Line() + 1);
					psz->RightAlign(sz, ' ', 4);
					sz.Kill();

					psz->Append(" */ ");
				}
			}

			pcToken->Print(psz);
			if (pcToken->NeedsNewLine())
			{
				psz->AppendNewLine();
			}
		}
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPPTokenList::Equals(CPPTokenList* pcOther)
{
	int					i;
	CPPToken*			pcThisToken;
	CPPToken*			pcOtherToken;
	bool				bAnyFailed;

	if (mcArray.NumElements() == pcOther->mcArray.NumElements())
	{
		bAnyFailed = false;
		for (i = 0; i < mcArray.NumElements(); i++)
		{
			pcThisToken = *mcArray.Get(i);
			pcOtherToken = *pcOther->mcArray.Get(i);

			if (!pcThisToken->Equals(pcOtherToken))
			{
				bAnyFailed = true;
				break;
			}
		}
		if (!bAnyFailed)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPPTokenList::SavageAppend(CChars* psz, int iDepth)
{
	CPPToken*			pcToken;
	int					i;
	CPPTokenListHolder*	pcHolder;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcToken = *mcArray.Get(i);
		if (pcToken->IsTokenListHolder())
		{
			pcHolder = (CPPTokenListHolder*)pcToken;
			pcHolder->GetTokenList()->SavageAppend(psz, iDepth+1);
		}
		else
		{
			psz->Append(iDepth);
			psz->Append(": ");
			psz->Append(' ', iDepth*2);
			pcToken->Print(psz);
			psz->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPPTokenList::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPPTokenList::Get(int iTokenIndex)
{
	CPPToken**	ppcToken;

	ppcToken = mcArray.SafeGet(iTokenIndex);
	if (ppcToken)
	{
		return *ppcToken;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CPPTokenList::NumTokens(void)
{
	return mcArray.NumElements();
}

