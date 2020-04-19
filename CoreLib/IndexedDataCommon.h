#ifndef __INDEXED_DATA_COMMON_H__
#define __INDEXED_DATA_COMMON_H__
#include "IndexedGeneral.h"
#include "IndexedDataDescriptor.h"
#include "IndexedFilesEvicting.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexWriteThrough.h"
#include "IndexedEvictionCallback.h"

class CIndexedDataCommon 
{
protected:
	CIndexedFilesEvicting		mcData;
	EIndexWriteThrough			meWriteThrough;
	CIndexedEvictionCallback*	mpcEvictionCallback;

public:
			void			Init(CIndexedEvictionCallback* pcEvictionCallback);
	virtual BOOL			Kill(void) =0;

			BOOL			IsCaching(void);
	
			BOOL			Add(OIndex oi, void* pvData, unsigned int uiDataSize);

			BOOL			Set(OIndex oi, void* pvData);
			BOOL			Set(OIndex oi, void* pvData, unsigned int uiDataSize);

			BOOL			Put(OIndex oi, void* pvData, unsigned int uiDataSize);

			unsigned int	Size(OIndex oi);
			BOOL			Get(OIndex oi, void* pvData);
			BOOL			Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxSize);

			BOOL			Contains(OIndex oi);

			int64			NumElements(void);

	virtual int64			NumIndices(void) =0;
	virtual int64			NumIndicesCached(void) =0;
			int64			NumDatas(void);
			int				NumDataCached(void);
			int				NumDataCached(int iSize);
			int				NumFiles(void);
			int64			NumDatas(unsigned int uiDataSize);

			BOOL			Remove(OIndex oi);

	virtual BOOL			IsDirty(OIndex oi) =0;

	virtual BOOL			EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor) =0;

			unsigned int	TestGetCachedObjectSize(OIndex oi);

protected:
	virtual BOOL			GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction = FALSE) =0;
	virtual BOOL			SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction = FALSE) =0;
	virtual BOOL			UpdateDescriptorCache(OIndex oi, void* pvCache, unsigned int uiDataSize) =0;
	virtual BOOL			RemoveDescriptor(OIndex oi) =0;

	virtual BOOL			Flush(BOOL bClearCache) =0;

			BOOL			DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors);
};


#endif // __INDEXED_DATA_COMMON_H__

