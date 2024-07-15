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
// To 'int8' Conversions.
void ConvertScaleTruncateDoubleToChar	(int8* psDest, 	double* pf);
void ConvertScaleTruncateFloatToChar	(int8* psDest, 	float* pf);
void ConvertScaleTruncateLongToChar		(int8* psDest, 	int64* pi);
void ConvertScaleTruncateIntToChar		(int8* psDest, 	int32* pi);
void ConvertScaleTruncateShortToChar	(int8* psDest, 	int16* pi);
void ConvertScaleTruncateCharToChar		(int8* psDest, 	int8* pi);
void ConvertScaleTruncateULongToChar	(int8* psDest, 	uint64* pi);
void ConvertScaleTruncateUIntToChar		(int8* psDest, 	uint32* pi);
void ConvertScaleTruncateUShortToChar	(int8* psDest, 	uint16* pi);
void ConvertScaleTruncateUCharToChar	(int8* psDest, 	uint8* pi);
void ConvertScaleTruncateDoubleToUChar	(uint8* psDest,	double* pf);
void ConvertScaleTruncateFloatToUChar	(uint8* psDest,	float* pf);
void ConvertScaleTruncateLongToUChar	(uint8* psDest,	int64* pi);
void ConvertScaleTruncateIntToUChar		(uint8* psDest,	int32* pi);
void ConvertScaleTruncateShortToUChar	(uint8* psDest,	int16* pi);
void ConvertScaleTruncateCharToUChar	(uint8* psDest,	int8* pi);
void ConvertScaleTruncateULongToUChar	(uint8* psDest,	uint64* pi);
void ConvertScaleTruncateUIntToUChar	(uint8* psDest,	uint32* pi);
void ConvertScaleTruncateUShortToUChar	(uint8* psDest,	uint16* pi);
void ConvertScaleTruncateUCharToUChar	(uint8* psDest,	uint8* pi);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int16' Conversions.
void ConvertScaleTruncateDoubleToShort	(int16* psDest,  	double* pf);
void ConvertScaleTruncateFloatToShort	(int16* psDest,  	float* pf);
void ConvertScaleTruncateLongToShort	(int16* psDest,  	int64* pi);
void ConvertScaleTruncateIntToShort		(int16* psDest,  	int32* pi);
void ConvertScaleTruncateShortToShort	(int16* psDest,  	int16* pi);
void ConvertScaleTruncateCharToShort	(int16* psDest,  	int8* pi);
void ConvertScaleTruncateULongToShort	(int16* psDest,  	uint64* pi);
void ConvertScaleTruncateUIntToShort	(int16* psDest,  	uint32* pi);
void ConvertScaleTruncateUShortToShort	(int16* psDest,  	uint16* pi);
void ConvertScaleTruncateUCharToShort	(int16* psDest,  	uint8* pi);
void ConvertScaleTruncateDoubleToUShort	(uint16* psDest,  	double* pf);
void ConvertScaleTruncateFloatToUShort	(uint16* psDest,  	float* pf);
void ConvertScaleTruncateLongToUShort	(uint16* psDest,  	int64* pi);
void ConvertScaleTruncateIntToUShort	(uint16* psDest,  	int32* pi);
void ConvertScaleTruncateShortToUShort	(uint16* psDest,  	int16* pi);
void ConvertScaleTruncateCharToUShort	(uint16* psDest,  	int8* pi);
void ConvertScaleTruncateULongToUShort	(uint16* psDest,  	uint64* pi);
void ConvertScaleTruncateUIntToUShort	(uint16* psDest,  	uint32* pi);
void ConvertScaleTruncateUShortToUShort	(uint16* psDest,  	uint16* pi);
void ConvertScaleTruncateUCharToUShort	(uint16* psDest,  	uint8* pi);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int32' Conversions.
void ConvertScaleTruncateDoubleToInt	(int32* piDest,  	double* pf);
void ConvertScaleTruncateFloatToInt		(int32* piDest,  	float* pf);
void ConvertScaleTruncateLongToInt		(int32* piDest,  	int64* pi);
void ConvertScaleTruncateIntToInt		(int32* piDest,  	int32* pi);
void ConvertScaleTruncateShortToInt		(int32* piDest,  	int16* pi);
void ConvertScaleTruncateCharToInt		(int32* piDest,  	int8* pi);
void ConvertScaleTruncateULongToInt		(int32* piDest,  	uint64* pi);
void ConvertScaleTruncateUIntToInt		(int32* piDest,  	uint32* pi);
void ConvertScaleTruncateUShortToInt	(int32* piDest,  	uint16* pi);
void ConvertScaleTruncateUCharToInt		(int32* piDest,  	uint8* pi);
void ConvertScaleTruncateDoubleToUInt	(uint32* psDest,	double* pf);
void ConvertScaleTruncateFloatToUInt	(uint32* piDest,	float* pf);
void ConvertScaleTruncateLongToUInt		(uint32* piDest,	int64* pi);
void ConvertScaleTruncateIntToUInt		(uint32* piDest,	int32* pi);
void ConvertScaleTruncateShortToUInt	(uint32* piDest,	int16* pi);
void ConvertScaleTruncateCharToUInt		(uint32* piDest,	int8* pi);
void ConvertScaleTruncateULongToUInt	(uint32* piDest,	uint64* pi);
void ConvertScaleTruncateUIntToUInt		(uint32* piDest,	uint32* pi);
void ConvertScaleTruncateUShortToUInt	(uint32* piDest,	uint16* pi);
void ConvertScaleTruncateUCharToUInt	(uint32* piDest,	uint8* pi);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'long' Conversions.
void ConvertScaleTruncateDoubleToLong	(int64* piDest,  	double* pf);
void ConvertScaleTruncateFloatToLong	(int64* piDest,  	float* pf);
void ConvertScaleTruncateLongToLong		(int64* piDest,  	int64* pi);
void ConvertScaleTruncateIntToLong		(int64* piDest,  	int32* pi);
void ConvertScaleTruncateShortToLong	(int64* piDest,  	int16* pi);
void ConvertScaleTruncateCharToLong		(int64* piDest,  	int8* pi);
void ConvertScaleTruncateULongToLong	(int64* piDest,  	uint64* pi);
void ConvertScaleTruncateUIntToLong		(int64* piDest,  	uint32* pi);
void ConvertScaleTruncateUShortToLong	(int64* piDest,  	uint16* pi);
void ConvertScaleTruncateUCharToLong	(int64* piDest,  	uint8* pi);
void ConvertScaleTruncateDoubleToULong	(uint64* psDest,	double* pf);
void ConvertScaleTruncateFloatToULong	(uint64* piDest,	float* pf);
void ConvertScaleTruncateLongToULong	(uint64* piDest,	int64* pi);
void ConvertScaleTruncateIntToULong		(uint64* piDest,	int32* pi);
void ConvertScaleTruncateShortToULong	(uint64* piDest,	int16* pi);
void ConvertScaleTruncateCharToULong	(uint64* piDest,	int8* pi);
void ConvertScaleTruncateULongToULong	(uint64* piDest, 	uint64* pi);
void ConvertScaleTruncateUIntToULong	(uint64* piDest,	uint32* pi);
void ConvertScaleTruncateUShortToULong	(uint64* piDest,	uint16* pi);
void ConvertScaleTruncateUCharToULong	(uint64* piDest,	uint8* pi);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'float' Conversions.
void ConvertScaleTruncateDoubleToFloat	(float* pfDest, double* pf);
void ConvertScaleTruncateFloatToFloat	(float* pfDest, float* pf);
void ConvertScaleTruncateLongToFloat	(float* pfDest, int64* pi);
void ConvertScaleTruncateIntToFloat		(float* pfDest, int32* pi);
void ConvertScaleTruncateShortToFloat	(float* pfDest, int16* pi);
void ConvertScaleTruncateCharToFloat	(float* pfDest, int8* pi);
void ConvertScaleTruncateULongToFloat	(float* pfDest, uint64* pi);
void ConvertScaleTruncateUIntToFloat	(float* pfDest, uint32* pi);
void ConvertScaleTruncateUShortToFloat	(float* pfDest, uint16* pi);
void ConvertScaleTruncateUCharToFloat	(float* pfDest, uint8* pi);

void ConvertScaleTruncateDoubleToDouble	(double* pfDest, double* pf);
void ConvertScaleTruncateFloatToDouble	(double* pfDest, float* pf);
void ConvertScaleTruncateLongToDouble	(double* pfDest, int64* pi);
void ConvertScaleTruncateIntToDouble	(double* pfDest, int32* pi);
void ConvertScaleTruncateShortToDouble	(double* pfDest, int16* pi);
void ConvertScaleTruncateCharToDouble	(double* pfDest, int8* pi);
void ConvertScaleTruncateULongToDouble	(double* pfDest, uint64* pi);
void ConvertScaleTruncateUIntToDouble	(double* pfDest, uint32* pi);
void ConvertScaleTruncateUShortToDouble	(double* pfDest, uint16* pi);
void ConvertScaleTruncateUCharToDouble	(double* pfDest, uint8* pi);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Sixbits' Conversions.
void ConvertScaleTruncateUCharToSixbits	(uint8* psDest, uint8* pi);		
void ConvertScaleTruncateUShortToSixbits(uint8* psDest, uint16* pi);		
void ConvertScaleTruncateUIntToSixbits	(uint8* psDest, uint32* pi);		
void ConvertScaleTruncateDoubleToSixbits(uint8* psDest, double* pf);				
void ConvertScaleTruncateFloatToSixbits	(uint8* psDest, float* pf);				


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nickle' Conversions.
void ConvertScaleTruncateUCharToNickle	(uint8* psDest, uint8* pi);		
void ConvertScaleTruncateUShortToNickle	(uint8* psDest, uint16* pi);		
void ConvertScaleTruncateUIntToNickle	(uint8* psDest, uint32* pi);		
void ConvertScaleTruncateDoubleToNickle	(uint8* psDest, double* pf);				
void ConvertScaleTruncateFloatToNickle	(uint8* psDest, float* pf);				


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nybble' Conversions.
void ConvertScaleTruncateUCharToNybble	(uint8* psDest, uint8* pi);
void ConvertScaleTruncateUShortToNybble	(uint8* psDest, uint16* pi);
void ConvertScaleTruncateUIntToNybble	(uint8* psDest, uint32* pi);
void ConvertScaleTruncateDoubleToNybble	(uint8* psDest, double* pf);
void ConvertScaleTruncateFloatToNybble	(uint8* psDest, float* pf);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Crumb' Conversions.
void ConvertScaleTruncateUCharToCrumb	(uint8* psDest, uint8* pi);
void ConvertScaleTruncateUShortToCrumb	(uint8* psDest, uint16* pi);
void ConvertScaleTruncateUIntToCrumb	(uint8* psDest, uint32* pi);
void ConvertScaleTruncateDoubleToCrumb	(uint8* psDest, double* pf);
void ConvertScaleTruncateFloatToCrumb	(uint8* psDest, float* pf);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Bit' Conversions.
void ConvertScaleTruncateUCharToBit	(uint8* psDest, uint8* pi);
void ConvertScaleTruncateUShortToBit(uint8* psDest, uint16* pi);
void ConvertScaleTruncateUIntToBit	(uint8* psDest, uint32* pi);
void ConvertScaleTruncateDoubleToBit(uint8* psDest, double* pf);
void ConvertScaleTruncateFloatToBit	(uint8* psDest, float* pf);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
//Other U.  Clean up later
void ConvertScaleTruncateSixbitsToUShort	(uint16* pDest,  uint8* pi);
void ConvertScaleTruncateNickleToUShort		(uint16* pDest,  uint8* pi);
void ConvertScaleTruncateNybbleToUShort		(uint16* pDest,  uint8* pi);
void ConvertScaleTruncateThreeBitsToUShort	(uint16* pDest,  uint8* pi);
void ConvertScaleTruncateCrumbToUShort		(uint16* pDest,  uint8* pi);
void ConvertScaleTruncateBitToUShort		(uint16* pDest,  uint8* pi);

void ConvertScaleTruncateSixbitsToUChar		(uint8* pDest,  uint8* pi);
void ConvertScaleTruncateNickleToUChar		(uint8* pDest,  uint8* pi);
void ConvertScaleTruncateNybbleToUChar		(uint8* pDest,  uint8* pi);
void ConvertScaleTruncateThreeBitsToUChar	(uint8* pDest,  uint8* pi);
void ConvertScaleTruncateCrumbToUChar		(uint8* pDest,  uint8* pi);
void ConvertScaleTruncateBitToUChar			(uint8* pDest,  uint8* pi);

void ConvertScaleTruncateSixbitsToUInt		(uint32* pDest, uint8* pi);
void ConvertScaleTruncateNickleToUInt		(uint32* pDest, uint8* pi);
void ConvertScaleTruncateNybbleToUInt		(uint32* pDest, uint8* pi);
void ConvertScaleTruncateThreeBitsToUInt	(uint32* pDest, uint8* pi);
void ConvertScaleTruncateCrumbToUInt		(uint32* pDest, uint8* pi);
void ConvertScaleTruncateBitToUInt			(uint32* pDest, uint8* pi);

void ConvertScaleTruncateSixbitsToFloat		(float* pDest,	uint8* pi);
void ConvertScaleTruncateNickleToFloat		(float* pDest,	uint8* pi);
void ConvertScaleTruncateNybbleToFloat		(float* pDest,	uint8* pi);
void ConvertScaleTruncateThreeBitsToFloat	(float* pDest,	uint8* pi);
void ConvertScaleTruncateCrumbToFloat		(float* pDest,	uint8* pi);
void ConvertScaleTruncateBitToFloat			(float* pDest,	uint8* pi);

void ConvertScaleTruncateIntToBit(void* pvDest, void* pvSource);
void ConvertScaleTruncateIntToCrumb(void* pvDest, void* pvSource);
void ConvertScaleTruncateIntToNybble(void* pvDest, void* pvSource);
void ConvertScaleTruncateIntToNickle(void* pvDest, void* pvSource);
void ConvertScaleTruncateIntToSixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncateShortToBit(void* pvDest, void* pvSource);
void ConvertScaleTruncateShortToCrumb(void* pvDest, void* pvSource);
void ConvertScaleTruncateShortToNybble(void* pvDest, void* pvSource);
void ConvertScaleTruncateShortToNickle(void* pvDest, void* pvSource);
void ConvertScaleTruncateShortToSixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncateCharToBit(void* pvDest, void* pvSource);
void ConvertScaleTruncateCharToCrumb(void* pvDest, void* pvSource);
void ConvertScaleTruncateCharToNybble(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToInt(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToShort(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToChar(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToBit(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToCrumb(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToNybble(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToNickle(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToSixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToInt(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToShort(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToChar(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToBit(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToCrumb(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToNybble(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToNickle(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToSixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToInt(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToShort(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToChar(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToBit(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToCrumb(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToNybble(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToNickle(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToSixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToInt(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToShort(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToChar(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToBit(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToCrumb(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToNybble(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToNickle(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToSixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToInt(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToShort(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToChar(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToBit(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToCrumb(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToNybble(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToNickle(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToSixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncateLongToBit(void* pvDest, void* pvSource);
void ConvertScaleTruncateLongToCrumb(void* pvDest, void* pvSource);
void ConvertScaleTruncateLongToNybble(void* pvDest, void* pvSource);
void ConvertScaleTruncateLongToNickle(void* pvDest, void* pvSource);
void ConvertScaleTruncateLongToSixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncateDoubleToNickle(void* pvDest, void* pvSource);
void ConvertScaleTruncateDoubleToSixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncateULongToBit(void* pvDest, void* pvSource);
void ConvertScaleTruncateULongToCrumb(void* pvDest, void* pvSource);
void ConvertScaleTruncateULongToNybble(void* pvDest, void* pvSource);
void ConvertScaleTruncateULongToNickle(void* pvDest, void* pvSource);
void ConvertScaleTruncateULongToSixbits(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToLong(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToDouble(void* pvDest, void* pvSource);
void ConvertScaleTruncateBitToULong(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToLong(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToDouble(void* pvDest, void* pvSource);
void ConvertScaleTruncateCrumbToULong(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToLong(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToDouble(void* pvDest, void* pvSource);
void ConvertScaleTruncateNybbleToULong(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToLong(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToDouble(void* pvDest, void* pvSource);
void ConvertScaleTruncateNickleToULong(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToLong(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToDouble(void* pvDest, void* pvSource);
void ConvertScaleTruncateSixbitsToULong(void* pvDest, void* pvSource);


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

