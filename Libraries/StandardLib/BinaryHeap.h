#ifndef __BINARY_HEAP_H__
#define __BINARY_HEAP_H__
#include "BaseLib/ArrayTemplate.h"


//A min-heap kept in a CArrayTemplate: the least element (by fLess) is first.  The array is the heap's only state, so
//anything that keeps one can be one without another class.


//////////////////////////////////////////////////////////////////////////
//
// Adds *pv, moving it up past larger parents.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void BinaryHeapPush(CArrayTemplate<M>* pacHeap, M* pv, bool (*fLess)(M*, M*))
{
	M		sItem;
	size	uiChild;
	size	uiParent;
	M*		psParent;

	sItem = *pv;
	uiChild = pacHeap->NumElements();
	pacHeap->Add();
	while (uiChild > 0)
	{
		uiParent = (uiChild - 1) / 2;
		psParent = pacHeap->Get(uiParent);
		if (!fLess(&sItem, psParent))
		{
			break;
		}
		*pacHeap->Get(uiChild) = *psParent;
		uiChild = uiParent;
	}
	*pacHeap->Get(uiChild) = sItem;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the least element out into *pvDest.  The heap must not be empty.
// The last element goes where it was and moves down past smaller children.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void BinaryHeapPop(CArrayTemplate<M>* pacHeap, M* pvDest, bool (*fLess)(M*, M*))
{
	M		sLast;
	size	uiNum;
	size	uiParent;
	size	uiChild;
	M*		psChild;

	*pvDest = *pacHeap->Get(0);
	pacHeap->Pop(&sLast);
	uiNum = pacHeap->NumElements();
	if (uiNum == 0)
	{
		return;
	}

	uiParent = 0;
	for (;;)
	{
		uiChild = uiParent * 2 + 1;
		if (uiChild >= uiNum)
		{
			break;
		}
		psChild = pacHeap->Get(uiChild);
		if ((uiChild + 1 < uiNum) && fLess(pacHeap->Get(uiChild + 1), psChild))
		{
			uiChild++;
			psChild = pacHeap->Get(uiChild);
		}
		if (!fLess(psChild, &sLast))
		{
			break;
		}
		*pacHeap->Get(uiParent) = *psChild;
		uiParent = uiChild;
	}
	*pacHeap->Get(uiParent) = sLast;
}


#endif // __BINARY_HEAP_H__
