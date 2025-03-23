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
#include "PPHashes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHashes::Init(size iLine, size iColumn, char* szFileName)
{
	CPPToken::Init(iLine, iColumn, szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHashes::Init(size iCount, size iLine, size iColumn, char* szFileName)
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
bool CPPHashes::IsHash(void)
{
	return true;
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
bool CPPHashes::Equals(CPPToken* pcOther)
{
	CPPHashes*	pcCast;	

	if (pcOther->IsHash())
	{
		pcCast = (CPPHashes*)pcOther;
		if (miCount == pcCast->miCount)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPHashes::IsEmpty(void)
{
	return false;
}

