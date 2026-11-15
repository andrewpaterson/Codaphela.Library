#include "BubbleSort.h"
#include "QuickSort.h"
#include "TimSort.h"
#include "SystemAllocator.h"
#include "NullAllocator.h"
#include "GlobalMemory.h"
#include "ArraySizer.h"
#include "StdRandom.h"
#include "ErrorHandler.h"
#include "ArrayBlock.h"
#include "ArrayBlockCode.inl"


//The only instances: ArrayBlock.h declares them extern.
template struct SArrayHeaderBase<SArrayNaturalSize>;
template struct SArrayHeaderBase<SArrayLargeSize>;
template class CArrayBlockBase<SArrayNaturalSize>;
template class CArrayBlockBase<SArrayLargeSize>;

