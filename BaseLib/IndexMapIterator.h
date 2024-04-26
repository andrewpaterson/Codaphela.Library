#ifndef __INDEX_MAP_ITERATOR_H__
#define __INDEX_MAP_ITERATOR_H__
#include "IndexBlock.h"
#include "MapIterator.h"


class CIndexMapIterator : public CMapIterator
{
protected:
	CIndexBlock*					mpcIndex;
	bool							mbStarted;
	SIndexTreeMemoryUnsafeIterator	msIter;

public:
	void	Init(CIndexBlock* pcIndex);
	void	Kill(void);

	void*	Iterate(size_t* piDataSize = NULL, void* pvDestKey = NULL, size_t* puiKeySize = NULL, size_t uiMaxKeySize = 0);
};


#endif // __INDEX_MAP_ITERATOR_H__

