/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __PRIMITIVE_INTEGER_H__
#define __PRIMITIVE_INTEGER_H__
#include "BaseLib/Logger.h"
#include "Primitive.h"


PRIMITIVE_TYPE_DEFINITION(Int8, int8, PT_int8, WriteInt8, ReadInt8)
PRIMITIVE_TYPE_DEFINITION(UInt8, uint8, PT_uint8, WriteInt8, ReadInt8)

PRIMITIVE_TYPE_DEFINITION(Int16, int16, PT_int16, WriteInt16, ReadInt16)
PRIMITIVE_TYPE_DEFINITION(UInt16, uint16, PT_uint16, WriteInt16, ReadInt16)

PRIMITIVE_TYPE_DEFINITION(Int32, int32, PT_int32, WriteInt32, ReadInt32)
PRIMITIVE_TYPE_DEFINITION(UInt32, uint32, PT_uint32, WriteInt32, ReadInt32)

PRIMITIVE_TYPE_DEFINITION(Int64, int64, PT_int64, WriteInt64, ReadInt64)
PRIMITIVE_TYPE_DEFINITION(UInt64, uint64, PT_uint64, WriteInt64, ReadInt64)

PRIMITIVE_TYPE_DEFINITION(Bool, bool, PT_bool, WriteBool, ReadBool)


#endif // __PRIMITIVE_INTEGER_H__

