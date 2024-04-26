/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INDEXED_DATA_H__
#define __INDEXED_DATA_H__
#include "TemporaryMemory.h"
#include "MemoryFile.h"
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
	bool 			Kill(void);

	bool			Flush(void);
	bool			Flush(bool bClearDataCache);

	bool			IsDurable(void);
	bool			IsFlushed(void);

	int64			NumIndices(void);
	int64			NumIndicesCached(void);
	int64			NumIndicesCached(size_t iSize);

	bool			IsDirty(OIndex oi);
	bool			EvictKey(OIndex oi);
	bool			EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

	bool			FlushKey(OIndex oi);

	CDurableFileController* GetDurableFileControl(void);
	size_t			GetIndiciesSystemMemorySize(void);
	size_t			GetDataSystemMemorySize(void);
	uint8			GetRootFlags(void);

	void			DumpIndex(void);
	bool			ValidateIndex(void);
	bool			ValidateConfigInitialised(void);
	bool			ValidateConfigKilled(void);

	OIndex 			StartIteration(SIndexTreeFileIterator* psIterator, void* ppvData, size_t* piDataSize, size_t iMaxDataSize);
	OIndex			Iterate(SIndexTreeFileIterator* psIterator, void* ppvData, size_t* piDataSize, size_t iMaxDataSize);

protected:
	bool			GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false);
	bool			SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false);
	bool			UpdateDescriptorCache(OIndex oi, void* pvCache, uint32 uiDataSize);
	bool			RemoveDescriptor(OIndex oi);

	bool			DescriptorEvicted(OIndex oi, void* pvCache, uint32 uiDataSize);
};


#endif // !__INDEXED_DATA_H__

