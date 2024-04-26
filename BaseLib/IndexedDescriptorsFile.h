#ifndef __INDEX_DESCRIPTORS_FILE_H__
#define __INDEX_DESCRIPTORS_FILE_H__
#include "PrimitiveTypes.h"
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
	void			Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, char* szSubDirectory, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy);
	void			Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, char* szSubDirectory, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	bool			Kill(void);

	bool			Remove(OIndex oi);
	bool			Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	bool			Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi, bool bNoEviction);
	bool			Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	bool			Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi, bool bNoEviction);
	bool			SetCache(void* pvCache, uint32 uiDataSize, OIndex oi);
	bool			Flush(void);

	int64			NumElements(void);
	int64			NumIndices(void);
	int				NumIndicesCached(void);
	int				NumIndicesCached(size_t iSize);
	int64			NumDatas(void);
	bool			IsFlushed(void);

	bool			Evict(OIndex oi);
	bool			Flush(OIndex oi);

	OIndex 			StartIteration(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize);
	OIndex			Iterate(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize);

	bool			IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);

	uint16			IndexTreeDataSize(uint16 uiSourceSize);
	bool			IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, uint16 uiSourceDataSize);
	bool			IndexTreeReadData(void* pvDest, void* pvDataBuffer, uint16 uiDestDataSize, int iFileDataSize);

	bool			GetIfInMemory(CIndexedDataDescriptor* pcDescriptor, OIndex oi);

	size_t			GetSystemMemorySize(void);
	uint8			GetRootFlags(void);

	void			Dump(void);
	bool			ValidateIndex(void);
};


#endif // !__INDEX_DESCRIPTORS_FILE_H__

