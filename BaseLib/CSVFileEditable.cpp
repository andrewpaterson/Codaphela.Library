/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "CSVFileEditable.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SCSVRowEditable::Init(void)
{
	aszData.Init(64);
	iFileOffset = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SCSVRowEditable::Kill(void)
{
	aszData.Kill();
	iFileOffset = -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SCSVRowEditable::Dump(void)
{
	int			i;
	CChars		szString;
	CChars		szLeft;
	CChars*	pszString;

	szString.Init();
	for (i = 0; i < aszData.NumElements(); i++)
	{
		pszString = aszData.Get(i);
		szLeft.Init();
		szLeft.LeftAlign(*pszString, ' ', 15);
		szString.Append(szLeft);
		szLeft.Kill();
		if (i != aszData.NumElements()-1)
		{
			szString.Append("| ");
		}
	}
	szString.AppendNewLine();
	szString.Dump();
	szString.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SCSVRowEditable::IndexOf(char* szString)
{
	int			i;
	CChars*	pcString;

	for (i = 0; i < aszData.NumElements(); i++)
	{
		pcString = aszData.Get(i);
		if (pcString->Equals(szString))
		{
			return i;
		}
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int SCSVRowEditable::NumCells(void)
{
	return aszData.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChars* SCSVRowEditable::Get(int iIndex)
{
	return aszData.Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFileEditable::Init(char cSeparator)
{
	CCSVFile::Init(cSeparator);
	masCSVRows.Init(32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFileEditable::Kill(void)
{
	int			i;
	SCSVRowEditable*	psCSVRow;

	for (i = 0; i < masCSVRows.NumElements(); i++)
	{
		psCSVRow = masCSVRows.Get(i);
		psCSVRow->Kill();
	}
	masCSVRows.Kill();

	CCSVFile::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCSVFileEditable::ReadLine(void)
{
	SCSVRowEditable*	psCSVRow;
	BOOL				bResult;

	psCSVRow = masCSVRows.Add();
	psCSVRow->Init();

	bResult = ReadLine(-1, psCSVRow);
	if (!bResult)
	{
		psCSVRow->Kill();
		masCSVRows.RemoveTail();
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCSVFileEditable::ReadLine(CChars* szString)
{
	return CCSVFile::ReadLine(szString);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCSVFileEditable::ReadLine(int iFileOffset, SCSVRowEditable* psCSVRow)
{
	CChars	cString;
	BOOL	bResult;

	if (iFileOffset != -1)
	{
		mcFile.Seek(iFileOffset, EFSO_SET);
	}

	cString.Init();
	bResult = ReadLine(&cString);
	if (bResult)
	{
		cString.Split(&psCSVRow->aszData, mcSeparator);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFileEditable::ReadAllLines(void)
{
	BOOL	bResult;

	do
	{
		bResult = ReadLine();
	} while (bResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFileEditable::Dump(void)
{
	int			i;
	SCSVRowEditable*	psCSVRow;

	for (i = 0; i < masCSVRows.NumElements(); i++)
	{
		psCSVRow = masCSVRows.Get(i);
		psCSVRow->Dump();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCSVFileEditable::NumRows(void)
{
	return masCSVRows.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCSVRowEditable* CCSVFileEditable::Get(int iRowNum)
{
	return masCSVRows.Get(iRowNum);
}

