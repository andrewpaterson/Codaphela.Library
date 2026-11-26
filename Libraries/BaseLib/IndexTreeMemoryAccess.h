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
	bool				Kill(void) override;
	bool				Flush(void) override;
	int64				NumElements(void) const override;
	bool				ValidateIndex(void) override;
	void				Dump(void) override;

	CIndexTreeIterator*	CreateIterator(void) override;
	void				FreeIterator(CIndexTreeIterator* pcIter) override;

protected:
	bool	Put(uint8* pvKey, size iKeySize, const void* pvData, size iDataSize) override;
	bool	Get(uint8* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize) override;
	bool	Remove(uint8* pvKey, size iKeySize) override;
	bool	Has(uint8* pvKey, size iKeySize) override;
	size	DataSize(uint8* pvKey, size iKeySize) override;

	bool	Flush(uint8* pvKey, size iKeySize) override;
	bool	Evict(uint8* pvKey, size iKeySize) override;
};


#endif // __INDEX_TREE_MEMORY_ACCESS_H__

