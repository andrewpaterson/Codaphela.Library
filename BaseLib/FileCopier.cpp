/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "FileBasic.h"
#include "Numbers.h"
#include "PointerFunctions.h"
#include "FileCopier.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileCopier::Copy(CAbstractFile* pcSource, CAbstractFile* pcDest)
{
	CFileBasic	cSource;
	CFileBasic	cDest;
	void*		pvTemp;
	filePos		iLength;
	int			iChunk;
	filePos		iRemaining;
	EFileMode	eWriteMode;

	if ((!pcSource) || (!pcDest))
	{
		return false;
	}

	cSource.Init(pcSource);
	cDest.Init(pcDest);

	if (!cSource.Open(EFM_Read))
	{
		cDest.Kill();
		cSource.Kill();
		return false;
	}

	iLength = cSource.GetFileLength();
	if (iLength == 0)
	{
		cDest.Close();
		cDest.Kill();
		cSource.Close();
		cSource.Kill();
		return true;
	}

	eWriteMode = EFM_Write_Create;
	if (!cDest.Open(eWriteMode))
	{
		cSource.Close();
		cDest.Kill();
		cSource.Kill();
		return false;
	}

	if (iLength > 10 MB)
	{
		pvTemp = malloc(10 MB);
	}
	else
	{
		pvTemp = malloc((int)iLength);
	}

	iRemaining = iLength;
	while (iRemaining != 0)
	{
		if (iRemaining > 10 MB)
		{
			iChunk = 10 MB;
		}
		else
		{
			iChunk = (int) iRemaining;
		}
		if (!cSource.ReadData(pvTemp, iChunk))
		{
			cSource.Close();
			cDest.Close();

			cSource.Kill();
			cDest.Kill();

			SafeFree(pvTemp);
			return false;
		}

		if (!cDest.WriteData(pvTemp, iChunk))
		{
			cSource.Close();
			cDest.Close();

			cSource.Kill();
			cDest.Kill();

			SafeFree(pvTemp);
			return false;
		}

		iRemaining -= iChunk;
	}

	cSource.Close();
	cDest.Close();

	cSource.Kill();
	cDest.Kill();

	SafeFree(pvTemp);
	return true;
}

