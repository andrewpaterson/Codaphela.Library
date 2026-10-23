/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "CPPErrors.h"


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
	psPosition->uiFile = ARRAY_ELEMENT_NOT_FOUND;
	if (szFile)
	{
		psPosition->uiFile = maszFiles.GetIndex(szFile);
		if (psPosition->uiFile == ARRAY_ELEMENT_NOT_FOUND)
		{
			psPosition->uiFile = maszFiles.NumElements();
			maszFiles.Add(szFile);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the errors after the first uiNumErrors.
//
//////////////////////////////////////////////////////////////////////////
void CCPPErrors::Truncate(size uiNumErrors)
{
	while (maszErrors.NumElements() > uiNumErrors)
	{
		maszErrors.RemoveTail();
		masPositions.Pop();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPErrors::NumErrors(void)
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
			pcDest->masPositions.Tail()->uiFile = pcDest->maszFiles.GetIndex(GetFile(i));
			if (pcDest->masPositions.Tail()->uiFile == ARRAY_ELEMENT_NOT_FOUND)
			{
				pcDest->masPositions.Tail()->uiFile = pcDest->maszFiles.NumElements();
				pcDest->maszFiles.Add(GetFile(i));
			}
		}
	}
}
