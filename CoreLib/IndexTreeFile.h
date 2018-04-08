#ifndef __INDEX_TREE_FILE_H__
#define __INDEX_TREE_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/IndexTree.h"
#include "BaseLib/IndexTreeRecursor.h"
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
	CChars						mszRootFileName;

public:
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szRootFileName);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szRootFileName, CMallocator* pcMalloc);
	void					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvObject, unsigned short* puiDataSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, unsigned short uiDataSize);
	BOOL					Delete(void* pvKey, int iKeySize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);

	int						NumElements(void);
	void					Flush(void);

	BOOL					StartIteration(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize);
	BOOL					Iterate(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize);

	void					FindKey(CIndexTreeNodeFile* pcNode, CArrayChar* pacKey);
	void					FindKey(CIndexTreeNodeFile* pcNode, unsigned char* uiKey, int* piKeySize);

	BOOL					Put(char* pszKey, void* pvObject, unsigned short uiDataSize);
	BOOL					Put(void* pvKey, int iKeySize, unsigned short uiDataSize);

	BOOL					Remove(char* pszKey);

	BOOL					HasKey(char* pszKey);

	CIndexTreeNodeFile*		GetNode(void* pvKey, int iKeySize);
	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetNodeForData(void* pvData);
	int						CountAllocatedNodes(void);
	int						RecurseSize(void);
	size_t					ByteSize(void);
	void					FindWithFlags(CArrayVoidPtr* papNodes, unsigned char uiFollowFlags, unsigned char uiAddFlags);

	void					FakeInit(void);
	void					RecurseKill(CIndexTreeNodeFile* pcNode);
	BOOL					InitRoot(char* szRootFileName);
	CFileIndex				LoadRootFileIndex(char* szRootFileName);

	BOOL					ValidateIndexTree(void);
	CIndexedFiles*			GetIndexFiles(void);
	int						NumNodes(void);

protected:
	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c);
	CIndexTreeNodeFile*		ReadNode(CIndexTreeChildNode* pcChild);

	CIndexTreeNodeFile*		AllocateRoot(void);
	CIndexTreeNodeFile*		AllocateRoot(CFileIndex cFileIndex);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent, unsigned short uiDataSize, unsigned char uiIndexInParent);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned short uiDataSize, unsigned char uiIndexInParent);

	CIndexTreeNodeFile*		ReallocateNodeForIndex(CIndexTreeNodeFile* pcNode, unsigned char uiIndex);
	CIndexTreeNodeFile*		ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, unsigned short uiDataSize);
	CIndexTreeNodeFile*		ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, unsigned short uiOriginalSize);
	CIndexTreeNodeFile*		ReallocateNodeForData(CIndexTreeNodeFile* pcNode, size_t tNewNodeSize, size_t tOldNodeSize);
	void					RemapChildParents(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNode);

	CIndexTreeNodeFile*		AllocateNodeIfUnallocated(CIndexTreeNodeFile* pcParent, unsigned char c);
	BOOL					Remove(CIndexTreeNodeFile* pcCurrent);

	int						RecurseSize(CIndexTreeNodeFile* pcNode);
	int						RecurseCountAllocatedNodes(CIndexTreeNodeFile* pcNode);
	int						CountListSize(void);
	int						RecurseCountListSize(CIndexTreeNodeFile* pcNode);
	size_t					RecurseByteSize(CIndexTreeNodeFile* pcNode);
	BOOL					ValidateLimits(void);
	BOOL					RecurseValidateLimits(CIndexTreeRecursor* pcCursor);
	void					RecurseFindWithFlags(CIndexTreeRecursor* pcCursor, unsigned char uiFollowFlags, unsigned char uiAddFlags, CArrayVoidPtr* papNodes);
	BOOL					ValidateParentIndex(void);
	BOOL					RecurseValidateParentIndex(CIndexTreeRecursor* pcCursor);
	int						RecurseNumNodes(CIndexTreeRecursor* pcCursor);

	BOOL					StepNext(SIndexTreeFileIterator* psIterator);

	void					FindKeyReversed(CIndexTreeNodeFile* pcNode, unsigned char* uiKeyReversed, int* piKeySize);

	BOOL					Read(CIndexTreeChildNode* pcChildNode);

public:
	BOOL					Write(CIndexTreeNodeFile* pcNode);
	CIndexTreeNodeFile*		SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiDataSize);
	CIndexTreeNodeFile*		SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiDataSize);
};


#endif // __INDEX_TREE_BLOCK_H__

