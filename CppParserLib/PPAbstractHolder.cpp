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
#include "PPAbstractHolder.h"
#include "TokenHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPAbstractHolder::Init(int iLine, int iColumn)
{
	Set(iLine, iColumn);
	mcTokens.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPAbstractHolder::Kill(void)
{
	mcTokens.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPAbstractHolder::IsAbstractHolder(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPAbstractHolder::Print(CChars* psz)
{
	if (mcTokens.mcArray.IsNotEmpty())
	{
		mcTokens.Print(psz);
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPAbstractHolder::Copy(CPPAbstractHolder* pcCast, CMemoryStackExtended* pcStack)
{
	int			i;
	CPPToken*	pcToken;

	Init(pcCast->miLine, pcCast->miColumn);

	for (i = 0; i < pcCast->mcTokens.mcArray.NumElements(); i++)
	{
		pcToken = DuplicatePPToken(*pcCast->mcTokens.mcArray.Get(i), pcStack);
		mcTokens.Add(pcToken);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPPAbstractHolder::TokenLength(void)
{
	return mcTokens.mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPPAbstractHolder::Get(int iIndex)
{
	return *mcTokens.mcArray.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPAbstractHolder::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPAbstractHolder::SavageDump(void)
{
	CChars	sz;

	sz.Init();
	mcTokens.SavageAppend(&sz, 0);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPAbstractHolder::Equals(CPPToken* pcOther)
{
	CPPAbstractHolder*	pcCast;	

	if (pcOther->IsAbstractHolder())
	{
		pcCast = (CPPAbstractHolder*)pcOther;

		return mcTokens.Equals(&pcCast->mcTokens);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPAbstractHolder::IsEmpty(void)
{
	int			i;
	CPPToken*	pcToken;

	if (mcTokens.mcArray.NumElements() == 0)
	{
		return TRUE;
	}

	for (i = 0; i < mcTokens.mcArray.NumElements(); i++)
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
CPPTokenHolder* CPPAbstractHolder::GetTokens(void)
{
	return &mcTokens;
}

