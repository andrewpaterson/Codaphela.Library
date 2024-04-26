#ifndef __INDEX_TREE_EVICTION_CALLBACK_H__
#define __INDEX_TREE_EVICTION_CALLBACK_H__


class CIndexTreeEvictionCallback
{
public:
	virtual bool IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize) =0;
};


#endif // __INDEX_TREE_EVICTION_CALLBACK_H__

