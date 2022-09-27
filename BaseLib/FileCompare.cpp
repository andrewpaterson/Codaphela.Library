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
#include "FileCompare.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileCompare::Compare(const char* szName1, const char* szName2, CChars* psz1, CChars* psz2)
{
	CNaiveFile		cFile1;
	CNaiveFile		cFile2;
	bool			bExists1;
	bool			bExists2;
	bool			bResult;


	cFile1.Init();
	bExists1 = cFile1.Read(szName1);
	cFile2.Init();
	bExists2 = cFile2.Read(szName2);

	if (!bExists1 && !bExists2)
	{
		return true;
	}

	if (bExists1 && !bExists2)
	{
		psz1->Append(szName1);
		psz1->Append(" exists");

		psz2->Append(szName2);
		psz2->Append(" does not exist");

		cFile1.Kill();
		return false;
	}

	if (bExists2 && !bExists1)
	{
		psz2->Append(szName2);
		psz2->Append(" exists");

		psz1->Append(szName1);
		psz1->Append(" does not exist");

		cFile2.Kill();
		return false;
	}

	bResult = Compare(szName1, szName2, psz1, psz2, cFile1.Get(), cFile2.Get(), cFile1.Size(), cFile2.Size());

	cFile1.Kill();
	cFile2.Kill();
	return bResult;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileCompare::Compare(const char* szName1, void* pvMemory, filePos iLength, CChars* psz1, CChars* psz2)
{
	CNaiveFile		cFile1;
	bool			bExists1;
	bool			bResult;

	cFile1.Init();
	bExists1 = cFile1.Read(szName1);

	if (!bExists1 && (pvMemory == NULL))
	{
		return true;
	}

	if (bExists1 && (pvMemory == NULL))
	{
		psz1->Append(szName1);
		psz1->Append(" exists");

		psz2->Append("Memory is NULL");
		cFile1.Kill();
		return false;
	}

	if ((pvMemory != NULL) && !bExists1)
	{
		psz1->Append(szName1);
		psz1->Append(" does not exist");

		psz2->Append("Memory is NOT NULL");
		return false;
	}

	bResult = Compare(szName1, "Memory", psz1, psz2, cFile1.Get(), pvMemory, cFile1.Size(), iLength);

	cFile1.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileCompare::Compare(const char* szName1, const char* szName2, CChars* psz1, CChars* psz2, void* pvMem1, void* pvMem2, filePos iLength1, filePos iLength2)
{
	return MemCompare(szName1, szName2, psz1, psz2, pvMem1, pvMem2, iLength1, iLength2);
}

