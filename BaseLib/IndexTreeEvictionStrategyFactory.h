#ifndef __INDEX_TREE_EVICTION_STRATEGY_FACTORY_H__
#define __INDEX_TREE_EVICTION_STRATEGY_FACTORY_H__
#include "IndexTreeEvctionStrategySystemTypes.h"
#include "IndexTreeEvictionStrategyRandom.h"
#include "IndexTreeEvictionStrategyDataOrderer.h"


class CIndexTreeEvictionStrategyFactory
{
public:
	static CLifeInit<CIndexTreeEvictionStrategy> Create(EIndexTreeEvctionStrategySystemType eType);
};


#endif // !__CODABASE_FACTORY_H__

