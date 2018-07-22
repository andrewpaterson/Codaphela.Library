#ifndef __INDEXED_FILES_EVICTION_CALLBACK_H__
#define __INDEXED_FILES_EVICTION_CALLBACK_H__
#include "BaseLib/ArrayVoidPtr.h"
#include "IndexedGeneral.h"
#include "IndexedDataDescriptor.h"


class CIndexedFilesEvictionCallback
{
public:
	virtual BOOL DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors) =0;
	virtual BOOL GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor) =0;
	virtual BOOL SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor) =0;
};


#endif // __INDEXED_FILES_EVICTION_CALLBACK_H__

