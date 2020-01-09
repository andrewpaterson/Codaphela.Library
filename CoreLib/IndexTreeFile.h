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
	BOOL					Init(CDurableFileController* pcDurableFileControl, EIndexKeyReverse eKeyReverse = IKR_No);
	BOOL					Init(CDurableFileController* pcDurableFileControl, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, CIndexTreeFileCallback* pcWriterCallback, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Init(CDurableFileController* pcDurableFileControl, CIndexTreeFileCallback* pcWriterCallback, CMallocator* pcMalloc, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	BOOL					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvObject, unsigned short* puiDataSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);
	unsigned short			ObjectSize(void* pvKey, int iKeySize);

	int						NumElements(void);
	BOOL					Flush(void);
	void					SetWriteThrough(EIndexWriteThrough eWriteThrough);

	BOOL					StartIteration(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize);
	BOOL					Iterate(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize);

	BOOL					Evict(void* pvKey, int iKeySize);
	BOOL					Evict(char* pszKey);
	BOOL					Evict(CIndexTreeNodeFile* pcNode);

	BOOL					Flush(void* pvKey, int iKeySize);

	BOOL					ValidateKey(char* pszKey);
	BOOL					IsFlushed(void);

	BOOL					HasKey(char* pszKey);
	int						FindKeySize(CIndexTreeNodeFile* pcNode);
	void					FindKey(CIndexTreeNodeFile* pcNode, CArrayChar* pacKey);
	void					FindKey(CIndexTreeNodeFile* pcNode, unsigned char* uiKey, int* piKeySize);

	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetNode(void* pvKey, int iKeySize);
	CIndexTreeNodeFile*		GetMemoryNode(void* pvKey, int iKeySize); 
	CIndexTreeNodeFile*		GetNodeForData(void* pvData);
	CIndexedFile*			GetFile(int iFile);
	int						CountAllocatedNodes(void);
	int						RecurseSize(void);
	size_t					ByteSize(void);
	void					FindWithFlags(CArrayVoidPtr* papNodes, unsigned char uiFollowFlags, unsigned char uiAddFlags);
	void					ClearNodesFlags(CArrayVoidPtr* papNodes, unsigned char uiFlags);
	size_t					GetUserMemorySize(void);
	size_t					GetSystemMemorySize(void);
	BOOL					IsWriteThrough(void);
	int						GetNodeKeySize(CIndexTreeNodeFile* pcNode);
	BOOL					GetNodeKey(CIndexTreeNodeFile* pcNode, unsigned char* pvDestKey, int iDestSize);

	CListTemplateMinimal<char>*	FindKeys(CArrayVoidPtr* apvNodes);
	CListCharsMinimal*		FindStringKeys(CArrayVoidPtr* apvNodes);

	void					FakeInit(EIndexKeyReverse eKeyReverse = IKR_No);
	void					RecurseKill(CIndexTreeNodeFile* pcNode);

	BOOL					ValidateIndexTree(void); 
	BOOL					ValidateIndexTree(BOOL bReadNodes);
	BOOL					ValidateKey(void* pvKey, int iKeySize);

	CIndexedFiles*			GetIndexFiles(void);
	int						NumNodes(void);
	int						NumMemoryNodes(void);
	int						NumMemoryElements(void);
	void					DebugKey(CChars* pszDest, void* pvKey, int iKeySize, BOOL bSkipRoot);
	void					Dump(void);
	void					Print(CChars* pszDest);

protected:
	BOOL					InitRoot(void);
	void					InitRootIndexFile(void);

	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c);
	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c, BOOL bReadNode);
	BOOL					Read(CIndexTreeChildNode* pcChildNode, CIndexTreeNodeFile* pcFileNodeParent, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		ReadMemoryNode(CIndexTreeNodeFile* pcParent, unsigned char c);
	
	CIndexTreeNodeFile*		AllocateRoot(void);
	CIndexTreeNodeFile*		AllocateRoot(CFileDataIndex cFileIndex);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent, unsigned short uiDataSize, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, void* pvBuffer, int iMaxBufferSize);

	CIndexTreeNodeFile*		ReallocateNodeForContainIndex(CIndexTreeNodeFile* pcNode, unsigned char uiIndex);
	CIndexTreeNodeFile*		ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, unsigned short uiDataSize);
	CIndexTreeNodeFile*		ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, unsigned short uiOriginalSize);
	CIndexTreeNodeFile*		ReallocateNodeForData(CIndexTreeNodeFile* pcNode, size_t tNewNodeSize, size_t tOldNodeSize);
	CIndexTreeNodeFile*		ReallocateNodeForUncontainIndex(CIndexTreeNodeFile* pcNode, unsigned char c, size_t tOldNodeSize);
	void					RemapChildParents(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		GetChildNodeOrAllocate(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		SetNodeObject(CIndexTreeNodeFile* pcCurrent, void* pvObject, unsigned short uiDataSize);

	CIndexTreeNodeFile*		RemoveWriteThrough(CIndexTreeNodeFile* pcCurrent);
	BOOL					RemoveWaitForFlush(CIndexTreeNodeFile* pcCurrent);
	BOOL					EvictNode(CIndexTreeNodeFile* pcCurrent);
	BOOL					Flush(CIndexTreeNodeFile** ppcCurrent);
	BOOL					CanEvict(CIndexTreeNodeFile* pcNode);

	int						RecurseSize(CIndexTreeNodeFile* pcNode);
	int						RecurseMemorySize(CIndexTreeNodeFile* pcNode);
	int						RecurseCountAllocatedNodes(CIndexTreeNodeFile* pcNode);
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

	BOOL					StepNext(SIndexTreeFileIterator* psIterator);

	void					FindKeyReversed(CIndexTreeNodeFile* pcNode, unsigned char* uiKeyReversed, int* piKeySize);
	int						FindKeysSize(CArrayVoidPtr* apvNodes);

	BOOL					FlushDeleted(void);
	BOOL					FlushDirty(void);
	BOOL					RecurseFlushDirty(CIndexTreeRecursor* pcCursor);
	BOOL					WriteBackPathWriteThrough(CIndexTreeNodeFile* pcNode);
	BOOL					WriteBackPathCaching(CIndexTreeNodeFile* pcNode);
	BOOL					SetDirtyPath(CIndexTreeNodeFile* pcCurrent);
	BOOL					RecurseIsFlushed(CIndexTreeRecursor* pcCursor);
	BOOL					ClearDeletedPath(CIndexTreeNodeFile* pcNode);
	BOOL					ClearDirtyPath(CIndexTreeNodeFile* pcNode);

	CFileDataIndex			ReadRootFileIndex(void);
	BOOL					WriteRootFileIndex(CFileDataIndex* pcRootIndex);
	BOOL					WriteRootFileIndex(BOOL bRootHasIndex, CFileDataIndex* pcRootIndex);

	CIndexTreeNodeFile*		DebugNode(CChars* pszDest, CIndexTreeNodeFile* pcParent, int uiIndexInParent);
	void					DebugNode(CChars* pszDest, int iFile, unsigned int uiIndex, int uIndexFromParent);
	void					DebugNodeChildren(CChars* pszDest, CIndexTreeNodeFile* pcCurrent, int uIndexFromParent);
	void					ReadDebugNode(SIndexTreeDebugNode* psDebugNode, int iFile, unsigned int uiIndex);
	void					PrintChildFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz);
	void					PrintNodeFileIndexes(CIndexTreeNodeFile* pcCurrent, CChars* psz);

	void					RecurseDump(CChars* pszDest, CIndexTreeRecursor* pcCursor);

public:
	BOOL					Write(CIndexTreeNodeFile* pcNode);
	BOOL					Delete(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiDataSize);
	CIndexTreeNodeFile*		SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiDataSize);

	void					GetFiles(CArrayIndexedFilePtr* pac);
};


#endif // __INDEX_TREE_BLOCK_H__

