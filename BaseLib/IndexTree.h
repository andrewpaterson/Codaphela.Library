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


#define MAX_DATA_SIZE ((size)(8 KB))


class CIndexTree
{
friend class CIndexTreeDataOrderer;
protected:
	CLife<CMallocator>				mcMallocLife;
	CMallocator*					mpcMalloc;

	CLife<CIndexTreeDataOrderer>	mcDataOrdererLife;
	CIndexTreeDataOrderer*			mpcDataOrderer;

	EIndexKeyReverse				meReverseKey;
	size							mtSizeofNode;
	size							mtSizeofNodePtr;
	size							mtSizeofDataNode;
	size							miMaxDataSize;
	size							miMaxKeySize;

public:
			bool					Init(CLifeInit<CMallocator> cMalloc, EIndexKeyReverse eKeyReverse, size tSizeofNode, size tSizeofDataNode, size tSizeofNodePtr, size iMaxDataSize, size	iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	virtual bool					Kill(void);

	virtual bool					Remove(void* pvKey, size iKeySize) =0;
	virtual bool					HasKey(void* pvKey, size iKeySize) =0;

			size					SizeofNode(void);
			size					SizeofDataNode(void);
			size					SizeofNodePtr(void);
			EIndexKeyReverse		ReverseKeys(void);

			size					CalculateRootNodeSize(void);
			size					CalculateNodeSize(size iRequiredIndices, size iDataSize);

			void					GetNodeKey(CIndexTreeNode* pcNode, CArrayChar* pacKey);
	virtual void					Dump(void) =0;

			CIndexTreeDataOrderer*	GetDataOrderer();
			CMallocator*			GetMallocator();

protected:
			void*					Malloc(size uiSize);
			void*					Realloc(void* pv, size uiSize);
			void					Free(void* pv);

			bool					ValidatePut(size iKeySize, size iDataSize);

			void					InsertReorderData(CIndexTreeNode* pcNode);
			void					GetReorderData(CIndexTreeNode* pcNode);
			void					PutReorderData(CIndexTreeNode* pcNode);
			void					RemoveReorderData(CIndexTreeNode* pcNode);
			void					HasKeyReorderData(CIndexTreeNode* pcNode);

			void					FreeNode(CIndexTreeNode* pcNode);

			bool					StartKey(size* pi, size iKeySize);
			bool					LoopKey(size* pi, size iKeySize);

			CIndexTreeNode*			GetNodeForDataNode(CIndexTreeDataNode* pcDataNode);
			CIndexTreeNode*			GetNodeForData(void* pvData);
			void*					GetDataForDataNode(CIndexTreeDataNode* pcDataNode);
			void*					GetDataForNode(CIndexTreeNode* pcNode);

	virtual	size					GetNodeKey(CIndexTreeNode* pcNode, uint8* pvDestKey, size iDestKeySize) =0;
	virtual	size					GetNodeKeySize(CIndexTreeNode* pcNode) =0;
	virtual	size					GetNodeDataSize(CIndexTreeNode* pcNode) =0;
	virtual	size					GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, size iDestDataSize) =0;
};


#endif // __INDEX_TREE_H__

