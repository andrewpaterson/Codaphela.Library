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
	CNamedIndexedDataConfig*				pcConfig;
	CValueIndexedDataConfig*				pcIndexConfig;
	CValueNamedIndexesConfig*				pcNamedConfig;
	CIndexTreeEvictionStrategyDataOrderer*	pcIndexEvictionStrategy;
	CAccessDataOrderer*						pcIndexAccessOrderer;
	CIndexTreeEvictionStrategyDataOrderer*	pcNamedEvictionStrategy;
	CAccessDataOrderer*						pcNamedAccessOrderer;

	pcIndexAccessOrderer = NewMalloc<CAccessDataOrderer>();
	pcIndexEvictionStrategy = NewMalloc<CIndexTreeEvictionStrategyDataOrderer>();
	pcIndexConfig = NewMalloc<CValueIndexedDataConfig>();
	pcIndexAccessOrderer->Init();
	pcIndexEvictionStrategy->Init(pcIndexAccessOrderer);
	pcIndexConfig->Init("Index", 16 MB, 8 MB, eWriteThrough, pcIndexEvictionStrategy, NULL, NULL, pcIndexAccessOrderer);

	pcNamedAccessOrderer = NewMalloc<CAccessDataOrderer>();
	pcNamedEvictionStrategy = NewMalloc<CIndexTreeEvictionStrategyDataOrderer>();
	pcNamedConfig = NewMalloc<CValueNamedIndexesConfig>();
	pcNamedAccessOrderer->Init();
	pcNamedEvictionStrategy->Init(pcNamedAccessOrderer);
	pcNamedConfig->Init("Names", 1 MB, pcNamedEvictionStrategy, eWriteThrough, NULL, pcNamedAccessOrderer);

	pcConfig = NewMalloc<CNamedIndexedDataConfig>();
	pcConfig->Init(pcIndexConfig, pcNamedConfig, TRUE, TRUE);

	pcDatabase = NewMalloc<CCodabase>();
	pcDatabase->Init(szDirectory, pcConfig);
	return pcDatabase;
}

