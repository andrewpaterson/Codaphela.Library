/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "PPHashes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHashes::Init(size iLine, size iColumn, char* szFileName)
{
	CPPToken::Init((int)iLine, (int)iColumn, szFileName);
	miCount = 0;
	mbDigraph = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPHashes::Init(size iCount, size iLine, size iColumn, char* szFileName)
{
	CPPToken::Init((int)iLine, (int)iColumn, szFileName);
	Set((int)iLine, (int)iColumn);
	miCount = iCount;
	mbDigraph = false;
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
bool CPPHashes::IsHash(void) const
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPHashes::Print(CChars* psz)
{
	size	i;

	if (mbDigraph)
	{
		for (i = 0; i < miCount; i++)
		{
			psz->Append("%:");
		}
	}
	else
	{
		psz->Append('#', miCount);
	}
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
		mbDigraph = pcCast->mbDigraph;
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPHashes::IsEmpty(void)
{
	return false;
}
