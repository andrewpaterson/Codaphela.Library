#ifndef __STACK_MEMORY_H__
#define __STACK_MEMORY_H__
#include "PointerFunctions.h"
#include "PrimitiveTypes.h"
#include "Numbers.h"


template<size I = 512>
class CStackMemory
{
private:
	union
	{
		uint8	macOnStack[I];
		uint8*	mpvOffStack;
	} u;
	uint16	muiSize;

public:
	void*	Init(void);
	void*	Init(size iSize);
	void	Kill(void);
	size	GetStackSize(void);
	void*	GetStackData(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<size I>
void* CStackMemory<I>::Init(void)
{
	u.mpvOffStack = NULL;
	muiSize = (uint16)I;
	return u.macOnStack;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<size I>
void* CStackMemory<I>::Init(size iSize)
{
	if (iSize > I)
	{
		muiSize = I + 1;
		u.mpvOffStack = (uint8*)malloc(iSize);
		return u.mpvOffStack;
	}
	else
	{
		muiSize = (uint16)iSize;
		u.mpvOffStack = NULL;
		return u.macOnStack;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<size I>
void CStackMemory<I>::Kill(void)
{
	if (muiSize == I + 1)
	{
		SafeFree(u.mpvOffStack);
	}
	muiSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<size I>
size CStackMemory<I>::GetStackSize(void)
{
	return I;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<size I>
void* CStackMemory<I>::GetStackData(void)
{
	if (muiSize == I + 1)
	{
		return u.mpvOffStack;
	}
	else
	{
		return u.macOnStack;
	}
}


#endif // __STACK_MEMORY_H__

