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
#include "PPTokenHolder.h"
#include "PPAbstractHolder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenHolder::Init(void)
{
	mcArray.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenHolder::Kill(void)
{
	int			i;
	CPPToken*	pcPPToken;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcPPToken = *(mcArray.Get(i));
		pcPPToken->Kill();
	}
	mcArray.Kill();
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CPPTokenHolder::Print(CChars* psz)
{
	CPPToken*	pcToken;
	int			i;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcToken = *mcArray.Get(i);
		pcToken->Print(psz);
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPPTokenHolder::Add(CPPToken* pcToken)
{
	mcArray.Add(&pcToken);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPPTokenHolder::Equals(CPPTokenHolder* pcOther)
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
void CPPTokenHolder::SavageAppend(CChars* psz, int iDepth)
{
	CPPToken*			pcToken;
	int					i;
	CPPAbstractHolder*	pcHolder;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcToken = *mcArray.Get(i);
		if (pcToken->IsAbstractHolder())
		{
			pcHolder = (CPPAbstractHolder*)pcToken;
			pcHolder->GetTokens()->SavageAppend(psz, iDepth+1);
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
void CPPTokenHolder::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.Dump();
	sz.Kill();
}

