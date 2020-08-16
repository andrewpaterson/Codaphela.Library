#ifndef __NAMED_INDEXED_DATA_CONFIG_H__
#define __NAMED_INDEXED_DATA_CONFIG_H__
#include "BaseLib/LifeCycle.h"
#include "IndexedDataConfig.h"
#include "NamedIndexesConfig.h"


class CNamedIndexedDataConfig
{
friend class CNamedIndexedData;
protected:
	CLifeInit<CIndexedDataConfig>	mcIndexConfig;
	CLifeInit<CNamedIndexesConfig>	mcNamedConfig;

public:
	void							Init(CLifeInit<CIndexedDataConfig> cIndexConfig, CLifeInit<CNamedIndexesConfig> cNamedConfig);
	void							Kill(void);

	BOOL							HasDatabaseConfig(void);
	CLifeInit<CIndexedDataConfig>	GetIndexConfig(void);
	CLifeInit<CNamedIndexesConfig>	GetNamedConfig(void);
};

#endif // __NAMED_INDEXED_DATA_CONFIG_H__


