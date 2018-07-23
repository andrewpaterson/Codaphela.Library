#ifndef __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__
#define __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__
#include "BaseLib/ListVariable.h"
#include "BaseLib/MapLongTemplate.h"
#include "IndexedFilesEvictionCallback.h"


class CIndexedFilesEvictedDescriptorList : public CIndexedFilesEvictionCallback
{
protected:
	CListVariable									mcDatas;
	CMapLongTemplate<CIndexedDataDescriptor>	mcDescriptors;

public:
	void					Init(void);
	void					Kill(void);

	CIndexedDataDescriptor*	AddDescriptor(OIndex oi, unsigned int uiDataSize);

	BOOL					DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors);
	BOOL					GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor);
	BOOL					SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor);
};


#endif // __INDEXED_FILES_EVICTED_DESCRIPTOR_LIST_H__

