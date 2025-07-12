#ifndef __INT_AND_POINTER_H__
#define __INT_AND_POINTER_H__
#include "PrimitiveTypes.h"
#include "IntAndPointer.h"


struct SIntAndPointer
{
	void*	pvData;
	uint	uiType;  //Arbitrary type values... no need to be a registered type.
};


struct SNameIntAndPointer : SIntAndPointer
{
	char*	szName;
};


#endif // __INT_AND_POINTER_H__

