#ifndef __EVICTION_CALLBACK_WRAPPER_H__
#define __EVICTION_CALLBACK_WRAPPER_H__
#include "IndexTreeEvictionCallback.h"


class CEvictionCallbackWrapper : public CIndexTreeEvictionCallback
{
protected:
	CIndexTreeEvictionCallback*		mpcCallback1;
	CIndexTreeEvictionCallback*		mpcCallback2;

public:
	void Init(CIndexTreeEvictionCallback* pcCallback1, CIndexTreeEvictionCallback* pcCallback2);

	BOOL IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);
};


#endif // __EVICTION_CALLBACK_WRAPPER_H__

