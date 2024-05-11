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
	bool	Put(void* pvKey, size iKeySize, void* pvData, size iDataSize);
	bool	Get(void* pvKey, size iKeySize, void* pvData, size* piDataSize, size uiMaxDataSize);
	size	DataSize(void* pvKey, size iKeySize);
	bool	Remove(void* pvKey, size iKeySize);
	bool	Has(void* pvKey, size iKeySize);
	bool	Flush(void* pvKey, size iKeySize);
	bool	Evict(void* pvKey, size iKeySize);
};


#endif // __INDEXED_DATA_ACCESS_H__

