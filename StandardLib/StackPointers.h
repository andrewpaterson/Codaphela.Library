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
	int				UsedPointers(void);

	void			Remove(CStackPointer* pcFirst);

protected:
	CStackPointer*	FindUnused(void);
};


#endif // __STACK_POINTERS_H__

