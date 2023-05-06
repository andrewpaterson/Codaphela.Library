#ifndef __NAMED_INDEXES_CONFIG_H__
#define __NAMED_INDEXES_CONFIG_H__
#include "IndexTreeDataOrderer.h"
#include "Constructable.h"
#include "Killable.h"
#include "LifeCycle.h"
#include "DurableFileController.h"
#include "IndexWriteThrough.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexTreeEvictionStrategy.h"


class CNamedIndexesConfig : public CConstructable, public CKillable
{
friend class CNamedIndexes;
CONSTRUCTABLE(CNamedIndexesConfig);
public:
	virtual void									Kill(void);

	virtual size_t									GetIndexCacheSize(void) =0;
	virtual CLifeInit<CIndexTreeEvictionStrategy>	GetEvictionStrategy(void) =0;
	virtual EIndexWriteThrough						GetWriteThrough(void) =0;
	virtual CIndexTreeEvictionCallback*				GetIndexTreeEvictionCallback(void) =0;
	virtual char*									GetSubDirectory(void) =0;
	virtual CLifeInit<CIndexTreeDataOrderer>		GetIndexTreeDataOrderer(void) =0;
};


#endif // !__NAMED_INDEXES_CONFIG_H__

