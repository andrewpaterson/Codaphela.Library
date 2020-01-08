#ifndef __INDEXED_DATA_ACCESS_H__
#define __INDEXED_DATA_ACCESS_H__
#include "BaseLib/IndexTreeAccess.h"
#include "IndexedData.h"

class CIndexedDataAccess : public CIndexTreeAccess
{
protected:
	CIndexedData*	mpcIndexData;

public:
	BOOL			Init(CIndexedData* pcIndexData);
	BOOL			Kill(void);
	BOOL			Flush(void);

protected:
	BOOL			Put(void* pvKey, int iKeySize, void* pvObject, unsigned int uiDataSize);
	BOOL			Get(void* pvKey, int iKeySize, void* pvObject, unsigned int* puiDataSize);
	unsigned int	DataSize(void* pvKey, int iKeySize);
	BOOL			Remove(void* pvKey, int iKeySize);
	BOOL			Has(void* pvKey, int iKeySize);
	BOOL			Flush(void* pvKey, int iKeySize);
};


#endif // __INDEXED_DATA_ACCESS_H__

