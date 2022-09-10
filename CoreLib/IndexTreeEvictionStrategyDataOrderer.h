#ifndef __INDEX_TREE_EVICTION_STRATEGY_LAST_ACCESSED_H__
#define __INDEX_TREE_EVICTION_STRATEGY_LAST_ACCESSED_H__
#include "BaseLib/AccessDataOrderer.h"
#include "IndexTreeEvictionStrategy.h"


class CIndexTreeEvictionStrategyDataOrderer : public CIndexTreeEvictionStrategy
{
protected:
	CLife<CIndexTreeDataOrderer>	mcLifeOrderer;
	CIndexTreeDataOrderer*			mpcOrderer;

public:
	void					Init(CLifeInit<CIndexTreeDataOrderer> cOrderer);
	void					Kill(void);

	void					SetIndexTree(CIndexTreeEvicting* pcIndexTree);

	bool					Run(CIndexTreeNodeFile* pcDontEvict);

	CIndexTreeDataOrderer*	GetDataOrderer(void);
};


#endif // !__INDEX_TREE_EVICTION_STRATEGY_LAST_ACCESSED_H__

