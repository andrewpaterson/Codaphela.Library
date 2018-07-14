#ifndef __INDEX_TREE_EVICTING_H__
#define __INDEX_TREE_EVICTING_H__
#include "BaseLib/StdRandom.h"
#include "FunctionCaller.h"
#include "IndexTreeEvictionCallback.h"
#include "IndexTreeFile.h"



class CIndexTreeEvicting;
typedef void(CIndexTreeEvicting::*NodeEvicted)(CIndexTreeNodeFile* pcNode);


class CIndexTreeEvicting
{
private:
	CIndexTreeFile					mcIndexTree;
	size_t							muiCutoff;

	CIndexTreeEvictionCallback*		mpcEvictionCallback;
	CRandom							mcRandom;
	
public:
	BOOL					Init(CDurableFileController* pcDurableFileControl, size_t uiCutoff, CIndexTreeEvictionCallback* pcEvictionCallback);
	BOOL					Init(CDurableFileController* pcDurableFileControl, size_t uiCutoff, CIndexTreeEvictionCallback* pcEvictionCallback, BOOL bWriteThrough);
	BOOL					Init(CDurableFileController* pcDurableFileControl, size_t uiCutoff, CIndexTreeEvictionCallback* pcEvictionCallback, CMallocator* pcMalloc, BOOL bWriteThrough);

	BOOL					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvObject, unsigned short* puiDataSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);
	unsigned short			ObjectSize(void* pvKey, int iKeySize);

	BOOL					IsWriteThrough(void);
	BOOL					Flush(void);

	CIndexedFiles*			GetIndexFiles(void);
	int						NumNodes(void);
	int						NumMemoryNodes(void);
	int						NumMemoryElements(void);
	void					DebugKey(void* pvKey, int iKeySize, BOOL bSkipRoot);
	void					Dump(void);

protected:
	void					PotentiallyEvict(void* pvKey, int iKeySize);
	BOOL					EvictRandomNode(CIndexTreeNodeFile* pcDontEvict);
	BOOL					EvictNodeCallback(CIndexTreeNodeFile* pcNode);
};


#endif // __INDEX_TREE_EVICTING_H__

