#ifndef __INDEX_TREE_FILE_H__
#define __INDEX_TREE_FILE_H__
#include "Chars.h"
#include "IndexTree.h"
#include "IndexTreeRecursor.h"
#include "ListCharsMinimal.h"
#include "CountingAllocator.h"
#include "IndexKeyReverse.h"
#include "IndexTreeIterator.h"
#include "DurableFileController.h"
#include "IndexTreeNodeFile.h"
#include "IndexTreeFileDebug.h"
#include "IndexedFiles.h"
#include "IndexTreeFileDiagnosticCallback.h"
#include "IndexWriteThrough.h"


struct SIndexTreeFileUnsafeIterator
{
	CIndexTreeNodeFile*		pcNode;
	size					iIndex;
};


struct SIndexTreeFileIterator
{
	uint8	pvKey[MAX_KEY_SIZE];
	size	iKeyLength;
	size	iIndex;
};


class CIndexTreeFile : public CIndexTree
{
friend class CIndexTreeEvicting;
friend class CIndexTreeWriter;
friend class CIndexTreeNodeFile;
friend class CIndexTreeFileIterator;
protected:
	CIndexTreeNodeFile*						mpcRoot;
	CIndexedFiles							mcIndexFiles;
	CDurableFileController*					mpcDurableFileControl;
	EIndexWriteThrough						meWriteThrough;
	CDurableFile							mcRootIndex;

	CCountingAllocator						mcMalloc;
	CMallocator*							mpcPassThruMallocLife;
	CLife<CMallocator>						mcPassThruMallocLife;

	CIndexTreeFileDataCallback*				mpcDataCallback;
	CIndexTreeFileDiagnosticCallback*		mpcDiagnosticCallback;

public:
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, int iMaxDataSize, int iMaxKeySize);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize);
	bool					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CLifeInit<CMallocator> cMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CLifeInit<CIndexTreeDataOrderer> cDataOrderer);
	bool					Kill(void);

	bool					Get(void* pvKey, size iKeySize, void* pvObject, size* piDataSize, size uiMaxDataSize);
	bool					Put(void* pvKey, size iKeySize, void* pvObject, size iDataSize);
	bool					Remove(void* pvKey, size iKeySize);
	bool					HasKey(void* pvKey, size iKeySize);
	size					GetDataSize(void* pvKey, size iKeySize);

	bool					Flush(void);

	bool					StartIteration(SIndexTreeFileIterator* psIterator, void* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* piDataSize, size iMaxDataSize);
	bool					Iterate(SIndexTreeFileIterator* psIterator, void* pvKey, size* piKeySize, size iMaxKeySize, void* pvData, size* piDataSize, size iMaxDataSize);
	bool					StartUnsafeIteration(SIndexTreeFileUnsafeIterator* psIterator, void* pvKey, size* piKeySize, size iMaxKeySize, void** ppvData, size* piDataSize);
	bool					UnsafeIterate(SIndexTreeFileUnsafeIterator* psIterator, void* pvKey, size* piKeySize, size iMaxKeySize, void** pvData, size* piDataSize);
	bool					StartUnsafeIteration(SIndexTreeFileUnsafeIterator* psIterator, void** ppvData, size* piDataSize);
	bool					UnsafeIterate(SIndexTreeFileUnsafeIterator* psIterator, void** ppvData, size* piDataSize);

	bool					Evict(void* pvKey, size iKeySize);
	bool					Flush(void* pvKey, size iKeySize);

	void					SetDiagnosticCallback(CIndexTreeFileDiagnosticCallback* pcCallback);

	bool					IsFlushed(void);
	bool					ValidateIndexTree(void);
	bool					ValidateParentIndex(void);

	uint32					NumElements(void);
	bool					IsWriteThrough(void);
	bool					HasDiagnosticCallback(void);
	CDurableFileController* GetController(void);

	void					Print(CChars* pszDest, bool bShowFlags, bool bShowSize);
	void					Dump(void);

protected:
	bool					Evict(CIndexTreeNodeFile* pcNode);

	void					SetWriteThrough(EIndexWriteThrough eWriteThrough);

	CIndexTreeNodeFile*		ParentPut(CIndexTreeNodeFile* pcParent, size uiIndexInParent, uint8 uiDataSize, uint8 uiFirstIndex, uint8 uiLastIndex);
	CIndexTreeNodeFile*		ParentPut(CIndexTreeNodeFile* pcParent, size uiIndexInParent, uint8 uiDataSize);

	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetNode(void* pvKey, size iKeySize);
	CIndexTreeNodeFile*		GetMemoryNode(void* pvKey, size iKeySize);
	CIndexTreeNodeFile*		GetNodeForData(void* pvData);
	size					GetNodeKeySize(CIndexTreeNode* pcNode);
	size					GetNodeKey(CIndexTreeNode* pcNode, uint8* pvDestKey, size iDestKeySize);
	size					GetNodeDataSize(CIndexTreeNode* pcNode);
	size					GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, size iDestDataSize);

	CListTemplateMinimal<char>* GetNodesKeys(CArrayVoidPtr* apvNodes);
	CListCharsMinimal*		GetNodesStringKeys(CArrayVoidPtr* apvNodes);
	int						GetNodeKeysSize(CArrayVoidPtr* apvNodes);

	bool					ValidateIndexTree(bool bReadNodes);
	bool					ValidateKey(void* pvKey, size iKeySize);
	bool					ValidateKey(char* pszKey);

	size					ByteSize(void);
	size					GetUserMemorySize(void);
	size					GetSystemMemorySize(void);

	CIndexedFiles*			GetIndexFiles(void);
	void					GetFiles(CArrayIndexedFilePtr* pac);
	CIndexedFile*			GetFile(int iFile);
	uint32					NumNodes(void);
	size					NumMemoryNodes(void);
	size					NumMemoryElements(void);
	size					NumMemoryElements(size iSize);

	void					DebugKey(CChars* pszDest, void* pvKey, size iKeySize, bool bSkipRoot, bool bShowFlags, bool bShowSize, bool bKeyAlreadyReversed);
	void					PrintChildren(CChars* pszDest, bool bShowFlags, bool bShowSize);

	bool					InitRoot(char* szSubDirectory);
	void					RecurseKill(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, size c);
	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, size c, bool bReadNode);
	bool					Read(CIndexTreeChildNode* pcChildNode, CIndexTreeNodeFile* pcFileNodeParent, size uiIndexInParent);
	CIndexTreeNodeFile*		ReadMemoryNode(CIndexTreeNodeFile* pcParent, size c);
	bool					HasMemoryNodes(CIndexTreeNodeFile* pcNode);
	
	bool					Write(CIndexTreeNodeFile* pcNode);
	bool					Delete(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		AllocateRoot(void);
	CIndexTreeNodeFile*		AllocateRoot(CFileDataIndex cFileIndex);
	CIndexTreeNodeFile*		AllocateNodeSingle(CIndexTreeNodeFile* pcParent, size uiIndexInParent, size uiDataSize);
	CIndexTreeNodeFile*		AllocateNodeRange(CIndexTreeNodeFile* pcParent, size uiIndexInParent, size uiFirstIndex, size uiLastIndex, size uiDataSize);
	CIndexTreeNodeFile*		AllocateNodeFromBuffer(CIndexTreeNodeFile* pcParent, size uiIndexInParent, void* pvBuffer, int iMaxBufferSize);

	CIndexTreeNodeFile*		ReallocateNodeForContainIndex(CIndexTreeNodeFile* pcNode, size uiIndex);
	CIndexTreeNodeFile*		ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, void* pvData, size uiDataSize);
	CIndexTreeNodeFile*		ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, void* pvData, size uiOriginalSize);
	CIndexTreeNodeFile*		ReallocateNodeForUncontainIndex(CIndexTreeNodeFile* pcNode, size c);

	CIndexTreeNodeFile*		AllocateChildNode(CIndexTreeNodeFile* pcParent, size uiIndexInParent);
	CIndexTreeNodeFile*		SetNodeData(CIndexTreeNodeFile* pcCurrent, void* pvObject, size uiDataSize);
	CIndexTreeNodeFile*		AllocateKey(void* pvKey, size iKeySize);
	bool					HasData(CIndexTreeNodeFile* pcNode);
	void					RemapNodePointers(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		RemoveWriteThrough(CIndexTreeNodeFile* pcCurrent);
	bool					RemoveWaitForFlush(CIndexTreeNodeFile* pcCurrent);
	bool					EvictNode(CIndexTreeNodeFile* pcCurrent);
	bool					Flush(CIndexTreeNodeFile** ppcCurrent);
	bool					CanEvict(CIndexTreeNodeFile* pcNode);

	void					ClearNodesFlags(CArrayVoidPtr* papNodes, uint8 uiFlags);
	void					FindWithFlags(CArrayVoidPtr* papNodes, uint8 uiFollowFlags, uint8 uiAddFlags);
	uint32					RecurseNumElements(CIndexTreeNodeFile* pcNode);
	size					RecurseNumMemoryElements(CIndexTreeNodeFile* pcNode);
	size					RecurseNumMemoryElements(CIndexTreeNodeFile* pcNode, size iSize);
	size					CountListSize(void);
	size					RecurseCountListSize(CIndexTreeNodeFile* pcNode);
	uint32					RecurseByteSize(CIndexTreeNodeFile* pcNode);
	void					RecurseFindWithFlags(CIndexTreeRecursor* pcCursor, uint8 uiFollowFlags, uint8 uiAddFlags, CArrayVoidPtr* papNodes);
	uint32					RecurseNumNodes(CIndexTreeRecursor* pcCursor);
	size					RecurseNumMemoryNodes(CIndexTreeRecursor* pcCursor);

	bool					ValidateLimits(bool bReadNodes);
	bool					RecurseValidateLimits(CIndexTreeRecursor* pcCursor, bool bReadNodes);
	bool					ValidateParentIndex(bool bReadNodes);
	bool					RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor, bool bReadNodes);
	bool					ValidateTransientFlags(bool bReadNodes);
	bool					RecurseValidateTransientFlags(CIndexTreeRecursor* pcCursor, bool bReadNodes);
	bool					ValidateMagic(bool bReadNodes);
	bool					RecurseValidateMagic(CIndexTreeRecursor* pcCursor, bool bReadNodes);
	bool					ValidateFileIndexes(bool bReadNodes);
	bool					RecurseValidateFileIndexes(CIndexTreeRecursor* pcCursor, bool bReadNodes);
	bool					ValidateKeys(bool bReadNodes);
	bool					RecurseValidateKeys(CIndexTreeRecursor* pcCursor, bool bReadNodes);

	CIndexTreeNodeFile*		StepNext(SIndexTreeFileIterator* psIterator);
	bool					StepNext(SIndexTreeFileUnsafeIterator* psIterator);

	bool					FlushDeleted(void);
	bool					FlushDirty(void);
	bool					RecurseFlushDirty(CIndexTreeRecursor* pcCursor);
	bool					WriteBackPathWriteThrough(CIndexTreeNodeFile* pcNode);
	bool					WriteBackPathCaching(CIndexTreeNodeFile* pcNode);
	bool					SetDirtyPath(CIndexTreeNodeFile* pcCurrent);
	bool					SetDeletedPath(CIndexTreeNodeFile* pcCurrent);
	bool					ClearDeletedPath(CIndexTreeNodeFile* pcNode);
	bool					ClearDirtyPath(CIndexTreeNodeFile* pcNode);
	bool					RemoveSetPaths(CIndexTreeNodeFile* pcCurrent);
	bool					DirtySetPaths(CIndexTreeNodeFile* pcCurrent);
	bool					RecurseIsFlushed(CIndexTreeRecursor* pcCursor);

	CFileDataIndex			ReadRootFileIndex(void);
	bool					WriteRootFileIndex(CFileDataIndex* pcRootIndex);

	CIndexTreeNodeFile*		DebugNode(CChars* pszDest, CIndexTreeNodeFile* pcParent, size uiIndexInParent, bool bShowFlags, bool bShowSize);
	void					DebugNode(CChars* pszDest, uint32 iFile, uint32 uiIndex, size uIndexFromParent, bool bShowFlags, bool bShowSize);
	void					DebugNodeChildren(CChars* pszDest, CIndexTreeNodeFile* pcCurrent, int uIndexFromParent, bool bShowFlags, bool bShowSize);
	void					ReadDebugNode(SIndexTreeDebugNode* psDebugNode, uint32 iFile, uint32 uiIndex);
	void					PrintChildFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz);
	void					PrintNodeFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz);

	void					RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, bool bShowFlags, bool bShowSize);

	void					DiagnosticFlushCallback(CIndexTreeNodeFile* pcNode);
	void					DiagnosticEvictCallback(CIndexTreeNodeFile* pcNode);
};


#endif // __INDEX_TREE_FILE_H__

