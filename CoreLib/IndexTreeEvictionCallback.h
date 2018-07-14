#ifndef INDEX_TREE_EVICTION_CALLBACK_H__
#define INDEX_TREE_EVICTION_CALLBACK_H__
#include "BaseLib/Bool.h"

class CIndexTreeNodeFile;
class CIndexTreeFile;
class CIndexTreeEvictionCallback
{
public:
	virtual BOOL NodeEvicted(CIndexTreeFile* pcIndexTree, unsigned char* pvKey, int iKeySize, void* pvData, int iDataSize) =0;
};


#endif // INDEX_TREE_EVICTION_CALLBACK_H__

