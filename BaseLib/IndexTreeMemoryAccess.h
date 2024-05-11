#ifndef __INDEX_TREE_MEMORY_ACCESS_H__
#define __INDEX_TREE_MEMORY_ACCESS_H__
#include "IndexTreeMemory.h"
#include "IndexAccess.h"
#include "IndexTreeIterator.h"

class CIndexTreeMemoryAccess : public CIndexAccess
{
private:
	CIndexTreeMemory*	mpcTree;

public:
	bool				Init(CIndexTreeMemory* pcTree);
	bool				Kill(void);
	bool				Flush(void);
	int64				NumElements(void);
	bool				ValidateIndex(void);
	void				Dump(void);

	CIndexTreeIterator* CreateIterator(void);
	void				FreeIterator(CIndexTreeIterator* pcIter);

protected:
	bool	Put(void* pvKey, size iKeySize, void* pvData, size iDataSize);
	bool	Get(void* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize);
	bool	Remove(void* pvKey, size iKeySize);
	bool	Has(void* pvKey, size iKeySize);
	size	DataSize(void* pvKey, size iKeySize);

	bool	Flush(void* pvKey, size iKeySize);
	bool	Evict(void* pvKey, size iKeySize);
};


#endif // __INDEX_TREE_MEMORY_ACCESS_H__

