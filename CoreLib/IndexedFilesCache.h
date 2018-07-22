#ifndef __INDEXED_FILES_CACHE_H__
#define __INDEXED_FILES_CACHE_H__
#include "DurableFileController.h"
#include "IndexedFiles.h"
#include "IndexedCache.h"


class CIndexedFilesEvictionCallback;
class CIndexedFilesCache
{
protected:
	CIndexedCache					mcDataCache;
	CIndexedFiles					mcDataFiles;

	BOOL							mbCaching;
	BOOL							mbWriteThrough;

	CIndexedFilesEvictionCallback*	mpcEvictionCallback;

public:
	void			Init(CDurableFileController* pcDurableFileControl, char* szDataExtension, char* szDescricptorName, char* szDescricptorRewrite, size_t iCacheSize, BOOL bWriteThrough, CIndexedFilesEvictionCallback* pcEvictionCallback);
	void			Kill(void);

	BOOL			GetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL			SetData(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL			SetData(CIndexedDataDescriptor* pcDescriptor, void* pvData, OIndex oi, unsigned int uiTimeStamp);

	BOOL			Flush(BOOL bClearCache);
	BOOL			Uncache(void);
	BOOL			IsCaching(void);

	int				NumCached(void);
	int				NumCached(int iSize);
	int				NumFiles(void);
	int64			NumData(int iDataSize);

	void			InvalidateData(CIndexedDataDescriptor* pcDescriptor);
	BOOL			EvictFromCache(CIndexedDataDescriptor* pcDescriptor);

	unsigned int	TestGetCachedObjectSize(OIndex oi);
	int				TestNumIgnoredCacheElements(void);

protected:
	BOOL			CacheRead(OIndex oi, CIndexedDataDescriptor* pcDescriptor);
	BOOL			CacheWrite(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, BOOL* pbWritten);

	BOOL			WriteEvictedData(CIndexedDataDescriptor* pcDescriptor, SIndexedCacheDescriptor* psCached);
	BOOL			WriteEvictedData(SIndexedCacheDescriptor* psCached);
	BOOL			WriteEvictedData(CArrayVoidPtr* papsIndexedCacheDescriptors);

	BOOL			CompareDiskToMemory(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL			ClearDescriptorCache(SIndexedCacheDescriptor* psCached);
};


#endif //__INDEXED_FILES_CACHE_H__

