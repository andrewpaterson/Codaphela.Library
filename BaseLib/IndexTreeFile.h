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
	int						iIndex;
};


struct SIndexTreeFileIterator
{
	char	pvKey[MAX_KEY_SIZE];
	int		iKeyLength;
	int		iIndex;
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

	bool					Get(void* pvKey, int iKeySize, void* pvObject, size_t* piDataSize, size_t uiMaxDataSize);
	bool					Put(void* pvKey, int iKeySize, void* pvObject, int iDataSize);
	bool					Remove(void* pvKey, int iKeySize);
	bool					HasKey(void* pvKey, int iKeySize);
	uint16					GetDataSize(void* pvKey, int iKeySize);

	bool					Flush(void);

	bool					StartIteration(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* piDataSize, size_t iMaxDataSize);
	bool					Iterate(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, int iMaxKeySize, void* pvData, size_t* piDataSize, size_t iMaxDataSize);
	bool					StartUnsafeIteration(SIndexTreeFileUnsafeIterator* psIterator, char* pvKey, int* piKeySize, int iMaxKeySize, void** ppvData, size_t* piDataSize);
	bool					UnsafeIterate(SIndexTreeFileUnsafeIterator* psIterator, char* pvKey, int* piKeySize, int iMaxKeySize, void** ppvData, size_t* piDataSize);
	bool					StartUnsafeIteration(SIndexTreeFileUnsafeIterator* psIterator, void** ppvData, size_t* piDataSize);
	bool					UnsafeIterate(SIndexTreeFileUnsafeIterator* psIterator, void** ppvData, size_t* piDataSize);

	bool					Evict(void* pvKey, int iKeySize);
	bool					Flush(void* pvKey, int iKeySize);

	void					SetDiagnosticCallback(CIndexTreeFileDiagnosticCallback* pcCallback);

	bool					IsFlushed(void);
	bool					ValidateIndexTree(void);
	bool					ValidateParentIndex(void);

	int						NumElements(void);
	bool					IsWriteThrough(void);
	bool					HasDiagnosticCallback(void);
	CDurableFileController* GetController(void);

	void					Print(CChars* pszDest, bool bShowFlags, bool bShowSize);
	void					Dump(void);

protected:
	bool					Evict(CIndexTreeNodeFile* pcNode);

	void					SetWriteThrough(EIndexWriteThrough eWriteThrough);

	CIndexTreeNodeFile*		ParentPut(CIndexTreeNodeFile* pcParent, uint8 uiIndexInParent, uint8 uiDataSize, uint8 uiFirstIndex, uint8 uiLastIndex);
	CIndexTreeNodeFile*		ParentPut(CIndexTreeNodeFile* pcParent, uint8 uiIndexInParent, uint8 uiDataSize);

	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetNode(void* pvKey, int iKeySize);
	CIndexTreeNodeFile*		GetMemoryNode(void* pvKey, int iKeySize);
	CIndexTreeNodeFile*		GetNodeForData(void* pvData);
	int						GetNodeKeySize(CIndexTreeNode* pcNode);
	int						GetNodeKey(CIndexTreeNode* pcNode, char* pvDestKey, int iDestKeySize);
	uint16					GetNodeDataSize(CIndexTreeNode* pcNode);
	uint16					GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, int iDestDataSize);

	CListTemplateMinimal<char>* GetNodesKeys(CArrayVoidPtr* apvNodes);
	CListCharsMinimal*		GetNodesStringKeys(CArrayVoidPtr* apvNodes);
	int						GetNodeKeysSize(CArrayVoidPtr* apvNodes);

	bool					ValidateIndexTree(bool bReadNodes);
	bool					ValidateKey(void* pvKey, int iKeySize);
	bool					ValidateKey(char* pszKey);

	size_t					ByteSize(void);
	size_t					GetUserMemorySize(void);
	size_t					GetSystemMemorySize(void);

	CIndexedFiles*			GetIndexFiles(void);
	void					GetFiles(CArrayIndexedFilePtr* pac);
	CIndexedFile*			GetFile(int iFile);
	int						NumNodes(void);
	int						NumMemoryNodes(void);
	int						NumMemoryElements(void);
	int						NumMemoryElements(size_t iSize);

	void					DebugKey(CChars* pszDest, void* pvKey, int iKeySize, bool bSkipRoot, bool bShowFlags, bool bShowSize, bool bKeyAlreadyReversed);
	void					PrintChildren(CChars* pszDest, bool bShowFlags, bool bShowSize);

	bool					InitRoot(char* szSubDirectory);
	void					RecurseKill(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, uint8 c);
	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, uint8 c, bool bReadNode);
	bool					Read(CIndexTreeChildNode* pcChildNode, CIndexTreeNodeFile* pcFileNodeParent, uint8 uiIndexInParent);
	CIndexTreeNodeFile*		ReadMemoryNode(CIndexTreeNodeFile* pcParent, uint8 c);
	bool					HasMemoryNodes(CIndexTreeNodeFile* pcNode);
	
	bool					Write(CIndexTreeNodeFile* pcNode);
	bool					Delete(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		AllocateRoot(void);
	CIndexTreeNodeFile*		AllocateRoot(CFileDataIndex cFileIndex);
	CIndexTreeNodeFile*		AllocateNodeSingle(CIndexTreeNodeFile* pcParent, uint8 uiIndexInParent, uint16 uiDataSize);
	CIndexTreeNodeFile*		AllocateNodeRange(CIndexTreeNodeFile* pcParent, uint8 uiIndexInParent, uint8 uiFirstIndex, uint8 uiLastIndex, uint16 uiDataSize);
	CIndexTreeNodeFile*		AllocateNodeFromBuffer(CIndexTreeNodeFile* pcParent, uint8 uiIndexInParent, void* pvBuffer, int iMaxBufferSize);

	CIndexTreeNodeFile*		ReallocateNodeForContainIndex(CIndexTreeNodeFile* pcNode, uint8 uiIndex);
	CIndexTreeNodeFile*		ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, void* pvData, uint16 uiDataSize);
	CIndexTreeNodeFile*		ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, void* pvData, uint16 uiOriginalSize);
	CIndexTreeNodeFile*		ReallocateNodeForUncontainIndex(CIndexTreeNodeFile* pcNode, uint8 c);

	CIndexTreeNodeFile*		AllocateChildNode(CIndexTreeNodeFile* pcParent, uint8 uiIndexInParent);
	CIndexTreeNodeFile*		SetNodeData(CIndexTreeNodeFile* pcCurrent, void* pvObject, uint16 uiDataSize);
	CIndexTreeNodeFile*		AllocateKey(void* pvKey, int iKeySize);
	bool					HasData(CIndexTreeNodeFile* pcNode);
	void					RemapNodePointers(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		RemoveWriteThrough(CIndexTreeNodeFile* pcCurrent);
	bool					RemoveWaitForFlush(CIndexTreeNodeFile* pcCurrent);
	bool					EvictNode(CIndexTreeNodeFile* pcCurrent);
	bool					Flush(CIndexTreeNodeFile** ppcCurrent);
	bool					CanEvict(CIndexTreeNodeFile* pcNode);

	void					ClearNodesFlags(CArrayVoidPtr* papNodes, uint8 uiFlags);
	void					FindWithFlags(CArrayVoidPtr* papNodes, uint8 uiFollowFlags, uint8 uiAddFlags);
	int						RecurseNumElements(CIndexTreeNodeFile* pcNode);
	int						RecurseNumMemoryElements(CIndexTreeNodeFile* pcNode);
	int						RecurseNumMemoryElements(CIndexTreeNodeFile* pcNode, size_t iSize);
	int						CountListSize(void);
	int						RecurseCountListSize(CIndexTreeNodeFile* pcNode);
	size_t					RecurseByteSize(CIndexTreeNodeFile* pcNode);
	void					RecurseFindWithFlags(CIndexTreeRecursor* pcCursor, uint8 uiFollowFlags, uint8 uiAddFlags, CArrayVoidPtr* papNodes);
	int						RecurseNumNodes(CIndexTreeRecursor* pcCursor);
	int						RecurseNumMemoryNodes(CIndexTreeRecursor* pcCursor);

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

	CIndexTreeNodeFile*		DebugNode(CChars* pszDest, CIndexTreeNodeFile* pcParent, int uiIndexInParent, bool bShowFlags, bool bShowSize);
	void					DebugNode(CChars* pszDest, int iFile, unsigned int uiIndex, int uIndexFromParent, bool bShowFlags, bool bShowSize);
	void					DebugNodeChildren(CChars* pszDest, CIndexTreeNodeFile* pcCurrent, int uIndexFromParent, bool bShowFlags, bool bShowSize);
	void					ReadDebugNode(SIndexTreeDebugNode* psDebugNode, int iFile, unsigned int uiIndex);
	void					PrintChildFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz);
	void					PrintNodeFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz);

	void					RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, bool bShowFlags, bool bShowSize);

	void					DiagnosticFlushCallback(CIndexTreeNodeFile* pcNode);
	void					DiagnosticEvictCallback(CIndexTreeNodeFile* pcNode);
};


#endif // !__INDEX_TREE_FILE_H__

