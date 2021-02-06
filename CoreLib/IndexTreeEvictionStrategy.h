#ifndef __INDEX_TREE_EVICTION_STRATEGY_H__
#define __INDEX_TREE_EVICTION_STRATEGY_H__
#include "BaseLib/IndexTreeDataOrderer.h"
#include "IndexTreeNodeFile.h"


class CIndexTreeEvicting;
class CIndexTreeEvictionStrategy
{
protected:
	CIndexTreeEvicting*		mpcIndexTree;

protected:
			void					Init(void);

public:
	virtual void					SetIndexTree(CIndexTreeEvicting* pcIndexTree);
	virtual BOOL					Run(CIndexTreeNodeFile* pcDontEvict) =0;
	virtual void					Kill(void) =0;

	virtual CIndexTreeDataOrderer*	GetDataOrderer(void);

protected:
			BOOL					EvictNode(CIndexTreeNodeFile* pcNode);
			CIndexTreeNodeFile*		GetRootNode(void);
};


#endif // __INDEX_TREE_EVICTION_STRATEGY_H__

