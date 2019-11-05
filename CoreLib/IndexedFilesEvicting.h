#ifndef __INDEXED_FILES_EVICTING_H__
#define __INDEXED_FILES_EVICTING_H__
#include "DurableFileController.h"
#include "IndexedFiles.h"
#include "IndexedCache.h"


class CIndexedFilesEvictionCallback;
class CIndexedFilesEvicting
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
	BOOL			SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	BOOL			Flush(BOOL bClearCache);
	BOOL			IsFlushed(void);
	BOOL			IsCaching(void);

	int				NumCached(void);
	int				NumCached(int iSize);
	int				NumFiles(void);
	int64			NumData(int iDataSize);

	unsigned int	GetCachedDataSize(CIndexedDataDescriptor* pcDescriptor);

	BOOL			Evict(OIndex oi, CIndexedDataDescriptor* pcDescriptor);  //Find a better name.  Maybe
	void			InvalidateData(CIndexedDataDescriptor* pcDescriptor);

	size_t			GetSystemMemorySize(void);

	unsigned int	TestGetCachedObjectSize(OIndex oi);

protected:
	BOOL			CacheAllocate(OIndex oi, CIndexedDataDescriptor* pcDescriptor);
	BOOL			DescriptorsEvicted(CArrayVoidPtr* paEvictedCacheDescriptors);

	BOOL			WriteEvictedData(SIndexedCacheDescriptor* psCached, BOOL bClearCache, BOOL bNoEviction);
	BOOL			WriteEvictedData(CArrayVoidPtr* papsIndexedCacheDescriptors);
	BOOL			WriteEvictedData(CIndexedDataDescriptor* pcDescriptor, BOOL bClearCache);

	BOOL			CompareDiskToMemory(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL			ClearDescriptorCache(SIndexedCacheDescriptor* psCached);
};


#endif //__INDEXED_FILES_EVICTING_H__

