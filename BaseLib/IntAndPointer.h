#ifndef __ARRAY_INT_AND_POINTER_H__
#define __ARRAY_INT_AND_POINTER_H__
#include "PrimitiveTypes.h"
#include "IntAndPointer.h"


struct SIntAndPointer
{
	void*	pvData;
	uint	uiType;  //Arbitrary type values... no need to be a registered type.
};


#endif // __ARRAY_INT_AND_POINTER_H__

