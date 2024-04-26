#ifndef __INDEX_TREE_EVICTION_STRATEGY_H__
#define __INDEX_TREE_EVICTION_STRATEGY_H__
#include "IndexTreeDataOrderer.h"
#include "IndexTreeNodeFile.h"


class CIndexTreeEvicting;
class CIndexTreeEvictionStrategy : public CKillable
{
protected:
	CIndexTreeEvicting*		mpcIndexTree;

protected:
			void					Init(void);

public:
	virtual void					SetIndexTree(CIndexTreeEvicting* pcIndexTree);
	virtual bool					Run(CIndexTreeNodeFile* pcDontEvict) =0;

	virtual CIndexTreeDataOrderer*	GetDataOrderer(void);

protected:
			bool					EvictNode(CIndexTreeNodeFile* pcNode);
			CIndexTreeNodeFile*		GetRootNode(void);
};


#endif // __INDEX_TREE_EVICTION_STRATEGY_H__

