#ifndef __STACK_POINTERS_H__
#define __STACK_POINTERS_H__
#include "BaseLib/FreeList.h"
#include "StackPointer.h"


class CStackPointers
{
protected:
	CFreeList		mcFreeList;

public:
	void			Init(void);
	void			Kill(void);

	CStackPointer*	Add(CPointer* pcPointer);
	CStackPointer*	Add(CPointer* pcPointer, CStackPointer* pcFirst);
	CStackPointer*	Add(CCollection* pcCollection);
	CStackPointer*	Add(CCollection* pcCollection, CStackPointer* pcFirst);
	bool			SetLastNext(CStackPointer* pcStackPointer, CStackPointer* pcFirst);

	void			RemoveAll(CStackPointer* pcFirst);
	CStackPointer* 	Remove(CStackPointer* pcFirst, CPointer* pcPointer, bool* pbRemoved);
	CStackPointer*	Remove(CStackPointer* pcFirst, CCollection* pcCollection, bool* pbRemoved);
	void			ClearAllPointers(void);

	size			NumElements(void);
	CStackPointer*	Get(size uiIndex);

	CStackPointer*	StartIteration(SFreeListIterator* psIter);
	CStackPointer*	Iterate(SFreeListIterator* psIter);
};


extern CStackPointers	gcStackPointers;


#endif // __STACK_POINTERS_H__

