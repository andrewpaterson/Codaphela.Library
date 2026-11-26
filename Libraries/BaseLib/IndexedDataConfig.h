#ifndef __INDEXED_DATA_CONFIG_H__
#define __INDEXED_DATA_CONFIG_H__
#include "LifeCycle.h"
#include "IndexTreeDataOrderer.h"
#include "Constructable.h"
#include "Killable.h"
#include "IndexWriteThrough.h"
#include "DurableFileController.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexedDataEvictionCallback.h"
#include "IndexTreeEvictionStrategy.h"


class CIndexedDataConfig : public CConstructable, public CKillable
{
friend class CIndexedData;
CONSTRUCTABLE(CIndexedDataConfig)
public:
			void									Kill(void) override;

	virtual const char*								GetSubdirectory(void) const =0;
	virtual lsize									GetDataCacheSize(void) const =0;
	virtual lsize									GetIndexCacheSize(void) const =0;
	virtual EIndexWriteThrough						GetWriteThrough(void) const =0;
	virtual CIndexTreeEvictionCallback*				GetIndexTreeEvictionUserCallback(void) const =0;
	virtual CIndexedDataEvictionCallback*			GetIndexedDataEvictionUserCallback(void) const =0;
	virtual CLifeInit<CIndexTreeEvictionStrategy>	GetEvictionStrategy(void) const =0;
	virtual CLifeInit<CIndexTreeDataOrderer>		GetIndexTreeDataOrderer(void) const =0;
};


#endif // __INDEXED_DATA_CONFIG_H__

