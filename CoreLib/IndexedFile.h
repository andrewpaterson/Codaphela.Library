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
#include "DurableFileController.h"


class CIndexedFile
{
public:
	int				miFileIndex;
	CDurableFile	mcFile;
	int				miDataSize;
	int				miNumDatas;
	int				miFileNumber;  //There may be more than one file of the same size.
	CChars			mszFileName;
	CChars			mszRewriteName;
	BOOL			mbNew;

	void	Init(CDurableFileController* pcDurableFileControl, int iFileIndex, char* szFileName, char* szRewriteName, int iDataSize, int iFileNum);
	BOOL	Kill(void);
	void	GetFileName(char* szName);
	BOOL	IsFull(void);
	int		Write(void* pvData);
	BOOL	Write(int iIndex, void* pvData);
	int		Write(void* pvData, int iCount);
	BOOL	Write(int iIndex, void* pvData, int iCount);
	BOOL	Read(int iIndex, void* pvData);
	BOOL	Read(int iIndex, void* pvData, int iCount);

	void	Dump(void);
};


typedef CArrayTemplate<CIndexedFile> CArrayIndexedFile;


#endif // __INDEXED_FILE_H__

