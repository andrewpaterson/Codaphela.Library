/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "CSVHelper.h"
#include "DiskFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVHelper::Init(char* szFilename, char cSeparator)
{
	mcFile.Init(cSeparator);
	mcFile.Open(DiskFile(szFilename));
	mcFile.ReadAllLines();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVHelper::Kill(void)
{
	mcFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCSVHelper::GetColumnIndex(char* szName)
{
	SCSVRowImmutable*	pcRow;
	size				i;
	char*				szHeader;
	CChars				sz;
	size				uiNumElements;

	pcRow = mcFile.Get(0);
	uiNumElements = pcRow->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		szHeader = pcRow->Get(i);
		sz.Init(szHeader);
		sz.StripWhitespace();
		if (sz.EqualsIgnoreCase(szName))
		{
			sz.Kill();
			return i;
		}
		sz.Kill();
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCSVHelper::Get(size iRow, size iColumn)
{
	SCSVRowImmutable*	pcRow;

	pcRow = mcFile.Get(iRow);
	return pcRow->Get(iColumn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCSVHelper::NumRows(void)
{
	return mcFile.NumRows();
}

