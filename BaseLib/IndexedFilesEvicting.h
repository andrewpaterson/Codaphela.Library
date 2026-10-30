#ifndef __INDEXED_FILES_EVICTING_H__
#define __INDEXED_FILES_EVICTING_H__
#include "DurableFileController.h"
#include "IndexedFiles.h"
#include "IndexedCacheResult.h"
#include "IndexedCache.h"
#include "IndexWriteThrough.h"


enum EClearCache
{
	CC_No,
	CC_Yes,
	CC_YesDontInvalidate
};


class CIndexedFilesEvictionCallback;
class CIndexedFilesEvicting : public CMemoryCacheEvictionCallback
{
protected:
	CIndexedCache					mcDataCache;
	CIndexedFiles					mcDataFiles;

	bool							mbCaching;
	EIndexWriteThrough				meWriteThrough;

	CIndexedFilesEvictionCallback*	mpcEvictionCallback;

public:
	void			Init(CDurableFileController* pcDurableFileControl, const char* szSubDirectory, const char* szDataExtension, const char* szDescriptorName, const char* szDescriptorRewrite, lsize iCacheSize, EIndexWriteThrough eWriteThrough, CIndexedFilesEvictionCallback* pcEvictionCallback);
	void			Kill(void);

	bool			GetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, lsize uiMaxDataSize);
	bool			SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, const void* pvData, lsize uiDataSize);

	bool			Flush(bool bClearCache);
	bool			IsFlushed(void);
	bool			IsCaching(void);
	bool			IsWriteThrough(void);

	lsize			NumCached(void);
	lsize			NumCached(lsize iSize);
	size			NumFiles(void);
	int64			NumDatas(void);
	int64			NumDatas(lsize iDataSize);

	bool			Evict(OIndex oi, CIndexedDataDescriptor* pcDescriptor);
	void			InvalidateData(CIndexedDataDescriptor* pcDescriptor);

	lsize			GetSystemMemorySize(void);

	bool			ValidateCache(void);

	lsize			TestGetCachedObjectSize(OIndex oi);

protected:
	void*				SetCacheData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, const void* pvData, lsize uiDataSize);
	CIndexedCacheResult	CacheAllocate(OIndex oi, lsize uiDataSize);

	CFilePosIndex		WriteThroughData(CIndexedDataDescriptor* pcDescriptor, const void* pvData, lsize uiDataSize);
	bool				WriteEvictedData1b(SIndexedCacheDescriptor* psCached, void* pvData, EClearCache eClearCache, bool bNoEviction);
	bool				EvictWriteData(CIndexedDataDescriptor* pcDescriptor, EClearCache eClearCache);
	bool				WriteEvictedData2(CIndexedDataDescriptor* pcDescriptor, OIndex oi, void* pvData, EClearCache eClearCache, bool bNoEviction);

	bool				CompareDiskToMemory(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	bool				ClearDescriptorCache(SIndexedCacheDescriptor* psCached);

	bool				CacheDataEvicted(void* pvData, SMemoryCacheDescriptor* psDescriptor);
};


#endif // __INDEXED_FILES_EVICTING_H__

