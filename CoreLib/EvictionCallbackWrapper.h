#ifndef __EVICTION_CALLBACK_WRAPPER_H__
#define __EVICTION_CALLBACK_WRAPPER_H__
#include "EvictionCallback.h"


class CEvictionCallbackWrapper : public CEvictionCallback
{
protected:
	CEvictionCallback*		mpcCallback1;
	CEvictionCallback*		mpcCallback2;

public:
	void Init(CEvictionCallback* pcCallback1, CEvictionCallback* pcCallback2);

	BOOL NodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);
};


#endif // __EVICTION_CALLBACK_WRAPPER_H__

