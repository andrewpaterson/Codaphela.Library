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
void CCSVHelper::Init(char* szFileName, char cSeparator)
{
	mcFile.Init(cSeparator);
	mcFile.Open(DiskFile(szFileName));
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
int CCSVHelper::GetColumnIndex(char* szName)
{
	SCSVRowImmutable*	pcRow;
	int					i;
	char*				szHeader;
	CChars				sz;

	pcRow = mcFile.Get(0);
	for (i = 0; i < pcRow->NumElements(); i++)
	{
		szHeader = pcRow->Get(i);
		sz.Init(szHeader);
		sz.StripWhiteSpace();
		if (sz.EqualsIgnoreCase(szName))
		{
			sz.Kill();
			return i;
		}
		sz.Kill();
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCSVHelper::Get(int iRow, int iColumn)
{
	SCSVRowImmutable*	pcRow;

	if ((iColumn < 0) || (iRow < 0))
	{
		return NULL;
	}
	pcRow = mcFile.Get(iRow);
	return pcRow->Get(iColumn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCSVHelper::NumRows(void)
{
	return mcFile.NumRows();
}

