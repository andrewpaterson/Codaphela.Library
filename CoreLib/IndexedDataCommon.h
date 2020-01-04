#ifndef __INDEXED_DATA_COMMON_H__
#define __INDEXED_DATA_COMMON_H__
#include "IndexedGeneral.h"
#include "IndexedDataDescriptor.h"
#include "IndexedFilesEvicting.h"
#include "EvictionCallback.h"
#include "IndexWriteThrough.h"
#include "EvictionCallbackWrapper.h"

class CIndexedDataCommon 
{
protected:
	CIndexedFilesEvicting		mcData;
	EIndexWriteThrough			meWriteThrough;
	CEvictionCallback*			mpcEvictionCallback;

public:
			void			Init(CEvictionCallback* pcEvictionCallback);
	virtual BOOL			Kill(void) =0;

			BOOL			IsCaching(void);
	
			BOOL			Add(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

			BOOL			Set(OIndex oi, void* pvData, unsigned int uiTimeStamp);
			BOOL			Set(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

			BOOL			SetOrAdd(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

			unsigned int	Size(OIndex oi);
			BOOL			Get(OIndex oi, void* pvData);
			BOOL			Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxSize);

			BOOL			Contains(OIndex oi);

	virtual int64			NumElements(void) =0;
			int				NumDataCached(void);
			int				NumDataCached(int iSize);
			int				NumFiles(void);
			int64			NumData(unsigned int uiDataSize);

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

