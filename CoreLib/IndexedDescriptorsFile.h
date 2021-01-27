#ifndef __INDEX_DESCRIPTORS_FILE_H__
#define __INDEX_DESCRIPTORS_FILE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "IndexTreeEvicting.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexedDataDescriptor.h"
#include "IndexTreeEvictionStrategy.h"
#include "IndexTreeEvictionCallbackWrapper.h"

#include "IndexTreeEvictionStrategyRandom.h"


class CIndexedDataCommon;
class CIndexedDescriptorsFile : public CIndexTreeEvictionCallback, public CIndexTreeFileDataCallback
{
protected:
	CIndexTreeEvicting					mcIndexTree;
	CIndexedDataCommon*					mpcIndexedData;

	CIndexTreeEvictionCallbackWrapper	mcEvictionCallbackWrapper;

public:
	void			Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, char* szSubDirectory, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy);
	void			Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, char* szSubDirectory, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeDataOrderer* pcDataOrderer);
	BOOL			Kill(void);

	BOOL			Remove(OIndex oi);
	BOOL			Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL			Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi, BOOL bNoEviction);
	BOOL			Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL			Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi, BOOL bNoEviction);
	BOOL			SetCache(void* pvCache, unsigned int uiDataSize, OIndex oi);
	BOOL			Flush(void);

	int64			NumElements(void);
	int64			NumIndices(void);
	int				NumIndicesCached(void);
	int				NumIndicesCached(size_t iSize);
	int64			NumDatas(void);
	BOOL			IsFlushed(void);

	BOOL			Evict(OIndex oi);
	BOOL			Flush(OIndex oi);

	BOOL			IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);

	unsigned short	IndexTreeDataSize(unsigned short uiSourceSize);
	BOOL			IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize);
	BOOL			IndexTreeReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize);

	BOOL			GetIfInMemory(CIndexedDataDescriptor* pcDescriptor, OIndex oi);

	size_t			GetSystemMemorySize(void);
	unsigned char	GetRootFlags(void);

	void			Dump(void);
	BOOL			ValidateIndex(void);
};


#endif // __INDEX_DESCRIPTORS_FILE_H__

