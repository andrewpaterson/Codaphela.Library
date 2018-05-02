#ifndef __ARRAY_SIZER_H__
#define __ARRAY_SIZER_H__
#include "Define.h"


int CalculateArraySized(int iUsedElements);
BOOL ArrayMustBeResizedForAdd(int iNewElements, int iOldElements, int* piNewArraySize);


#endif // __ARRAY_SIZER_H__

