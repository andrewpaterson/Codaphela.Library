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


#define INT_BYTE_SIZE		4
#define SHORT_BYTE_SIZE		2
#define CHAR_BYTE_SIZE		1
#define FLOAT_BYTE_SIZE		4
#define DOUBLE_BYTE_SIZE	82
#define LONG_BYTE_SIZE		8
#define FLOAT2_BYTE_SIZE	8
#define FLOAT3_BYTE_SIZE	12
#define FLOAT4_BYTE_SIZE	16
#define INT2_BYTE_SIZE		8
#define INT3_BYTE_SIZE		12
#define INT4_BYTE_SIZE		16
#define BOOL_BYTE_SIZE		4
#define VOID_BYTE_SIZE		0

#define BIT_SIZE			(1 | SIZE_IN_BITS)
#define CRUMB_SIZE			(2 | SIZE_IN_BITS)
#define NYBBLE_SIZE			(4 | SIZE_IN_BITS)
#define NICKLE_SIZE			(5 | SIZE_IN_BITS)
#define SIXBITS_SIZE		(6 | SIZE_IN_BITS)


class CFileReader;
class CFileWriter;


//These are not C/C++ types.
enum EPrimitiveType
{
	PT_Undefined,

	//Standard primitives (size in bits).
	PT_int,  	//32
	PT_short, 	//16
	PT_char,	//8
	PT_float,	//32
	PT_double,	//64
	PT_uint,	//32
	PT_ushort,	//16
	PT_uchar,	//8
	PT_long,	//64
	PT_ulong,	//64
	PT_float2,	//64
	PT_float3,	//96
	PT_float4,  //128
	PT_int2,	//64
	PT_int3,	//96
	PT_int4,	//128
	PT_bool, 	//32
	PT_void,	//0
	PT_wchar,	//16

	//"Complex" primitives, so widely used they're included
	PT_String,  //CChars
	PT_Number,	//CNumber
	PT_Date,  //CDate

	//Special primitives.
	PRIMTIVE_OPERATOR_END,
	PT_bit,
	PT_crumb,	//2
	PT_nybble,	//4
	PT_nickle,	//5
	PT_sixbits, //6
	PRIMTIVE_TYPE_END,

	//Pointers.
	PT_Pointer,  //An arbitrary pointer.

	NUM_PRIMITIVE_TYPES,
	STRUCT_TYPES = 0x40,  //Any type with an ID >= 0x40 is a struct type.
};


#define SIZE_IN_BITS		0x80000000 //If GetSize returns a size with this flag set then the size is in bits, not bytes.


#endif // __DATA_TYPES_H__

