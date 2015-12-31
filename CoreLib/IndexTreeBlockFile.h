#ifndef __INDEX_TREE_BLOCK_FILE_H__
#define __INDEX_TREE_BLOCK_FILE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/IndexTreeBlock.h"
#include "DurableFileController.h"
#include "IndexTreeNodeFile.h"
#include "IndexedFiles.h"


class CIndexTreeBlockFile : public CIndexTreeBlock
{
protected:
	CIndexTreeNodeFile*			mpcRoot;
	CIndexedFiles				mcIndexFiles;
	CDurableFileController*		mpcDurableFileControl;

public:
	void	Init(CDurableFileController* pcDurableFileControl);
	void	Init(CDurableFileController* pcDurableFileControl, CMallocator* pcMalloc);
	void	Kill(void);

	void*	Get(char* pszKey);
	void*	Get(void* pvKey, int iKeySize);

	int		NumElements(void);

	BOOL	Put(char* pszKey, void* pvObject, unsigned char uiObjectSize);
	BOOL	Put(void* pvKey, int iKeySize, void* pvObject, unsigned char uiObjectSize);
	void*	Put(void* pvKey, int iKeySize, unsigned char uiObjectSize);

	BOOL	Remove(char* pszKey);
	BOOL	Remove(void* pvKey, int iKeySize);

	BOOL	HasKey(char* pszKey);
	BOOL	HasKey(void* pvKey, int iKeySize);

protected:
	CIndexTreeNodeFile*		AllocateRoot(void);
	CIndexTreeNodeFile*		AllocateNode(CIndexTreeNodeFile* pcParent);

	CIndexTreeNodeFile*		GetIndexNode(void* pvKey, int iKeySize);
};


#endif // __INDEX_TREE_BLOCK_FILE_H__

