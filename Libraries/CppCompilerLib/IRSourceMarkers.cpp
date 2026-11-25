/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "IRBlock.h"
#include "IRSourceMarkers.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRLineMarkers::Init(void)
{
	maszMarkers.Init();
	maiLists.Init();
	maiPending.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRLineMarkers::Kill(void)
{
	maiPending.Kill();
	maiLists.Kill();
	maszMarkers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// No markers: a function's code is written from here.
//
//////////////////////////////////////////////////////////////////////////
void CIRLineMarkers::Clear(void)
{
	maszMarkers.ReInit();
	maiLists.ReInit();
	maiPending.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// A marker written before the next line.
//
//////////////////////////////////////////////////////////////////////////
void CIRLineMarkers::Add(const char* szMarker)
{
	int32	iMarker;

	iMarker = (int32)maszMarkers.NumElements();
	maszMarkers.Add(szMarker);
	maiPending.Add(&iMarker);
}


//////////////////////////////////////////////////////////////////////////
//
// The markers written since the last line, as a list for the line written
// now (IR_NO_MARKERS for none).
//
//////////////////////////////////////////////////////////////////////////
int32 CIRLineMarkers::TakePending(void)
{
	int32	iList;
	int32	iCount;
	size	ui;

	if (maiPending.NumElements() == 0)
	{
		return IR_NO_MARKERS;
	}
	iList = (int32)maiLists.NumElements();
	iCount = (int32)maiPending.NumElements();
	maiLists.Add(&iCount);
	for (ui = 0; ui < maiPending.NumElements(); ui++)
	{
		maiLists.Add(maiPending.Get(ui));
	}
	maiPending.ReInit();
	return iList;
}


//////////////////////////////////////////////////////////////////////////
//
// The markers of lines taken back (the code cut back to before them) are
// written again, before those written since.
//
//////////////////////////////////////////////////////////////////////////
void CIRLineMarkers::PutBack(int32 iList)
{
	CArrayTemplate<int32>	aiPending;
	int32					iCount;
	int32					iMarker;
	int32					i;

	if (iList == IR_NO_MARKERS)
	{
		return;
	}
	aiPending.Init();
	iCount = *maiLists.Get((size)iList);
	for (i = 1; i <= iCount; i++)
	{
		iMarker = *maiLists.Get((size)(iList + i));
		aiPending.Add(&iMarker);
	}
	for (i = 0; i < (int32)maiPending.NumElements(); i++)
	{
		aiPending.Add(maiPending.Get((size)i));
	}
	maiPending.ReInit();
	for (i = 0; i < (int32)aiPending.NumElements(); i++)
	{
		maiPending.Add(aiPending.Get((size)i));
	}
	aiPending.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The list of iFirst's markers then iSecond's (either IR_NO_MARKERS).
//
//////////////////////////////////////////////////////////////////////////
int32 CIRLineMarkers::Join(int32 iFirst, int32 iSecond)
{
	int32	iList;
	int32	iCount;
	int32	iFirstCount;
	int32	iSecondCount;
	int32	iMarker;
	int32	i;

	if (iFirst == IR_NO_MARKERS)
	{
		return iSecond;
	}
	if (iSecond == IR_NO_MARKERS)
	{
		return iFirst;
	}
	iFirstCount = *maiLists.Get((size)iFirst);
	iSecondCount = *maiLists.Get((size)iSecond);
	iCount = iFirstCount + iSecondCount;
	iList = (int32)maiLists.NumElements();
	maiLists.Add(&iCount);
	//Each copied out first: adding to the lists may move them.
	for (i = 1; i <= iFirstCount; i++)
	{
		iMarker = *maiLists.Get((size)(iFirst + i));
		maiLists.Add(&iMarker);
	}
	for (i = 1; i <= iSecondCount; i++)
	{
		iMarker = *maiLists.Get((size)(iSecond + i));
		maiLists.Add(&iMarker);
	}
	return iList;
}


//////////////////////////////////////////////////////////////////////////
//
// The list's markers, each a line: every place marker, and of the source
// markers only the last.
//
//////////////////////////////////////////////////////////////////////////
void CIRLineMarkers::Print(int32 iList, CChars* pszOut)
{
	int32	iCount;
	int32	i;
	int32	iLastSource;
	CChars*	pszMarker;

	if (iList == IR_NO_MARKERS)
	{
		return;
	}
	iCount = *maiLists.Get((size)iList);
	iLastSource = -1;
	for (i = 1; i <= iCount; i++)
	{
		if (maszMarkers.Get((size)*maiLists.Get((size)(iList + i)))->StartsWith(IR_SOURCE_MARKER))
		{
			iLastSource = i;
		}
	}
	for (i = 1; i <= iCount; i++)
	{
		pszMarker = maszMarkers.Get((size)*maiLists.Get((size)(iList + i)));
		if ((i == iLastSource) || !pszMarker->StartsWith(IR_SOURCE_MARKER))
		{
			pszOut->Append(*pszMarker);
			pszOut->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the line at szLine is one a debugger's markers made: a source
// marker, a place marker, or the label a place marker became
// (CCPP65IRFrontEnd::DebugRanges).
//
//////////////////////////////////////////////////////////////////////////
bool IRIsMarkerLine(const char* szLine)
{
	return (strncmp(szLine, IR_SOURCE_MARKER, StrLen(IR_SOURCE_MARKER)) == 0) || (strncmp(szLine, IR_PLACE_MARKER, StrLen(IR_PLACE_MARKER)) == 0) ||
		   (strncmp(szLine, IR_DEBUG_LABEL, StrLen(IR_DEBUG_LABEL)) == 0);
}
