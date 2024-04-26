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
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexKeyReverse eKeyReverse);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size_t uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);

	bool					Kill(void);

	bool					Get(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize);
	bool					Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
	bool					Remove(void* pvKey, int iKeySize);
	bool					HasKey(void* pvKey, int iKeySize);
	uint16					GetDataSize(void* pvKey, int iKeySize);

	bool					Evict(void* pvKey, int iKeySize);
	bool					Flush(void* pvKey, int iKeySize);

	bool					Flush(void);

	void					SetDiagnosticCallback(CIndexTreeFileDiagnosticCallback* pcCallback);

	bool					IsWriteThrough(void);
	bool					IsFlushed(void);
	bool					ValidateIndexTree(void);
	bool					ValidateIndexTree(bool bReadNodes);
	bool					HasDiagnosticCallback(void);
	CIndexTreeDataOrderer*	GetDataOrderer(void);

	int64					NumElements(void);

	bool					StartIteration(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* piDataSize, size_t iMaxDataSize);
	bool					Iterate(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* piDataSize, size_t iMaxDataSize);

	void					Print(CChars* pszDest, bool bShowFlags, bool bShowSize);
	void					Dump(void);

protected:
	CIndexedFiles*			GetIndexFiles(void);
	CIndexTreeFile*			GetIndexTree(void);

	int						NumNodes(void);
	int						NumMemoryNodes(void);
	int						NumMemoryElements(void);
	int						NumMemoryElements(size_t iSize);

	void					DebugKey(CChars* pszDest, void* pvKey, int iKeySize, bool bSkipRoot, bool bShowFlags, bool bShowSize, bool bKeyAlreadyReversed);

	bool					EvictNodeWithObject(CIndexTreeNodeFile* pcNode);
	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetMemoryNode(void* pvKey, int iKeySize);
	bool					EvictNode(CIndexTreeNodeFile* pcNode);
	int						GetNodeKey(CIndexTreeNodeFile* pcNode, char* pvDestKey, int iDestKeySize);
	int						GetNodeKeySize(CIndexTreeNodeFile* pcNode);

	size_t					GetSystemMemorySize(void);
	size_t					GetUserMemorySize(void);
	uint8					GetRootFlags(void);

	int						PotentiallyEvict(void* pvKey, int iKeySize);
	bool					GetWithoutEviction(void* pvKey, int iKeySize, void* pvObject, size_t* piDataSize, size_t uiMaxDataSize);
	bool					PutWithoutEviction(void* pvKey, int iKeySize, void* pvObject, size_t iDataSize);
};


#endif // __INDEX_TREE_EVICTING_H__

