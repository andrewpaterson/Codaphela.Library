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

	void									Kill(void) override;

	lsize									GetIndexCacheSize(void) override;
	CLifeInit<CIndexTreeEvictionStrategy>	GetEvictionStrategy(void) override;
	EIndexWriteThrough						GetWriteThrough(void) override;
	CIndexTreeEvictionCallback*				GetIndexTreeEvictionCallback(void) override;
	const char*									GetSubDirectory(void) override;
	CLifeInit<CIndexTreeDataOrderer>		GetIndexTreeDataOrderer(void) override;

public:
	static CLifeInit<CNamedIndexesConfig> Create(const char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough);
	static CLifeInit<CNamedIndexesConfig> Create(const char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback);
	static CLifeInit<CNamedIndexesConfig> Create(char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer);
};


#endif // __VALUE_NAMED_INDEXES_CONFIG_H__

