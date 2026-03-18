#ifndef __INDEX_PTR_H__
#define __INDEX_PTR_H__
#include "IndexTemplate.h"


class CIndexPtr : public CIndexTemplate<void*>
{
public:
	void*	GetPtr(uint8* pvKey, size iKeySize);

	void**	Put(uint8* pvKey, size iKeySize);
	bool	Put(uint8* pvKey, size iKeySize, void* pvData);

	bool	StartIteration(SIndexTreeMemoryUnsafeIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, void** ppvData);
	bool	Iterate(SIndexTreeMemoryUnsafeIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, void** ppvData);
	bool	StartIteration(SIndexTreeMemoryIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, void** ppvData);
	bool	Iterate(SIndexTreeMemoryIterator* psIterator, void* pvDestKey, size* puiKeySize, size uiMaxKeySize, void** ppvData);
};


#endif // __INDEX_PTR_H__


