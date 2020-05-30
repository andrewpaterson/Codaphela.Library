#ifndef __INDEX_TREE_FILE_H__
#define __INDEX_TREE_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/IndexTree.h"
#include "BaseLib/IndexTreeRecursor.h"
#include "BaseLib/ListCharsMinimal.h"
#include "BaseLib/CountingAllocator.h"
#include "BaseLib/IndexKeyReverse.h"
#include "DurableFileController.h"
#include "IndexTreeNodeFile.h"
#include "IndexTreeFileDebug.h"
#include "IndexedFiles.h"
#include "IndexWriteThrough.h"


struct SIndexTreeFileIterator
{
	CIndexTreeNodeFile*		pcNode;
	int						iIndex;
};


class CIndexTreeFile : public CIndexTree
{
protected:
	CIndexTreeNodeFile*			mpcRoot;
	CIndexedFiles				mcIndexFiles;
	CDurableFileController*		mpcDurableFileControl;
	EIndexWriteThrough			meWriteThrough;
	CDurableFile				mcRootIndex;
	CCountingAllocator			mcMalloc;
	CIndexTreeFileCallback*		mpcDataCallback;

public:
	BOOL					Init(void);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, int iMaxDataSize, int iMaxKeySize);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szSubDirectory, CIndexTreeFileCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, int iMaxDataSize, int iMaxKeySize);
	BOOL					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvObject, int* piDataSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, int iDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);
	unsigned short			GetDataSize(void* pvKey, int iKeySize);

	CIndexTreeNodeFile*		ParentPut(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned char uiDataSize, unsigned char uiFirstIndex, unsigned char uiLastIndex);
	CIndexTreeNodeFile*		ParentPut(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned char uiDataSize);

	BOOL					Flush(void);
	void					SetWriteThrough(EIndexWriteThrough eWriteThrough);

	BOOL					StartIteration(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize);
	BOOL					Iterate(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize);

	BOOL					Evict(void* pvKey, int iKeySize);
	BOOL					Evict(CIndexTreeNodeFile* pcNode);

	BOOL					Flush(void* pvKey, int iKeySize);

	BOOL					ValidateKey(char* pszKey);
	BOOL					IsFlushed(void);

	int						NumElements(void);
	BOOL					HasKey(char* pszKey);
	int						FindKeySize(CIndexTreeNodeFile* pcNode);
	void					FindKey(CIndexTreeNodeFile* pcNode, CArrayChar* pacKey);
	void					FindKey(CIndexTreeNodeFile* pcNode, unsigned char* uiKey, int* piKeySize);

	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetNode(void* pvKey, int iKeySize);
	CIndexTreeNodeFile*		GetMemoryNode(void* pvKey, int iKeySize);
	CIndexTreeNodeFile*		GetNodeForData(void* pvData);
	int						GetNodeKeySize(CIndexTreeNodeFile* pcNode);
	BOOL					GetNodeKey(CIndexTreeNodeFile* pcNode, unsigned char* pvDestKey, int iDestSize);

	BOOL					IsWriteThrough(void);

	CListTemplateMinimal<char>*	FindKeys(CArrayVoidPtr* apvNodes);
	CListCharsMinimal*		FindStringKeys(CArrayVoidPtr* apvNodes);

	BOOL					ValidateIndexTree(void); 
	BOOL					ValidateIndexTree(BOOL bReadNodes);
	BOOL					ValidateKey(void* pvKey, int iKeySize);

	BOOL					Write(CIndexTreeNodeFile* pcNode);
	BOOL					Delete(CIndexTreeNodeFile* pcNode);

//Methods for testing only.
	size_t					ByteSize(void);
	size_t					GetUserMemorySize(void);
	size_t					GetSystemMemorySize(void);

	CIndexedFiles*			GetIndexFiles(void);
	void					GetFiles(CArrayIndexedFilePtr* pac);
	CIndexedFile*			GetFile(int iFile);
	int						NumNodes(void);
	int						NumMemoryNodes(void);
	int						NumMemoryElements(void);
	void					DebugKey(CChars* pszDest, void* pvKey, int iKeySize, BOOL bSkipRoot, BOOL bShowFlags, BOOL bShowSize, BOOL bKeyAlreadyReversed);
	void					Dump(void);
	void					Print(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize);
	void					PrintChildren(CChars* pszDest, BOOL bShowFlags, BOOL bShowSize);

	void					FakeInit(EIndexKeyReverse eKeyReverse = IKR_No);

protected:
	BOOL					InitRoot(char* szSubDirectory);
	void					RecurseKill(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c);
	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c, BOOL bReadNode);
	BOOL					Read(CIndexTreeChildNode* pcChildNode, CIndexTreeNodeFile* pcFileNodeParent, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		ReadMemoryNode(CIndexTreeNodeFile* pcParent, unsigned char c);
	
	CIndexTreeNodeFile*		AllocateRoot(void);
	CIndexTreeNodeFile*		AllocateRoot(CFileDataIndex cFileIndex);
	CIndexTreeNodeFile*		AllocateNodeSingle(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned short uiDataSize);
	CIndexTreeNodeFile*		AllocateNodeRange(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize);
	CIndexTreeNodeFile*		AllocateNodeFromBuffer(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, void* pvBuffer, int iMaxBufferSize);

	CIndexTreeNodeFile*		ReallocateNodeForContainIndex(CIndexTreeNodeFile* pcNode, unsigned char uiIndex);
	CIndexTreeNodeFile*		ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, void* pvData, unsigned short uiDataSize);
	CIndexTreeNodeFile*		ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, void* pvData, unsigned short uiOriginalSize);
	CIndexTreeNodeFile*		ReallocateNodeForUncontainIndex(CIndexTreeNodeFile* pcNode, unsigned char c);

	CIndexTreeNodeFile*		GetOrAllocateChildNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		SetNodeData(CIndexTreeNodeFile* pcCurrent, void* pvObject, unsigned short uiDataSize);
	CIndexTreeNodeFile*		GetOrAllocateKey(void* pvKey, int iKeySize);

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

	void					FindKeyReversed(CIndexTreeNodeFile* pcNode, unsigned char* uiKeyReversed, int* piKeySize);
	int						FindKeysSize(CArrayVoidPtr* apvNodes);

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
	BOOL					WriteRootFileIndex(BOOL bRootHasIndex, CFileDataIndex* pcRootIndex);

	CIndexTreeNodeFile*		DebugNode(CChars* pszDest, CIndexTreeNodeFile* pcParent, int uiIndexInParent, BOOL bShowFlags, BOOL bShowSize);
	void					DebugNode(CChars* pszDest, int iFile, unsigned int uiIndex, int uIndexFromParent, BOOL bShowFlags, BOOL bShowSize);
	void					DebugNodeChildren(CChars* pszDest, CIndexTreeNodeFile* pcCurrent, int uIndexFromParent, BOOL bShowFlags, BOOL bShowSize);
	void					ReadDebugNode(SIndexTreeDebugNode* psDebugNode, int iFile, unsigned int uiIndex);
	void					PrintChildFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz);
	void					PrintNodeFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz);

	void					RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor, BOOL bShowFlags, BOOL bShowSize);
};


#endif // __INDEX_TREE_FILE_H__

