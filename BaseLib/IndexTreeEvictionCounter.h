#ifndef __INDEX_TREE_EVICTION_COUNTER_H__
#define __INDEX_TREE_EVICTION_COUNTER_H__
#include "PrimitiveTypes.h"
#include "IndexTreeEvictionCallback.h"


class CIndexTreeEvictionCounter : public CIndexTreeEvictionCallback
{
private:
	int64	miEvictions;

public:
	void	Init(void);
	void	Reset(void);
	bool	IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	int64	EvictionCount(void);
};


#endif // !__INDEX_TREE_EVICTION_COUNTER_H__

