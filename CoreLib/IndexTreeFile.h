#ifndef __INDEX_TREE_FILE_H__
#define __INDEX_TREE_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/IndexTree.h"
#include "BaseLib/IndexTreeRecursor.h"
#include "BaseLib/ListCharsMinimal.h"
#include "BaseLib/CountingAllocator.h"
#include "BaseLib/IndexKeyReverse.h"
#include "BaseLib/IndexTreeIterator.h"
#include "DurableFileController.h"
#include "IndexTreeNodeFile.h"
#include "IndexTreeFileDebug.h"
#include "IndexedFiles.h"
#include "IndexTreeFileDiagnosticCallback.h"
#include "IndexWriteThrough.h"


struct SIndexTreeFileIterator
{
	CIndexTreeNodeFile*		pcNode;
	int						iIndex;
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
	CIndexTreeFileDataCallback*				mpcDataCallback;

	CIndexTreeFileDiagnosticCallback*		mpcDiagnosticCallback;

public:
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, int iMaxDataSize, int iMaxKeySize);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CIndexTreeDataOrderer* pcDataOrderer);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CIndexTreeDataOrderer* pcDataOrderer);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileDataCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize, CIndexTreeDataOrderer* pcDataOrderer);
	BOOL					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvObject, int* piDataSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, int iDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);
	unsigned short			GetDataSize(void* pvKey, int iKeySize);

	BOOL					Flush(void);

	BOOL					StartUnsafeIteration(SIndexTreeFileIterator* psIterator, char* pvKey, int* piKeySize, void** ppvData, int* piDataSize);
	BOOL					UnsafeIterate(SIndexTreeFileIterator* psIterator, char* pvKey, int* piKeySize, void** ppvData, int* piDataSize);
	BOOL					StartUnsafeIteration(SIndexTreeFileIterator* psIterator, void** ppvData, int* piDataSize);
	BOOL					UnsafeIterate(SIndexTreeFileIterator* psIterator, void** ppvData, int* piDataSize);

	BOOL					Evict(void* pvKey, int iKeySize);
	BOOL					Flush(void* pvKey, int iKeySize);

	void					SetDiagnosticCallback(CIndexTreeFileDiagnosticCallback* pcCallback);

	BOOL					IsFlushed(void);
	BOOL					ValidateIndexTree(void);
	BOOL					ValidateParentIndex(void);

	int						NumElements(void);
	BOOL					IsWriteThrough(void);
	BOOL					HasDiagnosticCallback(void);
	CDurableFileController* GetController(void);

	void					Print(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize);
	void					Dump(void);

protected:
	BOOL					Evict(CIndexTreeNodeFile* pcNode);

	void					SetWriteThrough(EIndexWriteThrough eWriteThrough);

	CIndexTreeNodeFile*		ParentPut(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned char uiDataSize, unsigned char uiFirstIndex, unsigned char uiLastIndex);
	CIndexTreeNodeFile*		ParentPut(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned char uiDataSize);

	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetNode(void* pvKey, int iKeySize);
	CIndexTreeNodeFile*		GetMemoryNode(void* pvKey, int iKeySize);
	CIndexTreeNodeFile*		GetNodeForData(void* pvData);
	int						GetNodeKeySize(CIndexTreeNode* pcNode);
	int						GetNodeKey(CIndexTreeNode* pcNode, char* pvDestKey, int iDestKeySize);
	int						GetNodeDataSize(CIndexTreeNode* pcNode);
	int						GetNodeData(CIndexTreeNode* pcNode, void* pvDestData, int iDestDataSize);

	CListTemplateMinimal<char>* GetNodesKeys(CArrayVoidPtr* apvNodes);
	CListCharsMinimal*		GetNodesStringKeys(CArrayVoidPtr* apvNodes);
	int						GetNodeKeysSize(CArrayVoidPtr* apvNodes);

	BOOL					ValidateIndexTree(BOOL bReadNodes);
	BOOL					ValidateKey(void* pvKey, int iKeySize);
	BOOL					ValidateKey(char* pszKey);

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

	void					DebugKey(CChars* pszDest, void* pvKey, int iKeySize, BOOL bSkipRoot, BOOL bShowFlags, BOOL bShowSize, BOOL bKeyAlreadyReversed);
	void					PrintChildren(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize);

	BOOL					InitRoot(char* szSubDirectory);
	void					RecurseKill(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c);
	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c, BOOL bReadNode);
	BOOL					Read(CIndexTreeChildNode* pcChildNode, CIndexTreeNodeFile* pcFileNodeParent, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		ReadMemoryNode(CIndexTreeNodeFile* pcParent, unsigned char c);
	BOOL					HasMemoryNodes(CIndexTreeNodeFile* pcNode);
	
	BOOL					Write(CIndexTreeNodeFile* pcNode);
	BOOL					Delete(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		AllocateRoot(void);
	CIndexTreeNodeFile*		AllocateRoot(CFileDataIndex cFileIndex);
	CIndexTreeNodeFile*		AllocateNodeSingle(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned short uiDataSize);
	CIndexTreeNodeFile*		AllocateNodeRange(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize);
	CIndexTreeNodeFile*		AllocateNodeFromBuffer(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, void* pvBuffer, int iMaxBufferSize);

	CIndexTreeNodeFile*		ReallocateNodeForContainIndex(CIndexTreeNodeFile* pcNode, unsigned char uiIndex);
	CIndexTreeNodeFile*		ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, void* pvData, unsigned short uiDataSize);
	CIndexTreeNodeFile*		ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, void* pvData, unsigned short uiOriginalSize);
	CIndexTreeNodeFile*		ReallocateNodeForUncontainIndex(CIndexTreeNodeFile* pcNode, unsigned char c);

	CIndexTreeNodeFile*		AllocateChildNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		SetNodeData(CIndexTreeNodeFile* pcCurrent, void* pvObject, unsigned short uiDataSize);
	CIndexTreeNodeFile*		AllocateKey(void* pvKey, int iKeySize);
	BOOL					HasData(CIndexTreeNodeFile* pcNode);
	void					RemapNodePointers(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		RemoveWriteThrough(CIndexTreeNodeFile* pcCurrent);
	BOOL					RemoveWaitForFlush(CIndexTreeNodeFile* pcCurrent);
	BOOL					EvictNode(CIndexTreeNodeFile* pcCurrent);
	BOOL					Flush(CIndexTreeNodeFile** ppcCurrent);
	BOOL					CanEvict(CIndexTreeNodeFile* pcNode);

	void					ClearNodesFlags(CArrayVoidPtr* papNodes, unsigned char uiFlags);
	void					FindWithFlags(CArrayVoidPtr* papNodes, unsigned char uiFollowFlags, unsigned char uiAddFlags);
	int						RecurseNumElements(CIndexTreeNodeFile* pcNode);
	int						RecurseNumMemoryElements(CIndexTreeNodeFile* pcNode);
	int						RecurseNumMemoryElements(CIndexTreeNodeFile* pcNode, size_t iSize);
	int						CountListSize(void);
	int						RecurseCountListSize(CIndexTreeNodeFile* pcNode);
	size_t					RecurseByteSize(CIndexTreeNodeFile* pcNode);
	void					RecurseFindWithFlags(CIndexTreeRecursor* pcCursor, unsigned char uiFollowFlags, unsigned char uiAddFlags, CArrayVoidPtr* papNodes);
	int						RecurseNumNodes(CIndexTreeRecursor* pcCursor);
	int						RecurseNumMemoryNodes(CIndexTreeRecursor* pcCursor);

	BOOL					ValidateLimits(BOOL bReadNodes);
	BOOL					RecurseValidateLimits(CIndexTreeRecursor* pcCursor, BOOL bReadNodes);
	BOOL					ValidateParentIndex(BOOL bReadNodes);
	BOOL					RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor, BOOL bReadNodes);
	BOOL					ValidateTransientFlags(BOOL bReadNodes);
	BOOL					RecurseValidateTransientFlags(CIndexTreeRecursor* pcCursor, BOOL bReadNodes);
	BOOL					ValidateMagic(BOOL bReadNodes);
	BOOL					RecurseValidateMagic(CIndexTreeRecursor* pcCursor, BOOL bReadNodes);
	BOOL					ValidateFileIndexes(BOOL bReadNodes);
	BOOL					RecurseValidateFileIndexes(CIndexTreeRecursor* pcCursor, BOOL bReadNodes);
	BOOL					ValidateKeys(BOOL bReadNodes);
	BOOL					RecurseValidateKeys(CIndexTreeRecursor* pcCursor, BOOL bReadNodes);

	BOOL					StepNext(SIndexTreeFileIterator* psIterator);
	BOOL					StartIteration(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, void* pvData, int* piDataSize);
	BOOL					Iterate(SIndexTreeFileIterator* psIterator, void* pvKey, int* piKeySize, void* pvData, int* piDataSize);

	BOOL					FlushDeleted(void);
	BOOL					FlushDirty(void);
	BOOL					RecurseFlushDirty(CIndexTreeRecursor* pcCursor);
	BOOL					WriteBackPathWriteThrough(CIndexTreeNodeFile* pcNode);
	BOOL					WriteBackPathCaching(CIndexTreeNodeFile* pcNode);
	BOOL					SetDirtyPath(CIndexTreeNodeFile* pcCurrent);
	BOOL					SetDeletedPath(CIndexTreeNodeFile* pcCurrent);
	BOOL					ClearDeletedPath(CIndexTreeNodeFile* pcNode);
	BOOL					ClearDirtyPath(CIndexTreeNodeFile* pcNode);
	BOOL					RemoveSetPaths(CIndexTreeNodeFile* pcCurrent);
	BOOL					DirtySetPaths(CIndexTreeNodeFile* pcCurrent);
	BOOL					RecurseIsFlushed(CIndexTreeRecursor* pcCursor);

	CFileDataIndex			ReadRootFileIndex(void);
	BOOL					WriteRootFileIndex(CFileDataIndex* pcRootIndex);

	CIndexTreeNodeFile*		DebugNode(CChars* pszDest, CIndexTreeNodeFile* pcParent, int uiIndexInParent, BOOL bShowFlags, BOOL bShowSize);
	void					DebugNode(CChars* pszDest, int iFile, unsigned int uiIndex, int uIndexFromParent, BOOL bShowFlags, BOOL bShowSize);
	void					DebugNodeChildren(CChars* pszDest, CIndexTreeNodeFile* pcCurrent, int uIndexFromParent, BOOL bShowFlags, BOOL bShowSize);
	void					ReadDebugNode(SIndexTreeDebugNode* psDebugNode, int iFile, unsigned int uiIndex);
	void					PrintChildFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz);
	void					PrintNodeFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz);

	void					RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, BOOL bShowFlags, BOOL bShowSize);

	void					DiagnosticFlushCallback(CIndexTreeNodeFile* pcNode);
	void					DiagnosticEvictCallback(CIndexTreeNodeFile* pcNode);
};


#endif // __INDEX_TREE_FILE_H__

