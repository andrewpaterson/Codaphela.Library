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
void CPPTextWithSource::Init(int iLine, int iColumn)
{
	//Ignored
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTextWithSource::Init(EPreprocessorText eType, int iLine, int iColumn, char* szStart, int iLength)
{
	mszText = (char*)malloc(iLength+1);
	memcpy(mszText, szStart, iLength+1);
	CPPText::Init(eType, iLine, iColumn, mszText, iLength);

	if ((eType == PPT_Decorator) && (iLength != 1))
	{
		int xxx = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPPTextWithSource::Init(EPreprocessorText eType, int iLine, int iColumn, char* szStart, char* szEndExclusive)
{
	Init(eType, iLine, iColumn, szStart, (int)(szEndExclusive - szStart));
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
		Set(pcCast->miLine, pcCast->miColumn);
		meType = pcCast->meType;
		mszText = (char*)malloc(pcCast->mcText.miLen+1);
		memcpy(mszText, pcCast->mcText.msz, pcCast->mcText.miLen+1);
		mcText.Init(mszText, pcCast->mcText.miLen);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPPTextWithSource::HasSource(void)
{
	return TRUE;
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

