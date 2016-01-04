#ifndef __INDEX_TREE_BLOCK_H__
#define __INDEX_TREE_BLOCK_H__
#include "Chars.h"
#include "ArrayVoidPtr.h"
#include "ArrayTemplateEmbedded.h"
#include "MemoryCache.h"
#include "IndexTreeNode.h"


class CIndexTreeBlock
{
protected:
	CMallocator*	mpcMalloc;
	int				miLargestKeySize;

	size_t			mtSizeofNode;
	size_t			mtSizeofNodePtr;

public:
	void				Init(CMallocator* pcMalloc, size_t tSizeofNode, size_t tSizeofNodePtr);

	size_t				SizeofNode(void);
	size_t				SizeofNodePtr(void);

protected:
	void*				Malloc(size_t tSize);
	void*				Realloc(void* pv, size_t tSize);
	void				Free(void* pv);

	void				FreeNode(CIndexTreeNode* pcNode);
};


#endif // __INDEX_TREE_BLOCK_H__
