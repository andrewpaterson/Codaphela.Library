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
#include "IndexedConfig.h"
#include "IndexedCache.h"
#include "IndexedData.h"
#include "IndexedFiles.h"
#include "IndexDescriptorsFile.h"
#include "IndexSmartAccess.h"
#include "DurableFileController.h"


//This is the database class (not including transactions).
class CIndexedData
{
friend class CSubTransaction;
friend class CTransactionController;

protected:
	CDurableFileController	mcDurableFileControl;

	CIndexedCache			mcObjectCache;
	CIndexedFiles			mcObjectFiles;

	CIndexedDescriptors		mcIndices;
	CIndexDescriptorsFile	mcIndicesFile;
	CIndexSmartAccess		mcIndicesAccess;

	void*					mpvTemp;
	unsigned int			muiTempSize;
	BOOL					mbDurable;
	BOOL					mbCaching;
	BOOL					mbWriteThrough;

public:
	void 			Init(char* szWorkingDirectory, unsigned int uiCacheSize, BOOL bDurable);  //Old style for testing.
	void 			Init(CIndexedConfig* pcConfig);
	void 			Kill(void);

	BOOL			Add(OIndex OI, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);
	BOOL			Set(OIndex OI, void* pvData, unsigned int uiTimeStamp);
	BOOL			Set(OIndex OI, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);
	BOOL			SetOrAdd(OIndex OI, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	unsigned int	Size(OIndex OI);
	unsigned int	Flags(OIndex OI);
	BOOL			Get(OIndex OI, void* pvData);

	BOOL			Remove(OIndex OI);

	BOOL			Flush(void);
	void			DurableBegin(void);
	void			DurableEnd(void);

	BOOL			IsCaching(void);
	void			EvictFromCache(CIndexDescriptor* pcDescriptor);
	int				NumCached(void);
	int				NumFiles(void);
	OIndex			NumInFile(int iDataSize);
	OIndex			NumElements(void);
	int				TestNumCachedIndexes(void);
	int				TestIndexedDescriptorsLength(void);
	int				TestNumIgnoredCacheElements(void);
	CIndexAccess*	TestGetIndexAccess(void);
	unsigned int	TestGetCachedObjectSize(OIndex OI);
	CDurableFileController* GetDurableFileControl(void);

protected:
	void 			InitIndices(CIndexedConfig* pcConfig);

	BOOL			GetData(CIndexDescriptor* pcDescriptor, void* pvData);
	BOOL			GetDescriptor(OIndex OI, CIndexDescriptor* pcDescriptor);

	BOOL			ClearDescriptorCache(SIndexedCacheDescriptor* psCached);
	void			EvictFromCache(SIndexedCacheDescriptor* psExisting);
	void			EvictOverlappingFromCache(CArrayPointer* papsEvictedIndexedCacheDescriptors);

	void			Invalidate(CIndexDescriptor* pcDescriptor);
	BOOL			Uncache(void);
	BOOL			CacheRead(CIndexDescriptor* pcDescriptor);
	BOOL			CacheWrite(CIndexDescriptor* pcDescriptor, void* pvData, BOOL* pbWritten);
	BOOL			Write(CIndexDescriptor* pcDescriptor, void* pvData, unsigned int uiTimeStamp);

	BOOL			WriteEvictedData(CArrayPointer* papsIndexedCacheDescriptors);
	BOOL			WriteEvictedData(SIndexedCacheDescriptor* psCached);
	void			WriteEvictedData(CIndexDescriptor* pcDescriptor, SIndexedCacheDescriptor* psCached);
	BOOL			WriteData(CIndexDescriptor* pcDescriptor, void* pvData);

	void			AddFile(CDurableFile* pcFile);
	BOOL			CompareDiskToMemory(CIndexDescriptor* pcDescriptor, void* pvData);
};


#endif // __INDEXED_DATA_H__

