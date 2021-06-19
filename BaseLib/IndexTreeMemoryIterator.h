#ifndef __INDEX_TREE_MEMORY_ITERATOR_H__
#define __INDEX_TREE_MEMORY_ITERATOR_H__
#include "IndexTreeIterator.h"
#include "IndexTreeMemory.h"


#define INDEX_TREE_MEMORY_ITERATOR_TYPE 0x46782234


class CIndexTreeMemoryIterator : public CIndexTreeIterator
{
protected:
	SIndexTreeMemoryIterator	sIter;
	CIndexTreeMemory*			mpcTree;

public:
	void	Init(CIndexTreeMemory* pcIndexTree);
	void	Kill(void);

	BOOL	Iterate(void);
};


#endif // !__INDEX_TREE_MEMORY_ITERATOR_H__

