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

	lsize									GetIndexCacheSize(void) const override;
	CLifeInit<CIndexTreeEvictionStrategy>	GetEvictionStrategy(void) const override;
	EIndexWriteThrough						GetWriteThrough(void) const override;
	CIndexTreeEvictionCallback*				GetIndexTreeEvictionCallback(void) const override;
	const char*								GetSubDirectory(void) const override;
	CLifeInit<CIndexTreeDataOrderer>		GetIndexTreeDataOrderer(void) const override;

public:
	static CLifeInit<CNamedIndexesConfig>	Create(const char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough);
	static CLifeInit<CNamedIndexesConfig>	Create(const char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback);
	static CLifeInit<CNamedIndexesConfig>	Create(char* szSubDirectory, lsize uiIndexCacheSize, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cIndexTreeDataOrderer);
};


#endif // __VALUE_NAMED_INDEXES_CONFIG_H__

