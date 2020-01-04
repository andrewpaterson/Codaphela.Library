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
class CIndexedFilesEvicting
{
protected:
	CIndexedCache					mcDataCache;
	CIndexedFiles					mcDataFiles;

	BOOL							mbCaching;
	EIndexWriteThrough				meWriteThrough;

	CIndexedFilesEvictionCallback*	mpcEvictionCallback;

public:
	void			Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iCacheSize, EIndexWriteThrough eWriteThrough, CIndexedFilesEvictionCallback* pcEvictionCallback);
	void			Kill(void);

	BOOL			GetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL			SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	BOOL			Flush(BOOL bClearCache);
	BOOL			IsFlushed(void);
	BOOL			IsCaching(void);

	int				NumCached(void);
	int				NumCached(int iSize);
	int				NumFiles(void);
	int64			NumData(int iDataSize);

	unsigned int	GetCachedDataSize(CIndexedDataDescriptor* pcDescriptor);
	void*			GetCachedData(SIndexedCacheDescriptor* psDescriptor);

	BOOL			Evict(OIndex oi, CIndexedDataDescriptor* pcDescriptor);
	void			InvalidateData(CIndexedDataDescriptor* pcDescriptor);

	size_t			GetSystemMemorySize(void);

	unsigned int	TestGetCachedObjectSize(OIndex oi);

protected:
	void*				SetCacheData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize);
	CIndexedCacheResult	CacheAllocate(OIndex oi, unsigned int uiDataSize);
	BOOL				DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors);

	CFilePosIndex		WriteThroughData(CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize);
	BOOL				WriteEvictedData(SIndexedCacheDescriptor* psCached, EClearCache eClearCache, BOOL bNoEviction);
	BOOL				WriteEvictedData(CIndexedDataDescriptor* pcDescriptor, EClearCache eClearCache);
	BOOL				WriteEvictedData(CIndexedDataDescriptor* pcDescriptor, OIndex oi, void* pvData, EClearCache eClearCache, BOOL bNoEviction);

	BOOL				CompareDiskToMemory(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL				ClearDescriptorCache(SIndexedCacheDescriptor* psCached);
};


#endif //__INDEXED_FILES_EVICTING_H__

