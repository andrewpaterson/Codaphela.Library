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
#include "PPWhiteSpace.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPWhiteSpace::Init(int iLine, int iColumn, char* szFileName)
{
	CPPToken::Init(iLine, iColumn, szFileName);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPWhiteSpace::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPWhiteSpace::Print(CChars* psz)
{
	psz->Append(' ');
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPWhiteSpace::IsWhiteSpace(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPWhiteSpace::Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens)
{
	CPPWhiteSpace*	pcCast;

	if (pcSource->IsWhiteSpace())
	{
		pcCast = (CPPWhiteSpace*)pcSource;
		Init(pcCast->miLine, pcCast->miColumn, pcCast->ShortFileName());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPWhiteSpace::Equals(CPPToken* pcOther)
{
	if (pcOther->IsWhiteSpace())
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPWhiteSpace::IsEmpty(void)
{
	return true;
}

