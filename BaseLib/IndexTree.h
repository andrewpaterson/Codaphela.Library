#ifndef __INDEX_TREE_H__
#define __INDEX_TREE_H__
#include "Chars.h"
#include "Numbers.h"
#include "ArrayVoidPtr.h"
#include "ArrayTemplateEmbedded.h"
#include "MemoryCache.h"
#include "IndexTreeNode.h"
#include "IndexKeyReverse.h"


#define MAX_KEY_SIZE (4 KB)
#define MAX_DATA_SIZE (8 KB)


class CIndexTree
{
protected:
	CMallocator*		mpcMalloc;

	EIndexKeyReverse	meReverseKey;
	size_t				mtSizeofNode;
	size_t				mtSizeofNodePtr;
	int					miMaxDataSize;
	int					miMaxKeySize;

public:
	BOOL				Init(CMallocator* pcMalloc, EIndexKeyReverse eKeyReverse, size_t tSizeofNode, size_t tSizeofNodePtr, int iMaxDataSize, int iMaxKeySize);

	size_t				SizeofNode(void);
	size_t				SizeofNodePtr(void);
	EIndexKeyReverse	ReverseKeys(void);

	size_t				CalculateRootNodeSize(void);
	size_t				CalculateNodeSize(int iRequiredIndices, int iDataSize);

protected:
	void*				Malloc(size_t tSize);
	void*				Realloc(void* pv, size_t tNewSize, size_t tExistingSize);
	void				Free(void* pv);

	void				FreeNode(CIndexTreeNode* pcNode);

	BOOL				StartKey(int* pi, int iKeysSize);
	BOOL				LoopKey(int* pi, int iKeysSize);
};


#endif // __INDEX_TREE_H__

