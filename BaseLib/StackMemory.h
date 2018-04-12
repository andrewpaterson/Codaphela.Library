#ifndef __STACK_MEMORY_H__
#define __STACK_MEMORY_H__
#include "PointerFunctions.h"
#include "Numbers.h"


template<int I = 8 KB>
class CStackMemory
{
private:
	char	macOnStack[I];
	void*	mpvOffStack;

public:
	void*	Init(size_t iSize);
	void	Kill(void);
};


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


#endif // __STACK_MEMORY_H__

