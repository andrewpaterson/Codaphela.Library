#ifndef __VALUE_INDEXED_DATA_CONFIG_H__
#define __VALUE_INDEXED_DATA_CONFIG_H__
#include "IndexedDataConfig.h"
#include "IndexTreeEvictionStrategy.h"


class CValueIndexedDataConfig : public CIndexedDataConfig
{
protected:
	size									muiDataCacheSize;
	size									muiIndexCacheSize;
	EIndexWriteThrough						meWriteThrough;
	CIndexTreeEvictionCallback*				mpcIndexTreeEvictionUserCallback;
	CIndexedDataEvictionCallback*			mpcIndexedDataEvictionUserCallback;
	char*									mszSubDirectory;
	CLifeInit<CIndexTreeEvictionStrategy>	mcEvictionStrategy;
	CLifeInit<CIndexTreeDataOrderer>		mcIndexTreeDataOrderer;

public:
	void 									Init(char* szSubDirectory, size uiDataCacheSize, size uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy);
	void 									Init(char* szSubDirectory, size uiDataCacheSize, size uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeEvictionCallback* pcIndexTreeEvictionUserCallback, CIndexedDataEvictionCallback* pcIndexedDataEvictionUserCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer);
	void									Kill(void);

	char*									GetSubdirectory(void);
	size									GetDataCacheSize(void);
	size									GetIndexCacheSize(void);
	EIndexWriteThrough						GetWriteThrough(void);
	CIndexTreeEvictionCallback*				GetIndexTreeEvictionUserCallback(void);
	CIndexedDataEvictionCallback*			GetIndexedDataEvictionUserCallback(void);
	CLifeInit<CIndexTreeEvictionStrategy>	GetEvictionStrategy(void);
	CLifeInit<CIndexTreeDataOrderer>		GetIndexTreeDataOrderer(void);

public:
	static CLifeInit<CIndexedDataConfig>	Create(char* szSubDirectory, size uiDataCacheSize, size uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy);
	static CLifeInit<CIndexedDataConfig>	Create(char* szSubDirectory, size uiDataCacheSize, size uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeEvictionCallback* pcIndexTreeEvictionUserCallback, CIndexedDataEvictionCallback* pcIndexedDataEvictionUserCallback);
	static CLifeInit<CIndexedDataConfig>	Create(char* szSubDirectory, size uiDataCacheSize, size uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeEvictionCallback* pcIndexTreeEvictionUserCallback, CIndexedDataEvictionCallback* pcIndexedDataEvictionUserCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer);
};


#endif // __VALUE_INDEXED_DATA_CONFIG_H__

