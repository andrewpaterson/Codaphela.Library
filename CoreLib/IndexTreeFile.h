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
	int							miSize;  //Suspect
	CIndexTreeNodeFile*			mpcRoot;
	CIndexedFiles				mcIndexFiles;
	CDurableFileController*		mpcDurableFileControl;
	CChars						mszRootFileName;

public:
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szRootFileName);
	BOOL					Init(CDurableFileController* pcDurableFileControl, char* szRootFileName, CMallocator* pcMalloc);
	void					Kill(void);

	BOOL					Get(void* pvKey, int iKeySize, void* pvObject, unsigned char* puiDataSize);
	BOOL					Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiDataSize);
	BOOL					Remove(void* pvKey, int iKeySize);
	BOOL					HasKey(void* pvKey, int iKeySize);

	int						NumElements(void);

	BOOL					StartIteration(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize);
	BOOL					Iterate(SIndexTreeFileIterator* psIterator, void** pvData, int* piDataSize);

	BOOL					Put(char* pszKey, void* pvObject, unsigned char uiDataSize);
	BOOL					Put(void* pvKey, int iKeySize, unsigned char uiDataSize);

	BOOL					Remove(char* pszKey);

	BOOL					HasKey(char* pszKey);

	int						GetLargestKeySize(void);
	CIndexTreeNodeFile*		GetNode(void* pvKey, int iKeySize);
	CIndexTreeNodeFile*		GetRoot(void);
	CIndexTreeNodeFile*		GetNodeForData(void* pvData);
	int						CountAllocatedNodes(void);
	int						RecurseSize(void);
	size_t					ByteSize(void);

	void					FakeInit(void);
	void					RecurseKill(CIndexTreeNodeFile* pcNode);
	BOOL					InitRoot(char* szRootFileName);
	CFileIndex				LoadRootFileIndex(char* szRootFileName);

	BOOL					ValidateIndexTree(void);
	BOOL					ValidateSize(void);
	CIndexedFiles*			GetIndexFiles(void);

protected:
	CIndexTreeNodeFile*		ReadNode(CIndexTreeNodeFile* pcParent, unsigned char c);
	CIndexTreeNodeFile*		ReadNode(CIndexTreeChildNode* pcChild);

	CIndexTreeNodeFile*		AllocateRoot(void);
	CIndexTreeNodeFile*		AllocateRoot(CFileIndex cFileIndex);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent, unsigned char uiDataSize);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiDataSize);

	CIndexTreeNodeFile*		ReallocateNodeForIndex(CIndexTreeNodeFile* pcNode, unsigned char uiIndex);
	CIndexTreeNodeFile*		ReallocateNodeForLargerData(CIndexTreeNodeFile* pcNode, unsigned char uiDataSize);
	CIndexTreeNodeFile*		ReallocateNodeForSmallerData(CIndexTreeNodeFile* pcNode, unsigned char uiOriginalSize);
	CIndexTreeNodeFile*		ReallocateNodeForData(CIndexTreeNodeFile* pcNode, size_t tNewNodeSize, size_t tOldNodeSize);
	void					RemapChildParents(CIndexTreeNodeFile* pcOldNode, CIndexTreeNodeFile* pcNode);


	CIndexTreeNodeFile*		SetOldWithCurrent(CIndexTreeNodeFile* pcParent, unsigned char c);
	int						RecurseSize(CIndexTreeNodeFile* pcNode);
	int						RecurseCountAllocatedNodes(CIndexTreeNodeFile* pcNode);
	int						CountListSize(void);
	int						RecurseCountListSize(CIndexTreeNodeFile* pcNode);
	size_t					RecurseByteSize(CIndexTreeNodeFile* pcNode);
	BOOL					ValidateLimits(void);
	BOOL					RecurseValidateLimits(CIndexTreeRecursor* pcCursor);

	BOOL					StepNext(SIndexTreeFileIterator* psIterator);


	BOOL					Read(CIndexTreeChildNode* pcChildNode);

public:
	BOOL					Write(CIndexTreeNodeFile* pcNode);
	CIndexTreeNodeFile*		SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiFirstIndex, unsigned char uiLastIndex, unsigned char uiDataSize);
	CIndexTreeNodeFile*		SetParentWithExisting(CIndexTreeNodeFile* pcParent, unsigned char c, unsigned char uiDataSize);
};


#endif // __INDEX_TREE_BLOCK_H__

