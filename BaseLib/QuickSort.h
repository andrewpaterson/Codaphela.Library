#ifndef __QUICK_SORT_H__
#define __QUICK_SORT_H__
#include "Define.h"
#include "DataCallback.h"


void QuickSort(DataCompare fCompare, void* pvArray, size_t uiStride, int iNumElements);


#endif // !__QUICK_SORT_H__

