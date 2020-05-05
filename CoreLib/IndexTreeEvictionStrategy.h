#ifndef __INDEX_TREE_EVICTION_STRATEGY_H__
#define __INDEX_TREE_EVICTION_STRATEGY_H__
#include "IndexTreeNodeFile.h"


class CIndexTreeEvicting;
class CIndexTreeEvictionStrategy
{
protected:
	CIndexTreeEvicting*		mpcIndexTree;

public:
			void SetIndexTree(CIndexTreeEvicting* pcIndexTree);
	virtual BOOL Run(CIndexTreeNodeFile* pcDontEvict) =0;
	virtual void Kill(void) =0;
};


#endif // __INDEX_TREE_EVICTION_STRATEGY_H__

