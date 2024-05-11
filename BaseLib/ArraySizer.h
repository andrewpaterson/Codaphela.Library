#ifndef __ARRAY_SIZER_H__
#define __ARRAY_SIZER_H__
#include "Define.h"
#include "PrimitiveTypes.h"


size CalculateChunkSize(size iNumElements);
size CalculateChunkSize(size iNumElements, size iCurrentChunkSize);
size CalculateArraySize(size iNumElements, size iChunkeSize);


#endif // __ARRAY_SIZER_H__

