#ifndef __NAMED_INDEXED_DATA_CONFIG_H__
#define __NAMED_INDEXED_DATA_CONFIG_H__
#include "IndexedDataConfig.h"
#include "NamedIndexesConfig.h"


class CNamedIndexedDataConfig
{
protected:
	CIndexedDataConfig*		mpcIndexConfig;
	CNamedIndexesConfig*	mpcNamedConfig;
	BOOL					mbKillConfigs;

public:
	void					Init(CIndexedDataConfig* pcIndexConfig, CNamedIndexesConfig* pcNamedConfig, BOOL bKillConfigs = TRUE);
	void					Kill(void);

	BOOL					HasDatabaseConfig(void);
	CIndexedDataConfig*		GetIndexConfig(void);
	CNamedIndexesConfig*	GetNamedConfig(void);
};

#endif // __NAMED_INDEXED_DATA_CONFIG_H__


