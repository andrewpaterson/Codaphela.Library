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
#include "BaseLib/EscapeCodes.h"
#include "PPText.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPText::Init(int iLine, int iColumn, char* szFileName)
{
	CPPToken::Init(iLine, iColumn, szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPText::Init(EPreprocessorText eType, int iLine, int iColumn, char* szFileName, char* szStart, int iLength)
{
	CPPToken::Init(iLine, iColumn, szFileName);
	mcText.Init(szStart, iLength);
	meType = eType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPText::Init(EPreprocessorText eType, int iLine, int iColumn, char* szFileName, char* szStart, char* szEndExclusive)
{
	Init(eType, iLine, iColumn, szFileName, szStart, (int)(szEndExclusive - szStart));
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
bool CPPText::IsText(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPText::Print(CChars* psz)
{
	char	szDest[4];
	bool	bResult;
	int		i;
	char	c;

	if (!((meType == PPT_SingleQuoted) || (meType == PPT_DoubleQuoted)))
	{
		psz->Append(mcText.msz, mcText.miLen);
	}
	else
	{
		c = mcText.msz[0];
		psz->Append(c);
		for (i = 1; i < mcText.miLen -1; i++)
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
		c = mcText.msz[mcText.miLen - 1];
		psz->Append(c);
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPText::Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens)
{
	CPPText*	pcCast;

	if (pcSource->IsText())
	{
		pcCast = (CPPText*)pcSource;
		Init(pcCast->meType, pcCast->miLine, pcCast->miColumn, pcCast->ShortFileName(), pcCast->mcText.msz, pcCast->mcText.miLen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPText::HasSource(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPText::Equals(CPPToken* pcOther)
{
	CPPText*	pcCast;	

	if (pcOther->IsText())
	{
		pcCast = (CPPText*)pcOther;
		if (meType == pcCast->meType)
		{
			if (mcText.Equals(&pcCast->mcText))
			{
				return true;
			}
		}
	}
	return false;
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
bool CPPText::IsEmpty(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPPText::Length(void)
{
	return mcText.miLen;
}

