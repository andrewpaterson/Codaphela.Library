#ifndef __INDEXED_DATA_COMMON_H__
#define __INDEXED_DATA_COMMON_H__
#include "IndexedGeneral.h"
#include "IndexedDataDescriptor.h"
#include "IndexedConfig.h"
#include "IndexedFilesEvicting.h"


class CIndexedDataCommon
{
protected:
	CIndexedFilesEvicting	mcData;
	BOOL					mbWriteThrough;

public:
			void			Init(CIndexedConfig* pcConfig, CDurableFileController* pcDurableFileControl, CIndexedFilesEvictionCallback* pcEvictionCallback);
	virtual BOOL			Kill(void) =0;

			BOOL			IsCaching(void);
	
			BOOL			Add(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

			BOOL			Set(OIndex oi, void* pvData, unsigned int uiTimeStamp);
			BOOL			Set(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

			BOOL			SetOrAdd(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

			unsigned int	Size(OIndex oi);
			unsigned int	Flags(OIndex oi);
			BOOL			Get(OIndex oi, void* pvData);
			BOOL			Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxSize);

			BOOL			Contains(OIndex oi);

	virtual int64			NumElements(void) =0;
			int				NumCached(void);
			int				NumCached(int iSize);
			int				NumFiles(void);
			int64			NumData(unsigned int uiDataSize);

			BOOL			Remove(OIndex oi);

			int				TestNumIgnoredCacheElements(void);
			unsigned int	TestGetCachedObjectSize(OIndex oi);

protected:
			BOOL			SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiTimeStamp);
			BOOL			SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	virtual BOOL			GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor) =0;
	virtual BOOL			SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor) =0;
	virtual BOOL			UpdateDescriptorCache(OIndex oi, void* pvCache) =0;
	virtual BOOL			RemoveDescriptor(OIndex oi) =0;

	virtual void			InitIndices(CDurableFileController* pcDurableFileControl, BOOL bDirtyTesting) =0;

	virtual BOOL			Flush(BOOL bClearCache) =0;

			BOOL			DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors);
};


#endif // __INDEXED_DATA_COMMON_H__

