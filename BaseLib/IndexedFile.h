/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "FileBasic.h"
#include "ChunkFile.h"
#include "Chars.h"
#include "ArrayTemplate.h"
#include "ArrayBit.h"
#include "ListTemplate.h"
#include "DurableFile.h"
#include "DurableFileController.h"


#define INDEXED_FILE_WRITE_ERROR	((unsigned int)-1)
#define INDEX_FILE_EMPTY_CHAR		0x55


class CIndexedFile
{
private:
	int				miFileIndex;  //This is the index of the file in the CIndexedFiles.mcFiles array.

	CDurableFile	mcFile;

	unsigned int	muiDataSize;
	filePos			miNumDatas;
	int				miFileNumber;  //There may be more than one file of the same size.

public:
	bool			Init(CDurableFileController* pcDurableFileControl, int iFileIndex, char* szFilename, char* szRewriteName, unsigned int uiDataSize, int iFileNum);
	void			Kill(void);
	filePos			CalculateNumDatas(void);
	bool			IsFull(void);

	unsigned int	Write(void* pvData);
	bool			Write(filePos iIndex, void* pvData);
	bool			Write(filePos iIndex, void* pvData, filePos iCount);

	bool			Read(filePos iIndex, void* pvData);
	bool			Read(filePos iIndex, void* pvData, filePos iCount);

	bool			Delete(filePos iIndex);
	bool			Delete(filePos iIndex, filePos iCount);

	int				GetFileIndex(void);
	bool			IsFileIndex(int iFileIndex);

	CFileBasic*		DumpGetPrimaryFile(void);
	char*			GetFilename(void);
	int				GetFileNumber(void);
	char*			GetRewriteName(void);
	unsigned int	GetDataSize(void);
	filePos			NumDatas(void);
	int				GetUsedDataIndices(CArrayBit* pab);

	filePos			GetFileSize(void);
	void			Dump(void);

protected:
	unsigned int	Write(void* pvData, filePos iCount);
};


typedef CListTemplate<CIndexedFile> CListIndexedFile;
typedef CArrayTemplate<CIndexedFile*> CArrayIndexedFilePtr;


#endif // !__INDEXED_FILE_H__

