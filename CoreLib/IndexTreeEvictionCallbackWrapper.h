#include "IndexTreeEvictionCallback.h"


class CIndexTreeEvictionCallbackWrapper : public CIndexTreeEvictionCallback
{
protected:
	CIndexTreeEvictionCallback*		mpcCallback1;
	CIndexTreeEvictionCallback*		mpcCallback2;

public:
	void Init(CIndexTreeEvictionCallback* pcCallback1, CIndexTreeEvictionCallback* pcCallback2);
	BOOL NodeEvicted(CIndexTreeFile* pcIndexTree, unsigned char* pvKey, int iKeySize, void* pvData, int iDataSize);
};

