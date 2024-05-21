#ifndef __PRIMITIVE_INTEGER_H__
#define __PRIMITIVE_INTEGER_H__
#include "BaseLib/Logger.h"
#include "Primitive.h"


PRIMITIVE_TYPE_DEFINITION(Int8, int8, PT_char8, WriteByte, ReadByte)
PRIMITIVE_TYPE_DEFINITION(UInt8, uint8, PT_uint8, WriteByte, ReadByte)

PRIMITIVE_TYPE_DEFINITION(Int16, int16, PT_int16, WriteShort, ReadShort)
PRIMITIVE_TYPE_DEFINITION(UInt16, uint16, PT_uint16, WriteShort, ReadShort)

PRIMITIVE_TYPE_DEFINITION(Int32, int32, PT_int32, WriteInt32, ReadInt32)
PRIMITIVE_TYPE_DEFINITION(UInt32, uint32, PT_uint32, WriteInt32, ReadInt32)

PRIMITIVE_TYPE_DEFINITION(Int64, int64, PT_int64, WriteLong, ReadLong)
PRIMITIVE_TYPE_DEFINITION(UInt64, uint64, PT_uint64, WriteLong, ReadLong)

PRIMITIVE_TYPE_DEFINITION(Bool, bool, PT_bool, WriteBool, ReadBool)


#endif // __PRIMITIVE_INTEGER_H__

