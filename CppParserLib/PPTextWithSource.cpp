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
#include "PPTextWithSource.h"
#include "BaseLIb/PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTextWithSource::Init(int iLine, int iColumn, char* szFileName)
{
	CPPText::Init(iLine, iColumn, szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTextWithSource::Init(EPreprocessorText eType, int iLine, int iColumn, char* szFileName, char* szStart, int iLength)
{
	mszText = (char*)malloc(iLength + 1);
	memcpy(mszText, szStart, iLength + 1);

	CPPText::Init(eType, iLine, iColumn, szFileName, mszText, iLength);

	if ((eType == PPT_Decorator) && (iLength != 1))
	{
		gcUserError.Set("Decorators must be exactly one character long.");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTextWithSource::Init(EPreprocessorText eType, int iLine, int iColumn, char* szFileName, char* szStart, char* szEndExclusive)
{
	Init(eType, iLine, iColumn, szFileName, szStart, (int)(szEndExclusive - szStart));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTextWithSource::Kill(void)
{
	SafeFree(mszText);
	CPPText::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTextWithSource::Copy(CPPToken* pcSource, CPPTokenMemory* pcTokens)
{
	CPPTextWithSource*	pcCast;

	if (pcSource->IsText())
	{
		pcCast = (CPPTextWithSource*)pcSource;
		Init(pcCast->meType, pcCast->miLine, pcCast->miColumn, pcCast->ShortFileName(), pcCast->mcText.msz, pcCast->mcText.miLen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPPTextWithSource::HasSource(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPTextWithSource::TextStart(void)
{
	return mszText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPPTextWithSource::TextEndInclusive(void)
{
	int	iLen;

	iLen = (int)strlen(mszText);
	return mszText + iLen-1;
}

