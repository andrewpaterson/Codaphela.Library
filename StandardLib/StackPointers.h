#ifndef __STACK_POINTERS_H__
#define __STACK_POINTERS_H__
#include "StackPointer.h"


class CStackPointers
{
protected:
	CStackPointer*	mpcMemory;
	int				miNumPointers;
	
	int				miLastUsed;

public:
	void			Init(int iNumPointers);
	void			Kill(void);

	CStackPointer*	Add(CPointer* pcPointer);
	CStackPointer*	Add(CPointer* pcPointer, CStackPointer* pcFirst);
	void			Add(CStackPointer* pcStackPointer, CStackPointer* pcFirst);
	int				UsedPointers(void);

	void			RemoveAll(CStackPointer* pcFirst);
	CStackPointer* 	Remove(CStackPointer* pcFirst, CPointer* pcPointer);
	void			ClearAllPointers(void);

	CStackPointer*	FindUnused(void);
	void			TestAdd(int iIndex);
	void			TestKill(int iIndex);
	void			TestSetLastUsed(int iLastUsed);
	int				TestFindUnusedIndex(int iLastUsed);
};


extern CStackPointers	gcStackPointers;


#endif // !__STACK_POINTERS_H__

