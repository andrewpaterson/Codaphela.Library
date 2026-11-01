#ifndef __ARRAY_SIZER_H__
#define __ARRAY_SIZER_H__
#include "Define.h"
#include "PrimitiveTypes.h"


lsize CalculateChunkSize(lsize iNumElements);
lsize CalculateChunkSize(lsize iNumElements, lsize iCurrentChunkSize);
lsize CalculateArraySize(lsize iNumElements, lsize iChunkeSize);


#endif // __ARRAY_SIZER_H__

