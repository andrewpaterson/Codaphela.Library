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
#include "IndexedConfig.h"
#include "IndexedCache.h"
#include "IndexedData.h"
#include "IndexedFiles.h"
#include "DurableFileController.h"
#include "IndexedDescriptorsFile.h"


//This is the database class.
class CIndexedData
{
protected:
	//Data
	CIndexedCache			mcDataCache;
	CIndexedFiles			mcDataFiles;

	//Index
	CIndexedDescriptorsFile	mcIndices;

	CDurableFileController	mcDurableFileControl;

	CTemporaryMemory		mcTemp;
	BOOL					mbCaching;
	BOOL					mbWriteThrough;

public:
	void 			Init(char* szWorkingDirectory, char* szRewriteDirectory, unsigned int uiCacheSize);  //Old style for testing.
	void 			Init(CIndexedConfig* pcConfig);
	void 			Kill(void);

	BOOL			Add(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);
	BOOL			Set(OIndex oi, void* pvData, unsigned int uiTimeStamp);
	BOOL			Set(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);
	BOOL			SetOrAdd(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	unsigned int	Size(OIndex oi);
	unsigned int	Flags(OIndex oi);
	BOOL			Get(OIndex oi, void* pvData);
	void*			Get(OIndex oi, int* piDataSize);

	BOOL			Contains(OIndex oi);

	BOOL			Remove(OIndex oi);

	BOOL			Flush(BOOL bClearCache);
	void			DurableBegin(void);
	void			DurableEnd(void);

	void			KillEnd(void);

	BOOL			IsCaching(void);
	BOOL			IsDurable(void);
	int				NumCached(void);
	int				NumCached(int iSize);
	int				NumFiles(void);
	int64			NumData(int iDataSize);
	int64			NumElements(void);

	BOOL			EvictFromCache(CIndexedDataDescriptor* pcDescriptor);
	BOOL			Uncache(void);

	int				TestNumCachedIndexes(void);
	int				TestNumIgnoredCacheElements(void);
	unsigned int	TestGetCachedObjectSize(OIndex oi);
	CDurableFileController* GetDurableFileControl(void);

protected:
	void 			InitIndices(BOOL bDirtyTesting);

	BOOL			SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiTimeStamp);
	BOOL			SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	BOOL			GetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL			GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

	BOOL			ClearDescriptorCache(SIndexedCacheDescriptor* psCached);
	BOOL			EvictFromCache(SIndexedCacheDescriptor* psExisting);
	BOOL			EvictOverlappingFromCache(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors);

	void			InvalidateData(CIndexedDataDescriptor* pcDescriptor);
	BOOL			CacheRead(OIndex oi, CIndexedDataDescriptor* pcDescriptor);
	BOOL			CacheWrite(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, BOOL* pbWritten);
	BOOL			Write(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiTimeStamp);

	BOOL			WriteEvictedData(CArrayVoidPtr* papsIndexedCacheDescriptors);
	BOOL			WriteEvictedData(SIndexedCacheDescriptor* psCached);
	BOOL			WriteEvictedData(CIndexedDataDescriptor* pcDescriptor, SIndexedCacheDescriptor* psCached);

	BOOL			CompareDiskToMemory(CIndexedDataDescriptor* pcDescriptor, void* pvData);
};


#endif // __INDEXED_DATA_H__

