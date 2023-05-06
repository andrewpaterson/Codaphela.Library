#ifndef __INDEX_TREE_EVICTION_STRATEGY_RANDOM_H__
#define __INDEX_TREE_EVICTION_STRATEGY_RANDOM_H__
#include "StdRandom.h"
#include "IndexTreeEvictionStrategy.h"


class CIndexTreeEvictionStrategyRandom : public CIndexTreeEvictionStrategy
{
protected:
	CRandom		mcRandom;

public:
	void		Init(void);
	void		Init(int iSeed);
	void		Kill(void);

	bool		Run(CIndexTreeNodeFile* pcDontEvict);
	CRandom*	GetRandom(void);

protected:
	bool		EvictRandomNode(CIndexTreeNodeFile* pcDontEvict);
};


#endif // !__INDEX_TREE_EVICTION_STRATEGY_RANDOM_H__

