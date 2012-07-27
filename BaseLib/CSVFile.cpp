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
#include "CSVFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFile::Init(char cSeparator)
{
	mcFile.Init(NULL);
	miFileSize = 0;
	mcSeparator = cSeparator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFile::Kill(void)
{
	mcFile.Kill();
	miFileSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFile::Open(CAbstractFile* pcFile)
{
	mcFile.Init(pcFile);
	mcFile.Open(EFM_Read);
	miFileSize = mcFile.GetFileLength();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCSVFile::Close(void)
{
	mcFile.Close();
}


////////////////////////////////////////////////////////////////////////
																		//
																		//
////////////////////////////////////////////////////////////////////////
BOOL CCSVFile::ReadLine(CChars* szString)
{
	char	sBuffer[CSV_FILE_LINE_BUFFER_LENGTH+1];
	int		iLength;
	BOOL	bDoneAnything;

	bDoneAnything = FALSE;
	for (;;)
	{
		iLength = ReadLine(sBuffer);
		if (iLength == 0)
		{
			return bDoneAnything;
		}
		sBuffer[iLength] = 0;
		szString->Append(sBuffer);
		if (iLength < CSV_FILE_LINE_BUFFER_LENGTH)
		{
			return TRUE;
		}
		bDoneAnything = TRUE;
	}
}


////////////////////////////////////////////////////////////////////////
																		//
																		//
////////////////////////////////////////////////////////////////////////
BOOL CCSVFile::ReadLine(char* szBuffer, int iMaxLength)
{
	int		iLength;
	BOOL	bDoneAnything;
	int		iPosition;

	bDoneAnything = FALSE;
	iPosition = 0;
	for (;;)
	{
		iLength = ReadLine(&szBuffer[iPosition]);
		iPosition += iLength;
		if ((iLength > 0) && (iLength < CSV_FILE_LINE_BUFFER_LENGTH))
		{
			szBuffer[iPosition] = 0;
			return TRUE;
		}
		if (iLength == 0)
		{
			if (bDoneAnything)
			{
				szBuffer[iPosition] = 0;
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
		bDoneAnything = TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CCSVFile::ReadLine(char* sBuffer)
{
	int		iNewLinePosition;
	int		iBytesRead;
	filePos	iPosition;

	iPosition = mcFile.GetFilePos();
	iBytesRead = (int)mcFile.Read(sBuffer, 1, CSV_FILE_LINE_BUFFER_LENGTH);
	if (iBytesRead == 0)
	{
		return 0;
	}
	iNewLinePosition = FindFirstByte(sBuffer, '\n', CSV_FILE_LINE_BUFFER_LENGTH);
	if (iNewLinePosition != -1)
	{
		if (sBuffer[iNewLinePosition-1] == '\r')
		{
			mcFile.Seek(iPosition + iNewLinePosition + 1, EFSO_SET);
			iNewLinePosition--;
		}
		else
		{
			mcFile.Seek(iPosition + iNewLinePosition + 1, EFSO_SET);
		}
		return iNewLinePosition;
	}
	else
	{
		return CSV_FILE_LINE_BUFFER_LENGTH;
	}
}

