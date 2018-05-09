/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "PPLine.h"
#include "TokenHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPLine::Init(int iLine, int iColumn)
{
	CPPAbstractHolder::Init(iLine, iColumn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPLine::Kill(void)
{
	CPPAbstractHolder::Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPLine::Fake(CPPTokenHolder* pcTokens)
{
	Set(-1, -1);
	memcpy(&mcTokens, pcTokens, sizeof(CPPTokenHolder));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPLine::IsLine(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPLine::Append(CChars* psz)
{
	if (mcTokens.mcArray.NumElements() > 0)
	{
		CPPAbstractHolder::Append(psz);
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPLine::AppendNoNewline(CChars* psz)
{
	if (mcTokens.mcArray.NumElements() > 0)
	{
		CPPAbstractHolder::Append(psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPLine::Copy(CPPToken* pcSource, CMemoryStackExtended* pcStack)
{
	CPPLine*	pcCast;
	CPPToken*	pcToken;
	int			i;

	if (pcSource->IsLine())
	{
		pcCast = (CPPLine*)pcSource;
		Init(pcCast->miLine, pcCast->miColumn);

		for (i = 0; i < pcCast->mcTokens.mcArray.NumElements(); i++)
		{
			pcToken = DuplicatePPToken(*pcCast->mcTokens.mcArray.Get(i), pcStack);
			mcTokens.Add(&pcToken);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPLine::Dump(void)
{
	CChars	sz;

	sz.Init();
	Append(&sz);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPLine::Equals(CPPToken* pcOther)
{
	if (pcOther->IsLine())
	{
		return CPPAbstractHolder::Equals(pcOther);
	}
	return FALSE;
}
