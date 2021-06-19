/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

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
void ConvertLongToChar		(char* pcDest, 			long long* i);
void ConvertIntToChar		(char* pcDest, 			int* i);
void ConvertShortToChar		(char* pcDest, 			int16* s);
void ConvertCharToChar		(char* pcDest, 			char* c);
void ConvertULongToChar		(char* pcDest, 			unsigned long long* i);
void ConvertUIntToChar		(char* pcDest, 			unsigned int* i);
void ConvertUShortToChar	(char* pcDest, 			uint16* s);
void ConvertUCharToChar		(char* pcDest, 			unsigned char* c);
void ConvertDoubleToUChar	(unsigned char* pcDest, double* f);
void ConvertFloatToUChar	(unsigned char* pcDest, float* f);
void ConvertLongToUChar		(unsigned char* pcDest, long long* i);
void ConvertIntToUChar		(unsigned char* pcDest, int* i);
void ConvertShortToUChar	(unsigned char* pcDest, int16* s);
void ConvertCharToUChar		(unsigned char* pcDest, char* c);
void ConvertULongToUChar	(unsigned char* pcDest, unsigned long long* i);
void ConvertUIntToUChar		(unsigned char* pcDest, unsigned int* i);
void ConvertUShortToUChar	(unsigned char* pcDest, uint16* s);
void ConvertUCharToUChar	(unsigned char* pcDest, unsigned char* c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int16' Conversions.
void ConvertDoubleToShort	(int16* psDest,  			double* f);
void ConvertFloatToShort	(int16* psDest,  			float* f);
void ConvertLongToShort		(int16* psDest,  			long long* i);
void ConvertIntToShort		(int16* psDest,  			int* i);
void ConvertShortToShort	(int16* psDest,  			int16* s);
void ConvertCharToShort		(int16* psDest,  			char* c);
void ConvertULongToShort	(int16* psDest,  			unsigned long long* i);
void ConvertUIntToShort		(int16* psDest,  			unsigned int* i);
void ConvertUShortToShort	(int16* psDest,  			uint16* s);
void ConvertUCharToShort	(int16* psDest,  			unsigned char* c);
void ConvertDoubleToUShort	(uint16* psDest,  	double* f);
void ConvertFloatToUShort	(uint16* psDest,  	float* f);
void ConvertLongToUShort	(uint16* psDest,  	long long* i);
void ConvertIntToUShort		(uint16* psDest,  	int* i);
void ConvertShortToUShort	(uint16* psDest,  	int16* s);
void ConvertCharToUShort	(uint16* psDest,  	char* c);
void ConvertULongToUShort	(uint16* psDest,  	unsigned long long* i);
void ConvertUIntToUShort	(uint16* psDest,  	unsigned int* i);
void ConvertUShortToUShort	(uint16* psDest,  	uint16* s);
void ConvertUCharToUShort	(uint16* psDest,  	unsigned char* c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int' Conversions.
void ConvertDoubleToInt		(int* piDest,  			double* f);
void ConvertFloatToInt		(int* piDest,  			float* f);
void ConvertLongToInt		(int* piDest,  			long long* i);
void ConvertIntToInt		(int* piDest,  			int* i);
void ConvertShortToInt		(int* piDest,  			int16* s);
void ConvertCharToInt		(int* piDest,  			char* c);
void ConvertULongToInt		(int* piDest,  			unsigned long long* i);
void ConvertUIntToInt		(int* piDest,  			unsigned int* i);
void ConvertUShortToInt		(int* piDest,  			uint16* s);
void ConvertUCharToInt		(int* piDest,  			unsigned char* c);
void ConvertDoubleToUInt	(unsigned int* psDest,  double* f);
void ConvertFloatToUInt		(unsigned int* piDest,  float* f);
void ConvertLongToUInt		(unsigned int* piDest, 	long long* i);
void ConvertIntToUInt		(unsigned int* piDest,  int* i);
void ConvertShortToUInt		(unsigned int* piDest,  int16* s);
void ConvertCharToUInt		(unsigned int* piDest,  char* c);
void ConvertULongToUInt		(unsigned int* piDest, 	unsigned long long* i);
void ConvertUIntToUInt		(unsigned int* piDest,  unsigned int* i);
void ConvertUShortToUInt	(unsigned int* piDest,  uint16* s);
void ConvertUCharToUInt		(unsigned int* piDest,  unsigned char* c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'long' Conversions.
void ConvertDoubleToLong	(long long* piDest,  			double* f);
void ConvertFloatToLong		(long long* piDest,  			float* f);
void ConvertLongToLong		(long long* piDest,  			long long* i);
void ConvertIntToLong		(long long* piDest,  			int* i);
void ConvertShortToLong		(long long* piDest,  			int16* s);
void ConvertCharToLong		(long long* piDest,  			char* c);
void ConvertULongToLong		(long long* piDest,  			unsigned long long* i);
void ConvertUIntToLong		(long long* piDest,  			unsigned int* i);
void ConvertUShortToLong	(long long* piDest,  			uint16* s);
void ConvertUCharToLong		(long long* piDest,  			unsigned char* c);
void ConvertDoubleToULong	(unsigned long long* psDest,	double* f);
void ConvertFloatToULong	(unsigned long long* piDest,	float* f);
void ConvertLongToULong		(unsigned long long* piDest,	long long* i);
void ConvertIntToULong		(unsigned long long* piDest,	int* i);
void ConvertShortToULong	(unsigned long long* piDest,	int16* s);
void ConvertCharToULong		(unsigned long long* piDest,	char* c);
void ConvertULongToULong	(unsigned long long* piDest, 	unsigned long long* i);
void ConvertUIntToULong		(unsigned long long* piDest,	unsigned int* i);
void ConvertUShortToULong	(unsigned long long* piDest,	uint16* s);
void ConvertUCharToULong	(unsigned long long* piDest,	unsigned char* c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'float' Conversions.
void ConvertDoubleToFloat	(float* pfDest, double* f);
void ConvertFloatToFloat	(float* pfDest, float* f);
void ConvertLongToFloat		(float* pfDest, long long* i);
void ConvertIntToFloat		(float* pfDest, int* i);
void ConvertShortToFloat	(float* pfDest, int16* s);
void ConvertCharToFloat		(float* pfDest, char* c);
void ConvertULongToFloat	(float* pfDest, unsigned long long* i);
void ConvertUIntToFloat		(float* pfDest, unsigned int* i);
void ConvertUShortToFloat	(float* pfDest, uint16* s);
void ConvertUCharToFloat	(float* pfDest, unsigned char* c);

void ConvertDoubleToDouble	(double* pfDest, double* f);
void ConvertFloatToDouble	(double* pfDest, float* f);
void ConvertLongToDouble	(double* pfDest, long long* i);
void ConvertIntToDouble		(double* pfDest, int* i);
void ConvertShortToDouble	(double* pfDest, int16* s);
void ConvertCharToDouble	(double* pfDest, char* c);
void ConvertULongToDouble	(double* pfDest, unsigned long long* i);
void ConvertUIntToDouble	(double* pfDest, unsigned int* i);
void ConvertUShortToDouble	(double* pfDest, uint16* s);
void ConvertUCharToDouble	(double* pfDest, unsigned char* c);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Sixbits' Conversions.
void ConvertUCharToSixbits	(unsigned char* pcDest,  unsigned char* c);		
void ConvertUShortToSixbits	(unsigned char* pcDest,  uint16* s);		
void ConvertUIntToSixbits	(unsigned char* pcDest,  unsigned int* i);		
void ConvertDoubleToSixbits	(unsigned char* pcDest,  double* f);				
void ConvertFloatToSixbits	(unsigned char* pcDest,  float* f);				


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nickle' Conversions.
void ConvertUCharToNickle	(unsigned char* pcDest,  unsigned char* c);		
void ConvertUShortToNickle	(unsigned char* pcDest,  uint16* s);		
void ConvertUIntToNickle	(unsigned char* pcDest,  unsigned int* i);		
void ConvertDoubleToNickle	(unsigned char* pcDest,  double* f);				
void ConvertFloatToNickle	(unsigned char* pcDest,  float* f);				


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nybble' Conversions.
void ConvertUCharToNybble	(unsigned char* pcDest,  unsigned char* c);
void ConvertUShortToNybble	(unsigned char* pcDest,  uint16* s);
void ConvertUIntToNybble	(unsigned char* pcDest,  unsigned int* i);
void ConvertDoubleToNybble	(unsigned char* pcDest,  double* f);
void ConvertFloatToNybble	(unsigned char* pcDest,  float* f);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Crumb' Conversions.
void ConvertUCharToCrumb	(unsigned char* pcDest,  unsigned char* c);
void ConvertUShortToCrumb	(unsigned char* pcDest,  uint16* s);
void ConvertUIntToCrumb		(unsigned char* pcDest,  unsigned int* i);
void ConvertDoubleToCrumb	(unsigned char* pcDest,  double* f);
void ConvertFloatToCrumb	(unsigned char* pcDest,  float* f);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Bit' Conversions.
void ConvertUCharToBit		(unsigned char* pcDest,  unsigned char* c);
void ConvertUShortToBit		(unsigned char* pcDest,  uint16* s);
void ConvertUIntToBit		(unsigned char* pcDest,  unsigned int* i);
void ConvertDoubleToBit		(unsigned char* pcDest,  double* f);
void ConvertFloatToBit		(unsigned char* pcDest,  float* f);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
//Other U.  Clean up later
void ConvertSixbitsToUShort		(uint16* pDest,  unsigned char* c);
void ConvertNickleToUShort		(uint16* pDest,  unsigned char* c);
void ConvertNybbleToUShort		(uint16* pDest,  unsigned char* c);
void ConvertThreeBitsToUShort	(uint16* pDest,  unsigned char* c);
void ConvertCrumbToUShort		(uint16* pDest,  unsigned char* c);
void ConvertBitToUShort			(uint16* pDest,  unsigned char* c);

void ConvertSixbitsToUChar	(unsigned char* pDest,  unsigned char* c);
void ConvertNickleToUChar	(unsigned char* pDest,  unsigned char* c);
void ConvertNybbleToUChar	(unsigned char* pDest,  unsigned char* c);
void ConvertThreeBitsToUChar(unsigned char* pDest,  unsigned char* c);
void ConvertCrumbToUChar	(unsigned char* pDest,  unsigned char* c);
void ConvertBitToUChar		(unsigned char* pDest,  unsigned char* c);

void ConvertSixbitsToUInt	(unsigned int* pDest,  	unsigned char* c);
void ConvertNickleToUInt	(unsigned int* pDest,  	unsigned char* c);
void ConvertNybbleToUInt	(unsigned int* pDest,  	unsigned char* c);
void ConvertThreeBitsToUInt	(unsigned int* pDest,  	unsigned char* c);
void ConvertCrumbToUInt		(unsigned int* pDest,  	unsigned char* c);
void ConvertBitToUInt		(unsigned int* pDest,  	unsigned char* c);

void ConvertSixbitsToFloat	(float* pDest,  		unsigned char* c);
void ConvertNickleToFloat	(float* pDest,  		unsigned char* c);
void ConvertNybbleToFloat	(float* pDest,  		unsigned char* c);
void ConvertThreeBitsToFloat(float* pDest,  		unsigned char* c);
void ConvertCrumbToFloat	(float* pDest,  		unsigned char* c);
void ConvertBitToFloat		(float* pDest,  		unsigned char* c);

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

