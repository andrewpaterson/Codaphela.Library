#include "Counter.h"


template class CCounter<SNarrow>;
template class CCounter<SWide>;
template class CTally<int, SNarrow>;
template int Twice<int>(int);
template short Twice(short);

