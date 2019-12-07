#ifndef __EVICTION_CALLBACK_H__
#define __EVICTION_CALLBACK_H__
#include "BaseLib/Bool.h"

class CIndexTreeNodeFile;
class CIndexTreeFile;
class CEvictionCallback
{
public:
	virtual BOOL NodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize) =0;
};


#endif // __EVICTION_CALLBACK_H__

