#ifndef __VALUE_NAMED_INDEXES_CONFIG_H__
#define __VALUE_NAMED_INDEXES_CONFIG_H__
#include "IndexTreeEvictionStrategy.h"
#include "NamedIndexesConfig.h"


class CValueNamedIndexesConfig : public CNamedIndexesConfig
{
protected:
	size_t							muiCutoff;
	CIndexTreeEvictionStrategy*		mpcEvictionStrategy;
	EIndexWriteThrough				meWriteThrough;
	CIndexTreeEvictionCallback*		mpEvictionCallback;
	char*							mszSubDirectory;

public:
	void							Init(char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionStrategy* pcEvictionStrategy, EIndexWriteThrough eWriteThrough);
	void							Init(char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionStrategy* pcEvictionStrategy, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback);

	void							Kill(void);

	size_t							GetCutoff(void);
	CIndexTreeEvictionStrategy*		GetEvictionStrategy(void);
	EIndexWriteThrough				GetWriteThrough(void);
	CIndexTreeEvictionCallback*		GetIndexTreeEvictionCallback(void);
	char*							GetSubDirectory(void);
};


#endif // __VALUE_NAMED_INDEXES_CONFIG_H__

