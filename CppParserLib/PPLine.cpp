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
#include "PPLine.h"
#include "TokenHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPLine::Init(int iLine, int iColumn, char* szFileName)
{
	CPPTokenListHolder::Init(iLine, iColumn, szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPLine::Kill(void)
{
	CPPTokenListHolder::Kill();
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
void CPPLine::Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens)
{
	CPPLine*	pcCast;
	CPPToken*	pcToken;
	int			i;
	int			iNumTokens;

	if (pcSource->IsLine())
	{
		pcCast = (CPPLine*)pcSource;
		Init(pcCast->miLine, pcCast->miColumn, pcCast->mszFileName);

		iNumTokens = pcCast->mcTokens.NumTokens();
		for (i = 0; i < iNumTokens; i++)
		{
			pcToken = DuplicatePPToken(pcCast->mcTokens.Get(i), pcTokens);
			mcTokens.Add(pcToken);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPLine::Equals(CPPToken* pcOther)
{
	if (pcOther->IsLine())
	{
		return CPPTokenListHolder::Equals(pcOther);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPLine::NeedsNewLine(void)
{
	return TRUE;
}

