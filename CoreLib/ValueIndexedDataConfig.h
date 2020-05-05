#ifndef __VALUE_INDEXED_DATA_CONFIG_H__
#define __VALUE_INDEXED_DATA_CONFIG_H__
#include "IndexedDataConfig.h"


class CValueIndexedDataConfig : public CIndexedDataConfig
{
protected:
	size_t							muiDataCacheSize;
	size_t							muiIndexCacheSize;
	EIndexWriteThrough				meWriteThrough;
	CIndexTreeEvictionCallback*		mpcIndexEvictionUserCallback;
	CIndexedEvictionCallback*		mpcEvictionUserCallback;
	char*							mszSubDirectory;

public:
	void 							Init(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough);
	void 							Init(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcIndexEvictionUserCallback, CIndexedEvictionCallback* pcEvictionUserCallback);
	void							Kill(void);

	char*							GetSubdirectory(void);
	size_t							GetDataCacheSize(void);
	size_t							GetIndexCacheSize(void);
	EIndexWriteThrough				GetWriteThrough(void);
	CIndexTreeEvictionCallback*		GetIndexEvictionUserCallback(void);
	CIndexedEvictionCallback*		GetEvictionUserCallback(void);
};


#endif // __VALUE_INDEXED_DATA_CONFIG_H__

