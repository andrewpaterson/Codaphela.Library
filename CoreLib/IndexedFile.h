/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INDEXED_FILE_H__
#define __INDEXED_FILE_H__
#include "BaseLib/FileBasic.h"
#include "BaseLib/ChunkFile.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ListTemplate.h"
#include "DurableFile.h"
#include "DurableFileController.h"


class CIndexedFile
{
private:
	int				miFileIndex;  //This is the index of the file in the CIndexedFiles.mcFiles array.

public:
	CDurableFile	mcFile;

	int				miDataSize;
	filePos			miNumDatas;
	int				miFileNumber;  //There may be more than one file of the same size.

	BOOL			Init(CDurableFileController* pcDurableFileControl, int iFileIndex, char* szFileName, char* szRewriteName, int iDataSize, int iFileNum);
	void			Kill(void);
	filePos			CalculateNumDatas(void);
	BOOL			IsFull(void);

	filePos			Write(void* pvData);
	BOOL			Write(filePos iIndex, void* pvData);
	BOOL			Write(filePos iIndex, void* pvData, filePos iCount);

	BOOL			Read(filePos iIndex, void* pvData);
	BOOL			Read(filePos iIndex, void* pvData, filePos iCount);

	BOOL			Delete(filePos iIndex);
	BOOL			Delete(filePos iIndex, filePos iCount);

	int				GetFileIndex(void);
	BOOL			IsFileIndex(int iFileIndex);

	CFileBasic*		DumpGetPrimaryFile(void);
	char*			GetFileName(void);
	char*			GetRewriteName(void);

	void			Dump(void);

protected:
	filePos			Write(void* pvData, filePos iCount);
};


typedef CListTemplate<CIndexedFile> CListIndexedFile;


#endif // __INDEXED_FILE_H__

