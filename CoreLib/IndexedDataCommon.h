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
	virtual BOOL			Kill(void) =0;

			BOOL			IsCaching(void);
			BOOL			IsWriteThrough(void);
			BOOL			IsFlushed(void);
	
			BOOL			Add(OIndex oi, void* pvData, size_t uiDataSize);

			BOOL			Set(OIndex oi, void* pvData);
			BOOL			Set(OIndex oi, void* pvData, size_t uiDataSize);

			BOOL			Put(OIndex oi, void* pvData, size_t uiDataSize);

			BOOL			Get(OIndex oi, void* pvData, size_t uiMaxSize = 0);
			BOOL			Get(OIndex oi, size_t* puiDataSize, void* pvData, size_t uiMaxSize);

			unsigned int	Size(OIndex oi);
			BOOL			Contains(OIndex oi);

			int64			NumElements(void);

	virtual int64			NumIndices(void) =0;
	virtual int64			NumIndicesCached(void) =0;
			int64			NumDatas(void);
			int				NumDataCached(void);
			int				NumDataCached(size_t iSize);
			int				NumFiles(void);
			int64			NumDatas(size_t uiDataSize);

			BOOL			Remove(OIndex oi);

	virtual BOOL			IsDirty(OIndex oi) =0;

	virtual BOOL			EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor) =0;

			unsigned int	TestGetCachedObjectSize(OIndex oi);

protected:
	virtual BOOL			GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction = FALSE) =0;
	virtual BOOL			SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction = FALSE) =0;
	virtual BOOL			UpdateDescriptorCache(OIndex oi, void* pvCache, size_t uiDataSize) =0;
	virtual BOOL			RemoveDescriptor(OIndex oi) =0;

	virtual BOOL			Flush(BOOL bClearCache) =0;

			BOOL			GetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, size_t* puiDataSize, void* pvData, size_t uiMaxSize);
};


#endif // !__INDEXED_DATA_COMMON_H__

