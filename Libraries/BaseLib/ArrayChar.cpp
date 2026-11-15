#include "ArrayChar.h"
#include "ArrayCharCode.inl"


//The only instances: ArrayChar.h declares them extern.
template class CArrayCharBase<CArrayTemplatePrimitive<char> >;
template class CArrayCharBase<CArrayLargeTemplatePrimitive<char> >;

