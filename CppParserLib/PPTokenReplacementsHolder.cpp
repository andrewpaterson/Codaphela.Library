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
#include "PPTokenReplacementsHolder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenReplacementsHolder::Init(int iLine, int iColumn, char* szFileName)
{
	CPPTokenListHolder::Init(iLine, iColumn, szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenReplacementsHolder::Kill(void)
{
	CPPTokenListHolder::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTokenReplacementsHolder::Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens)
{
	if (pcSource->IsTokenReplacementsHolder())
	{
		CPPTokenListHolder::Copy((CPPTokenListHolder*)pcSource, pcTokens);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPTokenReplacementsHolder::Equals(CPPToken* pcOther)
{
	if (pcOther->IsTokenReplacementsHolder())
	{
		return CPPTokenListHolder::Equals(pcOther);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPTokenReplacementsHolder::IsHolder2(void)
{
	return true;
}

