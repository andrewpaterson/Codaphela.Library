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
	virtual void			Kill(void) =0;

			bool			IsCaching(void);
			bool			IsWriteThrough(void);
			bool			IsFlushed(void);
	
			bool			Add(OIndex oi, void* pvData, lsize uiDataSize);

			bool			Set(OIndex oi, void* pvData);
			bool			Set(OIndex oi, void* pvData, lsize uiDataSize);

			bool			Put(OIndex oi, const void* pvData, lsize uiDataSize);

			bool			Get(OIndex oi, void* pvData, lsize uiMaxSize = 0);
			bool			Get(OIndex oi, lsize* puiDataSize, void* pvData, lsize uiMaxSize);

			lsize	Size(OIndex oi);
			bool			Contains(OIndex oi);

			int64			NumElements(void);

	virtual int64			NumIndices(void) =0;
	virtual size			NumIndicesCached(void) =0;
			int64			NumDatas(void);
			size			NumDataCached(void);
			size			NumDataCached(size iSize);
			size			NumFiles(void);
			int64			NumDatas(lsize uiDataSize);

			bool			Remove(OIndex oi);

	virtual bool			IsDirty(OIndex oi) =0;

	virtual bool			EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor) =0;

			lsize	TestGetCachedObjectSize(OIndex oi);

protected:
	virtual bool			GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false) =0;
	virtual bool			SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false) =0;
	virtual bool			UpdateDescriptorCache(OIndex oi, void* pvCache, lsize uiDataSize) =0;
	virtual bool			RemoveDescriptor(OIndex oi) =0;

	virtual bool			Flush(bool bClearCache) =0;

			bool			GetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, lsize* puiDataSize, void* pvData, lsize uiMaxSize);
};


#endif // __INDEXED_DATA_COMMON_H__

