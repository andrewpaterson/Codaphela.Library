#ifndef __INDEX_TREE_H__
#define __INDEX_TREE_H__
#include "Chars.h"
#include "PrimitiveTypes.h"
#include "Numbers.h"
#include "ArrayVoidPtr.h"
#include "ArrayTemplateEmbedded.h"
#include "MemoryCache.h"
#include "IndexTreeNode.h"
#include "IndexKeyReverse.h"
#include "LifeCycle.h"
#include "MapHelper.h"
#include "IndexTreeDataOrderer.h"


#define MAX_DATA_SIZE (8 KB)


class CIndexTree
{
friend class CIndexTreeDataOrderer;
protected:
	CLife<CMallocator>				mcMallocLife;
	CMallocator*					mpcMalloc;

	CLife<CIndexTreeDataOrderer>	mcDataOrdererLife;
	CIndexTreeDataOrderer*			mpcDataOrderer;

	EIndexKeyReverse				meReverseKey;
	size_t							mtSizeofNode;
	size_t							mtSizeofNodePtr;
	size_t							mtSizeofDataNode;
	size_t							miMaxDataSize;
	int								miMaxKeySize;

public:
			BOOL					Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, size_t tSizeofNode, size_t tSizeofDataNode, size_t tSizeofNodePtr, int iMaxDataSize, int	iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	virtual BOOL					Kill(void);

	virtual BOOL					Remove(void* pvKey, int iKeySize) =0;
	virtual BOOL					HasKey(void* pvKey, int iKeySize) =0;

			size_t					SizeofNode(void);
			size_t					SizeofDataNode(void);
			size_t					SizeofNodePtr(void);
			EIndexKeyReverse		ReverseKeys(void);

			size_t					CalculateRootNodeSize(void);
			size_t					CalculateNodeSize(int iRequiredIndices, size_t iDataSize);

			void					GetNodeKey(CIndexTreeNode* pcNode, CArrayChar* pacKey);
	virtual void					Dump(void) =0;

			CIndexTreeDataOrderer*	GetDataOrderer();
			CMallocator*			GetMallocator();

protected:
			void*					Malloc(size_t tSize);
			void*					Realloc(void* pv, size_t tSize);
			void					Free(void* pv);

			BOOL					ValidatePut(int iKeySize, size_t iDataSize);

			void					InsertReorderData(CIndexTreeNode* pcNode);
			void					GetReorderData(CIndexTreeNode* pcNode);
			void					PutReorderData(CIndexTreeNode* pcNode);
			void					RemoveReorderData(CIndexTreeNode* pcNode);
			void					HasKeyReorderData(CIndexTreeNode* pcNode);

			void					FreeNode(CIndexTreeNode* pcNode);

			BOOL					StartKey(int* pi, int iKeySize);
			BOOL					LoopKey(int* pi, int iKeySize);

			CIndexTreeNode*			GetNodeForDataNode(CIndexTreeDataNode* pcDataNode);
			CIndexTreeNode*			GetNodeForData(void* pvData);
			void*					GetDataForDataNode(CIndexTreeDataNode* pcDataNode);
			void*					GetDataForNode(CIndexTreeNode* pcNode);

	virtual	int						GetNodeKey(CIndexTreeNode* pcNode, char* pvDestKey, int iDestKeySize) =0;
	virtual	int						GetNodeKeySize(CIndexTreeNode* pcNode) =0;
	virtual	uint16					GetNodeDataSize(CIndexTreeNode* pcNode) =0;
	virtual	uint16					GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, int iDestDataSize) =0;
};


#endif // !__INDEX_TREE_H__

