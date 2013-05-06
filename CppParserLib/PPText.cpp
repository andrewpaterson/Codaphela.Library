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
#include "PPText.h"
#include "CoreLib/EscapeCodes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPText::Init(int iLine, int iColumn)
{
	//Ignored
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPText::Init(EPreprocessorText eType, int iLine, int iColumn, char* szStart, int iLength)
{
	Set(iLine, iColumn);
	mcText.Init(szStart, iLength);
	meType = eType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPText::Init(EPreprocessorText eType, int iLine, int iColumn, char* szStart, char* szEndExclusive)
{
	Init(eType, iLine, iColumn, szStart, (int)(szEndExclusive - szStart));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPText::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPText::IsText(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPText::Append(CChars* psz)
{
	char	szDest[4];
	BOOL	bResult;
	int		i;
	char	c;

	if (!((meType == PPT_SingleQuoted) || (meType == PPT_DoubleQuoted)))
	{
		psz->Append(mcText.msz, mcText.miLen);
	}
	else
	{
		for (i = 0; i < mcText.miLen; i++)
		{
			c = mcText.msz[i];
			bResult = GetEscapeString(c, szDest);
			if (bResult)
			{
				psz->Append(szDest);
			}
			else
			{
				psz->Append(c);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPText::Copy(CPPToken* pcSource, CMemoryStackExtended* pcStack)
{
	CPPText*	pcCast;

	if (pcSource->IsText())
	{
		pcCast = (CPPText*)pcSource;
		Set(pcCast->miLine, pcCast->miColumn);
		meType = pcCast->meType;
		mcText.Init(pcCast->mcText.msz, pcCast->mcText.miLen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPText::HasSource(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPText::Equals(CPPToken* pcOther)
{
	CPPText*	pcCast;	

	if (pcOther->IsText())
	{
		pcCast = (CPPText*)pcOther;
		if (meType == pcCast->meType)
		{
			if (mcText.Equals(&pcCast->mcText))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPText::TextStart(void)
{
	return mcText.msz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPText::TextEndInclusive(void)
{
	return mcText.EndInclusive();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPText::IsEmpty(void)
{
	return FALSE;
}