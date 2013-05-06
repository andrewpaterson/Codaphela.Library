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
#include "CSVFileImutable.h"
#include "IntegerHelper.h"
#include "PointerRemapper.h"
#include "PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SCSVRowImmutable::Dump(void)
{	
	int			i;
	CChars		szString;
	CChars		szLeft;
	char*		pszString;

	szString.Init();
	for (i = 0; i < iNumFields; i++)
	{
		pszString = aszFields[i];
		szLeft.Init();
		szLeft.LeftAlign(pszString, ' ', 15);
		szString.Append(szLeft);
		szLeft.Kill();
		if (i != iNumFields-1)
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
int SCSVRowImmutable::IndexOf(char* szString)
{
	int	i;

	for (i = 0; i < iNumFields; i++)
	{
		if (strcmp(szString, aszFields[i]) == 0)
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
char* SCSVRowImmutable::Get(int iIndex)
{
	if (iIndex < iNumFields)
	{
		return aszFields[iIndex];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFileImmutable::Init(char cSeparator)
{
	CCSVFile::Init(cSeparator);
	mapsCSVRows.Init(16384);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFileImmutable::Kill(void)
{
	int					i;
	SCSVRowImmutable*	psCSVRow;

	for (i = 0; i < mapsCSVRows.NumElements(); i++)
	{
		psCSVRow = (SCSVRowImmutable*)mapsCSVRows.GetPtr(i);
		FreeRow(psCSVRow);
	}
	mapsCSVRows.Kill();

	CCSVFile::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCSVRowImmutable* CCSVFileImmutable::AllocateRow(char* szText)
{
	int					iLength;
	char				c;
	int					iNumSeparators;
	SCSVRowImmutable*	psCSVRow;
	int					iTotalSize;
	int					iStringStart;
	int					i;
	char*				pszFieldPos;

	iNumSeparators = 0;

	//The complier can optimise this better than you can... leave it alone.
	for (iLength = 0;; iLength++)
	{
		c = szText[iLength];
		
		//Check for the end of the row.
		if ((c == 0) || (c == '\n') || (c == '\r'))
		{
			break;
		}
		else if (c == mcSeparator)
		{
			iNumSeparators++;
		}
	}

	//Allocate the heading size and the first char*, then the rest of the char pointers (iNumSeparators is the number
	//of fields -1), then allocate the string and lastly the terminating zero.
	iStringStart = sizeof(SCSVRowImmutable) + iNumSeparators * sizeof(char*);
	iTotalSize = iStringStart + iLength + 1;
	psCSVRow = (SCSVRowImmutable*)malloc(iTotalSize);

	psCSVRow->iNumFields = iNumSeparators+1;
	psCSVRow->iRowLength = iLength+1;
	psCSVRow->iTotalSize = iTotalSize;

	pszFieldPos = (char*)RemapSinglePointer(psCSVRow, iStringStart);
	memcpy(pszFieldPos, szText, iLength);
	pszFieldPos[iLength] = mcSeparator;

	iNumSeparators = 0;
	psCSVRow->aszFields[0] = pszFieldPos;
	for (i = 0;; i++)
	{
		c = pszFieldPos[i];

		if (c == mcSeparator)
		{
			pszFieldPos[i] = 0;
			iNumSeparators++;
			if (iNumSeparators == psCSVRow->iNumFields)
			{
				break;
			}
			psCSVRow->aszFields[iNumSeparators] = &pszFieldPos[i+1];
		}
	}
	return psCSVRow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFileImmutable::FreeRow(SCSVRowImmutable* psCSVRow)
{
	SafeFree(psCSVRow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCSVFileImmutable::ReadLine(void)
{
	SCSVRowImmutable*	psCSVRow;
	BOOL				bResult;
	filePos				iFilePos;

	iFilePos = mcFile.GetFilePos();
	bResult = ReadLine(-1, &psCSVRow);
	if (!bResult)
	{
		return FALSE;
	}
	mapsCSVRows.Add(psCSVRow, (int)iFilePos);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCSVFileImmutable::ReadLine(char* szString, int iMaxLength)
{
	return CCSVFile::ReadLine(szString, iMaxLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CCSVFileImmutable::ReadLine(int iFileOffset, SCSVRowImmutable** ppsCSVRow)
{
	BOOL	bResult;
	char		szBuffer[1024*128];

	if (iFileOffset != -1)
	{
		mcFile.Seek(iFileOffset, EFSO_SET);
	}

	bResult = ReadLine(szBuffer, 1024*128);
	if (bResult)
	{
		*ppsCSVRow = AllocateRow(szBuffer);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFileImmutable::ReadAllLines(void)
{
	BOOL	bResult;
	int		iRow = 0;

	do
	{
		bResult = ReadLine();
		iRow++;
	} while (bResult);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFileImmutable::Dump(void)
{
	int					i;
	SCSVRowImmutable*	psCSVRow;

	for (i = 0; i < mapsCSVRows.NumElements(); i++)
	{
		psCSVRow = (SCSVRowImmutable*)mapsCSVRows.GetPtr(i);
		psCSVRow->Dump();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCSVFileImmutable::NumRows(void)
{
	return mapsCSVRows.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCSVRowImmutable* CCSVFileImmutable::Get(int iRowNum)
{
	return (SCSVRowImmutable*)mapsCSVRows.SafeGetPtr(iRowNum);
}

