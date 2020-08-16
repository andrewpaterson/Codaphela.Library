#include "BaseLib/ConstructorCall.h"
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
	CIndexTreeEvictionStrategyDataOrderer*	pcIndexEvictionStrategy;
	CAccessDataOrderer*						pcIndexAccessOrderer;
	CIndexTreeEvictionStrategyDataOrderer*	pcNamedEvictionStrategy;
	CAccessDataOrderer*						pcNamedAccessOrderer;

	pcIndexAccessOrderer = NewMalloc<CAccessDataOrderer>();
	pcIndexEvictionStrategy = NewMalloc<CIndexTreeEvictionStrategyDataOrderer>();
	pcIndexAccessOrderer->Init();
	pcIndexEvictionStrategy->Init(pcIndexAccessOrderer);
	cIndexConfig = CValueIndexedDataConfig::Create("Index", 16 MB, 8 MB, eWriteThrough, pcIndexEvictionStrategy, NULL, NULL, pcIndexAccessOrderer);

	pcNamedAccessOrderer = NewMalloc<CAccessDataOrderer>();
	pcNamedEvictionStrategy = NewMalloc<CIndexTreeEvictionStrategyDataOrderer>();
	pcNamedAccessOrderer->Init();
	pcNamedEvictionStrategy->Init(pcNamedAccessOrderer);
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", 1 MB, pcNamedEvictionStrategy, eWriteThrough, NULL, pcNamedAccessOrderer);
	

	pcDatabase = NewMalloc<CCodabase>();
	pcDatabase->Init(szDirectory, cIndexConfig, cNamedConfig);
	return pcDatabase;
}

