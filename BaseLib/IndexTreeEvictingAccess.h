#ifndef __INDEX_TREE_EVICTING_ACCESS_H__
#define __INDEX_TREE_EVICTING_ACCESS_H__
#include "IndexAccess.h"
#include "IndexTreeEvicting.h"


class CIndexTreeEvictingAccess : public CIndexAccess
{
private:
	CIndexTreeEvicting*		mpcTree;

public:
	bool				Init(CIndexTreeEvicting* pcTree);
	bool				Kill(void);
	bool				Flush(void);
	int64				NumElements(void);
	bool				ValidateIndex(void);
	void				Dump(void);

	CIndexTreeIterator* CreateIterator(void);
	void				FreeIterator(CIndexTreeIterator* pcIter);

protected:
	bool	Put(uint8* pvKey, size iKeySize, void* pvData, size iDataSize);
	bool	Get(uint8* pvKey, size iKeySize, void* pvData, size* uiDataSize, size uiMaxDataSize);
	bool	Remove(uint8* pvKey, size iKeySize);
	size	DataSize(uint8* pvKey, size iKeySize);
	bool	Has(uint8* pvKey, size iKeySize);
	bool	Evict(uint8* pvKey, size iKeySize);
	bool	Flush(uint8* pvKey, size iKeySize);
};


#endif // __INDEX_TREE_EVICTING_ACCESS_H__

