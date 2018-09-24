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
#ifndef __INDEXED_DATA_DESCRIPTOR_H__
#define __INDEXED_DATA_DESCRIPTOR_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Define.h"
#include "BaseLib/AbstractFile.h"
#include "BaseLib/FileIndex.h"
#include "IndexedGeneral.h"


struct SIndexedDataDescriptor
{
protected:
	unsigned int	muiDataSize;

	CFilePosIndex	mcFileIndex;
};


class CIndexedDataDescriptor : public SIndexedDataDescriptor
{
private:
	void*			mpvCache;  //NULL if object is not cached.

public:
	void 			Init(unsigned int uiDataSize);
	BOOL 			IsAllocated(void);
	unsigned int	GetDataSize(void);
	void			SetDataSize(unsigned int uiDataSize);
	void			Cache(void* pvCache);
	BOOL			HasFile(void);
	BOOL			IsCached(void);
	void*			GetCache(void);
	void			SetIndexes(int iFileIndex, filePos iIndexInFile);
	int				GetFileIndex(void);

	filePos			GetPositionInFile(void);
	unsigned int	GetDataIndexInFile(void);
	BOOL			Update(CIndexedDataDescriptor* pcNew);
};

typedef CArrayTemplate<CIndexedDataDescriptor> CArrayIndexDescriptor;


#endif // __INDEXED_DATA_DESCRIPTOR_H__


