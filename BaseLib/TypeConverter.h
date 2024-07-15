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
#ifndef __TYPE_CONVERTER_H__
#define __TYPE_CONVERTER_H__
#include "DataTypes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// _To_ 'int8' Conversions.
void ConvertScaleTruncate_float64_To_int8	(int8* psDest, 	float64* pf);
void ConvertScaleTruncate_float32_To_int8	(int8* psDest, 	float32* pf);
void ConvertScaleTruncate_int64_To_int8	(int8* psDest, 	int64* pi);
void ConvertScaleTruncate_int32_To_int8	(int8* psDest, 	int32* pi);
void ConvertScaleTruncate_int16_To_int8	(int8* psDest, 	int16* pi);
void ConvertScaleTruncate_int8_To_int8		(int8* psDest, 	int8* pi);
void ConvertScaleTruncate_uint64_To_int8	(int8* psDest, 	uint64* pi);
void ConvertScaleTruncate_uint32_To_int8	(int8* psDest, 	uint32* pi);
void ConvertScaleTruncate_uint16_To_int8	(int8* psDest, 	uint16* pi);
void ConvertScaleTruncate_uint8_To_int8	(int8* psDest, 	uint8* pi);
void ConvertScaleTruncate_float64_To_uint8	(uint8* psDest,	float64* pf);
void ConvertScaleTruncate_float32_To_uint8	(uint8* psDest,	float32* pf);
void ConvertScaleTruncate_int64_To_uint8	(uint8* psDest,	int64* pi);
void ConvertScaleTruncate_int32_To_uint8	(uint8* psDest,	int32* pi);
void ConvertScaleTruncate_int16_To_uint8	(uint8* psDest,	int16* pi);
void ConvertScaleTruncate_int8_To_uint8	(uint8* psDest,	int8* pi);
void ConvertScaleTruncate_uint64_To_uint8	(uint8* psDest,	uint64* pi);
void ConvertScaleTruncate_uint32_To_uint8	(uint8* psDest,	uint32* pi);
void ConvertScaleTruncate_uint16_To_uint8	(uint8* psDest,	uint16* pi);
void ConvertScaleTruncate_uint8_To_uint8	(uint8* psDest,	uint8* pi);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// _To_ 'int16' Conversions.
void ConvertScaleTruncate_float64_To_int16	(int16* psDest,  	float64* pf);
void ConvertScaleTruncate_float32_To_int16	(int16* psDest,  	float32* pf);
void ConvertScaleTruncate_int64_To_int16	(int16* psDest,  	int64* pi);
void ConvertScaleTruncate_int32_To_int16	(int16* psDest,  	int32* pi);
void ConvertScaleTruncate_int16_To_int16	(int16* psDest,  	int16* pi);
void ConvertScaleTruncate_int8_To_int16	(int16* psDest,  	int8* pi);
void ConvertScaleTruncate_uint64_To_int16	(int16* psDest,  	uint64* pi);
void ConvertScaleTruncate_uint32_To_int16	(int16* psDest,  	uint32* pi);
void ConvertScaleTruncate_uint16_To_int16	(int16* psDest,  	uint16* pi);
void ConvertScaleTruncate_uint8_To_int16	(int16* psDest,  	uint8* pi);
void ConvertScaleTruncate_float64_To_uint16	(uint16* psDest,  	float64* pf);
void ConvertScaleTruncate_float32_To_uint16	(uint16* psDest,  	float32* pf);
void ConvertScaleTruncate_int64_To_uint16	(uint16* psDest,  	int64* pi);
void ConvertScaleTruncate_int32_To_uint16	(uint16* psDest,  	int32* pi);
void ConvertScaleTruncate_int16_To_uint16	(uint16* psDest,  	int16* pi);
void ConvertScaleTruncate_int8_To_uint16	(uint16* psDest,  	int8* pi);
void ConvertScaleTruncate_uint64_To_uint16	(uint16* psDest,  	uint64* pi);
void ConvertScaleTruncate_uint32_To_uint16	(uint16* psDest,  	uint32* pi);
void ConvertScaleTruncate_uint16_To_uint16	(uint16* psDest,  	uint16* pi);
void ConvertScaleTruncate_uint8_To_uint16	(uint16* psDest,  	uint8* pi);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// _To_ 'int32' Conversions.
void ConvertScaleTruncate_float64_To_int32	(int32* piDest,  	float64* pf);
void ConvertScaleTruncate_float32_To_int32	(int32* piDest,  	float32* pf);
void ConvertScaleTruncate_int64_To_int32	(int32* piDest,  	int64* pi);
void ConvertScaleTruncate_int32_To_int32	(int32* piDest,  	int32* pi);
void ConvertScaleTruncate_int16_To_int32	(int32* piDest,  	int16* pi);
void ConvertScaleTruncate_int8_To_int32	(int32* piDest,  	int8* pi);
void ConvertScaleTruncate_uint64_To_int32	(int32* piDest,  	uint64* pi);
void ConvertScaleTruncate_uint32_To_int32	(int32* piDest,  	uint32* pi);
void ConvertScaleTruncate_uint16_To_int32	(int32* piDest,  	uint16* pi);
void ConvertScaleTruncate_uint8_To_int32	(int32* piDest,  	uint8* pi);
void ConvertScaleTruncate_float64_To_uint32	(uint32* psDest,	float64* pf);
void ConvertScaleTruncate_float32_To_uint32	(uint32* piDest,	float32* pf);
void ConvertScaleTruncate_int64_To_uint32	(uint32* piDest,	int64* pi);
void ConvertScaleTruncate_int32_To_uint32	(uint32* piDest,	int32* pi);
void ConvertScaleTruncate_int16_To_uint32	(uint32* piDest,	int16* pi);
void ConvertScaleTruncate_int8_To_uint32	(uint32* piDest,	int8* pi);
void ConvertScaleTruncate_uint64_To_uint32	(uint32* piDest,	uint64* pi);
void ConvertScaleTruncate_uint32_To_uint32	(uint32* piDest,	uint32* pi);
void ConvertScaleTruncate_uint16_To_uint32	(uint32* piDest,	uint16* pi);
void ConvertScaleTruncate_uint8_To_uint32	(uint32* piDest,	uint8* pi);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// _To_ 'long' Conversions.
void ConvertScaleTruncate_float64_To_int64	(int64* piDest,  	float64* pf);
void ConvertScaleTruncate_float32_To_int64	(int64* piDest,  	float32* pf);
void ConvertScaleTruncate_int64_To_int64	(int64* piDest,  	int64* pi);
void ConvertScaleTruncate_int32_To_int64	(int64* piDest,  	int32* pi);
void ConvertScaleTruncate_int16_To_int64	(int64* piDest,  	int16* pi);
void ConvertScaleTruncate_int8_To_int64	(int64* piDest,  	int8* pi);
void ConvertScaleTruncate_uint64_To_int64	(int64* piDest,  	uint64* pi);
void ConvertScaleTruncate_uint32_To_int64	(int64* piDest,  	uint32* pi);
void ConvertScaleTruncate_uint16_To_int64	(int64* piDest,  	uint16* pi);
void ConvertScaleTruncate_uint8_To_int64	(int64* piDest,  	uint8* pi);
void ConvertScaleTruncate_float64_To_uint64	(uint64* psDest,	float64* pf);
void ConvertScaleTruncate_float32_To_uint64	(uint64* piDest,	float32* pf);
void ConvertScaleTruncate_int64_To_uint64	(uint64* piDest,	int64* pi);
void ConvertScaleTruncate_int32_To_uint64	(uint64* piDest,	int32* pi);
void ConvertScaleTruncate_int16_To_uint64	(uint64* piDest,	int16* pi);
void ConvertScaleTruncate_int8_To_uint64	(uint64* piDest,	int8* pi);
void ConvertScaleTruncate_uint64_To_uint64	(uint64* piDest, 	uint64* pi);
void ConvertScaleTruncate_uint32_To_uint64	(uint64* piDest,	uint32* pi);
void ConvertScaleTruncate_uint16_To_uint64	(uint64* piDest,	uint16* pi);
void ConvertScaleTruncate_uint8_To_uint64	(uint64* piDest,	uint8* pi);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// _To_ 'float32' Conversions.
void ConvertScaleTruncate_float64_To_float32	(float32* pfDest, float64* pf);
void ConvertScaleTruncate_float32_To_float32	(float32* pfDest, float32* pf);
void ConvertScaleTruncate_int64_To_float32	(float32* pfDest, int64* pi);
void ConvertScaleTruncate_int32_To_float32	(float32* pfDest, int32* pi);
void ConvertScaleTruncate_int16_To_float32	(float32* pfDest, int16* pi);
void ConvertScaleTruncate_int8_To_float32	(float32* pfDest, int8* pi);
void ConvertScaleTruncate_uint64_To_float32	(float32* pfDest, uint64* pi);
void ConvertScaleTruncate_uint32_To_float32	(float32* pfDest, uint32* pi);
void ConvertScaleTruncate_uint16_To_float32	(float32* pfDest, uint16* pi);
void ConvertScaleTruncate_uint8_To_float32	(float32* pfDest, uint8* pi);

void ConvertScaleTruncate_float64_To_float64	(float64* pfDest, float64* pf);
void ConvertScaleTruncate_float32_To_float64	(float64* pfDest, float32* pf);
void ConvertScaleTruncate_int64_To_float64	(float64* pfDest, int64* pi);
void ConvertScaleTruncate_int32_To_float64	(float64* pfDest, int32* pi);
void ConvertScaleTruncate_int16_To_float64	(float64* pfDest, int16* pi);
void ConvertScaleTruncate_int8_To_float64	(float64* pfDest, int8* pi);
void ConvertScaleTruncate_uint64_To_float64	(float64* pfDest, uint64* pi);
void ConvertScaleTruncate_uint32_To_float64	(float64* pfDest, uint32* pi);
void ConvertScaleTruncate_uint16_To_float64	(float64* pfDest, uint16* pi);
void ConvertScaleTruncate_uint8_To_float64	(float64* pfDest, uint8* pi);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// _To_ 'Sixbits' Conversions.
void ConvertScaleTruncate_uint8_To_Sixbits	(uint8* psDest, uint8* pi);		
void ConvertScaleTruncate_uint16_To_Sixbits(uint8* psDest, uint16* pi);		
void ConvertScaleTruncate_uint32_To_Sixbits(uint8* psDest, uint32* pi);		
void ConvertScaleTruncate_float64_To_Sixbits(uint8* psDest, float64* pf);				
void ConvertScaleTruncate_float32_To_Sixbits	(uint8* psDest, float32* pf);				


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// _To_ 'Nickle' Conversions.
void ConvertScaleTruncate_uint8_To_Nickle	(uint8* psDest, uint8* pi);		
void ConvertScaleTruncate_uint16_To_Nickle	(uint8* psDest, uint16* pi);		
void ConvertScaleTruncate_uint32_To_Nickle	(uint8* psDest, uint32* pi);		
void ConvertScaleTruncate_float64_To_Nickle	(uint8* psDest, float64* pf);				
void ConvertScaleTruncate_float32_To_Nickle	(uint8* psDest, float32* pf);				


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// _To_ 'Nybble' Conversions.
void ConvertScaleTruncate_uint8_To_Nybble	(uint8* psDest, uint8* pi);
void ConvertScaleTruncate_uint16_To_Nybble	(uint8* psDest, uint16* pi);
void ConvertScaleTruncate_uint32_To_Nybble	(uint8* psDest, uint32* pi);
void ConvertScaleTruncate_float64_To_Nybble	(uint8* psDest, float64* pf);
void ConvertScaleTruncate_float32_To_Nybble	(uint8* psDest, float32* pf);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// _To_ 'Crumb' Conversions.
void ConvertScaleTruncate_uint8_To_Crumb	(uint8* psDest, uint8* pi);
void ConvertScaleTruncate_uint16_To_Crumb	(uint8* psDest, uint16* pi);
void ConvertScaleTruncate_uint32_To_Crumb	(uint8* psDest, uint32* pi);
void ConvertScaleTruncate_float64_To_Crumb	(uint8* psDest, float64* pf);
void ConvertScaleTruncate_float32_To_Crumb	(uint8* psDest, float32* pf);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// _To_ 'Bit' Conversions.
void ConvertScaleTruncate_uint8_To_Bit	(uint8* psDest, uint8* pi);
void ConvertScaleTruncate_uint16_To_Bit(uint8* psDest, uint16* pi);
void ConvertScaleTruncate_uint32_To_Bit(uint8* psDest, uint32* pi);
void ConvertScaleTruncate_float64_To_Bit(uint8* psDest, float64* pf);
void ConvertScaleTruncate_float32_To_Bit	(uint8* psDest, float32* pf);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
//Other U.  Clean up later
void ConvertScaleTruncate_Sixbits_To_uint16	(uint16* pDest,  uint8* pi);
void ConvertScaleTruncate_Nickle_To_uint16		(uint16* pDest,  uint8* pi);
void ConvertScaleTruncate_Nybble_To_uint16		(uint16* pDest,  uint8* pi);
void ConvertScaleTruncate_ThreeBits_To_uint16	(uint16* pDest,  uint8* pi);
void ConvertScaleTruncate_Crumb_To_uint16		(uint16* pDest,  uint8* pi);
void ConvertScaleTruncate_Bit_To_uint16		(uint16* pDest,  uint8* pi);

void ConvertScaleTruncate_Sixbits_To_uint8		(uint8* pDest,  uint8* pi);
void ConvertScaleTruncate_Nickle_To_uint8		(uint8* pDest,  uint8* pi);
void ConvertScaleTruncate_Nybble_To_uint8		(uint8* pDest,  uint8* pi);
void ConvertScaleTruncate_ThreeBits_To_uint8	(uint8* pDest,  uint8* pi);
void ConvertScaleTruncate_Crumb_To_uint8		(uint8* pDest,  uint8* pi);
void ConvertScaleTruncate_Bit_To_uint8			(uint8* pDest,  uint8* pi);

void ConvertScaleTruncate_Sixbits_To_uint32	(uint32* pDest, uint8* pi);
void ConvertScaleTruncate_Nickle_To_uint32		(uint32* pDest, uint8* pi);
void ConvertScaleTruncate_Nybble_To_uint32		(uint32* pDest, uint8* pi);
void ConvertScaleTruncate_ThreeBits_To_uint32	(uint32* pDest, uint8* pi);
void ConvertScaleTruncate_Crumb_To_uint32		(uint32* pDest, uint8* pi);
void ConvertScaleTruncate_Bit_To_uint32		(uint32* pDest, uint8* pi);

void ConvertScaleTruncate_Sixbits_To_float32		(float32* pDest,	uint8* pi);
void ConvertScaleTruncate_Nickle_To_float32		(float32* pDest,	uint8* pi);
void ConvertScaleTruncate_Nybble_To_float32		(float32* pDest,	uint8* pi);
void ConvertScaleTruncate_ThreeBits_To_float32	(float32* pDest,	uint8* pi);
void ConvertScaleTruncate_Crumb_To_float32		(float32* pDest,	uint8* pi);
void ConvertScaleTruncate_Bit_To_float32			(float32* pDest,	uint8* pi);

void ConvertScaleTruncate_int32_To_Bit(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int32_To_Crumb(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int32_To_Nybble(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int32_To_Nickle(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int32_To_Sixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int16_To_Bit(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int16_To_Crumb(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int16_To_Nybble(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int16_To_Nickle(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int16_To_Sixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int8_To_Bit(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int8_To_Crumb(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int8_To_Nybble(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_int32(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_int16(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_int8(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_Bit(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_Crumb(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_Nybble(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_Nickle(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_Sixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_int32(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_int16(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_int8(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_Bit(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_Crumb(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_Nybble(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_Nickle(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_Sixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_int32(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_int16(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_int8(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_Bit(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_Crumb(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_Nybble(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_Nickle(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_Sixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_int32(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_int16(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_int8(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_Bit(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_Crumb(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_Nybble(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_Nickle(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_Sixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_int32(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_int16(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_int8(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_Bit(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_Crumb(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_Nybble(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_Nickle(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_Sixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int64_To_Bit(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int64_To_Crumb(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int64_To_Nybble(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int64_To_Nickle(void* pvDest, void* pvSource);
void ConvertScaleTruncate_int64_To_Sixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncate_float64_To_Nickle(void* pvDest, void* pvSource);
void ConvertScaleTruncate_float64_To_Sixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncate_uint64_To_Bit(void* pvDest, void* pvSource);
void ConvertScaleTruncate_uint64_To_Crumb(void* pvDest, void* pvSource);
void ConvertScaleTruncate_uint64_To_Nybble(void* pvDest, void* pvSource);
void ConvertScaleTruncate_uint64_To_Nickle(void* pvDest, void* pvSource);
void ConvertScaleTruncate_uint64_To_Sixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_int64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_float64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Bit_To_uint64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_int64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_float64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Crumb_To_uint64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_int64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_float64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nybble_To_uint64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_int64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_float64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Nickle_To_uint64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_int64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_float64(void* pvDest, void* pvSource);
void ConvertScaleTruncate_Sixbits_To_uint64(void* pvDest, void* pvSource);


typedef void	(*ConversionFunc)		(void* pvDest, void* pvSource);


class CTypeConverter
{
	ConversionFunc	mapvScaleTuncateConversion[PRIMTIVE_CONVERTER_END][PRIMTIVE_CONVERTER_END];  //Dest, source

public:
	void Init(void);
	void Kill(void);
	void Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eSource, void* pvSource);
};


extern CTypeConverter	gcTypeConverter;


void TypeConverterInit(void);
void TypeConverterKill(void);


#endif // __TYPE_CONVERTER_H__

