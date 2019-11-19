#ifndef __INDEX_TREE_EVICTING_H__
#define __INDEX_TREE_EVICTING_H__
#include "FunctionCaller.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexTreeEvictionStrategy.h"
#include "IndexTreeFile.h"


class CIndexTreeEvicting;
typedef void(CIndexTreeEvicting::*NodeEvicted)(CIndexTreeNodeFile* pcNode);


class CIndexTreeEvicting
{
private:
	CIndexTreeFile					mcIndexTree;
	size_t							muiCutoff;

	CIndexTreeEvictionCallback*		mpcEvictionCallback;
	CIndexTreeEvictionStrategy*		mpcEvictionStrategy;
	
public:
	BOOL					Init(CDurableFileController* pcDurableFileControl, size_t uiCutoff, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileCallback* pcWriterCallback);
	BOOL					Init(CDurableFileController* pcDurableFileControl, size_t uiCutoff, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileCallback* pcWriterCallback, BOOL bWriteThrough);
	BOOL					Init(CDurableFileController* pcDurableFileControl, size_t uiCutoff, CIndexTreeEvictionCallback* pcEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileCallback* pcWriterCallback, CMallocator* pcMalloc, BOOL bWriteThrough);

	BOOL					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvObject, unsigned short* puiDataSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);
	unsigned short			ObjectSize(void* pvKey, int iKeySize);

	BOOL					IsWriteThrough(void);
	BOOL					Flush(void);

	int						NumElements(void);
	int						NumMemoryElements(void);

	CIndexedFiles*			GetIndexFiles(void);
	int						NumNodes(void);
	int						NumMemoryNodes(void);
	void					DebugKey(void* pvKey, int iKeySize, BOOL bSkipRoot);
	void					Dump(void);
	BOOL					Evict(void* pvKey, int iKeySize);

	BOOL					EvictNodeCallback(CIndexTreeNodeFile* pcNode);
	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetMemoryNode(void* pvKey, int iKeySize);
	BOOL					Evict(CIndexTreeNodeFile* pcNode);

	size_t					GetSystemMemorySize(void);

protected:
	int						PotentiallyEvict(void* pvKey, int iKeySize);
	BOOL					GetWithoutEviction(void* pvKey, int iKeySize, void* pvObject, unsigned short* puiDataSize);
	BOOL					PutWithoutEviction(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize);
};


#endif // __INDEX_TREE_EVICTING_H__

