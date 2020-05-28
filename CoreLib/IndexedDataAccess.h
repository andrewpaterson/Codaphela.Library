#ifndef __INDEXED_DATA_ACCESS_H__
#define __INDEXED_DATA_ACCESS_H__
#include "BaseLib/IndexTreeAccess.h"
#include "IndexedData.h"

class CIndexedDataAccess : public CIndexTreeAccess
{
protected:
	CIndexedData*	mpcIndexData;

public:
	BOOL		Init(CIndexedData* pcIndexData);
	BOOL		Kill(void);
	BOOL		Flush(void);
	int64		NumElements(void);

protected:
	BOOL		Put(void* pvKey, int iKeySize, void* pvData, int iDataSize);
	BOOL		Get(void* pvKey, int iKeySize, void* pvData, int* piDataSize);
	int			DataSize(void* pvKey, int iKeySize);
	BOOL		Remove(void* pvKey, int iKeySize);
	BOOL		Has(void* pvKey, int iKeySize);
	BOOL		Flush(void* pvKey, int iKeySize);
	BOOL		Evict(void* pvKey, int iKeySize);
};


#endif // __INDEXED_DATA_ACCESS_H__

