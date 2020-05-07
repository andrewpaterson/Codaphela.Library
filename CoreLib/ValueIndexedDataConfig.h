#ifndef __VALUE_INDEXED_DATA_CONFIG_H__
#define __VALUE_INDEXED_DATA_CONFIG_H__
#include "IndexedDataConfig.h"
#include "IndexTreeEvictionStrategy.h"


class CValueIndexedDataConfig : public CIndexedDataConfig
{
protected:
	size_t							muiDataCacheSize;
	size_t							muiIndexCacheSize;
	EIndexWriteThrough				meWriteThrough;
	CIndexTreeEvictionCallback*		mpcIndexEvictionUserCallback;
	CIndexedDataEvictionCallback*		mpcEvictionUserCallback;
	char*							mszSubDirectory;

public:
	void 							Init(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcIndexTreeEvictionStrategy);
	void 							Init(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcIndexTreeEvictionStrategy, CIndexTreeEvictionCallback* pcIndexEvictionUserCallback, CIndexedDataEvictionCallback* pcEvictionUserCallback);
	void							Kill(void);

	char*							GetSubdirectory(void);
	size_t							GetDataCacheSize(void);
	size_t							GetIndexCacheSize(void);
	EIndexWriteThrough				GetWriteThrough(void);
	CIndexTreeEvictionCallback*		GetIndexEvictionUserCallback(void);
	CIndexedDataEvictionCallback*		GetEvictionUserCallback(void);
};


#endif // __VALUE_INDEXED_DATA_CONFIG_H__

