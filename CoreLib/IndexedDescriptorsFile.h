#ifndef __INDEX_DESCRIPTORS_FILE_H__
#define __INDEX_DESCRIPTORS_FILE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "IndexTreeEvicting.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexedDataDescriptor.h"
#include "IndexTreeEvictionStrategyRandom.h"
#include "EvictionCallbackWrapper.h"


class CIndexedDataCommon;
class CIndexedDescriptorsFile : public CIndexTreeEvictionCallback, public CIndexTreeFileCallback
{
protected:
	CIndexTreeEvicting									mcIndexTree;
	CIndexTreeEvictionStrategyRandom					mcEvictionStrategy;
	CIndexedDataCommon*									mpcIndexedData;

	CEvictionCallbackWrapper							mcEvictionCallbackWrapper;

public:
	void			Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, BOOL bDirtyTesting, size_t uiCutoff, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	void			Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, BOOL bDirtyTesting, size_t uiCutoff, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CIndexTreeEvictionCallback* pcEvictionCallback);
	void			Kill(void);

	BOOL			Remove(OIndex oi);
	BOOL			Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL			Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi, BOOL bNoEviction);
	BOOL			Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi);
	BOOL			Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi, BOOL bNoEviction);
	BOOL			SetCache(void* pvCache, unsigned int uiDataSize, OIndex oi);
	BOOL			Flush(void);

	int64			NumElements(void);
	int				NumCachedDatas(void);

	BOOL			Evict(OIndex oi);
	BOOL			NodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);

	unsigned short	DataBufferSize(unsigned short uiSourceSize);
	BOOL			WriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize);
	BOOL			ReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize);

	BOOL			GetIfInMemory(CIndexedDataDescriptor* pcDescriptor, OIndex oi);

	size_t			GetSystemMemorySize(void);
	unsigned char	GetRootFlags(void);

	void			Dump(void);
};


#endif // __INDEX_DESCRIPTORS_FILE_H__

