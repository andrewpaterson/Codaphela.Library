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
	size								muiCutoff;

	CIndexTreeEvictionCallback*			mpcIndexTreeEvictionCallback;
	CLife<CIndexTreeEvictionStrategy>	mcEvictionStrategy;
	CIndexTreeEvictionStrategy*			mpcEvictionStrategy;
	
public:
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexKeyReverse eKeyReverse);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, size uiCutoff, CIndexTreeEvictionCallback* pcIndexTreeEvictionCallback, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);

	bool					Kill(void);

	bool					Get(uint8* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize);
	bool					Put(uint8* pvKey, size iKeySize, void* pvData, size iDataSize);
	bool					Remove(uint8* pvKey, size iKeySize);
	bool					HasKey(uint8* pvKey, size iKeySize);
	size					GetDataSize(uint8* pvKey, size iKeySize);

	bool					Evict(uint8* pvKey, size iKeySize);
	bool					Flush(uint8* pvKey, size iKeySize);

	bool					Flush(void);

	void					SetDiagnosticCallback(CIndexTreeFileDiagnosticCallback* pcCallback);

	bool					IsWriteThrough(void);
	bool					IsFlushed(void);
	bool					ValidateIndexTree(void);
	bool					ValidateIndexTree(bool bReadNodes);
	bool					HasDiagnosticCallback(void);
	CIndexTreeDataOrderer*	GetDataOrderer(void);

	int64					NumElements(void);

	bool					StartIteration(SIndexTreeFileIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* piDataSize, size iMaxDataSize);
	bool					Iterate(SIndexTreeFileIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* piDataSize, size iMaxDataSize);

	void					Print(CChars* pszDest, bool bShowFlags, bool bShowSize);
	void					Dump(void);

protected:
	CIndexedFiles*			GetIndexFiles(void);
	CIndexTreeFile*			GetIndexTree(void);

	size					NumNodes(void);
	size					NumMemoryNodes(void);
	size					NumMemoryElements(void);
	size					NumMemoryElements(size iSize);

	void					DebugKey(CChars* pszDest, uint8* pvKey, size iKeySize, bool bSkipRoot, bool bShowFlags, bool bShowSize, bool bKeyAlreadyReversed);

	bool					EvictNodeWithObject(CIndexTreeNodeFile* pcNode);
	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetMemoryNode(uint8* pvKey, size iKeySize);
	bool					EvictNode(CIndexTreeNodeFile* pcNode);
	size					GetNodeKey(CIndexTreeNodeFile* pcNode, uint8* pvDestKey, size iDestKeySize);
	size					GetNodeKeySize(CIndexTreeNodeFile* pcNode);

	size					GetSystemMemorySize(void);
	size					GetUserMemorySize(void);
	size					GetRootFlags(void);

	size					PotentiallyEvict(uint8* pvKey, size iKeySize);
	bool					GetWithoutEviction(uint8* pvKey, size iKeySize, void* pvObject, size* piDataSize, size uiMaxDataSize);
	bool					PutWithoutEviction(uint8* pvKey, size iKeySize, void* pvObject, size iDataSize);
};


#endif // __INDEX_TREE_EVICTING_H__

