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
#ifndef __INDEXED_DESCRIPTORS_H__
#define __INDEXED_DESCRIPTORS_H__
#include "BaseLib/Chars.h"
#include "DurableFile.h"
#include "DurableFileController.h"
#include "IndexedGeneral.h"


enum EDataDescriptorAccessStyle
{
	DDAS_DiskDirect,
	DDAS_MemoryArray,
	DDAS_HugeSupport,
};


class CIndexedDataDescriptor;
class CIndexAccess;
class CIndexedDescriptors
{
public:
	CIndexAccess*				mpcAccess;

	void	Init(CIndexAccess* pcAccess);
	void	Kill(void);

	BOOL	Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL	Set(CIndexedDataDescriptor* pcDescriptor);
	BOOL	Remove(OIndex oi);

	BOOL	Save(void);
	BOOL	Load(void);
	void	UpdateFile(void);

	BOOL	RemoveFile(void);

	OIndex	Length(void);
	OIndex	NumElements(void);
	OIndex	NumCachedDatas(void);
};


#endif // __INDEXED_DESCRIPTORS_H__

