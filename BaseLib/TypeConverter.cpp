/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (pi) 2022 Andrew Paterson

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
#include "ErrorHandler.h"
#include "DataTypes.h"
#include "TypeConverter.h"


CTypeConverter gcTypeConverter;


#define FLOAT_S(dest_type, source_type, min, max)	\
if (*pf >= (source_type)0)							\
	if (*pf >= (source_type)1)						\
		*psDest = (dest_type)max;					\
	else											\
		*psDest = (dest_type)(max * *pf);			\
else												\
	if (*pf <= -(source_type)1)						\
		*psDest = (dest_type)min;					\
	else											\
		*psDest = (dest_type)(min * *pf)

#define FLOAT_U(dest_type, source_type, min, max)	\
	if (*pf >= (source_type)0)						\
		if (*pf >= (source_type)1)					\
			* psDest = (dest_type)max;				\
		else										\
			* psDest = (dest_type)(max * *pf);		\
	else											\
		* psDest = (dest_type)min

#define SCALE_TRUNCATE_FLOAT_TO_SINT(source_type, dest_type, min, max) \
void ConvertScaleTruncate_##source_type##_To_##dest_type(dest_type* psDest, source_type* pf) { FLOAT_S(dest_type, source_type, min, max); }

#define SCALE_TRUNCATE_FLOAT_TO_UINT(source_type, dest_type, min, max) \
void ConvertScaleTruncate_##source_type##_To_##dest_type(dest_type* psDest, source_type* pf) { FLOAT_U(dest_type, source_type, min, max); }

#define SCALE_TRUNCATE_BIG_SINT_TO_SMALL_SINT(source_type, dest_type, dest_max, source_max) \
void ConvertScaleTruncate_##source_type##_To_##dest_type(dest_type* psDest, source_type* pi) { *psDest = (dest_type)(*pi / (((u##source_type)source_max + 1) / (dest_max + 1))); }

#define SCALE_TRUNCATE_BIG_UINT_TO_SMALL_SINT(source_type, dest_type, dest_max, source_max) \
void ConvertScaleTruncate_##source_type##_To_##dest_type(dest_type* psDest, source_type* pi) { *psDest = (dest_type)(*pi / ((((source_type)source_max + 1) / (dest_max + 1)) * 2)); }

#define SCALE_TRUNCATE_BIG_UINT_TO_SMALL_UINT(source_type, dest_type, dest_max, source_max) \
void ConvertScaleTruncate_##source_type##_To_##dest_type(dest_type* psDest, source_type* pi) { *psDest = (dest_type)(*pi / (((source_type)source_max + 1) / (dest_max + 1))); }

#define SCALE_TRUNCATE_BIG_SINT_TO_SMALL_UINT(source_type, dest_type, dest_max, source_max) \
void ConvertScaleTruncate_##source_type##_To_##dest_type(dest_type* psDest, source_type* pi) { *psDest = *pi >= 0 ? (dest_type)(*pi / ((((u##source_type)source_max + 1) / (dest_max + 1)) / 2)) : 0; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int8' Conversions.
SCALE_TRUNCATE_FLOAT_TO_SINT(float64, int8, MIN_CHAR, MAX_CHAR)
SCALE_TRUNCATE_FLOAT_TO_SINT(float32, int8, MIN_CHAR, MAX_CHAR)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_SINT(int64, int8, MAX_CHAR, MAX_LONG)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_SINT(int32, int8, MAX_CHAR, MAX_INT)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_SINT(int16, int8, MAX_CHAR, MAX_SHORT)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_SINT(int8, int8, MAX_CHAR, MAX_CHAR)
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_SINT(uint64, int8, MAX_CHAR, MAX_LONG)
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_SINT(uint32, int8, MAX_CHAR, MAX_INT)
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_SINT(uint16, int8, MAX_CHAR, MAX_SHORT)
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_SINT(uint8, int8, MAX_CHAR, MAX_CHAR)
SCALE_TRUNCATE_FLOAT_TO_UINT(float64, uint8, MIN_UCHAR, MAX_UCHAR)
SCALE_TRUNCATE_FLOAT_TO_UINT(float32, uint8, MIN_UCHAR, MAX_UCHAR)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_UINT(int64, uint8, MAX_CHAR, MAX_LONG)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_UINT(int32, uint8, MAX_CHAR, MAX_INT)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_UINT(int16, uint8, MAX_CHAR, MAX_SHORT)
void ConvertScaleTruncate_int8_To_uint8(uint8* psDest,	int8* pi)	{ *psDest = *pi >= 0 ? (*pi * 2 + (*pi > 0x40 ? 1 : 0)) : 0; }
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_UINT(uint64, uint8, MAX_CHAR, MAX_LONG)
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_UINT(uint32, uint8, MAX_CHAR, MAX_INT)
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_UINT(uint16, uint8, MAX_CHAR, MAX_SHORT)
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_UINT(uint8, uint8, MAX_CHAR, MAX_CHAR)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int16' Conversions.
SCALE_TRUNCATE_FLOAT_TO_SINT(float64, int16, MIN_SHORT, MAX_SHORT)
SCALE_TRUNCATE_FLOAT_TO_SINT(float32, int16, MIN_SHORT, MAX_SHORT)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_SINT(int64, int16, MAX_SHORT, MAX_LONG)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_SINT(int32, int16, MAX_SHORT, MAX_INT)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_SINT(int16, int16, MAX_SHORT, MAX_SHORT)
void ConvertScaleTruncate_int8_To_int16(int16* psDest, int8* pi) { *psDest = ((int16)*pi << 8) | *pi; }
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_SINT(uint64, int16, MAX_SHORT, MAX_LONG)
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_SINT(uint32, int16, MAX_SHORT, MAX_INT)
SCALE_TRUNCATE_BIG_UINT_TO_SMALL_SINT(uint16, int16, MAX_SHORT, MAX_SHORT)
void ConvertScaleTruncate_uint8_To_int16(int16* psDest, uint8* pi) { *psDest = ((int16)*pi << 8) | *pi; }
SCALE_TRUNCATE_FLOAT_TO_UINT(float64, uint16, MIN_USHORT, MAX_USHORT)
SCALE_TRUNCATE_FLOAT_TO_UINT(float32, uint16, MIN_USHORT, MAX_USHORT)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_UINT(int64, uint16, MAX_SHORT, MAX_LONG)
SCALE_TRUNCATE_BIG_SINT_TO_SMALL_UINT(int32, uint16, MAX_SHORT, MAX_INT)
void ConvertScaleTruncate_int16_To_uint16(uint16* psDest, int16* pi) { *psDest = *pi >= 0 ? *pi / 2 : 0; }
void ConvertScaleTruncate_int8_To_uint16	(uint16* psDest,	int8* pi)		{ *psDest = *pi >= 0 ? ((uint16)*pi << 9) | *pi << 1: 0; }
void ConvertScaleTruncate_uint64_To_uint16	(uint16* psDest,	uint64* pi)		{ *psDest = (uint16)(*pi / 0x1000100010001LL); }
void ConvertScaleTruncate_uint32_To_uint16	(uint16* psDest,	uint32* pi)		{ *psDest = *pi / (0xffffffff/0xffff); }
void ConvertScaleTruncate_uint16_To_uint16	(uint16* psDest,	uint16* pi)		{ *psDest = *pi; }
void ConvertScaleTruncate_uint8_To_uint16	(uint16* psDest,	uint8* pi)		{ *psDest = ((uint16)*pi << 8) | *pi; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int32' Conversions.
void ConvertScaleTruncate_float64_To_int32	(int32* piDest,		float64* pf) 	{ *piDest = (int32)((float64)0xffffffff * *pf); }
void ConvertScaleTruncate_float32_To_int32		(int32* piDest,		float32* pf) 	{ *piDest = (int32)((float32)0xffffffff * *pf); }
void ConvertScaleTruncate_int64_To_int32		(int32* piDest,		int64* pi)		{ *piDest = (int32)(*pi / 0x100000001LL); }
void ConvertScaleTruncate_int32_To_int32		(int32* piDest,		int32* pi)		{ *piDest = *pi; }	   
void ConvertScaleTruncate_int16_To_int32		(int32* piDest,		int16* pi) 		{ *piDest = ((int32)*pi << 16) | *pi; }
void ConvertScaleTruncate_int8_To_int32		(int32* piDest,		int8* pi)		{ *piDest = ((int32)*pi << 24) | ((int32)*pi << 16) | ((int32)*pi << 8) | *pi; }
void ConvertScaleTruncate_uint64_To_int32		(int32* piDest,		uint64* pi)		{ *piDest = (int32)(*pi / 0x100000001LL); }
void ConvertScaleTruncate_uint32_To_int32		(int32* piDest,		uint32* pi)		{ *piDest = *pi; }
void ConvertScaleTruncate_uint16_To_int32	(int32* piDest,		uint16* pi) 	{ *piDest = ((int32)*pi << 16) | *pi; }
void ConvertScaleTruncate_uint8_To_int32		(int32* piDest,		uint8* pi)		{ *piDest = ((int32)*pi << 24) | ((int32)*pi << 16) | ((int32)*pi << 8) | *pi; }
void ConvertScaleTruncate_float64_To_uint32	(uint32* psDest,	float64* pf) 	{ *psDest = (uint32)((float64)0xffffffff * *pf); }
void ConvertScaleTruncate_float32_To_uint32	(uint32* piDest,	float32* pf) 	{ *piDest = (uint32)((float32)0xffffffff * *pf); }
void ConvertScaleTruncate_int64_To_uint32		(uint32* piDest, 	int64* pi)		{ *piDest = (uint32)(*pi / 0x100000001LL); }
void ConvertScaleTruncate_int32_To_uint32		(uint32* piDest,	int32* pi)		{ *piDest = *pi; }
void ConvertScaleTruncate_int16_To_uint32	(uint32* piDest,	int16* pi) 		{ *piDest = ((uint32)*pi << 16) | *pi; }
void ConvertScaleTruncate_int8_To_uint32		(uint32* piDest,	int8* pi)		{ *piDest = ((uint32)*pi << 24) | ((int32)*pi << 16) | ((int32)*pi << 8) | *pi; }
void ConvertScaleTruncate_uint64_To_uint32	(uint32* piDest, 	uint64* pi)		{ *piDest = (uint32)(*pi / 0x100000001LL); }
void ConvertScaleTruncate_uint32_To_uint32		(uint32* piDest,	uint32* pi)		{ *piDest = *pi; }
void ConvertScaleTruncate_uint16_To_uint32	(uint32* piDest,	uint16* pi)		{ *piDest = ((uint32)*pi << 16) | *pi; }
void ConvertScaleTruncate_uint8_To_uint32	(uint32* piDest,	uint8* pi)		{ *piDest = ((uint32)*pi << 24) | ((int32)*pi << 16) | ((int32)*pi << 8) | *pi; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'long' Conversions.
void ConvertScaleTruncate_float64_To_int64	(int64* piDest,  	float64* pf) 	{ *piDest = (int64)((float64)0xffffffffffffffffLL * *pf); }
void ConvertScaleTruncate_float32_To_int64	(int64* piDest,  	float32* pf) 	{ *piDest = (int64)((float32)0xffffffffffffffffLL * *pf); }
void ConvertScaleTruncate_int64_To_int64		(int64* piDest,  	int64* pi)		{ *piDest = *pi; }
void ConvertScaleTruncate_int32_To_int64		(int64* piDest,  	int32* pi)		{ *piDest = ((int64)*pi << 32) | *pi; }
void ConvertScaleTruncate_int16_To_int64	(int64* piDest,  	int16* pi) 		{ *piDest = ((int64)*pi << 48) | ((int64)*pi << 32) | ((int64)*pi << 16) | *pi; }
void ConvertScaleTruncate_int8_To_int64		(int64* piDest,  	int8* pi)		{ *piDest = ((int64)*pi << 56) | ((int64)*pi << 48) | ((int64)*pi << 40) | ((int64)*pi << 32) | ((int64)*pi << 24) | ((int64)*pi << 16) | ((int64)*pi << 8) | *pi; }
void ConvertScaleTruncate_uint64_To_int64	(int64* piDest,  	uint64* pi)		{ *piDest = *pi; }
void ConvertScaleTruncate_uint32_To_int64		(int64* piDest,  	uint32* pi)		{ *piDest = ((int64)*pi << 32) | *pi; }
void ConvertScaleTruncate_uint16_To_int64	(int64* piDest,  	uint16* pi) 	{ *piDest = ((int64)*pi << 48) | ((int64)*pi << 32) | ((int64)*pi << 16) | *pi; }
void ConvertScaleTruncate_uint8_To_int64	(int64* piDest,  	uint8* pi)		{ *piDest = ((int64)*pi << 56) | ((int64)*pi << 48) | ((int64)*pi << 40) | ((int64)*pi << 32) | ((int64)*pi << 24) | ((int64)*pi << 16) | ((int64)*pi << 8) | *pi; }
void ConvertScaleTruncate_float64_To_uint64	(uint64* psDest,	float64* pf) 	{ *psDest = (uint64)((float64)0xffffffffffffffffLL * *pf); }
void ConvertScaleTruncate_float32_To_uint64	(uint64* piDest,	float32* pf) 	{ *piDest = (uint64)((float32)0xffffffffffffffffLL * *pf); }
void ConvertScaleTruncate_int64_To_uint64	(uint64* piDest,	int64* pi)		{ *piDest = *pi; }
void ConvertScaleTruncate_int32_To_uint64		(uint64* piDest,	int32* pi)		{ *piDest = ((uint64)*pi << 32) | *pi; }
void ConvertScaleTruncate_int16_To_uint64	(uint64* piDest,	int16* pi) 		{ *piDest = ((uint64)*pi << 48) | ((uint64)*pi << 32) | ((uint64)*pi << 16) | *pi; }
void ConvertScaleTruncate_int8_To_uint64	(uint64* piDest,	int8* pi)		{ *piDest = ((uint64)*pi << 56) | ((uint64)*pi << 48) | ((uint64)*pi << 40) | ((uint64)*pi << 32) | ((uint64)*pi << 24) | ((uint64)*pi << 16) | ((uint64)*pi << 8) | *pi; }
void ConvertScaleTruncate_uint64_To_uint64	(uint64* piDest, 	uint64* pi)		{ *piDest = *pi; }
void ConvertScaleTruncate_uint32_To_uint64	(uint64* piDest,	uint32* pi)		{ *piDest = ((uint64)*pi << 32) | *pi; }
void ConvertScaleTruncate_uint16_To_uint64	(uint64* piDest,	uint16* pi)		{ *piDest = ((uint64)*pi << 48) | ((uint64)*pi << 32) | ((uint64)*pi << 16) | *pi; }
void ConvertScaleTruncate_uint8_To_uint64	(uint64* piDest,	uint8* pi)		{ *piDest = ((uint64)*pi << 56) | ((uint64)*pi << 48) | ((uint64)*pi << 40) | ((uint64)*pi << 32) | ((uint64)*pi << 24) | ((uint64)*pi << 16) | ((uint64)*pi << 8) | *pi; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'float32' Conversions.
void ConvertScaleTruncate_float64_To_float32	(float32* pfDest,	float64* pf) 	{ *pfDest = (float32)*pf; }
void ConvertScaleTruncate_float32_To_float32	(float32* pfDest,	float32* pf) 	{ *pfDest = *pf; }
void ConvertScaleTruncate_int64_To_float32	(float32* pfDest,	int64* pi)		{ *pfDest = (float32)*pi / 0xffffffffffffffffLL; }
void ConvertScaleTruncate_int32_To_float32		(float32* pfDest,	int32* pi)		{ *pfDest = (float32)*pi / 0xffffffff; }
void ConvertScaleTruncate_int16_To_float32	(float32* pfDest,	int16* pi) 		{ *pfDest = (float32)*pi / 0xffff; }
void ConvertScaleTruncate_int8_To_float32	(float32* pfDest,	int8* pi)		{ *pfDest = (float32)*pi / 0xff; }
void ConvertScaleTruncate_uint64_To_float32	(float32* pfDest,	uint64* pi)		{ *pfDest = (float32)*pi / 0xffffffffffffffffLL; }
void ConvertScaleTruncate_uint32_To_float32	(float32* pfDest,	uint32* pi)		{ *pfDest = (float32)*pi / 0xffffffff; }
void ConvertScaleTruncate_uint16_To_float32	(float32* pfDest,	uint16* pi) 	{ *pfDest = (float32)*pi / 0xffff; }
void ConvertScaleTruncate_uint8_To_float32	(float32* pfDest,	uint8* pi)		{ *pfDest = (float32)*pi / 0xff; }
void ConvertScaleTruncate_float64_To_float64	(float64* pfDest,	float64* pf) 	{ *pfDest = *pf; }
void ConvertScaleTruncate_float32_To_float64	(float64* pfDest,	float32* pf) 	{ *pfDest = *pf; }
void ConvertScaleTruncate_int64_To_float64	(float64* pfDest,	int64* pi)		{ *pfDest = (float64)*pi / 0xffffffffffffffffLL; }
void ConvertScaleTruncate_int32_To_float64	(float64* pfDest,	int32* pi)		{ *pfDest = (float64)*pi / 0xffffffff; }
void ConvertScaleTruncate_int16_To_float64	(float64* pfDest,	int16* pi) 		{ *pfDest = (float64)*pi / 0xffff; }
void ConvertScaleTruncate_int8_To_float64	(float64* pfDest,	int8* pi)		{ *pfDest = (float64)*pi / 0xff; }
void ConvertScaleTruncate_uint64_To_float64	(float64* pfDest,	uint64* pi)		{ *pfDest = (float64)*pi / 0xffffffffffffffffLL; }
void ConvertScaleTruncate_uint32_To_float64	(float64* pfDest,	uint32* pi)		{ *pfDest = (float64)*pi / 0xffffffff; }
void ConvertScaleTruncate_uint16_To_float64	(float64* pfDest,	uint16* pi)		{ *pfDest = (float64)*pi / 0xffff; }
void ConvertScaleTruncate_uint8_To_float64	(float64* pfDest,	uint8* pi)		{ *pfDest = (float64)*pi / 0xff; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Sixbits' Conversions.
void ConvertScaleTruncate_uint8_To_Sixbits	(uint8* psDest,  uint8* pi)		{ *psDest = *pi / (0xff/0x3f); }
void ConvertScaleTruncate_uint16_To_Sixbits(uint8* psDest,  uint16* pi)		{ *psDest = *pi / (0xffff/0x3f); }
void ConvertScaleTruncate_uint32_To_Sixbits	(uint8* psDest,  uint32* pi)		{ *psDest = *pi / (0xffffffff/0x3f); }
void ConvertScaleTruncate_float64_To_Sixbits(uint8* psDest,  float64* pf)	{ *psDest = (uint8)(*pf * (float64)0x3f); }
void ConvertScaleTruncate_float32_To_Sixbits	(uint8* psDest,  float32* pf)	{ *psDest = (uint8)(*pf * (float32)0x3f); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nickle' Conversions.
void ConvertScaleTruncate_uint8_To_Nickle	(uint8* psDest,  uint8* pi)		{ *psDest = *pi / (0xff/0x1f); }
void ConvertScaleTruncate_uint16_To_Nickle	(uint8* psDest,  uint16* pi)		{ *psDest = *pi / (0xffff/0x1f); }
void ConvertScaleTruncate_uint32_To_Nickle	(uint8* psDest,  uint32* pi)		{ *psDest = *pi / (0xffffffff/0x1f); }
void ConvertScaleTruncate_float64_To_Nickle	(uint8* psDest,  float64* pf)		{ *psDest = (uint8)(*pf * (float64)0x1f); }
void ConvertScaleTruncate_float32_To_Nickle	(uint8* psDest,  float32* pf)		{ *psDest = (uint8)(*pf * (float32)0x1f); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nybble' Conversions.
void ConvertScaleTruncate_uint8_To_Nybble	(uint8* psDest,  uint8* pi)		{ *psDest = *pi / (0xff/0xf); }
void ConvertScaleTruncate_uint16_To_Nybble	(uint8* psDest,  uint16* pi)		{ *psDest = *pi / (0xffff/0xf); }
void ConvertScaleTruncate_uint32_To_Nybble	(uint8* psDest,  uint32* pi)		{ *psDest = *pi / (0xffffffff/0xf); }
void ConvertScaleTruncate_float64_To_Nybble	(uint8* psDest,  float64* pf)		{ *psDest = (uint8)(*pf * (float64)0xf); }
void ConvertScaleTruncate_float32_To_Nybble	(uint8* psDest,  float32* pf)		{ *psDest = (uint8)(*pf * (float32)0xf); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Crumb' Conversions.
void ConvertScaleTruncate_uint8_To_Crumb	(uint8* psDest,  uint8* pi)		{ *psDest = *pi / (0xff/0x3); } 
void ConvertScaleTruncate_uint16_To_Crumb	(uint8* psDest,  uint16* pi)		{ *psDest = *pi / (0xffff/0x3); }
void ConvertScaleTruncate_uint32_To_Crumb	(uint8* psDest,  uint32* pi)		{ *psDest = *pi / (0xffffffff/0x3); }
void ConvertScaleTruncate_float64_To_Crumb	(uint8* psDest,  float64* pf)		{ *psDest = (uint8)(*pf * (float64)0x3); }
void ConvertScaleTruncate_float32_To_Crumb	(uint8* psDest,  float32* pf)		{ *psDest = (uint8)(*pf * (float32)0x3); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Bit' Conversions.
void ConvertScaleTruncate_uint8_To_Bit		(uint8* psDest,  uint8* pi)		{ *psDest = *pi / (0xff); }
void ConvertScaleTruncate_uint16_To_Bit	(uint8* psDest,  uint16* pi)		{ *psDest = *pi / (0xffff); }
void ConvertScaleTruncate_uint32_To_Bit		(uint8* psDest,  uint32* pi)		{ *psDest = *pi / (0xffffffff); }
void ConvertScaleTruncate_float64_To_Bit	(uint8* psDest,  float64* pf)		{ *psDest = (uint8)*pf; }
void ConvertScaleTruncate_float32_To_Bit		(uint8* psDest,  float32* pf)		{ *psDest = (uint8)*pf; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
//Other U.  Clean up later
void ConvertScaleTruncate_Sixbits_To_uint16	(uint16* pDest, uint8* pi)	{ *pDest = (uint16)((float32)*pi * (65535.0f/63.0f)); }
void ConvertScaleTruncate_Nickle_To_uint16		(uint16* pDest, uint8* pi)	{ *pDest = (uint16)((float32)*pi * (65535.0f/31.0f)); }
void ConvertScaleTruncate_Nybble_To_uint16		(uint16* pDest, uint8* pi)	{ *pDest = (uint16)((float32)*pi * (65535.0f/15.0f)); }
void ConvertScaleTruncate_ThreeBits_To_uint16	(uint16* pDest, uint8* pi)	{ *pDest = (uint16)((float32)*pi * (65535.0f/7.0f)); }
void ConvertScaleTruncate_Crumb_To_uint16		(uint16* pDest, uint8* pi)	{ *pDest = (uint16)*pi * (0xffff/3); }
void ConvertScaleTruncate_Bit_To_uint16		(uint16* pDest, uint8* pi)	{ *pDest = (uint16)*pi * 0xffff; }

void ConvertScaleTruncate_Sixbits_To_uint8		(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)((float32)*pi * (255.0f/63.0f)); }
void ConvertScaleTruncate_Nickle_To_uint8		(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)((float32)*pi * (255.0f/31.0f)); }
void ConvertScaleTruncate_Nybble_To_uint8		(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)((float32)*pi * (255.0f/15.0f)); }
void ConvertScaleTruncate_ThreeBits_To_uint8	(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)((float32)*pi * (255.0f/7.0f)); }
void ConvertScaleTruncate_Crumb_To_uint8		(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)*pi * (0xff/3); }
void ConvertScaleTruncate_Bit_To_uint8			(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)*pi * 0xff; }
									 
void ConvertScaleTruncate_Sixbits_To_uint32		(uint32* pDest, uint8* pi)	{ *pDest = (uint32)((float32)*pi * (4294967295.0f/63.0f)); }
void ConvertScaleTruncate_Nickle_To_uint32		(uint32* pDest, uint8* pi)	{ *pDest = (uint32)((float32)*pi * (4294967295.0f/31.0f)); }
void ConvertScaleTruncate_Nybble_To_uint32		(uint32* pDest, uint8* pi)	{ *pDest = (uint32)((float32)*pi * (4294967295.0f/15.0f)); }
void ConvertScaleTruncate_ThreeBits_To_uint32	(uint32* pDest, uint8* pi)	{ *pDest = (uint32)((float32)*pi * (4294967295.0f/7.0f)); }
void ConvertScaleTruncate_Crumb_To_uint32		(uint32* pDest, uint8* pi)	{ *pDest = (uint32)*pi * (0xffffffff/3); }
void ConvertScaleTruncate_Bit_To_uint32			(uint32* pDest, uint8* pi)	{ *pDest = (uint32)*pi * 0xffffffff; }
									 
void ConvertScaleTruncate_Sixbits_To_float32		(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/63.0f); }
void ConvertScaleTruncate_Nickle_To_float32		(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/31.0f); }
void ConvertScaleTruncate_Nybble_To_float32		(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/15.0f); }
void ConvertScaleTruncate_ThreeBits_To_float32	(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/7.0f); }
void ConvertScaleTruncate_Crumb_To_float32		(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/3.0f); }
void ConvertScaleTruncate_Bit_To_float32			(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/1.0f); }

void ConvertScaleTruncate_int32_To_Bit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int32_To_Crumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int32_To_Nybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int32_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int32_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int16_To_Bit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int16_To_Crumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int16_To_Nybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int16_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int16_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int8_To_Bit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int8_To_Crumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int8_To_Nybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int8_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int8_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_float32_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_float32_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Bit_To_int32(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Bit_To_int16(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Bit_To_int8(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Bit_To_Bit(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x01; }
void ConvertScaleTruncate_Bit_To_Crumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Bit_To_Nybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Bit_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Bit_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Crumb_To_int32(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Crumb_To_int16(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Crumb_To_int8(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Crumb_To_Bit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Crumb_To_Crumb(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x03; }
void ConvertScaleTruncate_Crumb_To_Nybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Crumb_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Crumb_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nybble_To_int32(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nybble_To_int16(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nybble_To_int8(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nybble_To_Bit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nybble_To_Crumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nybble_To_Nybble(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x0f; }
void ConvertScaleTruncate_Nybble_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nybble_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nickle_To_int32(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nickle_To_int16(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nickle_To_int8(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nickle_To_Bit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nickle_To_Crumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nickle_To_Nybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nickle_To_Nickle(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x1f; }
void ConvertScaleTruncate_Nickle_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_int32(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_int16(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_int8(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_Bit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_Crumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_Nybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_Sixbits(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x3f; }
void ConvertScaleTruncate_int64_To_Bit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int64_To_Crumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int64_To_Nybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int64_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_int64_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_float64_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_float64_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_uint64_To_Bit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_uint64_To_Crumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_uint64_To_Nybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_uint64_To_Nickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_uint64_To_Sixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Bit_To_int64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Bit_To_float64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Bit_To_uint64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Crumb_To_int64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Crumb_To_float64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Crumb_To_uint64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nybble_To_int64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nybble_To_float64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nybble_To_uint64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nickle_To_int64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nickle_To_float64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Nickle_To_uint64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_int64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_float64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncate_Sixbits_To_uint64(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }

void ConversionInvalid(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types (invalid)."); }

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeConverter::Init(void)
{
	int32		iDest;
	int32		iSource;

	for (iDest = 0; iDest < PRIMTIVE_CONVERTER_END; iDest++)
	{
		for (iSource = 0; iSource < PRIMTIVE_CONVERTER_END; iSource++)
		{
			mapvScaleTuncateConversion[iDest][iSource] = ConversionInvalid;
		}
	}

#include "TypeConverter_Mapping.inl"
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeConverter::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeConverter::Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eSource, void* pvSource)
{
	if ((eSource < PRIMTIVE_CONVERTER_END) && (eDest < PRIMTIVE_CONVERTER_END))
	{
		mapvScaleTuncateConversion[eDest][eSource](pvDest, pvSource);
		return;
	}
	gcUserError.Set("Conversion not defined for types (out of bounds)."); 
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TypeConverterInit(void)
{
	gcTypeConverter.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TypeConverterKill(void)
{
	gcTypeConverter.Kill();
}

