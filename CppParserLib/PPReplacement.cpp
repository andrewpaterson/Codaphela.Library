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
#include "PPReplacement.h"



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPReplacement::Init(int iLine, int iColumn)
{
	//Ignored.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPReplacement::Init(int iDefineIndex, int iArgIndex, int iLine, int iColumn)
{
	Set(iLine, iColumn);
	miArgIndex = iArgIndex;
	miDefineIndex = iDefineIndex;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPReplacement::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPReplacement::Append(CChars* psz)
{
	psz->Append("/* Replacement: ");
	psz->Append(miArgIndex);
	psz->Append("*/");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPReplacement::IsReplacement(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPReplacement::Copy(CPPToken* pcSource, CMemoryStackExtended* pcStack)
{
	CPPReplacement*	pcCast;

	if (pcSource->IsReplacement())
	{
		pcCast = (CPPReplacement*)pcSource;
		Set(pcCast->miLine, pcCast->miColumn);
		miArgIndex = pcCast->miArgIndex;
		miDefineIndex = pcCast->miDefineIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPReplacement::Equals(CPPToken* pcOther)
{
	CPPReplacement*	pcCast;	

	if (pcOther->IsReplacement())
	{
		pcCast = (CPPReplacement*)pcOther;
		if (miArgIndex == pcCast->miArgIndex)
		{
			//I'm assuming it's safe to ignore this until such time as it comes back and bites me.
			//if (miDefineIndex == pcCast->miDefineIndex)
			//{
				return TRUE;
			//}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPReplacement::IsEmpty(void)
{
	return FALSE;
}