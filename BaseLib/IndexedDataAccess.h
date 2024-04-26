#ifndef __INDEXED_DATA_ACCESS_H__
#define __INDEXED_DATA_ACCESS_H__
#include "IndexAccess.h"
#include "IndexedData.h"

class CIndexedDataAccess : public CIndexAccess
{
protected:
	CIndexedData*	mpcIndexData;

public:
	bool				Init(CIndexedData* pcIndexData);
	bool				Kill(void);
	bool				Flush(void);
	int64				NumElements(void);
	bool				ValidateIndex(void);
	void				Dump(void);

	CIndexTreeIterator* CreateIterator(void);
	void				FreeIterator(CIndexTreeIterator* pcIter);

protected:
	bool	Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
	bool	Get(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize);
	int		DataSize(void* pvKey, int iKeySize);
	bool	Remove(void* pvKey, int iKeySize);
	bool	Has(void* pvKey, int iKeySize);
	bool	Flush(void* pvKey, int iKeySize);
	bool	Evict(void* pvKey, int iKeySize);
};


#endif // __INDEXED_DATA_ACCESS_H__

