#ifndef __INDEXED_DATA_CONFIG_H__
#define __INDEXED_DATA_CONFIG_H__
#include "BaseLib/LifeCycle.h"
#include "BaseLib/IndexTreeDataOrderer.h"
#include "BaseLib/Constructable.h"
#include "BaseLib/Killable.h"
#include "IndexWriteThrough.h"
#include "DurableFileController.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexedDataEvictionCallback.h"
#include "IndexTreeEvictionStrategy.h"


class CIndexedDataConfig : public CConstructable, public CKillable
{
friend class CIndexedData;
CONSTRUCTABLE(CIndexedDataConfig);
public:
			void									Kill(void);

	virtual char*									GetSubdirectory(void) =0;
	virtual size_t									GetDataCacheSize(void) =0;
	virtual size_t									GetIndexCacheSize(void) =0;
	virtual EIndexWriteThrough						GetWriteThrough(void) =0;
	virtual CIndexTreeEvictionCallback*				GetIndexTreeEvictionUserCallback(void) =0;
	virtual CIndexedDataEvictionCallback*			GetIndexedDataEvictionUserCallback(void) =0;
	virtual CLifeInit<CIndexTreeEvictionStrategy>	GetEvictionStrategy(void) =0;
	virtual CLifeInit<CIndexTreeDataOrderer>		GetIndexTreeDataOrderer(void) =0;
};


#endif // __INDEXED_DATA_CONFIG_H__

