/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __DATA_TYPES_H__
#define __DATA_TYPES_H__
#include "ErrorHandler.h"
#include "Chars.h"
#include "ArrayTemplateMinimal.h"


#define MAX_CHAR	((char)0x7f)
#define MIN_CHAR	((char)0x80)
#define MAX_UCHAR	((unsigned char)0xff)
#define MIN_UCHAR	((unsigned char)0x00)

#define MAX_SHORT	((int16)0x7fff)
#define MIN_SHORT	((int16)0x8000)
#define MAX_USHORT	((uint16)0xffff)
#define MIN_USHORT	((uint16)0x0000)

#define MAX_INT		((int)0x7fffffff)
#define MIN_INT		((int)0x80000000)
#define MAX_UINT	((unsigned int)0xffffffff)
#define MIN_UINT	((unsigned int)0x00000000)

#define MAX_LONG	((long long)0x7fffffffffffffffLL)
#define MIN_LONG	((long long)0x8000000000000000LL)
#define MAX_ULONG	((unsigned long long)0xffffffffffffffffLL)
#define MIN_ULONG	((unsigned long long)0x0000000000000000LL)


#define INT_BYTE_SIZE		sizeof(int32)
#define SHORT_BYTE_SIZE		sizeof(int16)
#define CHAR_BYTE_SIZE		sizeof(char8)
#define BYTE_BYTE_SIZE		1
#define WIDE_CHAR_BYTE_SIZE	sizeof(char16)
#define FLOAT_BYTE_SIZE		sizeof(float32)
#define DOUBLE_BYTE_SIZE	sizeof(float64)
#define LONG_BYTE_SIZE		sizeof(int64)
#define FLOAT2_BYTE_SIZE	sizeof(SFloat2)
#define FLOAT3_BYTE_SIZE	sizeof(SFloat3)
#define FLOAT4_BYTE_SIZE	sizeof(SFloat4)
#define DOUBLE2_BYTE_SIZE	sizeof(SDouble2)
#define DOUBLE3_BYTE_SIZE	sizeof(SDouble3)
#define DOUBLE4_BYTE_SIZE	sizeof(SDouble4)
#define INT2_BYTE_SIZE		sizeof(SInt2)
#define INT3_BYTE_SIZE		sizeof(SInt3)
#define INT4_BYTE_SIZE		sizeof(SInt4)
#define BOOL_BYTE_SIZE		sizeof(BOOL)
#define VOID_BYTE_SIZE		0
#define FLOAT4x4_BYTE_SIZE	sizeof(SFloat4x4)
#define DOUBLE4x4_BYTE_SIZE	sizeof(SDouble4x4)

#define BIT_SIZE			(1 | SIZE_IN_BITS)
#define CRUMB_SIZE			(2 | SIZE_IN_BITS)
#define TRIBBLE_SIZE		(3 | SIZE_IN_BITS)
#define NYBBLE_SIZE			(4 | SIZE_IN_BITS)
#define NICKLE_SIZE			(5 | SIZE_IN_BITS)
#define SIXBITS_SIZE		(6 | SIZE_IN_BITS)


class CFileReader;
class CFileWriter;


enum EPrimitiveType
{
	PT_Undefined,

	//Standard primitives (size in bits).
	PT_int8,		//8
	PT_int16, 		//16
	PT_int32,  		//32
	PT_int64,		//64
	PT_uint8,		//8
	PT_uint16,		//16
	PT_uint32,		//32
	PT_uint64,		//64
	PT_float32,		//32
	PT_float64,		//64
	PT_char8,		//8  ANSI or UTF-8, zero terminated
	PT_char16,		//16 UTF-16, zero zero terminated
	PT_M2float32,	//64
	PT_M3float32,	//96
	PT_M4float32,   //128
	PT_M2float64,	//128
	PT_M3float64,	//192
	PT_M4float64,   //256
	PT_M2int32,		//64
	PT_M3int32,		//96
	PT_M4int32,		//128
	PT_bool, 		//32
	PRIMTIVE_OPERATOR_END,  //See Operator*.inl and Equal*.inl

	//Special primitives.
	PT_bit,
	PT_crumb,		//2
	PT_tribble,		//3
	PT_nybble,		//4
	PT_nickle,		//5
	PT_sixbits,		//6
	PRIMTIVE_CONVERTER_END, //See TypeConverter_Mapping.inl

	PT_M4x4float32,
	PT_M4x4float64,

	//"Complex" primitives
	PT_String,		//CChars
	PT_Number,		//CNumber
	PT_Date,		//CDate
	PT_DateTime,	//CDateTime

	PT_void,		//0

	//Pointers.
	PT_Pointer,		//An arbitrary void pointer.
	PT_Data,		//8 Unknown arrays of bytes.  Data implies you should not perform operations/transformations directy on the data array.

	NUM_PRIMITIVE_TYPES,
	CLASS_TYPES = 0x40,  //Any type with an ID >= 0x40 is a class type.
};


#define SIZE_IN_BITS		0x80000000 //If GetSize returns a size with this flag set then the size is in bits, not bytes.


#endif // !__DATA_TYPES_H__

