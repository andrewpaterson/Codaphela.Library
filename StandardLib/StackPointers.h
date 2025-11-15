#ifndef __STACK_POINTERS_H__
#define __STACK_POINTERS_H__
#include "StackPointer.h"


class CStackPointers
{
protected:
	CStackPointer*	mpcMemory;
	size			miAllocatedPointers;
	
	size			miLastUsed;

public:
	void			Init(size iNumPointers);
	void			Kill(void);

	CStackPointer*	Add(CPointer* pcPointer);
	CStackPointer*	Add(CPointer* pcPointer, CStackPointer* pcFirst);
	CStackPointer*	Add(CCollection* pcCollection);
	CStackPointer*	Add(CCollection* pcCollection, CStackPointer* pcFirst);
	void			Add(CStackPointer* pcStackPointer, CStackPointer* pcFirst);

	void			RemoveAll(CStackPointer* pcFirst);
	CStackPointer* 	Remove(CStackPointer* pcFirst, CPointer* pcPointer);
	void			ClearAllPointers(void);

	CStackPointer*	FindUnused(void);
	size			NumElements(void);
	SStackPointer*	Get(size uiIndex);

	void			TestAdd(size iIndex);
	void			TestKill(size iIndex);
	void			TestSetLastUsed(size iLastUsed);
	size			TestFindUnusedIndex(size iLastUsed);
};


extern CStackPointers	gcStackPointers;


#endif // __STACK_POINTERS_H__

