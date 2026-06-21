#ifndef __STACK_POINTER_H__
#define __STACK_POINTER_H__
#include "BaseLib/Define.h"
#include "BaseLib/FreeList.h"


enum EStackPointerType
{
	SPT_Unknown,
	SPT_Pointer,
	SPT_Collection
};


class CPointer;
class CCollection;
struct SStackPointer
{
	EStackPointerType	meType;
	union
	{
		CPointer*		pcPointer;  //Pointer on the stack
		CCollection*	pcCollection;
	} u;
};


class CStackPointer
{
protected:
	CStackPointer*	mpcNext;  //The object pointer to by the stack pointer points to 'this' stack pointer and additional stack pointers are linked through next.
	SStackPointer	msPointer;

public:
	void				Init(void);
	void				Init(CPointer* pcPointer);
	void				Init(CCollection* pcCollection);
	void				Kill(void);

	void				SetNext(CStackPointer* pcNext);
	int					NumPointers(void);
	CStackPointer*		FindLast(void);
	CStackPointer*		GetNext(void);
	
	CStackPointer*		Remove(CPointer* pcPointer, CFreeList* pcFreeList);
	CStackPointer*		Remove(CCollection* pcPointer, CFreeList* pcFreeList);
	void				RemoveAll(CFreeList* pcFreeList);
	void				ClearPointer(void);
	CStackPointer*		ClearPointerGetNext(void);

	EStackPointerType	GetType(void);
	SStackPointer*		Get(void);
};


#endif // __STACK_POINTER_H__

