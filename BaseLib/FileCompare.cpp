/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "FileCompare.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileCompare::Compare(char* szName1, char* szName2, CChars* psz1, CChars* psz2)
{
	CNaiveFile		cFile1;
	CNaiveFile		cFile2;
	BOOL			bExists1;
	BOOL			bExists2;
	BOOL			bResult;


	cFile1.Init();
	bExists1 = cFile1.Read(szName1);
	cFile2.Init();
	bExists2 = cFile2.Read(szName2);

	if (!bExists1 && !bExists2)
	{
		return TRUE;
	}

	if (bExists1 && !bExists2)
	{
		psz1->Append(szName1);
		psz1->Append(" exists");

		psz2->Append(szName2);
		psz2->Append(" does not exist");

		cFile1.Kill();
		return FALSE;
	}

	if (bExists2 && !bExists1)
	{
		psz2->Append(szName2);
		psz2->Append(" exists");

		psz1->Append(szName1);
		psz1->Append(" does not exist");

		cFile2.Kill();
		return FALSE;
	}

	bResult = Compare(szName1, szName2, psz1, psz2, cFile1.mpvMem, cFile2.mpvMem, cFile1.miSize, cFile2.miSize);

	cFile1.Kill();
	cFile2.Kill();
	return bResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileCompare::Compare(char* szName1, void* pvMemory, filePos iLength, CChars* psz1, CChars* psz2)
{
	CNaiveFile		cFile1;
	BOOL			bExists1;
	BOOL			bResult;

	cFile1.Init();
	bExists1 = cFile1.Read(szName1);

	if (!bExists1 && (pvMemory == NULL))
	{
		return TRUE;
	}

	if (bExists1 && (pvMemory == NULL))
	{
		psz1->Append(szName1);
		psz1->Append(" exists");

		psz2->Append("Memory is NULL");
		cFile1.Kill();
		return FALSE;
	}

	if ((pvMemory != NULL) && !bExists1)
	{
		psz1->Append(szName1);
		psz1->Append(" does not exist");

		psz2->Append("Memory is NOT NULL");
		return FALSE;
	}

	bResult = Compare(szName1, "Memory", psz1, psz2, cFile1.mpvMem, pvMemory, cFile1.miSize, iLength);

	cFile1.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileCompare::ErrorNames(char* szName1, char* szName2, CChars* psz1, CChars* psz2)
{
	psz1->Append(szName1);
	psz2->Append(szName2);
	if (psz1->Length() > psz2->Length())
	{
		psz2->Append(' ', psz1->Length() - psz2->Length());
	}
	else if (psz2->Length() > psz1->Length())
	{
		psz2->Append(' ', psz2->Length() - psz1->Length());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileCompare::Difference(CChars* psz1, filePos iPosition, char c)
{
	psz1->Append(" ");
	psz1->Append(iPosition);
	psz1->Append(":[");
	psz1->AppendHexHiLo(&c, 1);
	psz1->Append("]");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileCompare::Compare(char* szName1, char* szName2, CChars* psz1, CChars* psz2, void* pvMem1, void* pvMem2, filePos iLength1, filePos iLength2)
{
	int				i;
	unsigned char*	pcMem1;
	unsigned char*	pcMem2;
	int				iCount;

	if (iLength1 != iLength2)
	{
		psz1->Append(szName1);
		psz1->Append(" size [");
		psz1->Append(iLength1);
		psz1->Append("]");

		psz1->Append(szName2);
		psz2->Append(" size [");
		psz2->Append(iLength2);
		psz2->Append("]");

		return FALSE;
	}

	//Can't compare files that don't exist.
	if ((iLength1 == 0) && (iLength2 == 0))
	{
		return TRUE;
	}

	pcMem1 = (unsigned char*)pvMem1;
	pcMem2 = (unsigned char*)pvMem2;
	iCount = 0;
	for (i = 0; i < iLength1; i++)
	{
		if (pcMem1[i] != pcMem2[i])
		{
			if (iCount == 0)
			{
				ErrorNames(szName1, "Memory", psz1, psz2);
			}

			if (iCount < 16)
			{
				Difference(psz1, i, pcMem1[i]);
				Difference(psz2, i, pcMem2[i]);
			}

			iCount++;
		}
	}

	if (iCount >= 16)
	{
		psz1->Append(" ... ");
		psz1->Append(iCount - 16);
		psz1->Append(" more differences");

		psz2->Append(" ... ");
		psz2->Append(iCount - 16);
		psz2->Append(" more differences");
	}

	return iCount == 0;
}

