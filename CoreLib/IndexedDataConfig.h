#ifndef __INDEXED_DATA_CONFIG_H__
#define __INDEXED_DATA_CONFIG_H__
#include "IndexWriteThrough.h"
#include "DurableFileController.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexedDataEvictionCallback.h"


class CIndexedDataConfig
{
public:
	virtual void							Kill(void);

	virtual char*							GetSubdirectory(void) =0;
	virtual size_t							GetDataCacheSize(void) =0;
	virtual size_t							GetIndexCacheSize(void) =0;
	virtual EIndexWriteThrough				GetWriteThrough(void) =0;
	virtual CIndexTreeEvictionCallback*		GetIndexTreeEvictionUserCallback(void) =0;
	virtual CIndexedDataEvictionCallback*	GetIndexedDataEvictionUserCallback(void) =0;
};


#endif // __INDEXED_DATA_CONFIG_H__

