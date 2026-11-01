#include "ConstructorCall.h"
#include "IndexTreeEvictionStrategyFactory.h"
#include "ValueNamedIndexesConfig.h"
#include "ValueIndexedDataConfig.h"
#include "IndexTreeEvictionStrategyDataOrderer.h"
#include "CodabaseFactory.h"


//The caches' sizes.  The W65C816 addresses 16MB, so it has smaller ones.
#ifdef __W65C816__
#define CODABASE_DATA_CACHE_SIZE	(2 MB)
#define CODABASE_INDEX_CACHE_SIZE	(1 MB)
#define CODABASE_NAMES_CACHE_SIZE	(256 KB)
#else
#define CODABASE_DATA_CACHE_SIZE	(16 MB)
#define CODABASE_INDEX_CACHE_SIZE	(8 MB)
#define CODABASE_NAMES_CACHE_SIZE	(1 MB)
#endif // __W65C816__


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCodabase* CCodabaseFactory::Create(const char* szDirectory, EIndexWriteThrough eWriteThrough, EDurableSync eSync)
{
	CCodabase*								pcDatabase;
	CLifeInit<CIndexedDataConfig>			cIndexConfig;
	CLifeInit<CNamedIndexesConfig>			cNamedConfig;
	CLifeInit<CIndexTreeEvictionStrategy>	cIndexEvictionStrategy;
	CLifeInit<CIndexTreeEvictionStrategy>	cNamedEvictionStrategy;

	cIndexEvictionStrategy = CIndexTreeEvictionStrategyFactory::Create(ITESST_Access);

	//TODO: Shouldn't these parameter be configurable?
	cIndexConfig = CValueIndexedDataConfig::Create("Index", CODABASE_DATA_CACHE_SIZE, CODABASE_INDEX_CACHE_SIZE, eWriteThrough, cIndexEvictionStrategy, NULL, NULL);

	cNamedEvictionStrategy = CIndexTreeEvictionStrategyFactory::Create(ITESST_Access);
	cNamedConfig = CValueNamedIndexesConfig::Create("Names", CODABASE_NAMES_CACHE_SIZE, cNamedEvictionStrategy, eWriteThrough, NULL);

	pcDatabase = NewMalloc<CCodabase>();
	pcDatabase->Init(szDirectory, eSync, cIndexConfig, cNamedConfig);
	if (!pcDatabase->Start())
	{
		SafeKill(pcDatabase);
		return NULL;
	}
	return pcDatabase;
}

