#ifndef __NAMED_INDEX_CONFIG_H__
#define __NAMED_INDEX_CONFIG_H__
#include "CoreLib/IndexConfig.h"


class CNamedIndexConfig
{
protected:
	CIndexConfig*	mpcIndexConfig;
	CIndexConfig*	mpcNamedConfig;
	BOOL			mbKillConfigs;

public:
	void			Init(CIndexConfig* pcIndexConfig, CIndexConfig* pcNamedConfig, BOOL bKillConfigs = TRUE);
	void			Kill(void);

	BOOL			HasDatabaseConfig(void);
	CIndexConfig*	GetIndexConfig(void);
	CIndexConfig*	GetNamedConfig(void);
};

#endif // __NAMED_INDEX_CONFIG_H__


