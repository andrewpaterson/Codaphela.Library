#ifndef __INDEX_TREE_EVICTION_CALLBACK_H__
#define __INDEX_TREE_EVICTION_CALLBACK_H__
#include "BaseLib/Bool.h"

class CIndexTreeEvictionCallback
{
public:
	virtual BOOL NodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize) =0;
};


#endif // __INDEX_TREE_EVICTION_CALLBACK_H__

