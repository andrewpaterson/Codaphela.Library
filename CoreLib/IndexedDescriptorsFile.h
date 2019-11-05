#ifndef __INDEX_DESCRIPTORS_FILE_H__
#define __INDEX_DESCRIPTORS_FILE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "IndexTreeTemplateEvicting.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexedDataDescriptor.h"
#include "IndexTreeEvictionStrategyRandom.h"
#include "IndexedDescriptorsFileCallback.h"
#include "IndexTreeEvictionCallbackWrapper.h"


class CIndexedDataCommon;
class CIndexedDescriptorsFile : public CIndexTreeEvictionCallback
{
protected:
	CIndexTreeTemplateEvicting<CIndexedDataDescriptor>	mcIndexTree;
	CIndexTreeEvictionStrategyRandom					mcEvictionStrategy;
	CIndexedDataCommon*									mpcIndexedData;
	CIndexedDescriptorsFileCallback						mcDescriptorsCallback;
	CIndexTreeEvictionCallbackWrapper					mcIndexTreeEvictionCallbackWrapper;

public:
	void	Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, BOOL bDirtyTesting, size_t uiCutoff, BOOL bWriteThrough);
	void	Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, BOOL bDirtyTesting, size_t uiCutoff, BOOL bWriteThrough, CIndexTreeEvictionCallback* pcIndexEvictionUserCallback);
	void	Kill(void);

	BOOL	Remove(OIndex oi);
	BOOL	Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL	Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi, BOOL bNoEviction);
	BOOL	Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL	Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi, BOOL bNoEviction);
	BOOL	SetCache(void* pvCache, OIndex oi);
	BOOL	Flush(void);

	int64	NumElements(void);
	int		NumCachedDatas(void);

	BOOL	Evict(OIndex oi);
	BOOL	NodeEvicted(CIndexTreeFile* pcIndexTree, unsigned char* pvKey, int iKeySize, void* pvData, int iDataSize);

	BOOL	GetIfInMemory(CIndexedDataDescriptor* pcDescriptor, OIndex oi);

	size_t	GetSystemMemorySize(void);

	void	Dump(void);
};


#endif // __INDEX_DESCRIPTORS_FILE_H__

