#ifndef __INDEX_TREE_FILE_H__
#define __INDEX_TREE_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/IndexTree.h"
#include "BaseLib/IndexTreeRecursor.h"
#include "BaseLib/ListCharsMinimal.h"
#include "DurableFileController.h"
#include "IndexTreeNodeFile.h"
#include "IndexedFiles.h"


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
	BOOL						mbWriteThrough;
	CDurableFile				mcRootIndex;

public:
	BOOL					Init(CDurableFileController* pcDurableFileControl);
	BOOL					Init(CDurableFileController* pcDurableFileControl, BOOL bWriteThrough);
	BOOL					Init(CDurableFileController* pcDurableFileControl, CMallocator* pcMalloc, BOOL bWriteThrough);
	void					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvObject, unsigned short* puiDataSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);

	int						NumElements(void);
	BOOL					Flush(void);
	void					SetWriteThrough(BOOL bWriteThrough);

	BOOL					StartIteration(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize);
	BOOL					Iterate(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize);

	BOOL					Put(char* pszKey, void* pvObject, unsigned short uiDataSize);
	BOOL					Put(void* pvKey, int iKeySize, unsigned short uiDataSize);

	BOOL					Remove(char* pszKey);

	BOOL					HasKey(char* pszKey);
	int						FindKeySize(CIndexTreeNodeFile* pcNode);
	void					FindKey(CIndexTreeNodeFile* pcNode, CArrayChar* pacKey);
	void					FindKey(CIndexTreeNodeFile* pcNode, unsigned char* uiKey, int* piKeySize);

	CIndexTreeNodeFile*		GetNode(void* pvKey, int iKeySize);
	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetNodeForData(void* pvData);
	CIndexedFile*			GetFile(int iFile);
	int						CountAllocatedNodes(void);
	int						RecurseSize(void);
	size_t					ByteSize(void);
	void					FindWithFlags(CArrayVoidPtr* papNodes, unsigned char uiFollowFlags, unsigned char uiAddFlags);
	void					ClearNodesFlags(CArrayVoidPtr* papNodes, unsigned char uiFlags);

	CListTemplateMinimal<char>*	FindKeys(CArrayVoidPtr* apvNodes);
	CListCharsMinimal*		FindStringKeys(CArrayVoidPtr* apvNodes);

	void					FakeInit(void);
	void					RecurseKill(CIndexTreeNodeFile* pcNode);

	BOOL					ValidateIndexTree(void);
	CIndexedFiles*			GetIndexFiles(void);
	int						NumNodes(void);
	void					Debug(void* pvKey, int iKeySize);

protected:
	BOOL					InitRoot(void);
	void					InitRootIndexFile(void);

	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c);
	BOOL					Read(CIndexTreeChildNode* pcChildNode, CIndexTreeNodeFile* pcFileNodeParent, unsigned char uiIndexInParent);

	CIndexTreeNodeFile*		AllocateRoot(void);
	CIndexTreeNodeFile*		AllocateRoot(CFileDataIndex cFileIndex);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent, unsigned short uiDataSize, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent, void* pvBuffer, int iBufferSize);

	CIndexTreeNodeFile*		ReallocateNodeForIndex(CIndexTreeNodeFile* pcNode, unsigned char uiIndex);
	CIndexTreeNodeFile*		ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, unsigned short uiDataSize);
	CIndexTreeNodeFile*		ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, unsigned short uiOriginalSize);
	CIndexTreeNodeFile*		ReallocateNodeForData(CIndexTreeNodeFile* pcNode, size_t tNewNodeSize, size_t tOldNodeSize);
	void					RemapChildParents(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		GetChildNodeOrAllocate(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent);
	BOOL					RemoveWriteThrough(CIndexTreeNodeFile* pcCurrent);
	BOOL					RemoveWaitForFlush(CIndexTreeNodeFile* pcCurrent);

	int						RecurseSize(CIndexTreeNodeFile* pcNode);
	int						RecurseCountAllocatedNodes(CIndexTreeNodeFile* pcNode);
	int						CountListSize(void);
	int						RecurseCountListSize(CIndexTreeNodeFile* pcNode);
	size_t					RecurseByteSize(CIndexTreeNodeFile* pcNode);
	void					RecurseFindWithFlags(CIndexTreeRecursor* pcCursor, unsigned char uiFollowFlags, unsigned char uiAddFlags, CArrayVoidPtr* papNodes);
	int						RecurseNumNodes(CIndexTreeRecursor* pcCursor);

	BOOL					ValidateLimits(void);
	BOOL					RecurseValidateLimits(CIndexTreeRecursor* pcCursor);
	BOOL					ValidateParentIndex(void);
	BOOL					RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor);
	BOOL					ValidateNoFlushFlags(void);
	BOOL					RecurseValidateNoFlushFlags(CIndexTreeRecursor* pcCursor);
	BOOL					ValidateMagic(void);
	BOOL					RecurseValidateMagic(CIndexTreeRecursor* pcCursor);
	BOOL					ValidateFileIndexes(void);
	BOOL					RecurseValidateFileIndexes(CIndexTreeRecursor* pcCursor);

	BOOL					StepNext(SIndexTreeFileIterator* psIterator);

	void					FindKeyReversed(CIndexTreeNodeFile* pcNode, unsigned char* uiKeyReversed, int* piKeySize);
	int						FindKeysSize(CArrayVoidPtr* apvNodes);

	BOOL					FlushRemoved(void);
	BOOL					FlushDirty(void);
	BOOL					RecurseFlushDirty(CIndexTreeRecursor* pcCursor);

	CFileDataIndex			ReadRootFileIndex(void);
	BOOL					WriteRootFileIndex(CFileDataIndex* pcRootIndex);
	BOOL					WriteRootFileIndex(BOOL bRootHasIndex, CFileDataIndex* pcRootIndex);

	CIndexTreeNodeFile*		DebugNode(CIndexTreeNodeFile* pcParent, unsigned char uiIndexInParent);
	void					DebugNode(CIndexTreeNodeFile* pcCurrent);
	void					DebugNode(int iFile, unsigned int uiIndex, unsigned int uIndexFromParent);

public:
	BOOL					Write(CIndexTreeNodeFile* pcNode);
	BOOL					Delete(CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiDataSize);
	CIndexTreeNodeFile*		SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiDataSize);
};


#endif // __INDEX_TREE_BLOCK_H__

