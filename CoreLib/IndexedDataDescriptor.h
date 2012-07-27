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
#include "IndexedGeneral.h"


#define INDEXED_DESCRIPTOR_DIRTY	0x80000000

//Find out what the difference between IsAllocated is and INDEXED_DESCRIPTOR_REMOVED.
#define INDEXED_DESCRIPTOR_REMOVED	0x40000000

#define INDEXED_DESCRIPTOR_MASK		0xc0000000


class CIndexedDataDescriptor
{
private:
	OIndex			moi;

	unsigned int	muiDataSize;
	unsigned int	msFlags;  //These include the user flags.

	int				miFileIndex;  //Which file.
	filePos			miIndexInFile;  //Where in the file.

	void*			mpvCache;  //Null if object is not cached.
	unsigned int	muiTimeStamp;

public:
	void 			Init(OIndex oi, unsigned int uiDataSize);
	BOOL 			IsAllocated(void);
	BOOL 			IsDirty(void);
	void 			Dirty(BOOL bDirty);
	BOOL 			SetUserFlags(int iFlags);
	int  			GetUserFlags(void);
	unsigned int	GetDataSize(void);
	void			Cache(void* pvCache);
	OIndex			GetIndex(void);
	BOOL			HasFile(void);
	BOOL			IsCached(void);
	void*			GetCache(void);
	void			TimeStamp(unsigned int uiTimeStamp);
	void			File(int iFileIndex, filePos iIndexInFile);
	int				GetFileIndex(void);
	filePos			GetIndexInFile(void);
	unsigned int	GetTimeStamp(void);
};

typedef CArrayTemplate<CIndexedDataDescriptor> CArrayIndexDescriptor;


#endif // __INDEXED_DATA_DESCRIPTOR_H__


