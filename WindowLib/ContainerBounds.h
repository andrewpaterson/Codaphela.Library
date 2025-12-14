#ifndef __CONTAINER_BOUNDS_H__
#define __CONTAINER_BOUNDS_H__
#include "BaseLib/Int2.h"


struct SContainerBounds
{
	SInt2	msTopLeft;
	SInt2	msSize;

	void	Init(void);
};


#endif // __CONTAINER_BOUNDS_H__

