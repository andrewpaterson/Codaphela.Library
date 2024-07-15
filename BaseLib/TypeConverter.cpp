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

#define FLOAT_US(dest_type, source_type, min, max)	\
	if (*pf >= (source_type)0)						\
		if (*pf >= (source_type)1)					\
			* psDest = (dest_type)max;				\
		else										\
			* psDest = (dest_type)(max * *pf);		\
	else											\
		* psDest = (dest_type)min

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int8' Conversions.
void ConvertScaleTruncateDoubleToChar	(int8* psDest, 	float64* pf)	{ FLOAT_S(int8, float64, MIN_CHAR, MAX_CHAR); }
void ConvertScaleTruncateFloatToChar	(int8* psDest, 	float32* pf)	{ FLOAT_S(int8, float32, MIN_CHAR, MAX_CHAR); }
void ConvertScaleTruncateLongToChar		(int8* psDest, 	int64* pi)		{ *psDest = (int8)(*pi / 0x200000000000000LL); }
void ConvertScaleTruncateIntToChar		(int8* psDest, 	int32* pi)		{ *psDest = (int8)(*pi / 0x2000000); }
void ConvertScaleTruncateShortToChar	(int8* psDest, 	int16* pi) 		{ *psDest = (int8)(*pi / 0x200); }
void ConvertScaleTruncateCharToChar		(int8* psDest, 	int8* pi)		{ *psDest = *pi; }
void ConvertScaleTruncateULongToChar	(int8* psDest, 	uint64* pi)		{ *psDest = (int8)(*pi / 0x200000000000000LL); }
void ConvertScaleTruncateUIntToChar		(int8* psDest, 	uint32* pi)		{ *psDest = (int8)(*pi / 0x2000000); }
void ConvertScaleTruncateUShortToChar	(int8* psDest, 	uint16* pi) 	{ *psDest = (int8)(*pi / 0x200); }  // (MAX_USHORT+1) / (MAX_CHAR+1)
void ConvertScaleTruncateUCharToChar	(int8* psDest, 	uint8* pi)		{ *psDest = *pi > MAX_CHAR ? MAX_CHAR : *pi; }
void ConvertScaleTruncateDoubleToUChar	(uint8* psDest, float64* pf)	{ FLOAT_US(uint8, float64, MIN_UCHAR, MAX_UCHAR); }
void ConvertScaleTruncateFloatToUChar	(uint8* psDest,	float32* pf) 	{ FLOAT_US(uint8, float32, MIN_UCHAR, MAX_UCHAR); }
void ConvertScaleTruncateLongToUChar	(uint8* psDest,	int64* pi)		{ *psDest = (uint8)(*pi / 0x100000000000000LL); }
void ConvertScaleTruncateIntToUChar		(uint8* psDest,	int32* pi)		{ *psDest = (uint8)(*pi / 0x1000000); }
void ConvertScaleTruncateShortToUChar	(uint8* psDest,	int16* pi) 		{ *psDest = *pi >= 0 ? (uint8)(*pi / 0x80) : 0; }
void ConvertScaleTruncateCharToUChar	(uint8* psDest,	int8* pi)		{ *psDest = *pi; }
void ConvertScaleTruncateULongToUChar	(uint8* psDest,	uint64* pi)		{ *psDest = (uint8)(*pi / 0x101010101010101LL); }
void ConvertScaleTruncateUIntToUChar	(uint8* psDest,	uint32* pi)		{ *psDest = *pi / (0xffffffff/0xff); }
void ConvertScaleTruncateUShortToUChar	(uint8* psDest,	uint16* pi)		{ *psDest = *pi / 0x100; }
void ConvertScaleTruncateUCharToUChar	(uint8* psDest,	uint8* pi)		{ *psDest = *pi; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int16' Conversions.
void ConvertScaleTruncateDoubleToShort	(int16* psDest,		float64* pf) 	{ FLOAT_S(int16, float64, MIN_SHORT, MAX_SHORT); }
void ConvertScaleTruncateFloatToShort	(int16* psDest,		float32* pf) 	{ FLOAT_S(int16, float32, MIN_SHORT, MAX_SHORT); }
void ConvertScaleTruncateLongToShort	(int16* psDest,		int64* pi)		{ *psDest = (int16)(*pi / 0x1000100010001LL); }
void ConvertScaleTruncateIntToShort		(int16* psDest,		int32* pi)		{ *psDest = *pi / (0xffffffff/0xffff); }
void ConvertScaleTruncateShortToShort	(int16* psDest,		int16* pi) 		{ *psDest = *pi; }
void ConvertScaleTruncateCharToShort	(int16* psDest,		int8* pi)		{ *psDest = ((int16)*pi << 8) | *pi; }
void ConvertScaleTruncateULongToShort	(int16* psDest,		uint64* pi)		{ *psDest = (int16)(*pi / 0x1000100010001LL); }
void ConvertScaleTruncateUIntToShort	(int16* psDest,		uint32* pi)		{ *psDest = *pi / (0xffffffff/0xffff); }
void ConvertScaleTruncateUShortToShort	(int16* psDest,		uint16* pi) 	{ *psDest = *pi; }
void ConvertScaleTruncateUCharToShort	(int16* psDest,		uint8* pi)		{ *psDest = ((int16)*pi << 8) | *pi; }
void ConvertScaleTruncateDoubleToUShort	(uint16* psDest,	float64* pf) 	{ FLOAT_US(uint16, float64, MIN_USHORT, MAX_USHORT); }
void ConvertScaleTruncateFloatToUShort	(uint16* psDest,	float32* pf) 	{ FLOAT_US(uint16, float32, MIN_USHORT, MAX_USHORT); }
void ConvertScaleTruncateLongToUShort	(uint16* psDest,	int64* pi)		{ *psDest = (uint16)(*pi / 0x1000100010001LL); }
void ConvertScaleTruncateIntToUShort	(uint16* psDest,	int32* pi)		{ *psDest = *pi / (0xffffffff/0xffff); }
void ConvertScaleTruncateShortToUShort	(uint16* psDest,	int16* pi) 		{ *psDest = *pi; }
void ConvertScaleTruncateCharToUShort	(uint16* psDest,	int8* pi)		{ *psDest = ((uint16)*pi << 8) | *pi; }
void ConvertScaleTruncateULongToUShort	(uint16* psDest,	uint64* pi)		{ *psDest = (uint16)(*pi / 0x1000100010001LL); }
void ConvertScaleTruncateUIntToUShort	(uint16* psDest,	uint32* pi)		{ *psDest = *pi / (0xffffffff/0xffff); }
void ConvertScaleTruncateUShortToUShort	(uint16* psDest,	uint16* pi)		{ *psDest = *pi; }
void ConvertScaleTruncateUCharToUShort	(uint16* psDest,	uint8* pi)		{ *psDest = ((uint16)*pi << 8) | *pi; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int32' Conversions.
void ConvertScaleTruncateDoubleToInt	(int32* piDest,		float64* pf) 	{ *piDest = (int32)((float64)0xffffffff * *pf); }
void ConvertScaleTruncateFloatToInt		(int32* piDest,		float32* pf) 	{ *piDest = (int32)((float32)0xffffffff * *pf); }
void ConvertScaleTruncateLongToInt		(int32* piDest,		int64* pi)		{ *piDest = (int32)(*pi / 0x100000001LL); }
void ConvertScaleTruncateIntToInt		(int32* piDest,		int32* pi)		{ *piDest = *pi; }	   
void ConvertScaleTruncateShortToInt		(int32* piDest,		int16* pi) 		{ *piDest = ((int32)*pi << 16) | *pi; }
void ConvertScaleTruncateCharToInt		(int32* piDest,		int8* pi)		{ *piDest = ((int32)*pi << 24) | ((int32)*pi << 16) | ((int32)*pi << 8) | *pi; }
void ConvertScaleTruncateULongToInt		(int32* piDest,		uint64* pi)		{ *piDest = (int32)(*pi / 0x100000001LL); }
void ConvertScaleTruncateUIntToInt		(int32* piDest,		uint32* pi)		{ *piDest = *pi; }
void ConvertScaleTruncateUShortToInt	(int32* piDest,		uint16* pi) 	{ *piDest = ((int32)*pi << 16) | *pi; }
void ConvertScaleTruncateUCharToInt		(int32* piDest,		uint8* pi)		{ *piDest = ((int32)*pi << 24) | ((int32)*pi << 16) | ((int32)*pi << 8) | *pi; }
void ConvertScaleTruncateDoubleToUInt	(uint32* psDest,	float64* pf) 	{ *psDest = (uint32)((float64)0xffffffff * *pf); }
void ConvertScaleTruncateFloatToUInt	(uint32* piDest,	float32* pf) 	{ *piDest = (uint32)((float32)0xffffffff * *pf); }
void ConvertScaleTruncateLongToUInt		(uint32* piDest, 	int64* pi)		{ *piDest = (uint32)(*pi / 0x100000001LL); }
void ConvertScaleTruncateIntToUInt		(uint32* piDest,	int32* pi)		{ *piDest = *pi; }
void ConvertScaleTruncateShortToUInt	(uint32* piDest,	int16* pi) 		{ *piDest = ((uint32)*pi << 16) | *pi; }
void ConvertScaleTruncateCharToUInt		(uint32* piDest,	int8* pi)		{ *piDest = ((uint32)*pi << 24) | ((int32)*pi << 16) | ((int32)*pi << 8) | *pi; }
void ConvertScaleTruncateULongToUInt	(uint32* piDest, 	uint64* pi)		{ *piDest = (uint32)(*pi / 0x100000001LL); }
void ConvertScaleTruncateUIntToUInt		(uint32* piDest,	uint32* pi)		{ *piDest = *pi; }
void ConvertScaleTruncateUShortToUInt	(uint32* piDest,	uint16* pi)		{ *piDest = ((uint32)*pi << 16) | *pi; }
void ConvertScaleTruncateUCharToUInt	(uint32* piDest,	uint8* pi)		{ *piDest = ((uint32)*pi << 24) | ((int32)*pi << 16) | ((int32)*pi << 8) | *pi; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'long' Conversions.
void ConvertScaleTruncateDoubleToLong	(int64* piDest,  	float64* pf) 	{ *piDest = (int64)((float64)0xffffffffffffffffLL * *pf); }
void ConvertScaleTruncateFloatToLong	(int64* piDest,  	float32* pf) 	{ *piDest = (int64)((float32)0xffffffffffffffffLL * *pf); }
void ConvertScaleTruncateLongToLong		(int64* piDest,  	int64* pi)		{ *piDest = *pi; }
void ConvertScaleTruncateIntToLong		(int64* piDest,  	int32* pi)		{ *piDest = ((int64)*pi << 32) | *pi; }
void ConvertScaleTruncateShortToLong	(int64* piDest,  	int16* pi) 		{ *piDest = ((int64)*pi << 48) | ((int64)*pi << 32) | ((int64)*pi << 16) | *pi; }
void ConvertScaleTruncateCharToLong		(int64* piDest,  	int8* pi)		{ *piDest = ((int64)*pi << 56) | ((int64)*pi << 48) | ((int64)*pi << 40) | ((int64)*pi << 32) | ((int64)*pi << 24) | ((int64)*pi << 16) | ((int64)*pi << 8) | *pi; }
void ConvertScaleTruncateULongToLong	(int64* piDest,  	uint64* pi)		{ *piDest = *pi; }
void ConvertScaleTruncateUIntToLong		(int64* piDest,  	uint32* pi)		{ *piDest = ((int64)*pi << 32) | *pi; }
void ConvertScaleTruncateUShortToLong	(int64* piDest,  	uint16* pi) 	{ *piDest = ((int64)*pi << 48) | ((int64)*pi << 32) | ((int64)*pi << 16) | *pi; }
void ConvertScaleTruncateUCharToLong	(int64* piDest,  	uint8* pi)		{ *piDest = ((int64)*pi << 56) | ((int64)*pi << 48) | ((int64)*pi << 40) | ((int64)*pi << 32) | ((int64)*pi << 24) | ((int64)*pi << 16) | ((int64)*pi << 8) | *pi; }
void ConvertScaleTruncateDoubleToULong	(uint64* psDest,	float64* pf) 	{ *psDest = (uint64)((float64)0xffffffffffffffffLL * *pf); }
void ConvertScaleTruncateFloatToULong	(uint64* piDest,	float32* pf) 	{ *piDest = (uint64)((float32)0xffffffffffffffffLL * *pf); }
void ConvertScaleTruncateLongToULong	(uint64* piDest,	int64* pi)		{ *piDest = *pi; }
void ConvertScaleTruncateIntToULong		(uint64* piDest,	int32* pi)		{ *piDest = ((uint64)*pi << 32) | *pi; }
void ConvertScaleTruncateShortToULong	(uint64* piDest,	int16* pi) 		{ *piDest = ((uint64)*pi << 48) | ((uint64)*pi << 32) | ((uint64)*pi << 16) | *pi; }
void ConvertScaleTruncateCharToULong	(uint64* piDest,	int8* pi)		{ *piDest = ((uint64)*pi << 56) | ((uint64)*pi << 48) | ((uint64)*pi << 40) | ((uint64)*pi << 32) | ((uint64)*pi << 24) | ((uint64)*pi << 16) | ((uint64)*pi << 8) | *pi; }
void ConvertScaleTruncateULongToULong	(uint64* piDest, 	uint64* pi)		{ *piDest = *pi; }
void ConvertScaleTruncateUIntToULong	(uint64* piDest,	uint32* pi)		{ *piDest = ((uint64)*pi << 32) | *pi; }
void ConvertScaleTruncateUShortToULong	(uint64* piDest,	uint16* pi)		{ *piDest = ((uint64)*pi << 48) | ((uint64)*pi << 32) | ((uint64)*pi << 16) | *pi; }
void ConvertScaleTruncateUCharToULong	(uint64* piDest,	uint8* pi)		{ *piDest = ((uint64)*pi << 56) | ((uint64)*pi << 48) | ((uint64)*pi << 40) | ((uint64)*pi << 32) | ((uint64)*pi << 24) | ((uint64)*pi << 16) | ((uint64)*pi << 8) | *pi; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'float32' Conversions.
void ConvertScaleTruncateDoubleToFloat	(float32* pfDest,	float64* pf) 	{ *pfDest = (float32)*pf; }
void ConvertScaleTruncateFloatToFloat	(float32* pfDest,	float32* pf) 	{ *pfDest = *pf; }
void ConvertScaleTruncateLongToFloat	(float32* pfDest,	int64* pi)		{ *pfDest = (float32)*pi / 0xffffffffffffffffLL; }
void ConvertScaleTruncateIntToFloat		(float32* pfDest,	int32* pi)		{ *pfDest = (float32)*pi / 0xffffffff; }
void ConvertScaleTruncateShortToFloat	(float32* pfDest,	int16* pi) 		{ *pfDest = (float32)*pi / 0xffff; }
void ConvertScaleTruncateCharToFloat	(float32* pfDest,	int8* pi)		{ *pfDest = (float32)*pi / 0xff; }
void ConvertScaleTruncateULongToFloat	(float32* pfDest,	uint64* pi)		{ *pfDest = (float32)*pi / 0xffffffffffffffffLL; }
void ConvertScaleTruncateUIntToFloat	(float32* pfDest,	uint32* pi)		{ *pfDest = (float32)*pi / 0xffffffff; }
void ConvertScaleTruncateUShortToFloat	(float32* pfDest,	uint16* pi) 	{ *pfDest = (float32)*pi / 0xffff; }
void ConvertScaleTruncateUCharToFloat	(float32* pfDest,	uint8* pi)		{ *pfDest = (float32)*pi / 0xff; }
void ConvertScaleTruncateDoubleToDouble	(float64* pfDest,	float64* pf) 	{ *pfDest = *pf; }
void ConvertScaleTruncateFloatToDouble	(float64* pfDest,	float32* pf) 	{ *pfDest = *pf; }
void ConvertScaleTruncateLongToDouble	(float64* pfDest,	int64* pi)		{ *pfDest = (float64)*pi / 0xffffffffffffffffLL; }
void ConvertScaleTruncateIntToDouble	(float64* pfDest,	int32* pi)		{ *pfDest = (float64)*pi / 0xffffffff; }
void ConvertScaleTruncateShortToDouble	(float64* pfDest,	int16* pi) 		{ *pfDest = (float64)*pi / 0xffff; }
void ConvertScaleTruncateCharToDouble	(float64* pfDest,	int8* pi)		{ *pfDest = (float64)*pi / 0xff; }
void ConvertScaleTruncateULongToDouble	(float64* pfDest,	uint64* pi)		{ *pfDest = (float64)*pi / 0xffffffffffffffffLL; }
void ConvertScaleTruncateUIntToDouble	(float64* pfDest,	uint32* pi)		{ *pfDest = (float64)*pi / 0xffffffff; }
void ConvertScaleTruncateUShortToDouble	(float64* pfDest,	uint16* pi)		{ *pfDest = (float64)*pi / 0xffff; }
void ConvertScaleTruncateUCharToDouble	(float64* pfDest,	uint8* pi)		{ *pfDest = (float64)*pi / 0xff; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Sixbits' Conversions.
void ConvertScaleTruncateUCharToSixbits	(uint8* psDest,  uint8* pi)		{ *psDest = *pi / (0xff/0x3f); }
void ConvertScaleTruncateUShortToSixbits(uint8* psDest,  uint16* pi)		{ *psDest = *pi / (0xffff/0x3f); }
void ConvertScaleTruncateUIntToSixbits	(uint8* psDest,  uint32* pi)		{ *psDest = *pi / (0xffffffff/0x3f); }
void ConvertScaleTruncateDoubleToSixbits(uint8* psDest,  float64* pf)	{ *psDest = (uint8)(*pf * (float64)0x3f); }
void ConvertScaleTruncateFloatToSixbits	(uint8* psDest,  float32* pf)	{ *psDest = (uint8)(*pf * (float32)0x3f); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nickle' Conversions.
void ConvertScaleTruncateUCharToNickle	(uint8* psDest,  uint8* pi)		{ *psDest = *pi / (0xff/0x1f); }
void ConvertScaleTruncateUShortToNickle	(uint8* psDest,  uint16* pi)		{ *psDest = *pi / (0xffff/0x1f); }
void ConvertScaleTruncateUIntToNickle	(uint8* psDest,  uint32* pi)		{ *psDest = *pi / (0xffffffff/0x1f); }
void ConvertScaleTruncateDoubleToNickle	(uint8* psDest,  float64* pf)		{ *psDest = (uint8)(*pf * (float64)0x1f); }
void ConvertScaleTruncateFloatToNickle	(uint8* psDest,  float32* pf)		{ *psDest = (uint8)(*pf * (float32)0x1f); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nybble' Conversions.
void ConvertScaleTruncateUCharToNybble	(uint8* psDest,  uint8* pi)		{ *psDest = *pi / (0xff/0xf); }
void ConvertScaleTruncateUShortToNybble	(uint8* psDest,  uint16* pi)		{ *psDest = *pi / (0xffff/0xf); }
void ConvertScaleTruncateUIntToNybble	(uint8* psDest,  uint32* pi)		{ *psDest = *pi / (0xffffffff/0xf); }
void ConvertScaleTruncateDoubleToNybble	(uint8* psDest,  float64* pf)		{ *psDest = (uint8)(*pf * (float64)0xf); }
void ConvertScaleTruncateFloatToNybble	(uint8* psDest,  float32* pf)		{ *psDest = (uint8)(*pf * (float32)0xf); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Crumb' Conversions.
void ConvertScaleTruncateUCharToCrumb	(uint8* psDest,  uint8* pi)		{ *psDest = *pi / (0xff/0x3); } 
void ConvertScaleTruncateUShortToCrumb	(uint8* psDest,  uint16* pi)		{ *psDest = *pi / (0xffff/0x3); }
void ConvertScaleTruncateUIntToCrumb	(uint8* psDest,  uint32* pi)		{ *psDest = *pi / (0xffffffff/0x3); }
void ConvertScaleTruncateDoubleToCrumb	(uint8* psDest,  float64* pf)		{ *psDest = (uint8)(*pf * (float64)0x3); }
void ConvertScaleTruncateFloatToCrumb	(uint8* psDest,  float32* pf)		{ *psDest = (uint8)(*pf * (float32)0x3); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Bit' Conversions.
void ConvertScaleTruncateUCharToBit		(uint8* psDest,  uint8* pi)		{ *psDest = *pi / (0xff); }
void ConvertScaleTruncateUShortToBit	(uint8* psDest,  uint16* pi)		{ *psDest = *pi / (0xffff); }
void ConvertScaleTruncateUIntToBit		(uint8* psDest,  uint32* pi)		{ *psDest = *pi / (0xffffffff); }
void ConvertScaleTruncateDoubleToBit	(uint8* psDest,  float64* pf)		{ *psDest = (uint8)*pf; }
void ConvertScaleTruncateFloatToBit		(uint8* psDest,  float32* pf)		{ *psDest = (uint8)*pf; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
//Other U.  Clean up later
void ConvertScaleTruncateSixbitsToUShort	(uint16* pDest, uint8* pi)	{ *pDest = (uint16)((float32)*pi * (65535.0f/63.0f)); }
void ConvertScaleTruncateNickleToUShort		(uint16* pDest, uint8* pi)	{ *pDest = (uint16)((float32)*pi * (65535.0f/31.0f)); }
void ConvertScaleTruncateNybbleToUShort		(uint16* pDest, uint8* pi)	{ *pDest = (uint16)((float32)*pi * (65535.0f/15.0f)); }
void ConvertScaleTruncateThreeBitsToUShort	(uint16* pDest, uint8* pi)	{ *pDest = (uint16)((float32)*pi * (65535.0f/7.0f)); }
void ConvertScaleTruncateCrumbToUShort		(uint16* pDest, uint8* pi)	{ *pDest = (uint16)*pi * (0xffff/3); }
void ConvertScaleTruncateBitToUShort		(uint16* pDest, uint8* pi)	{ *pDest = (uint16)*pi * 0xffff; }

void ConvertScaleTruncateSixbitsToUChar		(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)((float32)*pi * (255.0f/63.0f)); }
void ConvertScaleTruncateNickleToUChar		(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)((float32)*pi * (255.0f/31.0f)); }
void ConvertScaleTruncateNybbleToUChar		(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)((float32)*pi * (255.0f/15.0f)); }
void ConvertScaleTruncateThreeBitsToUChar	(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)((float32)*pi * (255.0f/7.0f)); }
void ConvertScaleTruncateCrumbToUChar		(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)*pi * (0xff/3); }
void ConvertScaleTruncateBitToUChar			(uint8* pDest,  uint8* pi)	{ *pDest = (uint8)*pi * 0xff; }
									 
void ConvertScaleTruncateSixbitsToUInt		(uint32* pDest, uint8* pi)	{ *pDest = (uint32)((float32)*pi * (4294967295.0f/63.0f)); }
void ConvertScaleTruncateNickleToUInt		(uint32* pDest, uint8* pi)	{ *pDest = (uint32)((float32)*pi * (4294967295.0f/31.0f)); }
void ConvertScaleTruncateNybbleToUInt		(uint32* pDest, uint8* pi)	{ *pDest = (uint32)((float32)*pi * (4294967295.0f/15.0f)); }
void ConvertScaleTruncateThreeBitsToUInt	(uint32* pDest, uint8* pi)	{ *pDest = (uint32)((float32)*pi * (4294967295.0f/7.0f)); }
void ConvertScaleTruncateCrumbToUInt		(uint32* pDest, uint8* pi)	{ *pDest = (uint32)*pi * (0xffffffff/3); }
void ConvertScaleTruncateBitToUInt			(uint32* pDest, uint8* pi)	{ *pDest = (uint32)*pi * 0xffffffff; }
									 
void ConvertScaleTruncateSixbitsToFloat		(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/63.0f); }
void ConvertScaleTruncateNickleToFloat		(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/31.0f); }
void ConvertScaleTruncateNybbleToFloat		(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/15.0f); }
void ConvertScaleTruncateThreeBitsToFloat	(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/7.0f); }
void ConvertScaleTruncateCrumbToFloat		(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/3.0f); }
void ConvertScaleTruncateBitToFloat			(float32* pDest,  uint8* pi)	{ *pDest = (float32)*pi * (1.0f/1.0f); }

void ConvertScaleTruncateIntToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateIntToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateIntToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateIntToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateIntToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateShortToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateShortToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateShortToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateShortToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateShortToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCharToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCharToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCharToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCharToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCharToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateFloatToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateFloatToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateBitToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateBitToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateBitToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateBitToBit(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x01; }
void ConvertScaleTruncateBitToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateBitToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateBitToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateBitToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCrumbToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCrumbToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCrumbToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCrumbToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCrumbToCrumb(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x03; }
void ConvertScaleTruncateCrumbToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCrumbToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCrumbToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNybbleToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNybbleToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNybbleToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNybbleToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNybbleToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNybbleToNybble(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x0f; }
void ConvertScaleTruncateNybbleToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNybbleToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNickleToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNickleToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNickleToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNickleToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNickleToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNickleToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNickleToNickle(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x1f; }
void ConvertScaleTruncateNickleToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToSixbits(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x3f; }
void ConvertScaleTruncateLongToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateLongToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateLongToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateLongToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateLongToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateDoubleToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateDoubleToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateULongToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateULongToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateULongToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateULongToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateULongToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateBitToLong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateBitToDouble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateBitToULong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCrumbToLong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCrumbToDouble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateCrumbToULong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNybbleToLong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNybbleToDouble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNybbleToULong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNickleToLong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNickleToDouble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateNickleToULong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToLong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToDouble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertScaleTruncateSixbitsToULong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }

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

