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
	BOOL	Get(int iIndex, void** pv);
	void**	Get(int iIndex);
	void	QuickSort(void);
	int		Find(void* pv);
	void 	Push(void* pv);
	void*	Pop(void);
};


#endif // !__ARRAY_VOID_POINTER_H__

