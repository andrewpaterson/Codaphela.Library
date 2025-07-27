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
	 void*			Put(void* pvKey, size iKeySize, void* pvData, size uiDataSize);
	 void*			Get(void* pvKey, size iKeySize, size* puiDataSize);
	 size			DataSize(void* pvKey, size iKeySize);
	 bool			Remove(void* pvKey, size iKeySize);
	 bool			Has(void* pvKey, size iKeySize);
};


#endif // _INDEX_MAP_ACCESS_H__

