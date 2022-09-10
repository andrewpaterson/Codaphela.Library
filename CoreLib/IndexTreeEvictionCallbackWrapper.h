#ifndef __INDEX_TREE_EVICTION_CALLBACK_WRAPPER_H__
#define __INDEX_TREE_EVICTION_CALLBACK_WRAPPER_H__
#include "IndexTreeEvictionCallback.h"


class CIndexTreeEvictionCallbackWrapper : public CIndexTreeEvictionCallback
{
protected:
	CIndexTreeEvictionCallback*		mpcCallback1;
	CIndexTreeEvictionCallback*		mpcCallback2;

public:
	void Init(CIndexTreeEvictionCallback* pcCallback1, CIndexTreeEvictionCallback* pcCallback2);

	bool IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);
};


#endif // !__EVICTION_CALLBACK_WRAPPER_H__

