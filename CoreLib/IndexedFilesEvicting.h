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

	void			InvalidateData(CIndexedDataDescriptor* pcDescriptor);

	unsigned int	TestGetCachedObjectSize(OIndex oi);
	int				TestNumIgnoredCacheElements(void);

protected:
	BOOL			CacheAllocate(OIndex oi, CIndexedDataDescriptor* pcDescriptor);

	BOOL			WriteEvictedData(SIndexedCacheDescriptor* psCached, BOOL bClearCache);
	BOOL			WriteEvictedData(CArrayVoidPtr* papsIndexedCacheDescriptors);

	BOOL			CompareDiskToMemory(CIndexedDataDescriptor* pcDescriptor, void* pvData);
	BOOL			ClearDescriptorCache(SIndexedCacheDescriptor* psCached);
};


#endif //__INDEXED_FILES_EVICTING_H__

