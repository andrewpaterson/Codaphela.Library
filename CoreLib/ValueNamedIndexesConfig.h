#ifndef __VALUE_NAMED_INDEXES_CONFIG_H__
#define __VALUE_NAMED_INDEXES_CONFIG_H__
#include "BaseLib/IndexTreeDataOrderer.h"
#include "BaseLib/LifeCycle.h"
#include "IndexTreeEvictionStrategy.h"
#include "NamedIndexesConfig.h"


class CValueNamedIndexesConfig : public CNamedIndexesConfig
{
protected:
	size_t							muiIndexCacheSize;
	CIndexTreeEvictionStrategy*		mpcNamedEvictionStrategy;
	EIndexWriteThrough				meWriteThrough;
	CIndexTreeEvictionCallback*		mpEvictionCallback;
	char*							mszSubDirectory;
	CIndexTreeDataOrderer*			mpcIndexTreeDataOrderer;

public:
	void							Init(char* szSubDirectory, size_t uiIndexCacheSize, CIndexTreeEvictionStrategy* pcNamedEvictionStrategy, EIndexWriteThrough eWriteThrough);
	void							Init(char* szSubDirectory, size_t uiIndexCacheSize, CIndexTreeEvictionStrategy* pcNamedEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeDataOrderer* pcIndexTreeDataOrderer);

	void							Kill(void);

	size_t							GetIndexCacheSize(void);
	CIndexTreeEvictionStrategy*		GetEvictionStrategy(void);
	EIndexWriteThrough				GetWriteThrough(void);
	CIndexTreeEvictionCallback*		GetIndexTreeEvictionCallback(void);
	char*							GetSubDirectory(void);
	CIndexTreeDataOrderer*			GetIndexTreeDataOrderer(void);

public:
	static CLifeInit<CNamedIndexesConfig> Create(char* szSubDirectory, size_t uiIndexCacheSize, CIndexTreeEvictionStrategy* pcNamedEvictionStrategy, EIndexWriteThrough eWriteThrough);
	static CLifeInit<CNamedIndexesConfig> Create(char* szSubDirectory, size_t uiIndexCacheSize, CIndexTreeEvictionStrategy* pcNamedEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeDataOrderer* pcIndexTreeDataOrderer);
};


#endif // __VALUE_NAMED_INDEXES_CONFIG_H__

