#ifndef __SIMPLE_INDEX_CONFIG_H__
#define __SIMPLE_INDEX_CONFIG_H__
#include "IndexConfig.h"


class CSimpleIndexConfig : public CIndexConfig
{
protected:
	CDurableFileController					mcDurableFileController;
	size_t									muiDataCacheSize;
	size_t									muiIndexCacheSize;
	EIndexWriteThrough						meWriteThrough;
	CIndexTreeEvictionCallback*						mpcIndexEvictionUserCallback;
	CIndexedEvictionCallback*		mpcEvictionUserCallback;

public:
	void 									Init(char* szWorkingDirectory, char* szRewriteDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough);
	void 									Init(char* szWorkingDirectory, char* szRewriteDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcIndexEvictionUserCallback, CIndexedEvictionCallback* pcEvictionUserCallback);
	void									Kill(void);

	CDurableFileController*					GetDurableFileControl(void);
	size_t									GetDataCacheSize(void);
	size_t									GetIndexCacheSize(void);
	EIndexWriteThrough						GetWriteThrough(void);
	CIndexTreeEvictionCallback*						GetIndexEvictionUserCallback(void);
	CIndexedEvictionCallback*		GetEvictionUserCallback(void);
};


#endif // __SIMPLE_INDEX_CONFIG_H__

