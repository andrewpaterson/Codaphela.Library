#ifndef __NAMED_INDEXES_CONFIG_H__
#define __NAMED_INDEXES_CONFIG_H__
#include "BaseLib/IndexTreeDataOrderer.h"
#include "DurableFileController.h"
#include "IndexWriteThrough.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexTreeEvictionStrategy.h"


class CNamedIndexesConfig
{
public:
	virtual void							Kill(void);

	virtual size_t							GetIndexCacheSize(void) =0;
	virtual CIndexTreeEvictionStrategy*		GetEvictionStrategy() =0;
	virtual EIndexWriteThrough				GetWriteThrough(void) =0;
	virtual CIndexTreeEvictionCallback*		GetIndexTreeEvictionCallback(void) =0;
	virtual char*							GetSubDirectory(void) =0;
	virtual CIndexTreeDataOrderer*			GetIndexTreeDataOrderer(void) =0;
};


#endif // __NAMED_INDEXES_CONFIG_H__

