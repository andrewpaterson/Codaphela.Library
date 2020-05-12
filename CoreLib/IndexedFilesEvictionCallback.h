#ifndef __INDEXED_FILES_EVICTION_CALLBACK_H__
#define __INDEXED_FILES_EVICTION_CALLBACK_H__
#include "BaseLib/ArrayVoidPtr.h"
#include "IndexedGeneral.h"
#include "IndexedDataDescriptor.h"


class CIndexedFilesEvictionCallback
{
public:
	virtual BOOL DescriptorEvicted(OIndex oi, void* pvCache, unsigned int uiDataSize) =0;
	virtual BOOL GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction = FALSE) =0;
	virtual BOOL SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction = FALSE) =0;
	virtual BOOL UpdateDescriptorCache(OIndex oi, void* pvCache, unsigned int uiDataSize) =0;
};


#endif // __INDEXED_FILES_EVICTION_CALLBACK_H__

