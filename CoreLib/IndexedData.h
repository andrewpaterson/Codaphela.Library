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
#ifndef __INDEXED_DATA_H__
#define __INDEXED_DATA_H__
#include "BaseLib/TemporaryMemory.h"
#include "BaseLib/MemoryFile.h"
#include "DurableFileController.h"
#include "IndexedFilesEvictionCallback.h"
#include "IndexedDataCommon.h"
#include "IndexedDescriptorsFile.h"
#include "IndexedDataConfig.h"


class CIndexedData : public CIndexedDataCommon, public CIndexedFilesEvictionCallback
{
	//The following are in CIndexedDataCommon.
	//	CIndexedFilesEvicting			mcData;
	//	EIndexWriteThrough				meWriteThrough;
	//	CIndexedDataEvictionCallback*	mpcIndexedDataEvictionCallback;

protected:
	//The following are in CIndexedDescriptorsFile:
	//	CIndexTreeEvicting					mcIndexTree;  <---
	//	CIndexedDataCommon*					mpcIndexedData;
	//	CIndexTreeEvictionCallbackWrapper	mcEvictionCallbackWrapper;

	CIndexedDescriptorsFile		mcIndices;
	CDurableFileController*		mpcDurableFileControl;
	CLife<CIndexedDataConfig>	mcConfig;
	CIndexedDataConfig*			mpcConfig;

public:
	void 			Init(CDurableFileController* pcController, CLifeInit<CIndexedDataConfig> cConfig);
	BOOL 			Kill(void);

	BOOL			Flush(void);
	BOOL			Flush(BOOL bClearDataCache);

	BOOL			IsDurable(void);
	BOOL			IsFlushed(void);

	int64			NumIndices(void);
	int64			NumIndicesCached(void);
	int64			NumIndicesCached(size_t iSize);

	BOOL			IsDirty(OIndex oi);
	BOOL			EvictKey(OIndex oi);
	BOOL			EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

	BOOL			FlushKey(OIndex oi);

	CDurableFileController* GetDurableFileControl(void);
	size_t			GetIndiciesSystemMemorySize(void);
	size_t			GetDataSystemMemorySize(void);
	unsigned char	GetRootFlags(void);

	void			DumpIndex(void);
	BOOL			ValidateIndex(void);
	BOOL			ValidateConfigInitialised(void);
	BOOL			ValidateConfigKilled(void);

	OIndex 			StartIteration(SIndexTreeFileIterator* psIterator, void* ppvData, size_t* piDataSize, size_t iMaxDataSize);
	OIndex			Iterate(SIndexTreeFileIterator* psIterator, void* ppvData, size_t* piDataSize, size_t iMaxDataSize);

protected:
	BOOL			GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction = FALSE);
	BOOL			SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction = FALSE);
	BOOL			UpdateDescriptorCache(OIndex oi, void* pvCache, unsigned int uiDataSize);
	BOOL			RemoveDescriptor(OIndex oi);

	BOOL			DescriptorEvicted(OIndex oi, void* pvCache, unsigned int uiDataSize);
};


#endif // !__INDEXED_DATA_H__

