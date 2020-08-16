#ifndef __VALUE_INDEXED_DATA_CONFIG_H__
#define __VALUE_INDEXED_DATA_CONFIG_H__
#include "BaseLib/LifeCycle.h"
#include "IndexedDataConfig.h"
#include "IndexTreeEvictionStrategy.h"


class CValueIndexedDataConfig : public CIndexedDataConfig
{
protected:
	size_t							muiDataCacheSize;
	size_t							muiIndexCacheSize;
	EIndexWriteThrough				meWriteThrough;
	CIndexTreeEvictionCallback*		mpcIndexTreeEvictionUserCallback;
	CIndexedDataEvictionCallback*	mpcIndexedDataEvictionUserCallback;
	char*							mszSubDirectory;
	CIndexTreeEvictionStrategy*		mpcEvictionStrategy;
	CIndexTreeDataOrderer*			mpcIndexTreeDataOrderer;

public:
	void 							Init(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy);
	void 							Init(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeEvictionCallback* pcIndexTreeEvictionUserCallback, CIndexedDataEvictionCallback* pcIndexedDataEvictionUserCallback, CIndexTreeDataOrderer* pcIndexTreeDataOrderer);
	void							Kill(void);

	char*							GetSubdirectory(void);
	size_t							GetDataCacheSize(void);
	size_t							GetIndexCacheSize(void);
	EIndexWriteThrough				GetWriteThrough(void);
	CIndexTreeEvictionCallback*		GetIndexTreeEvictionUserCallback(void);
	CIndexedDataEvictionCallback*	GetIndexedDataEvictionUserCallback(void);
	CIndexTreeEvictionStrategy*		GetEvictionStrategy(void);
	CIndexTreeDataOrderer*			GetIndexTreeDataOrderer(void);

public:
	static CLifeInit<CIndexedDataConfig> Create(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy);
	static CLifeInit<CIndexedDataConfig> Create(char* szSubDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeEvictionCallback* pcIndexTreeEvictionUserCallback, CIndexedDataEvictionCallback* pcIndexedDataEvictionUserCallback, CIndexTreeDataOrderer* pcIndexTreeDataOrderer);
};


#endif // __VALUE_INDEXED_DATA_CONFIG_H__

