#ifndef __INDEXED_DATA_COMMON_H__
#define __INDEXED_DATA_COMMON_H__
#include "IndexedGeneral.h"
#include "IndexedDataDescriptor.h"
#include "IndexedFilesEvicting.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexWriteThrough.h"
#include "IndexedDataEvictionCallback.h"


class CIndexedDataCommon 
{
protected:
	CIndexedFilesEvicting			mcData;
	EIndexWriteThrough				meWriteThrough;
	CIndexedDataEvictionCallback*	mpcIndexedDataEvictionCallback;

public:
			void			Init(CIndexedDataEvictionCallback* pcIndexedDataEvictionCallback);
	virtual bool			Kill(void) =0;

			bool			IsCaching(void);
			bool			IsWriteThrough(void);
			bool			IsFlushed(void);
	
			bool			Add(OIndex oi, void* pvData, size_t uiDataSize);

			bool			Set(OIndex oi, void* pvData);
			bool			Set(OIndex oi, void* pvData, size_t uiDataSize);

			bool			Put(OIndex oi, void* pvData, size_t uiDataSize);

			bool			Get(OIndex oi, void* pvData, size_t uiMaxSize = 0);
			bool			Get(OIndex oi, size_t* puiDataSize, void* pvData, size_t uiMaxSize);

			uint32	Size(OIndex oi);
			bool			Contains(OIndex oi);

			int64			NumElements(void);

	virtual int64			NumIndices(void) =0;
	virtual int64			NumIndicesCached(void) =0;
			int64			NumDatas(void);
			int				NumDataCached(void);
			int				NumDataCached(size_t iSize);
			int				NumFiles(void);
			int64			NumDatas(size_t uiDataSize);

			bool			Remove(OIndex oi);

	virtual bool			IsDirty(OIndex oi) =0;

	virtual bool			EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor) =0;

			uint32	TestGetCachedObjectSize(OIndex oi);

protected:
	virtual bool			GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false) =0;
	virtual bool			SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false) =0;
	virtual bool			UpdateDescriptorCache(OIndex oi, void* pvCache, size_t uiDataSize) =0;
	virtual bool			RemoveDescriptor(OIndex oi) =0;

	virtual bool			Flush(bool bClearCache) =0;

			bool			GetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, size_t* puiDataSize, void* pvData, size_t uiMaxSize);
};


#endif // !__INDEXED_DATA_COMMON_H__

