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
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CPPTokenList::Print(CChars* psz)
{
	CPPToken*	pcToken;
	int			i;
	int			iNumTokens;

	iNumTokens = mcArray.NumElements();
	for (i = 0; i < iNumTokens; i++)
	{
		pcToken = *mcArray.Get(i);
		pcToken->Print(psz);
		if (pcToken->NeedsNewLine())
		{
			psz->AppendNewLine();
		}
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPPTokenList::Add(CPPToken* pcToken)
{
	mcArray.Add(&pcToken);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPPTokenList::Equals(CPPTokenList* pcOther)
{
	int					i;
	CPPToken*			pcThisToken;
	CPPToken*			pcOtherToken;
	BOOL				bAnyFailed;

	if (mcArray.NumElements() == pcOther->mcArray.NumElements())
	{
		bAnyFailed = FALSE;
		for (i = 0; i < mcArray.NumElements(); i++)
		{
			pcThisToken = *mcArray.Get(i);
			pcOtherToken = *pcOther->mcArray.Get(i);

			if (!pcThisToken->Equals(pcOtherToken))
			{
				bAnyFailed = TRUE;
				break;
			}
		}
		if (!bAnyFailed)
		{
			return TRUE;
		}
	}
	return FALSE;
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

