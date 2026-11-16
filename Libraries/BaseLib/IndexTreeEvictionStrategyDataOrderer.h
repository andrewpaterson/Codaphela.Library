#ifndef __INDEX_TREE_EVICTION_STRATEGY_LAST_ACCESSED_H__
#define __INDEX_TREE_EVICTION_STRATEGY_LAST_ACCESSED_H__
#include "AccessDataOrderer.h"
#include "IndexTreeEvictionStrategy.h"


class CIndexTreeEvictionStrategyDataOrderer : public CIndexTreeEvictionStrategy
{
protected:
	CLife<CIndexTreeDataOrderer>	mcLifeOrderer;
	CIndexTreeDataOrderer*			mpcOrderer;

public:
	void					Init(CLifeInit<CIndexTreeDataOrderer> cOrderer);
	void					Kill(void) override;

	void					SetIndexTree(CIndexTreeEvicting* pcIndexTree) override;

	bool					Run(CIndexTreeNodeFile* pcDontEvict) override;

	CIndexTreeDataOrderer*	GetDataOrderer(void) override;
};


#endif // __INDEX_TREE_EVICTION_STRATEGY_LAST_ACCESSED_H__

