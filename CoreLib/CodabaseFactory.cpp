#include "BaseLib/ConstructorCall.h"
#include "CoreLib/IndexTreeEvictionStrategyFactory.h"
#include "ValueNamedIndexesConfig.h"
#include "ValueIndexedDataConfig.h"
#include "IndexTreeEvictionStrategyDataOrderer.h"
#include "CodabaseFactory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCodabase* CCodabaseFactory::Create(char* szDirectory, EIndexWriteThrough eWriteThrough)
{
	CCodabase*								pcDatabase;
	CLifeInit<CIndexedDataConfig>			cIndexConfig;
	CLifeInit<CNamedIndexesConfig>			cNamedConfig;
	CLifeInit<CIndexTreeEvictionStrategy>	cIndexEvictionStrategy;
	CLifeInit<CIndexTreeEvictionStrategy>	cNamedEvictionStrategy;

	cIndexEvictionStrategy = CIndexTreeEvictionStrategyFactory::Create(ITESST_Access);

	cIndexConfig = CValueIndexedDataConfig::Create("Index", 16 MB, 8 MB, eWriteThrough, cIndexEvictionStrategy, NULL, NULL);

	cNamedEvictionStrategy = CIndexTreeEvictionStrategyFactory::Create(ITESST_Access);
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 1 MB, cNamedEvictionStrategy, eWriteThrough, NULL);

	pcDatabase = NewMalloc<CCodabase>();
	pcDatabase->Init(szDirectory, cIndexConfig, cNamedConfig);
	return pcDatabase;
}

