#ifndef __STACK_MEMORY_H__
#define __STACK_MEMORY_H__
#include "PointerFunctions.h"
#include "Numbers.h"


template<int I = 4 KB>
class CStackMemory
{
private:
	char	macOnStack[I];
	void*	mpvOffStack;

public:
	void*	Init(void);
	void*	Init(size_t iSize);
	void	Kill(void);
	int		GetStackSize(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<int I>
void* CStackMemory<I>::Init(void)
{
	mpvOffStack = NULL;
	return macOnStack;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<int I>
void* CStackMemory<I>::Init(size_t iSize)
{
	if (iSize > I)
	{
		mpvOffStack = malloc(iSize);
		return mpvOffStack;
	}
	else
	{
		mpvOffStack = NULL;
		return macOnStack;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<int I>
void CStackMemory<I>::Kill(void)
{
	SafeFree(mpvOffStack);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<int I>
int CStackMemory<I>::GetStackSize(void)
{
	return I;
}


#endif // __STACK_MEMORY_H__

