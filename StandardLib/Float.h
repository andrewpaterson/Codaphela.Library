#ifndef __PRIMITIVE_FLOAT_H__
#define __PRIMITIVE_FLOAT_H__
#include "BaseLib/Logger.h"
#include "Primitive.h"


PRIMITIVE_TYPE_DEFINITION(Float32, float32, PT_float32, WriteFloat, ReadFloat)
PRIMITIVE_TYPE_DEFINITION(Float64, float64, PT_float64, WriteFloat, ReadFloat)


#endif // __PRIMITIVE_FLOAT_H__

