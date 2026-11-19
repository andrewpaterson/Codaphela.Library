/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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


#define FILE_COPIER_BLOCK_SIZE	(1 MB)		//The most read before it is written.


//////////////////////////////////////////////////////////////////////////
//
// pcSource's bytes in pcDest, which is replaced (emptied first, so it ends
// up the same length).  Both are killed (and freed if they must be).
// False if one couldn't be opened or read or written.
//
//////////////////////////////////////////////////////////////////////////
bool CFileCopier::Copy(CAbstractFile* pcSource, CAbstractFile* pcDest)
{
	CFileBasic	cSource;
	CFileBasic	cDest;
	void*		pvData;
	filePos		iRemaining;
	lsize		iChunk;
	bool		bResult;

	if (!pcSource || !pcDest)
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
	if (!cDest.Open(EFM_Write_Replace))
	{
		cSource.Close();
		cDest.Kill();
		cSource.Kill();
		return false;
	}

	bResult = true;
	pvData = NULL;
	iRemaining = cSource.GetFileLength();
	if (iRemaining > 0)
	{
		pvData = malloc((iRemaining > FILE_COPIER_BLOCK_SIZE) ? FILE_COPIER_BLOCK_SIZE : (lsize)iRemaining);
	}
	while (bResult && (iRemaining > 0))
	{
		iChunk = (iRemaining > FILE_COPIER_BLOCK_SIZE) ? FILE_COPIER_BLOCK_SIZE : (lsize)iRemaining;
		bResult = cSource.ReadData(pvData, iChunk) && cDest.WriteData(pvData, iChunk);
		iRemaining -= iChunk;
	}
	SafeFree(pvData);

	bResult &= cDest.Close();
	cSource.Close();
	cDest.Kill();
	cSource.Kill();
	return bResult;
}

