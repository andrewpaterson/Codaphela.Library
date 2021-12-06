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
#include "TokenHelper.h"
#include "PPTokenMemory.h"
#include "PPTokenListHolder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenListHolder::Init(int iLine, int iColumn)
{
	Set(iLine, iColumn);
	mcTokens.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenListHolder::Kill(void)
{
	mcTokens.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPTokenListHolder::IsTokenListHolder(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPTokenListHolder::Print(CChars* psz)
{
	if (mcTokens.NumTokens() != 0)
	{
		mcTokens.Print(psz);
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenListHolder::Copy(CPPTokenListHolder* pcCast, CPPTokenMemory* pcTokens)
{
	int			i;
	CPPToken*	pcToken;
	int			iNumTokens;

	Init(pcCast->miLine, pcCast->miColumn);

	iNumTokens = pcCast->mcTokens.NumTokens();
	for (i = 0; i < iNumTokens; i++)
	{
		pcToken = DuplicatePPToken(pcCast->Get(i), pcTokens);
		mcTokens.Add(pcToken);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPPTokenListHolder::TokenLength(void)
{
	return mcTokens.NumTokens();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPPTokenListHolder::Get(int iIndex)
{
	return mcTokens.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenListHolder::SavageDump(void)
{
	CChars	sz;

	sz.Init();
	mcTokens.SavageAppend(&sz, 0);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPTokenListHolder::Equals(CPPToken* pcOther)
{
	CPPTokenListHolder*	pcCast;	

	if (pcOther->IsTokenListHolder())
	{
		pcCast = (CPPTokenListHolder*)pcOther;

		return mcTokens.Equals(&pcCast->mcTokens);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPTokenListHolder::IsEmpty(void)
{
	int			i;
	CPPToken*	pcToken;
	int			iNumTokens;

	iNumTokens = mcTokens.NumTokens();
	if (iNumTokens == 0)
	{
		return TRUE;
	}

	for (i = 0; i < iNumTokens; i++)
	{
		pcToken = Get(i);
		if (!pcToken->IsEmpty())
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPTokenList* CPPTokenListHolder::GetTokenList(void)
{
	return &mcTokens;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenListHolder::Fake(CPPTokenList* pcTokens)
{
	Set(-1, -1);
	memcpy(&mcTokens, pcTokens, sizeof(CPPTokenList));
}

