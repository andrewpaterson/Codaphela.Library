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
friend class CIndexTreeEvictionStrategy;
friend class CIndexTreeEvictingAccess;
friend class CNamedIndexes;
private:
	CIndexTreeFile						mcIndexTree;
	size_t								muiCutoff;

	CIndexTreeEvictionCallback*			mpcIndexTreeEvictionCallback;
	CLife<CIndexTreeEvictionStrategy>	mcEvictionStrategy;
	CIndexTreeEvictionStrategy*			mpcEvictionStrategy;
	
public:
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);

	BOOL					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvObject, size_t* piDataSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, size_t iDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);
	uint16					GetDataSize(void* pvKey, int iKeySize);

	BOOL					Evict(void* pvKey, int iKeySize);
	BOOL					Flush(void* pvKey, int iKeySize);

	BOOL					Flush(void);

	void					SetDiagnosticCallback(CIndexTreeFileDiagnosticCallback* pcCallback);

	BOOL					IsWriteThrough(void);
	BOOL					IsFlushed(void);
	BOOL					ValidateIndexTree(void);
	BOOL					ValidateIndexTree(BOOL bReadNodes);
	BOOL					HasDiagnosticCallback(void);
	CIndexTreeDataOrderer*	GetDataOrderer(void);

	int						NumElements(void);

	BOOL					StartIteration(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* piDataSize, size_t iMaxDataSize);
	BOOL					Iterate(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* piDataSize, size_t iMaxDataSize);

	void					Print(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize);
	void					Dump(void);

protected:
	CIndexedFiles*			GetIndexFiles(void);
	CIndexTreeFile*			GetIndexTree(void);

	int						NumNodes(void);
	int						NumMemoryNodes(void);
	int						NumMemoryElements(void);
	int						NumMemoryElements(size_t iSize);

	void					DebugKey(CChars* pszDest, void* pvKey, int iKeySize, BOOL bSkipRoot, BOOL bShowFlags, BOOL bShowSize, BOOL bKeyAlreadyReversed);

	BOOL					EvictNodeWithObject(CIndexTreeNodeFile* pcNode);
	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetMemoryNode(void* pvKey, int iKeySize);
	BOOL					EvictNode(CIndexTreeNodeFile* pcNode);
	int						GetNodeKey(CIndexTreeNodeFile* pcNode, char* pvDestKey, int iDestKeySize);
	int						GetNodeKeySize(CIndexTreeNodeFile* pcNode);

	size_t					GetSystemMemorySize(void);
	unsigned char			GetRootFlags(void);

	int						PotentiallyEvict(void* pvKey, int iKeySize);
	BOOL					GetWithoutEviction(void* pvKey, int iKeySize, void* pvObject, size_t* piDataSize);
	BOOL					PutWithoutEviction(void* pvKey, int iKeySize, void* pvObject, size_t iDataSize);
};


#endif // __INDEX_TREE_EVICTING_H__

