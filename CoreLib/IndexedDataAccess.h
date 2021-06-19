#ifndef __INDEXED_DATA_ACCESS_H__
#define __INDEXED_DATA_ACCESS_H__
#include "BaseLib/IndexAccess.h"
#include "IndexedData.h"

class CIndexedDataAccess : public CIndexAccess
{
protected:
	CIndexedData*	mpcIndexData;

public:
	BOOL				Init(CIndexedData* pcIndexData);
	BOOL				Kill(void);
	BOOL				Flush(void);
	int64				NumElements(void);
	BOOL				ValidateIndex(void);
	void				Dump(void);

	CIndexTreeIterator* CreateIterator(void);
	void				FreeIterator(CIndexTreeIterator* pcIter);

protected:
	BOOL	Put(void* pvKey, int iKeySize, void* pvData, size_t iDataSize);
	BOOL	Get(void* pvKey, int iKeySize, void* pvData, size_t* piDataSize, size_t uiMaxDataSize);
	int		DataSize(void* pvKey, int iKeySize);
	BOOL	Remove(void* pvKey, int iKeySize);
	BOOL	Has(void* pvKey, int iKeySize);
	BOOL	Flush(void* pvKey, int iKeySize);
	BOOL	Evict(void* pvKey, int iKeySize);
};


#endif // !__INDEXED_DATA_ACCESS_H__

