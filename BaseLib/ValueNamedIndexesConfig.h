#ifndef __VALUE_NAMED_INDEXES_CONFIG_H__
#define __VALUE_NAMED_INDEXES_CONFIG_H__
#include "IndexTreeDataOrderer.h"
#include "LifeCycle.h"
#include "IndexTreeEvictionStrategy.h"
#include "NamedIndexesConfig.h"


class CValueNamedIndexesConfig : public CNamedIndexesConfig
{
protected:
	lsize									muiIndexCacheSize;
	EIndexWriteThrough						meWriteThrough;
	CIndexTreeEvictionCallback*				mpEvictionCallback;
	const char*								mszSubDirectory;
	CLifeInit<CIndexTreeEvictionStrategy>	mcEvictionStrategy;
	CLifeInit<CIndexTreeDataOrderer>		mcIndexTreeDataOrderer;

public:
	void									Init(const char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough);
	void									Init(const char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer);

	void									Kill(void);

	lsize									GetIndexCacheSize(void);
	CLifeInit<CIndexTreeEvictionStrategy>	GetEvictionStrategy(void);
	EIndexWriteThrough						GetWriteThrough(void);
	CIndexTreeEvictionCallback*				GetIndexTreeEvictionCallback(void);
	const char*									GetSubDirectory(void);
	CLifeInit<CIndexTreeDataOrderer>		GetIndexTreeDataOrderer(void);

public:
	static CLifeInit<CNamedIndexesConfig> Create(const char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough);
	static CLifeInit<CNamedIndexesConfig> Create(const char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback);
	static CLifeInit<CNamedIndexesConfig> Create(char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer);
};


#endif // __VALUE_NAMED_INDEXES_CONFIG_H__

