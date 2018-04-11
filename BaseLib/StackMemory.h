#ifndef __STACK_MEMORY_H__
#define __STACK_MEMORY_H__
#include "Numbers.h"


//Make this a template
class CStackMemory
{
private:
	char	macOnStack[8 KB];
	void*	mpvOffStack;

public:
	void*	Init(size_t iSize);
	void	Kill(void);
};

#endif // __STACK_MEMORY_H__

