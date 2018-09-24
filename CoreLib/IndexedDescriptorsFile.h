#ifndef __INDEX_DESCRIPTORS_FILE_H__
#define __INDEX_DESCRIPTORS_FILE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "IndexTreeTemplateEvicting.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexedDataDescriptor.h"
#include "IndexTreeEvictionStrategyRandom.h"


class CIndexedDataCommon;
class CIndexedDescriptorsFile : public CIndexTreeEvictionCallback
{
protected:
	CIndexTreeTemplateEvicting<CIndexedDataDescriptor>	mcIndexTree;
	CIndexTreeEvictionStrategyRandom					mcEvictionStrategy;
	CIndexedDataCommon*									mpcIndexedData;

public:
	void	Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, BOOL bDirtyTesting, size_t uiCutoff, BOOL bWriteThrough);
	void	Kill(void);

	BOOL	Remove(OIndex oi);
	BOOL	Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL	Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL	SetCache(void* pvCache, OIndex oi);
	BOOL	Flush(void);

	int64	NumElements(void);
	int		NumCachedDatas(void);

	BOOL	GetIfInMemory(CIndexedDataDescriptor* pcDescriptor, OIndex oi);

	BOOL	NodeEvicted(CIndexTreeFile* pcIndexTree, unsigned char* pvKey, int iKeySize, void* pvData, int iDataSize);
};


#endif // __INDEX_DESCRIPTORS_FILE_H__

