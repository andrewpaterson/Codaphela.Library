#ifndef __CONTAINER_BOUNDS_H__
#define __CONTAINER_BOUNDS_H__
#include "BaseLib/Int32Vec2.h"


struct SContainerBounds
{
	SInt32Vec2	msTopLeft;
	SInt32Vec2	msSize;

	void	Init(void);
};


#endif // __CONTAINER_BOUNDS_H__

