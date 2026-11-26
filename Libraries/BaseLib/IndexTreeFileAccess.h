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
	bool				Kill(void) override;
	bool				Flush(void) override;
	int64				NumElements(void) const override;
	bool				ValidateIndex(void) override;
	void				Dump(void) override;

	CIndexTreeIterator*	CreateIterator(void) override;
	void				FreeIterator(CIndexTreeIterator* pcIter) override;

	CIndexTreeFile*		GetTree(void) const;

protected:
	bool	Put(uint8* pvKey, size iKeySize, const void* pvData, size iDataSize) override;
	bool	Get(uint8* pvKey, size iKeySize, void* pvData, size* uiDataSize, size uiMaxDataSize) override;
	bool	Remove(uint8* pvKey, size iKeySize) override;
	size	DataSize(uint8* pvKey, size iKeySize) override;
	bool	Has(uint8* pvKey, size iKeySize) override;
	bool	Flush(uint8* pvKey, size iKeySize) override;
	bool	Evict(uint8* pvKey, size iKeySize) override;
};


#endif // __INDEX_TREE_FILE_ACCESS_H__


