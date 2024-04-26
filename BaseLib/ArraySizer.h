#ifndef __ARRAY_SIZER_H__
#define __ARRAY_SIZER_H__
#include "Define.h"


int CalculateChunkSize(int iNumElements);
int CalculateChunkSize(int iNumElements, int iCurrentChunkSize);
int CalculateArraySize(int iNumElements, int iChunkeSize);


#endif // __ARRAY_SIZER_H__

