/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/Logger.h"
#include "ImageReaderCommon.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 ReadLittleEndianUInt16(uint8* pui)
{
	return (uint16)(pui[0] | (pui[1] << 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 ReadLittleEndianUInt32(uint8* pui)
{
	return (uint32)pui[0] | ((uint32)pui[1] << 8) | ((uint32)pui[2] << 16) | ((uint32)pui[3] << 24);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* LoadFileIntoMemory(char* szFilename, size* puiFileSize)
{
	CFileBasic	cFile;
	filePos		iFileSize;
	uint8*		pvFile;

	cFile.Init(DiskFile(szFilename));
	if (!cFile.Open(EFM_Read))
	{
		cFile.Kill();
		gcLogger.Error2(__METHOD__, " Could not open [", szFilename, "].", NULL);
		return NULL;
	}
	iFileSize = cFile.GetFileLength();
	if ((iFileSize <= 0) || (iFileSize > 0x7FFFFFFF))
	{
		cFile.Close();
		cFile.Kill();
		gcLogger.Error2(__METHOD__, " Could not read [", szFilename, "].", NULL);
		return NULL;
	}
	pvFile = (uint8*)malloc((size)iFileSize);
	if ((pvFile == NULL) || (cFile.Read(pvFile, (size)iFileSize, 1) != 1))
	{
		free(pvFile);
		cFile.Close();
		cFile.Kill();
		gcLogger.Error2(__METHOD__, " Could not read [", szFilename, "].", NULL);
		return NULL;
	}
	cFile.Close();
	cFile.Kill();

	*puiFileSize = (size)iFileSize;
	return pvFile;
}
