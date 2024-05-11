#ifndef __BINARY_SEARCH_H__
#define __BINARY_SEARCH_H__
#include "PrimitiveTypes.h"
#include "DataCompare.h"


typedef void*	(*BinarySearchGet)(size iIndex);

bool BinarySearch(void* pvData, size iLeft, size iRight, DataCompare fCompare, size* piIndex, BinarySearchGet fGet);


#endif // __BINARY_SEARCH_H__

