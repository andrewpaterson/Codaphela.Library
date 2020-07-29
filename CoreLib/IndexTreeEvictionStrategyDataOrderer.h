#ifndef __INDEX_TREE_EVICTION_STRATEGY_LAST_ACCESSED_H__
#define __INDEX_TREE_EVICTION_STRATEGY_LAST_ACCESSED_H__
#include "BaseLib/AccessDataOrderer.h"
#include "IndexTreeEvictionStrategy.h"


class CIndexTreeEvictionStrategyDataOrderer : public CIndexTreeEvictionStrategy
{
protected:
	CIndexTreeDataOrderer*	mpcOrderer;

public:
	void Init(CIndexTreeDataOrderer* pcOrderer);
	void Kill(void);

	BOOL Run(CIndexTreeNodeFile* pcDontEvict);
};


#endif // __INDEX_TREE_EVICTION_STRATEGY_LAST_ACCESSED_H__

