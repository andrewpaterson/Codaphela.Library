#ifndef __VALUE_INDEXED_DESCRIPTORS_FILE_CONFIG_H__
#define __VALUE_INDEXED_DESCRIPTORS_FILE_CONFIG_H__
#include "IndexWriteThrough.h"
#include "DurableFileController.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexTreeEvictionStrategy.h"
#include "IndexedDataEvictionCallback.h"
#include "IndexedDescriptorsFileConfig.h"


class CValueIndexedDescriptorsFileConfig : public CIndexedDescriptorsFileConfig
{
private:
	char*							mszSubdirectory;
	size_t							muiIndexCacheSize;
	EIndexWriteThrough				meWriteThrough;
	CIndexTreeEvictionStrategy*		mpcEvictionStrategy;
	CIndexedDataEvictionCallback*		mpcIndexedDataEvictionUserCallback;
	CIndexTreeEvictionCallback*		mpcIndexTreeEvictionCallback;

public:
	void							Init(char* szSubdirectory, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy);
	void							Init(char* szSubdirectory, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexedDataEvictionCallback* pcEvictionUserCallback, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback);
	void							Kill(void);

	char*							GetSubdirectory(void);
	size_t							GetIndexCacheSize(void);
	EIndexWriteThrough				GetWriteThrough(void);
	CIndexTreeEvictionStrategy*		GetEvictionStrategy();
	CIndexedDataEvictionCallback*		GetIndexedDataEvictionUserCallback(void);
	CIndexTreeEvictionCallback*		GetIndexTreeEvictionCallback(void);
};


#endif // __VALUE_INDEXED_DESCRIPTORS_FILE_CONFIG_H__

