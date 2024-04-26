#ifndef __INDEXED_FILES_EVICTION_CALLBACK_H__
#define __INDEXED_FILES_EVICTION_CALLBACK_H__
#include "ArrayVoidPtr.h"
#include "IndexedGeneral.h"
#include "IndexedDataDescriptor.h"


class CIndexedFilesEvictionCallback
{
public:
	virtual bool DescriptorEvicted(OIndex oi, void* pvCache, uint32 uiDataSize) =0;
	virtual bool GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false) =0;
	virtual bool SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction = false) =0;
	virtual bool UpdateDescriptorCache(OIndex oi, void* pvCache, uint32 uiDataSize) =0;
};


#endif // __INDEXED_FILES_EVICTION_CALLBACK_H__

