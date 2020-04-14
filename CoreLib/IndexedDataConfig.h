#ifndef __INDEXED_DATA_CONFIG_H__
#define __INDEXED_DATA_CONFIG_H__
#include "IndexWriteThrough.h"
#include "DurableFileController.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexedEvictionCallback.h"


class CIndexedDataConfig
{
public:
	virtual void							Kill(void);

	virtual CDurableFileController*			GetDurableFileControl(void) =0;
	virtual char*							GetSubdirectory(void) =0;
	virtual size_t							GetDataCacheSize(void) =0;
	virtual size_t							GetIndexCacheSize(void) =0;
	virtual EIndexWriteThrough				GetWriteThrough(void) =0;
	virtual CIndexTreeEvictionCallback*		GetIndexEvictionUserCallback(void) =0;
	virtual CIndexedEvictionCallback*		GetEvictionUserCallback(void) =0;
};


#endif // __INDEXED_DATA_CONFIG_H__

