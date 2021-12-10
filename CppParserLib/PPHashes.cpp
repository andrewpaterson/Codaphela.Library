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
#include "PPHashes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHashes::Init(int iLine, int iColumn, char* szFileName)
{
	CPPToken::Init(iLine, iColumn, szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHashes::Init(int iCount, int iLine, int iColumn, char* szFileName)
{
	CPPToken::Init(iLine, iColumn, szFileName);
	Set(iLine, iColumn);
	miCount = iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHashes::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPHashes::IsHash(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPHashes::Print(CChars* psz)
{
	psz->Append('#', miCount);
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHashes::Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens)
{
	CPPHashes*	pcCast;

	if (pcSource->IsHash())
	{
		pcCast = (CPPHashes*)pcSource;
		Init(pcCast->miCount, pcCast->miLine, pcCast->miColumn, pcCast->ShortFileName());
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPHashes::Equals(CPPToken* pcOther)
{
	CPPHashes*	pcCast;	

	if (pcOther->IsHash())
	{
		pcCast = (CPPHashes*)pcOther;
		if (miCount == pcCast->miCount)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPHashes::IsEmpty(void)
{
	return FALSE;
}

