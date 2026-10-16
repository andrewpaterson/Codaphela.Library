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
#include "ImageCopier.h"
#include "ImageReaderCommon.h"


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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CopyRGBAIntoImage(uint8* puiRGBA, bool bGrey, Ptr<CImage> pcImage)
{
	CImage			cImageImport;
	CImageCopier	cCopier;
	bool			bResult;

	if (bGrey)
	{
		cImageImport.Init(pcImage->GetWidth(), pcImage->GetHeight(), puiRGBA, PT_uint8, IMAGE_DIFFUSE_GREY, IMAGE_IGNORED, IMAGE_IGNORED, IMAGE_OPACITY, CHANNEL_STOP);
	}
	else
	{
		cImageImport.Init(pcImage->GetWidth(), pcImage->GetHeight(), puiRGBA, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, IMAGE_OPACITY, CHANNEL_STOP);
	}

	bResult = cCopier.Init(&cImageImport, pcImage);
	if (bResult)
	{
		cCopier.Copy(0, 0);
	}
	cCopier.Kill();
	cImageImport.Kill();
	return bResult;
}
