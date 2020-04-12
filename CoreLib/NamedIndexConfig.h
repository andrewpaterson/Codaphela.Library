#ifndef __NAMED_INDEX_CONFIG_H__
#define __NAMED_INDEX_CONFIG_H__
#include "IndexedDataConfig.h"


class CNamedIndexConfig
{
protected:
	CIndexedDataConfig*		mpcIndexConfig;
	CIndexedDataConfig*		mpcNamedConfig;
	BOOL					mbKillConfigs;

public:
	void					Init(CIndexedDataConfig* pcIndexConfig, CIndexedDataConfig* pcNamedConfig, BOOL bKillConfigs = TRUE);
	void					Kill(void);

	BOOL					HasDatabaseConfig(void);
	CIndexedDataConfig*		GetIndexConfig(void);
	CIndexedDataConfig*		GetNamedConfig(void);
};

#endif // __NAMED_INDEX_CONFIG_H__


