#ifndef __INDEX_TREE_FILE_ACCESS_H__
#define __INDEX_TREE_FILE_ACCESS_H__
#include "IndexAccess.h"
#include "IndexTreeFile.h"


class CIndexTreeFileAccess : public CIndexAccess
{
private:
	CIndexTreeFile*		mpcTree;

public:
	bool				Init(CIndexTreeFile* pcTree);
	bool				Kill(void);
	bool				Flush(void);
	int64				NumElements(void);
	bool				ValidateIndex(void);
	void				Dump(void);

	CIndexTreeIterator* CreateIterator(void);
	void				FreeIterator(CIndexTreeIterator* pcIter);

	CIndexTreeFile*		GetTree(void);

protected:
	bool	Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
	bool	Get(void* pvKey, int iKeySize, void* pvData, size_t* uiDataSize, size_t uiMaxDataSize);
	bool	Remove(void* pvKey, int iKeySize);
	int		DataSize(void* pvKey, int iKeySize);
	bool	Has(void* pvKey, int iKeySize);
	bool	Flush(void* pvKey, int iKeySize);
	bool	Evict(void* pvKey, int iKeySize);
};


#endif // !__INDEX_TREE_FILE_ACCESS_H__


