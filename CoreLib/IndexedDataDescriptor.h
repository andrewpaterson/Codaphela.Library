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
#ifndef __INDEXED_DATA_DESCRIPTOR_H__
#define __INDEXED_DATA_DESCRIPTOR_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Define.h"
#include "BaseLib/AbstractFile.h"
#include "BaseLib/FileIndex.h"
#include "IndexedGeneral.h"


struct SIndexedFileDataDescriptor
{
public:
	unsigned int	muiDataSize;
	CFilePosIndex	mcFileIndex;
};


class CIndexedDataDescriptor
{
private:
	SIndexedFileDataDescriptor	msFileDescriptor;

	void*						mpvCache;  //NULL if object is not cached.
	unsigned int				muiCacheDataSize;

public:
	void 			Init(unsigned int uiDataSize, CFilePosIndex* pcFilePosIndex);
	void 			Init(unsigned int uiDataSize, void* pvCache);
	void 			Init(unsigned int uiFileDataSize, CFilePosIndex* pcFilePosIndex, unsigned int uiCacheDataSize, void* pvCache);

	unsigned int	GetFileDataSize(void);
	unsigned int	GetCacheDataSize(void);
	unsigned int	GetDataSize(void);
	void			Cache(void* pvCache, unsigned int uiDataSize);
	void			ClearCache(void);
	BOOL			HasFile(void);
	BOOL			IsCached(void);
	void*			GetCache(void);
	void			SetIndexes(int iFileIndex, filePos iIndexInFile);
	int				GetFileIndex(void);

	filePos			GetPositionInFile(void);
	CFileDataIndex	GetFileDataIndex(void);
	CFilePosIndex*	GetFilePosIndex(void);
	BOOL			Update(CIndexedDataDescriptor* pcNew);
};

typedef CArrayTemplate<CIndexedDataDescriptor> CArrayIndexDescriptor;


#endif // !__INDEXED_DATA_DESCRIPTOR_H__


