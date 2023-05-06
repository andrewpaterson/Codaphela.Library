#ifndef __VALUE_NAMED_INDEXES_CONFIG_H__
#define __VALUE_NAMED_INDEXES_CONFIG_H__
#include "IndexTreeDataOrderer.h"
#include "LifeCycle.h"
#include "IndexTreeEvictionStrategy.h"
#include "NamedIndexesConfig.h"


class CValueNamedIndexesConfig : public CNamedIndexesConfig
{
protected:
	size_t									muiIndexCacheSize;
	EIndexWriteThrough						meWriteThrough;
	CIndexTreeEvictionCallback*				mpEvictionCallback;
	char*									mszSubDirectory;
	CLifeInit<CIndexTreeEvictionStrategy>	mcEvictionStrategy;
	CLifeInit<CIndexTreeDataOrderer>		mcIndexTreeDataOrderer;

public:
	void									Init(char* szSubDirectory, size_t uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough);
	void									Init(char* szSubDirectory, size_t uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer);

	void									Kill(void);

	size_t									GetIndexCacheSize(void);
	CLifeInit<CIndexTreeEvictionStrategy>	GetEvictionStrategy(void);
	EIndexWriteThrough						GetWriteThrough(void);
	CIndexTreeEvictionCallback*				GetIndexTreeEvictionCallback(void);
	char*									GetSubDirectory(void);
	CLifeInit<CIndexTreeDataOrderer>		GetIndexTreeDataOrderer(void);

public:
	static CLifeInit<CNamedIndexesConfig> Create(char* szSubDirectory, size_t uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough);
	static CLifeInit<CNamedIndexesConfig> Create(char* szSubDirectory, size_t uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback);
	static CLifeInit<CNamedIndexesConfig> Create(char* szSubDirectory, size_t uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer);
};


#endif // !__VALUE_NAMED_INDEXES_CONFIG_H__

