#ifndef __STACK_POINTER_H__
#define __STACK_POINTER_H__
#include "BaseLib/Define.h"


class CPointer;
class CStackPointer
{
protected:
	CStackPointer*		mpcNext;  //The object pointer to by the stack pointer points to 'this' stack pointer and additional stack pointers are linked through next.
	CPointer*			mpcPointer;  //Pointer on the stack

public:
	BOOL				mbUsed;

public:
	void			Init(CPointer* pcPointer);
	void			Kill(void);

	void			SetNext(CStackPointer* pcNext);
	int				NumPointers(void);
	CStackPointer*	FindLast(void);
	CStackPointer*	GetNext(void);
	
	CStackPointer*	Remove(CPointer* pcPointer);
	void			RemoveAll(void);
	void			ClearPointer(void);
	CStackPointer*	ClearPointerGetNext(void);
	CPointer*		GetPointer(void);
};


#endif // __STACK_POINTER_H__

