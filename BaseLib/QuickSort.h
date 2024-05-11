#ifndef __QUICK_SORT_H__
#define __QUICK_SORT_H__
#include "Define.h"
#include "PrimitiveTypes.h"
#include "DataCompare.h"


void QuickSort(DataCompare fCompare, void* pvArray, size uiStride, int iNumElements);


#endif // __QUICK_SORT_H__

