#ifndef _INDEX_MAP_ACCESS_H__
#define _INDEX_MAP_ACCESS_H__
#include "MapAccess.h"


class CIndexBlock;
class CIndexMapAccess : public CMapAccess
{
protected:
	CIndexBlock*	mpcIndex;

public:
	void			Init(CIndexBlock* pcIndex);
	void			Kill(void);

	int64			NumElements(void);
	void			Dump(void);

	CMapIterator*	CreateIterator(void);
	void			FreeIterator(CMapIterator* pcIter);

protected:
	 void*			Put(const void* pvKey, size iKeySize, const void* pvData, size uiDataSize);
	 void*			Get(const void* pvKey, size iKeySize, size* puiDataSize);
	 size			DataSize(void* pvKey, size iKeySize);
	 bool			Remove(const void* pvKey, size iKeySize);
	 bool			Has(const void* pvKey, size iKeySize);
};


#endif // _INDEX_MAP_ACCESS_H__

