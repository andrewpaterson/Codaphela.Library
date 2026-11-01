#ifndef __TEST_INDEX_TREE_MEMORY_H__
#define __TEST_INDEX_TREE_MEMORY_H__
#include "BaseLib/IndexTreeMemory.h"


class CTestIndexTreeMemory : public CIndexTreeMemory
{
public:
	void FakeInit(EIndexKeyReverse eKeyReverse)
	{
		//This exists so that TreeNodes can be tested without a full tree.  All they need to do is query the size of their child nodes.
		//Kill should not be called.
		CLifeInit<CMallocator> cMalloc;

		cMalloc.Init(&gcSystemAllocator, false, false);
		CIndexTree::Init(cMalloc, eKeyReverse, sizeof(CIndexTreeNodeMemory), sizeof(CIndexTreeNodeMemory) + sizeof(CIndexTreeDataNode), sizeof(CIndexTreeNodeMemory*), MAX_DATA_SIZE, MAX_KEY_SIZE, LifeNull<CIndexTreeDataOrderer>());
		mpcRoot = NULL;
		miSize = 0;
	}

	CIndexTreeNodeMemory* GetRoot(void)
	{
		return CIndexTreeMemory::GetRoot();
	}

	CIndexTreeNodeMemory* GetNode(uint8* pvKey, size iKeySize)
	{
		return CIndexTreeMemory::GetNode(pvKey, iKeySize);
	}

	bool ValidateSize(void)
	{
		return CIndexTreeMemory::ValidateSize();
	}

	size NumAllocatedNodes(void)
	{
		return CIndexTreeMemory::NumAllocatedNodes();
	}

	size RecurseSize(void)
	{
		return CIndexTreeMemory::RecurseSize();
	}
};


#endif // __TEST_INDEX_TREE_MEMORY_H__

