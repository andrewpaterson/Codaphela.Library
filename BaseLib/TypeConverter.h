/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
// To 'char' Conversions.
void ConvertDoubleToChar	(char* pcDest, 			double* f);
void ConvertFloatToChar		(char* pcDest, 			float* f);
void ConvertLongToChar		(char* pcDest, 			int64* i);
void ConvertIntToChar		(char* pcDest, 			int* i);
void ConvertShortToChar		(char* pcDest, 			int16* s);
void ConvertCharToChar		(char* pcDest, 			char* c);
void ConvertULongToChar		(char* pcDest, 			uint64* i);
void ConvertUIntToChar		(char* pcDest, 			uint32* i);
void ConvertUShortToChar	(char* pcDest, 			uint16* s);
void ConvertUCharToChar		(char* pcDest, 			uint8* c);
void ConvertDoubleToUChar	(uint8* pcDest, double* f);
void ConvertFloatToUChar	(uint8* pcDest, float* f);
void ConvertLongToUChar		(uint8* pcDest, int64* i);
void ConvertIntToUChar		(uint8* pcDest, int* i);
void ConvertShortToUChar	(uint8* pcDest, int16* s);
void ConvertCharToUChar		(uint8* pcDest, char* c);
void ConvertULongToUChar	(uint8* pcDest, uint64* i);
void ConvertUIntToUChar		(uint8* pcDest, uint32* i);
void ConvertUShortToUChar	(uint8* pcDest, uint16* s);
void ConvertUCharToUChar	(uint8* pcDest, uint8* c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int16' Conversions.
void ConvertDoubleToShort	(int16* psDest,  			double* f);
void ConvertFloatToShort	(int16* psDest,  			float* f);
void ConvertLongToShort		(int16* psDest,  			int64* i);
void ConvertIntToShort		(int16* psDest,  			int* i);
void ConvertShortToShort	(int16* psDest,  			int16* s);
void ConvertCharToShort		(int16* psDest,  			char* c);
void ConvertULongToShort	(int16* psDest,  			uint64* i);
void ConvertUIntToShort		(int16* psDest,  			uint32* i);
void ConvertUShortToShort	(int16* psDest,  			uint16* s);
void ConvertUCharToShort	(int16* psDest,  			uint8* c);
void ConvertDoubleToUShort	(uint16* psDest,  	double* f);
void ConvertFloatToUShort	(uint16* psDest,  	float* f);
void ConvertLongToUShort	(uint16* psDest,  	int64* i);
void ConvertIntToUShort		(uint16* psDest,  	int* i);
void ConvertShortToUShort	(uint16* psDest,  	int16* s);
void ConvertCharToUShort	(uint16* psDest,  	char* c);
void ConvertULongToUShort	(uint16* psDest,  	uint64* i);
void ConvertUIntToUShort	(uint16* psDest,  	uint32* i);
void ConvertUShortToUShort	(uint16* psDest,  	uint16* s);
void ConvertUCharToUShort	(uint16* psDest,  	uint8* c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int' Conversions.
void ConvertDoubleToInt		(int* piDest,  			double* f);
void ConvertFloatToInt		(int* piDest,  			float* f);
void ConvertLongToInt		(int* piDest,  			int64* i);
void ConvertIntToInt		(int* piDest,  			int* i);
void ConvertShortToInt		(int* piDest,  			int16* s);
void ConvertCharToInt		(int* piDest,  			char* c);
void ConvertULongToInt		(int* piDest,  			uint64* i);
void ConvertUIntToInt		(int* piDest,  			uint32* i);
void ConvertUShortToInt		(int* piDest,  			uint16* s);
void ConvertUCharToInt		(int* piDest,  			uint8* c);
void ConvertDoubleToUInt	(uint32* psDest,  double* f);
void ConvertFloatToUInt		(uint32* piDest,  float* f);
void ConvertLongToUInt		(uint32* piDest, 	int64* i);
void ConvertIntToUInt		(uint32* piDest,  int* i);
void ConvertShortToUInt		(uint32* piDest,  int16* s);
void ConvertCharToUInt		(uint32* piDest,  char* c);
void ConvertULongToUInt		(uint32* piDest, 	uint64* i);
void ConvertUIntToUInt		(uint32* piDest,  uint32* i);
void ConvertUShortToUInt	(uint32* piDest,  uint16* s);
void ConvertUCharToUInt		(uint32* piDest,  uint8* c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'long' Conversions.
void ConvertDoubleToLong	(int64* piDest,  			double* f);
void ConvertFloatToLong		(int64* piDest,  			float* f);
void ConvertLongToLong		(int64* piDest,  			int64* i);
void ConvertIntToLong		(int64* piDest,  			int* i);
void ConvertShortToLong		(int64* piDest,  			int16* s);
void ConvertCharToLong		(int64* piDest,  			char* c);
void ConvertULongToLong		(int64* piDest,  			uint64* i);
void ConvertUIntToLong		(int64* piDest,  			uint32* i);
void ConvertUShortToLong	(int64* piDest,  			uint16* s);
void ConvertUCharToLong		(int64* piDest,  			uint8* c);
void ConvertDoubleToULong	(uint64* psDest,	double* f);
void ConvertFloatToULong	(uint64* piDest,	float* f);
void ConvertLongToULong		(uint64* piDest,	int64* i);
void ConvertIntToULong		(uint64* piDest,	int* i);
void ConvertShortToULong	(uint64* piDest,	int16* s);
void ConvertCharToULong		(uint64* piDest,	char* c);
void ConvertULongToULong	(uint64* piDest, 	uint64* i);
void ConvertUIntToULong		(uint64* piDest,	uint32* i);
void ConvertUShortToULong	(uint64* piDest,	uint16* s);
void ConvertUCharToULong	(uint64* piDest,	uint8* c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'float' Conversions.
void ConvertDoubleToFloat	(float* pfDest, double* f);
void ConvertFloatToFloat	(float* pfDest, float* f);
void ConvertLongToFloat		(float* pfDest, int64* i);
void ConvertIntToFloat		(float* pfDest, int* i);
void ConvertShortToFloat	(float* pfDest, int16* s);
void ConvertCharToFloat		(float* pfDest, char* c);
void ConvertULongToFloat	(float* pfDest, uint64* i);
void ConvertUIntToFloat		(float* pfDest, uint32* i);
void ConvertUShortToFloat	(float* pfDest, uint16* s);
void ConvertUCharToFloat	(float* pfDest, uint8* c);

void ConvertDoubleToDouble	(double* pfDest, double* f);
void ConvertFloatToDouble	(double* pfDest, float* f);
void ConvertLongToDouble	(double* pfDest, int64* i);
void ConvertIntToDouble		(double* pfDest, int* i);
void ConvertShortToDouble	(double* pfDest, int16* s);
void ConvertCharToDouble	(double* pfDest, char* c);
void ConvertULongToDouble	(double* pfDest, uint64* i);
void ConvertUIntToDouble	(double* pfDest, uint32* i);
void ConvertUShortToDouble	(double* pfDest, uint16* s);
void ConvertUCharToDouble	(double* pfDest, uint8* c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Sixbits' Conversions.
void ConvertUCharToSixbits	(uint8* pcDest,  uint8* c);		
void ConvertUShortToSixbits	(uint8* pcDest,  uint16* s);		
void ConvertUIntToSixbits	(uint8* pcDest,  uint32* i);		
void ConvertDoubleToSixbits	(uint8* pcDest,  double* f);				
void ConvertFloatToSixbits	(uint8* pcDest,  float* f);				


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nickle' Conversions.
void ConvertUCharToNickle	(uint8* pcDest,  uint8* c);		
void ConvertUShortToNickle	(uint8* pcDest,  uint16* s);		
void ConvertUIntToNickle	(uint8* pcDest,  uint32* i);		
void ConvertDoubleToNickle	(uint8* pcDest,  double* f);				
void ConvertFloatToNickle	(uint8* pcDest,  float* f);				


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nybble' Conversions.
void ConvertUCharToNybble	(uint8* pcDest,  uint8* c);
void ConvertUShortToNybble	(uint8* pcDest,  uint16* s);
void ConvertUIntToNybble	(uint8* pcDest,  uint32* i);
void ConvertDoubleToNybble	(uint8* pcDest,  double* f);
void ConvertFloatToNybble	(uint8* pcDest,  float* f);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Crumb' Conversions.
void ConvertUCharToCrumb	(uint8* pcDest,  uint8* c);
void ConvertUShortToCrumb	(uint8* pcDest,  uint16* s);
void ConvertUIntToCrumb		(uint8* pcDest,  uint32* i);
void ConvertDoubleToCrumb	(uint8* pcDest,  double* f);
void ConvertFloatToCrumb	(uint8* pcDest,  float* f);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Bit' Conversions.
void ConvertUCharToBit		(uint8* pcDest,  uint8* c);
void ConvertUShortToBit		(uint8* pcDest,  uint16* s);
void ConvertUIntToBit		(uint8* pcDest,  uint32* i);
void ConvertDoubleToBit		(uint8* pcDest,  double* f);
void ConvertFloatToBit		(uint8* pcDest,  float* f);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
//Other U.  Clean up later
void ConvertSixbitsToUShort		(uint16* pDest,  uint8* c);
void ConvertNickleToUShort		(uint16* pDest,  uint8* c);
void ConvertNybbleToUShort		(uint16* pDest,  uint8* c);
void ConvertThreeBitsToUShort	(uint16* pDest,  uint8* c);
void ConvertCrumbToUShort		(uint16* pDest,  uint8* c);
void ConvertBitToUShort			(uint16* pDest,  uint8* c);

void ConvertSixbitsToUChar	(uint8* pDest,  uint8* c);
void ConvertNickleToUChar	(uint8* pDest,  uint8* c);
void ConvertNybbleToUChar	(uint8* pDest,  uint8* c);
void ConvertThreeBitsToUChar(uint8* pDest,  uint8* c);
void ConvertCrumbToUChar	(uint8* pDest,  uint8* c);
void ConvertBitToUChar		(uint8* pDest,  uint8* c);

void ConvertSixbitsToUInt	(uint32* pDest,  	uint8* c);
void ConvertNickleToUInt	(uint32* pDest,  	uint8* c);
void ConvertNybbleToUInt	(uint32* pDest,  	uint8* c);
void ConvertThreeBitsToUInt	(uint32* pDest,  	uint8* c);
void ConvertCrumbToUInt		(uint32* pDest,  	uint8* c);
void ConvertBitToUInt		(uint32* pDest,  	uint8* c);

void ConvertSixbitsToFloat	(float* pDest,  		uint8* c);
void ConvertNickleToFloat	(float* pDest,  		uint8* c);
void ConvertNybbleToFloat	(float* pDest,  		uint8* c);
void ConvertThreeBitsToFloat(float* pDest,  		uint8* c);
void ConvertCrumbToFloat	(float* pDest,  		uint8* c);
void ConvertBitToFloat		(float* pDest,  		uint8* c);

void ConvertIntToBit(void* pvDest, void* pvSource);
void ConvertIntToCrumb(void* pvDest, void* pvSource);
void ConvertIntToNybble(void* pvDest, void* pvSource);
void ConvertIntToNickle(void* pvDest, void* pvSource);
void ConvertIntToSixbits(void* pvDest, void* pvSource);
void ConvertShortToBit(void* pvDest, void* pvSource);
void ConvertShortToCrumb(void* pvDest, void* pvSource);
void ConvertShortToNybble(void* pvDest, void* pvSource);
void ConvertShortToNickle(void* pvDest, void* pvSource);
void ConvertShortToSixbits(void* pvDest, void* pvSource);
void ConvertCharToBit(void* pvDest, void* pvSource);
void ConvertCharToCrumb(void* pvDest, void* pvSource);
void ConvertCharToNybble(void* pvDest, void* pvSource);
void ConvertBitToInt(void* pvDest, void* pvSource);
void ConvertBitToShort(void* pvDest, void* pvSource);
void ConvertBitToChar(void* pvDest, void* pvSource);
void ConvertBitToBit(void* pvDest, void* pvSource);
void ConvertBitToCrumb(void* pvDest, void* pvSource);
void ConvertBitToNybble(void* pvDest, void* pvSource);
void ConvertBitToNickle(void* pvDest, void* pvSource);
void ConvertBitToSixbits(void* pvDest, void* pvSource);
void ConvertCrumbToInt(void* pvDest, void* pvSource);
void ConvertCrumbToShort(void* pvDest, void* pvSource);
void ConvertCrumbToChar(void* pvDest, void* pvSource);
void ConvertCrumbToBit(void* pvDest, void* pvSource);
void ConvertCrumbToCrumb(void* pvDest, void* pvSource);
void ConvertCrumbToNybble(void* pvDest, void* pvSource);
void ConvertCrumbToNickle(void* pvDest, void* pvSource);
void ConvertCrumbToSixbits(void* pvDest, void* pvSource);
void ConvertNybbleToInt(void* pvDest, void* pvSource);
void ConvertNybbleToShort(void* pvDest, void* pvSource);
void ConvertNybbleToChar(void* pvDest, void* pvSource);
void ConvertNybbleToBit(void* pvDest, void* pvSource);
void ConvertNybbleToCrumb(void* pvDest, void* pvSource);
void ConvertNybbleToNybble(void* pvDest, void* pvSource);
void ConvertNybbleToNickle(void* pvDest, void* pvSource);
void ConvertNybbleToSixbits(void* pvDest, void* pvSource);
void ConvertNickleToInt(void* pvDest, void* pvSource);
void ConvertNickleToShort(void* pvDest, void* pvSource);
void ConvertNickleToChar(void* pvDest, void* pvSource);
void ConvertNickleToBit(void* pvDest, void* pvSource);
void ConvertNickleToCrumb(void* pvDest, void* pvSource);
void ConvertNickleToNybble(void* pvDest, void* pvSource);
void ConvertNickleToNickle(void* pvDest, void* pvSource);
void ConvertNickleToSixbits(void* pvDest, void* pvSource);
void ConvertSixbitsToInt(void* pvDest, void* pvSource);
void ConvertSixbitsToShort(void* pvDest, void* pvSource);
void ConvertSixbitsToChar(void* pvDest, void* pvSource);
void ConvertSixbitsToBit(void* pvDest, void* pvSource);
void ConvertSixbitsToCrumb(void* pvDest, void* pvSource);
void ConvertSixbitsToNybble(void* pvDest, void* pvSource);
void ConvertSixbitsToNickle(void* pvDest, void* pvSource);
void ConvertSixbitsToSixbits(void* pvDest, void* pvSource);
void ConvertLongToBit(void* pvDest, void* pvSource);
void ConvertLongToCrumb(void* pvDest, void* pvSource);
void ConvertLongToNybble(void* pvDest, void* pvSource);
void ConvertLongToNickle(void* pvDest, void* pvSource);
void ConvertLongToSixbits(void* pvDest, void* pvSource);
void ConvertDoubleToNickle(void* pvDest, void* pvSource);
void ConvertDoubleToSixbits(void* pvDest, void* pvSource);
void ConvertULongToBit(void* pvDest, void* pvSource);
void ConvertULongToCrumb(void* pvDest, void* pvSource);
void ConvertULongToNybble(void* pvDest, void* pvSource);
void ConvertULongToNickle(void* pvDest, void* pvSource);
void ConvertULongToSixbits(void* pvDest, void* pvSource);
void ConvertBitToLong(void* pvDest, void* pvSource);
void ConvertBitToDouble(void* pvDest, void* pvSource);
void ConvertBitToULong(void* pvDest, void* pvSource);
void ConvertCrumbToLong(void* pvDest, void* pvSource);
void ConvertCrumbToDouble(void* pvDest, void* pvSource);
void ConvertCrumbToULong(void* pvDest, void* pvSource);
void ConvertNybbleToLong(void* pvDest, void* pvSource);
void ConvertNybbleToDouble(void* pvDest, void* pvSource);
void ConvertNybbleToULong(void* pvDest, void* pvSource);
void ConvertNickleToLong(void* pvDest, void* pvSource);
void ConvertNickleToDouble(void* pvDest, void* pvSource);
void ConvertNickleToULong(void* pvDest, void* pvSource);
void ConvertSixbitsToLong(void* pvDest, void* pvSource);
void ConvertSixbitsToDouble(void* pvDest, void* pvSource);
void ConvertSixbitsToULong(void* pvDest, void* pvSource);


typedef void	(*ConversionFunc)		(void* pvDest, void* pvSource);


class CTypeConverter
{
	ConversionFunc	mapvConversion[PRIMTIVE_CONVERTER_END][PRIMTIVE_CONVERTER_END];  //Dest, source

public:
	void Init(void);
	void Kill(void);
	void Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eSource, void* pvSource);
};


extern CTypeConverter	gcTypeConverter;


void TypeConverterInit(void);
void TypeConverterKill(void);


#endif // !__TYPE_CONVERTER_H__

