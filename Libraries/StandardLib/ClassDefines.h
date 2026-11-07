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
#ifndef __CLASS_DEFINES_H__
#define __CLASS_DEFINES_H__
#include "BaseLib/PrimitiveTypes.h"  // SIZE_IS_LSIZE


#define M_Pointer(name)		Pointer(name.This(), #name)
#define M_Embedded(name)	Embedded(&name, #name);

#define M_Int8(name)		Primitive(&name, #name)
#define M_UInt8(name)		Primitive(&name, #name)
#define M_Int16(name)		Primitive(&name, #name)
#define M_UInt16(name)		Primitive(&name, #name)
#define M_Int32(name)		Primitive(&name, #name)
#define M_UInt32(name)		Primitive(&name, #name)
#define M_Int64(name)		Primitive(&name, #name)
#define M_UInt64(name)		Primitive(&name, #name)
#define M_Bool(name)		Primitive(&name, #name)
#define M_Float32(name)		Primitive(&name, #name)
#define M_Float64(name)		Primitive(&name, #name)
#define M_Char8(name)		Primitive(&name, #name)
#define M_Char16(name)		Primitive(&name, #name)


//Unmanaged fields are saved and loaded by CBaseObject::SaveUnmanaged, by what they are:
//  numbers, bools, chars, enums, vectors and U_Data as their bytes, so U_Data is only for plain data (no pointers, no
//  memory of its own, no virtual functions);
//  U_String as its text;
//  U_Pointer and U_Unknown not at all: a pointer loads as NULL, and an unknown (a container, or anything else that
//  isn't plain data) is the class's to Save and Load if it is to be kept.
//Describe a field one way: a field the class saves itself in Save is U_Unknown (or U_Pointer), not U_Data.
#define U_Int8(name)					UnmanagedInt8(&name, #name)
#define U_Int8Array(name, length)		UnmanagedInt8(name, length, #name)
#define U_Int16(name)					UnmanagedInt16(&name, #name)
#define U_Int16Array(name, length)		UnmanagedInt16(name, length, #name)
#define U_Int32(name)					UnmanagedInt32(&name, #name)
#define U_Int32Array(name, length)		UnmanagedInt32(name, length, #name)
#define U_Int64(name)					UnmanagedInt64(&name, #name)
#define U_Int64Array(name, length)		UnmanagedInt64(name, length, #name)
#define U_SInt(name)					UnmanagedSInt(&name, #name)
#define U_SIntArray(name, length)		UnmanagedSInt(name, length, #name)
#define U_UInt8(name)					UnmanagedUInt8(&name, #name)
#define U_UInt8Array(name, length)		UnmanagedUInt8(name, length, #name)
#define U_UInt16(name)					UnmanagedUInt16(&name, #name)
#define U_UInt16Array(name, length)		UnmanagedUInt16(name, length, #name)
#define U_UInt32(name)					UnmanagedUInt32(&name, #name)
#define U_UInt32Array(name, length)		UnmanagedUInt32(name, length, #name)
#define U_UInt64(name)					UnmanagedUInt64(&name, #name)
#define U_UInt64Array(name, length)		UnmanagedUInt64(name, length, #name)
#define U_SUInt(name)					UnmanagedSUInt(&name, #name)
#define U_SUIntArray(name, length)		UnmanagedSUInt(name, length, #name)
#define U_Size(name)					UnmanagedSize(&name, #name)
#define U_SizeArray(name, length)		UnmanagedSize(name, length, #name)
#ifdef SIZE_IS_LSIZE
#define U_LSize(name)					UnmanagedSize(&name, #name)
#else
#define U_LSize(name)					UnmanagedUInt32(&name, #name)
#endif // SIZE_IS_LSIZE
#define U_Float32(name)					UnmanagedFloat(&name, #name)
#define U_Float32Array(name, length)	UnmanagedFloat(name, length, #name)
#define U_Float64(name)					UnmanagedFloat(&name, #name)
#define U_Float64Array(name, length)	UnmanagedFloat(name, length, #name)
#define U_Float96(name)					UnmanagedFloat(&name, #name)
#define U_Float96Array(name, length)	UnmanagedFloat(name, length, #name)
#define U_Char(name)					UnmanagedChar(&name, #name)
#define U_CharArray(name, length)		UnmanagedChar(name, length, #name)
#define U_Char16(name)					UnmanagedChar(&name, #name)
#define U_Char16Array(name, length)		UnmanagedChar(name, length, #name)
#define U_Bool(name)					UnmanagedBool(&name, #name)
#define U_BoolArray(name, length)		UnmanagedBool(name, length, #name)

#define U_Enum(name)					UnmanagedEnum(&name, #name)

#define U_String(name)					UnmanagedString(&name, #name)  /* CChars or CCharsImmutable */
#define U_Number(name)					UnmanagedNumber(&name, #name)
#define U_Date(name)					UnmanagedDate(&name, #name)

#define U_Data(type, name)				UnmanagedData(&name, sizeof(type), #name)
#define U_Unknown(type, name)			UnmanagedUnknown(&name, sizeof(type), #name)
#define U_Pointer(name)					UnmanagedPointer((void**)&name, #name)


#define U_2Float32(name)				Unmanaged2Float(&name, #name)
#define U_2Float32Array(name, length)	Unmanaged2Float(name, length, #name)
#define U_3Float32(name)				Unmanaged3Float(&name, #name)
#define U_3Float32Array(name, length)	Unmanaged3Float(name, length, #name)
#define U_4Float32(name)				Unmanaged4Float(&name, #name)
#define U_4Float32Array(name, length)	Unmanaged4Float(name, length, #name)
#define U_4x4Float32(name)				Unmanaged4x4Float(&name, #name)
#define U_4x4Float32Array(name, length)	Unmanaged4x4Float(name, length, #name)

#define U_2Int32(name)					Unmanaged2Int(&name, #name)
#define U_2Int32Array(name, length)		Unmanaged2Int(name, length, #name)
#define U_3Int32(name)					Unmanaged3Int(&name, #name)
#define U_3Int32Array(name, length)		Unmanaged3Int(name, length, #name)
#define U_4Int32(name)					Unmanaged4Int(&name, #name)
#define U_4Int32Array(name, length)		Unmanaged4Int(name, length, #name)


#endif // __CLASS_DEFINES_H__

