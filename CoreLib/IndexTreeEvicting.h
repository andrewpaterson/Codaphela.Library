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
friend class CIndexedDescriptorsFile;
friend class CIndexTreeEvictionStrategyRandom;
private:
	CIndexTreeFile					mcIndexTree;
	size_t							muiCutoff;

	CIndexTreeEvictionCallback*		mpcIndexTreeEvictionCallback;
	CIndexTreeEvictionStrategy*		mpcEvictionStrategy;
	
public:
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileCallback* pcWriterCallback, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CIndexTreeEvictionStrategy* pcEvictionStrategy, CIndexTreeFileCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);

	BOOL					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvObject, int* piDataSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, int iDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);
	unsigned short			GetDataSize(void* pvKey, int iKeySize);

	BOOL					Evict(void* pvKey, int iKeySize);
	BOOL					Flush(void* pvKey, int iKeySize);

	BOOL					Flush(void);

	BOOL					IsWriteThrough(void);
	BOOL					IsFlushed(void);

	int						NumElements(void);

	void					Print(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize);
	void					Dump(void);

protected:
	CIndexedFiles*			GetIndexFiles(void);

	int						NumNodes(void);
	int						NumMemoryNodes(void);
	int						NumMemoryElements(void);

	void					DebugKey(CChars* pszDest, void* pvKey, int iKeySize, BOOL bSkipRoot, BOOL bShowFlags, BOOL bShowSize, BOOL bKeyAlreadyReversed);

	BOOL					EvictNodeWithObject(CIndexTreeNodeFile* pcNode);
	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetMemoryNode(void* pvKey, int iKeySize);
	BOOL					EvictNode(CIndexTreeNodeFile* pcNode);

	size_t					GetSystemMemorySize(void);
	unsigned char			GetRootFlags(void);

	int						PotentiallyEvict(void* pvKey, int iKeySize);
	BOOL					GetWithoutEviction(void* pvKey, int iKeySize, void* pvObject, int* piDataSize);
	BOOL					PutWithoutEviction(void* pvKey, int iKeySize, void* pvObject, int iDataSize);
};


#endif // __INDEX_TREE_EVICTING_H__

