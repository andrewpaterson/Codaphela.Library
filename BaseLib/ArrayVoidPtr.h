#ifndef __ARRAY_VOID_POINTER_H__
#define __ARRAY_VOID_POINTER_H__
#include "ArrayTemplate.h"


typedef CArrayTemplate<void*>	__CArrayVoidPtr;


class CArrayVoidPtr : public __CArrayVoidPtr
{
public:
	void	Init(void);
	void	Kill(void);
	void	Add(void* pv);
	void*	GetPtr(int iIndex);
	void	QuickSort(void);
};


int CompareVoidPtr(const void* arg1, const void* arg2);


#endif // __ARRAY_VOID_POINTER_H__

