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

	void*	Iterate(size* piDataSize = NULL, void* pvDestKey = NULL, size* puiKeySize = NULL, size uiMaxKeySize = 0);
};


#endif // __INDEX_MAP_ITERATOR_H__

