#ifndef __INDEX_TREE_BLOCK_FILE_H__
#define __INDEX_TREE_BLOCK_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/IndexTree.h"
#include "DurableFileController.h"
#include "IndexTreeNodeFile.h"
#include "IndexedFiles.h"


class CIndexTreeFile : public CIndexTree
{
protected:
	CIndexTreeNodeFile*			mpcRoot;
	CIndexedFiles				mcIndexFiles;
	CDurableFileController*		mpcDurableFileControl;
	CChars						mszRootFileName;

public:
	BOOL	Init(CDurableFileController* pcDurableFileControl, char* szRootFileName);
	BOOL	Init(CDurableFileController* pcDurableFileControl, char* szRootFileName, CMallocator* pcMalloc);
	void	FakeInit(void);
	void	Kill(void);

	BOOL	InitRoot(char* szRootFileName);

	void*	Get(char* pszKey);
	void*	Get(void* pvKey, int iKeySize);

	int		NumElements(void);

	BOOL	Put(char* pszKey, void* pvObject, unsigned char uiObjectSize);
	BOOL	Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiObjectSize);
	BOOL	Put(void* pvKey, int iKeySize, unsigned char uiObjectSize);

	BOOL	Remove(char* pszKey);
	BOOL	Remove(void* pvKey, int iKeySize);

	BOOL	HasKey(char* pszKey);
	BOOL	HasKey(void* pvKey, int iKeySize);

	BOOL	PutPtr(void* pvKey, int iKeySize, void* pvPointer);
	BOOL	PutPtr(char* pszKey, void* pvPointer);
	BOOL	PutLong(char* pszKey, int64 lliIndex);
	BOOL	PutLong(void* pvKey, int iKeySize, int64 lliIndex);

	CIndexTreeNodeFile*		GetIndexNode(void* pvKey, int iKeySize);
	void					FindAll(CArrayVoidPtr* papvElements);

protected:
	CIndexTreeNodeFile*		AllocateRoot(void);
	CIndexTreeNodeFile*		AllocateRoot(CFileIndex cFileIndex);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent);
	size_t					CalculateRootNodeSize(void);
	CFileIndex				LoadRootFileIndex(char* szRootFileName);
};


#endif // __INDEX_TREE_BLOCK_FILE_H__
