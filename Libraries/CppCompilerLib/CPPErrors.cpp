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
#include "CPPErrors.h"
#include "CPPCompileStatistics.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPErrors::Init(void)
{
	maszErrors.Init();
	masPositions.Init();
	maszFiles.Init();
	mszText.Init();
	mpcStatistics = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPErrors::Kill(void)
{
	mszText.Kill();
	maszFiles.Kill();
	masPositions.Kill();
	maszErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPErrors::Clear(void)
{
	maszErrors.ReInit();
	masPositions.ReInit();
	maszFiles.ReInit();
	mszText.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds "File (line, column): message [detail]".  The file and the position are left
// out when they are not known (szFile NULL, iLine -1).  szDetail is usually the
// token's text.
//
//////////////////////////////////////////////////////////////////////////
void CCPPErrors::Add(const char* szFile, int iLine, int iColumn, int iToken, const char* szMessage, const char* szDetail)
{
	CChars*				pszError;
	SCPPErrorPosition*	psPosition;

	pszError = maszErrors.Add();
	if (szFile)
	{
		pszError->Append(szFile);
		pszError->Append(iLine >= 0 ? " " : ": ");
	}
	if (iLine >= 0)
	{
		pszError->Append('(');
		pszError->Append(iLine + 1);
		if (iColumn >= 0)
		{
			pszError->Append(", ");
			pszError->Append(iColumn + 1);
		}
		pszError->Append("): ");
	}
	pszError->Append(szMessage);
	if (szDetail)
	{
		pszError->Append(" [");
		pszError->Append(szDetail);
		pszError->Append(']');
	}

	psPosition = masPositions.Add();
	psPosition->iLine = iLine;
	psPosition->iColumn = iColumn;
	psPosition->iToken = iToken;
	psPosition->uiFile = szFile ? GetFileIndex(szFile) : ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of szFile in the file names, which is added if it isn't there
// yet.
//
//////////////////////////////////////////////////////////////////////////
size CCPPErrors::GetFileIndex(const char* szFile)
{
	size	uiFile;

	uiFile = maszFiles.GetIndex(szFile);
	if (uiFile == ARRAY_ELEMENT_NOT_FOUND)
	{
		uiFile = maszFiles.NumElements();
		maszFiles.Add(szFile);
	}
	return uiFile;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the errors after the first uiNumErrors.
//
//////////////////////////////////////////////////////////////////////////
void CCPPErrors::Truncate(size uiNumErrors)
{
	if (mpcStatistics && (maszErrors.NumElements() > uiNumErrors))
	{
		mpcStatistics->Count(CPPCC_ErrorsTakenAway, maszErrors.NumElements() - uiNumErrors);
	}
	while (maszErrors.NumElements() > uiNumErrors)
	{
		maszErrors.RemoveTail();
		masPositions.Pop();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The errors Truncate takes away are counted in pcStatistics from now on
// (CPPCC_ErrorsTakenAway): an error made and then taken away is work done
// for nothing, or an error that has to be made again to be said.
//
//////////////////////////////////////////////////////////////////////////
void CCPPErrors::CountTakenAway(CCPPCompileStatistics* pcStatistics)
{
	mpcStatistics = pcStatistics;
}


//////////////////////////////////////////////////////////////////////////
//
// The errors from uiFirst on are pcDest's, after its own, and no longer
// this list's.  They are kept, not taken away: nothing is counted.
//
//////////////////////////////////////////////////////////////////////////
void CCPPErrors::MoveTailTo(CCPPErrors* pcDest, size uiFirst)
{
	size	i;

	for (i = uiFirst; i < maszErrors.NumElements(); i++)
	{
		pcDest->maszErrors.Add(maszErrors.Get(i));
		pcDest->masPositions.Add(masPositions.Get(i));
		if (masPositions.Get(i)->uiFile != ARRAY_ELEMENT_NOT_FOUND)
		{
			pcDest->masPositions.Tail()->uiFile = pcDest->GetFileIndex(GetFile(i));
		}
	}
	while (maszErrors.NumElements() > uiFirst)
	{
		maszErrors.RemoveTail();
		masPositions.Pop();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPErrors::Remove(size uiIndex)
{
	maszErrors.Remove(uiIndex);
	masPositions.RemoveAt(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the list has an error that reads szError: the same place and the
// same words.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPErrors::Has(const char* szError)
{
	size	i;

	for (i = 0; i < maszErrors.NumElements(); i++)
	{
		if (maszErrors.Get(i)->Equals(szError))
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
size CCPPErrors::NumErrors(void) const
{
	return maszErrors.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPErrors::Get(size iIndex)
{
	return maszErrors.GetText(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPErrors::GetFile(size iIndex)
{
	size	uiFile;

	uiFile = masPositions.Get(iIndex)->uiFile;
	return (uiFile == ARRAY_ELEMENT_NOT_FOUND) ? NULL : maszFiles.GetText(uiFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPErrors::GetLine(size iIndex)
{
	return masPositions.Get(iIndex)->iLine;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPErrors::GetColumn(size iIndex)
{
	return masPositions.Get(iIndex)->iColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPErrors::GetToken(size iIndex)
{
	return masPositions.Get(iIndex)->iToken;
}


//////////////////////////////////////////////////////////////////////////
//
// All the errors, one a line.
//
//////////////////////////////////////////////////////////////////////////
char* CCPPErrors::Text(void)
{
	size	i;

	mszText.Clear();
	for (i = 0; i < maszErrors.NumElements(); i++)
	{
		mszText.Append(maszErrors.Get(i));
		mszText.AppendNewLine();
	}
	return mszText.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds these errors to pcDest's.  Token indices are kept, so they only mean
// something if the stages share their tokens.
//
//////////////////////////////////////////////////////////////////////////
void CCPPErrors::AppendTo(CCPPErrors* pcDest)
{
	size	i;

	for (i = 0; i < maszErrors.NumElements(); i++)
	{
		pcDest->maszErrors.Add(maszErrors.Get(i));
		pcDest->masPositions.Add(masPositions.Get(i));
		if (masPositions.Get(i)->uiFile != ARRAY_ELEMENT_NOT_FOUND)
		{
			pcDest->masPositions.Tail()->uiFile = pcDest->GetFileIndex(GetFile(i));
		}
	}
}
