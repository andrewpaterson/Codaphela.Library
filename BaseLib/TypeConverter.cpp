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
#include "TypeConverter.h"


CTypeConverter gcTypeConverter;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'char' Conversions.
void ConvertDoubleToChar	(char* pcDest, 			double* f) 				{ *pcDest = (char)((double)0xff * *f); }
void ConvertFloatToChar		(char* pcDest, 			float* f) 				{ *pcDest = (char)((float)0xff * *f); }
void ConvertLongToChar		(char* pcDest, 			int64* i)			{ *pcDest = (char)(*i / 0x101010101010101LL); }
void ConvertIntToChar		(char* pcDest, 			int* i)					{ *pcDest = *i / (0xffffffff/0xff); }
void ConvertShortToChar		(char* pcDest, 			int16* s) 				{ *pcDest = *s / (0xffff/0xff); }
void ConvertCharToChar		(char* pcDest, 			char* c)				{ *pcDest = *c; }
void ConvertULongToChar		(char* pcDest, 			uint64* i)	{ *pcDest = (char)(*i / 0x101010101010101LL); }
void ConvertUIntToChar		(char* pcDest, 			uint32* i)		{ *pcDest = *i / (0xffffffff/0xff); }
void ConvertUShortToChar	(char* pcDest, 			uint16* s) 		{ *pcDest = *s / (0xffff/0xff); }
void ConvertUCharToChar		(char* pcDest, 			uint8* c)		{ *pcDest = *c; }
void ConvertDoubleToUChar	(uint8* pcDest, double* f) 				{ *pcDest = (uint8)((double)0xff * *f); }
void ConvertFloatToUChar	(uint8* pcDest, float* f) 				{ *pcDest = (uint8)((float)0xff * *f); }
void ConvertLongToUChar		(uint8* pcDest, int64* i)			{ *pcDest = (uint8)(*i / 0x101010101010101LL); }
void ConvertIntToUChar		(uint8* pcDest, int* i)					{ *pcDest = *i / (0xffffffff/0xff); }
void ConvertShortToUChar	(uint8* pcDest, int16* s) 				{ *pcDest = *s / (0xffff/0xff); }
void ConvertCharToUChar		(uint8* pcDest, char* c)				{ *pcDest = *c; }
void ConvertULongToUChar	(uint8* pcDest, uint64* i)	{ *pcDest = (uint8)(*i / 0x101010101010101LL); }
void ConvertUIntToUChar		(uint8* pcDest, uint32* i)		{ *pcDest = *i / (0xffffffff/0xff); }
void ConvertUShortToUChar	(uint8* pcDest, uint16* s)		{ *pcDest = *s / (0xffff/0xff); }
void ConvertUCharToUChar	(uint8* pcDest, uint8* c)		{ *pcDest = *c; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int16' Conversions.
void ConvertDoubleToShort	(int16* psDest,  			double* f) 				{ *psDest = (int16)((double)0xffff * *f); }
void ConvertFloatToShort	(int16* psDest,  			float* f) 				{ *psDest = (int16)((float)0xffff * *f); }
void ConvertLongToShort		(int16* psDest,  			int64* i)			{ *psDest = (int16)(*i / 0x1000100010001LL); }
void ConvertIntToShort		(int16* psDest,  			int* i)					{ *psDest = *i / (0xffffffff/0xffff); }
void ConvertShortToShort	(int16* psDest,  			int16* s) 				{ *psDest = *s; }
void ConvertCharToShort		(int16* psDest,  			char* c)				{ *psDest = ((int16)*c << 8) | *c; }
void ConvertULongToShort	(int16* psDest,  			uint64* i)	{ *psDest = (int16)(*i / 0x1000100010001LL); }
void ConvertUIntToShort		(int16* psDest,  			uint32* i)		{ *psDest = *i / (0xffffffff/0xffff); }
void ConvertUShortToShort	(int16* psDest,  			uint16* s) 		{ *psDest = *s; }
void ConvertUCharToShort	(int16* psDest,  			uint8* c)		{ *psDest = ((int16)*c << 8) | *c; }
void ConvertDoubleToUShort	(uint16* psDest,  	double* f) 				{ *psDest = (uint16)((double)0xffff * *f); }
void ConvertFloatToUShort	(uint16* psDest,  	float* f) 				{ *psDest = (uint16)((float)0xffff * *f); }
void ConvertLongToUShort	(uint16* psDest,  	int64* i)			{ *psDest = (uint16)(*i / 0x1000100010001LL); }
void ConvertIntToUShort		(uint16* psDest,  	int* i)					{ *psDest = *i / (0xffffffff/0xffff); }
void ConvertShortToUShort	(uint16* psDest,  	int16* s) 				{ *psDest = *s; }
void ConvertCharToUShort	(uint16* psDest,  	char* c)				{ *psDest = ((uint16)*c << 8) | *c; }
void ConvertULongToUShort	(uint16* psDest,  	uint64* i)	{ *psDest = (uint16)(*i / 0x1000100010001LL); }
void ConvertUIntToUShort	(uint16* psDest,  	uint32* i)		{ *psDest = *i / (0xffffffff/0xffff); }
void ConvertUShortToUShort	(uint16* psDest,  	uint16* s)		{ *psDest = *s; }
void ConvertUCharToUShort	(uint16* psDest,  	uint8* c)		{ *psDest = ((uint16)*c << 8) | *c; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int' Conversions.
void ConvertDoubleToInt		(int* piDest,  			double* f) 				{ *piDest = (int)((double)0xffffffff * *f); }
void ConvertFloatToInt		(int* piDest,  			float* f) 				{ *piDest = (int)((float)0xffffffff * *f); }
void ConvertLongToInt		(int* piDest,  			int64* i)			{ *piDest = (int)(*i / 0x100000001LL); }
void ConvertIntToInt		(int* piDest,  			int* i)					{ *piDest = *i; }	   
void ConvertShortToInt		(int* piDest,  			int16* s) 				{ *piDest = ((int)*s << 16) | *s; }
void ConvertCharToInt		(int* piDest,  			char* c)				{ *piDest = ((int)*c << 24) | ((int)*c << 16) | ((int)*c << 8) | *c; }
void ConvertULongToInt		(int* piDest,  			uint64* i)	{ *piDest = (int)(*i / 0x100000001LL); }
void ConvertUIntToInt		(int* piDest,  			uint32* i)		{ *piDest = *i; }
void ConvertUShortToInt		(int* piDest,  			uint16* s) 		{ *piDest = ((int)*s << 16) | *s; }
void ConvertUCharToInt		(int* piDest,  			uint8* c)		{ *piDest = ((int)*c << 24) | ((int)*c << 16) | ((int)*c << 8) | *c; }
void ConvertDoubleToUInt	(uint32* psDest,  double* f) 				{ *psDest = (uint32)((double)0xffffffff * *f); }
void ConvertFloatToUInt		(uint32* piDest,  float* f) 				{ *piDest = (uint32)((float)0xffffffff * *f); }
void ConvertLongToUInt		(uint32* piDest, 	int64* i)			{ *piDest = (uint32)(*i / 0x100000001LL); }
void ConvertIntToUInt		(uint32* piDest,  int* i)					{ *piDest = *i; }
void ConvertShortToUInt		(uint32* piDest,  int16* s) 				{ *piDest = ((uint32)*s << 16) | *s; }
void ConvertCharToUInt		(uint32* piDest,  char* c)				{ *piDest = ((uint32)*c << 24) | ((int)*c << 16) | ((int)*c << 8) | *c; }
void ConvertULongToUInt		(uint32* piDest, 	uint64* i)	{ *piDest = (uint32)(*i / 0x100000001LL); }
void ConvertUIntToUInt		(uint32* piDest,  uint32* i)		{ *piDest = *i; }
void ConvertUShortToUInt	(uint32* piDest,  uint16* s)		{ *piDest = ((uint32)*s << 16) | *s; }
void ConvertUCharToUInt		(uint32* piDest,  uint8* c)		{ *piDest = ((uint32)*c << 24) | ((int)*c << 16) | ((int)*c << 8) | *c; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'long' Conversions.
void ConvertDoubleToLong	(int64* piDest,  			double* f) 				{ *piDest = (int64)((double)0xffffffffffffffffLL * *f); }
void ConvertFloatToLong		(int64* piDest,  			float* f) 				{ *piDest = (int64)((float)0xffffffffffffffffLL * *f); }
void ConvertLongToLong		(int64* piDest,  			int64* i)			{ *piDest = *i; }
void ConvertIntToLong		(int64* piDest,  			int* i)					{ *piDest = ((int64)*i << 32) | *i; }
void ConvertShortToLong		(int64* piDest,  			int16* s) 				{ *piDest = ((int64)*s << 48) | ((int64)*s << 32) | ((int64)*s << 16) | *s; }
void ConvertCharToLong		(int64* piDest,  			char* c)				{ *piDest = ((int64)*c << 56) | ((int64)*c << 48) | ((int64)*c << 40) | ((int64)*c << 32) | ((int64)*c << 24) | ((int64)*c << 16) | ((int64)*c << 8) | *c; }
void ConvertULongToLong		(int64* piDest,  			uint64* i)	{ *piDest = *i; }
void ConvertUIntToLong		(int64* piDest,  			uint32* i)		{ *piDest = ((int64)*i << 32) | *i; }
void ConvertUShortToLong	(int64* piDest,  			uint16* s) 		{ *piDest = ((int64)*s << 48) | ((int64)*s << 32) | ((int64)*s << 16) | *s; }
void ConvertUCharToLong		(int64* piDest,  			uint8* c)		{ *piDest = ((int64)*c << 56) | ((int64)*c << 48) | ((int64)*c << 40) | ((int64)*c << 32) | ((int64)*c << 24) | ((int64)*c << 16) | ((int64)*c << 8) | *c; }
void ConvertDoubleToULong	(uint64* psDest,	double* f) 				{ *psDest = (uint64)((double)0xffffffffffffffffLL * *f); }
void ConvertFloatToULong	(uint64* piDest,	float* f) 				{ *piDest = (uint64)((float)0xffffffffffffffffLL * *f); }
void ConvertLongToULong		(uint64* piDest,	int64* i)			{ *piDest = *i; }
void ConvertIntToULong		(uint64* piDest,	int* i)					{ *piDest = ((uint64)*i << 32) | *i; }
void ConvertShortToULong	(uint64* piDest,	int16* s) 				{ *piDest = ((uint64)*s << 48) | ((uint64)*s << 32) | ((uint64)*s << 16) | *s; }
void ConvertCharToULong		(uint64* piDest,	char* c)				{ *piDest = ((uint64)*c << 56) | ((uint64)*c << 48) | ((uint64)*c << 40) | ((uint64)*c << 32) | ((uint64)*c << 24) | ((uint64)*c << 16) | ((uint64)*c << 8) | *c; }
void ConvertULongToULong	(uint64* piDest, 	uint64* i)	{ *piDest = *i; }
void ConvertUIntToULong		(uint64* piDest,	uint32* i)		{ *piDest = ((uint64)*i << 32) | *i; }
void ConvertUShortToULong	(uint64* piDest,	uint16* s)		{ *piDest = ((uint64)*s << 48) | ((uint64)*s << 32) | ((uint64)*s << 16) | *s; }
void ConvertUCharToULong	(uint64* piDest,	uint8* c)		{ *piDest = ((uint64)*c << 56) | ((uint64)*c << 48) | ((uint64)*c << 40) | ((uint64)*c << 32) | ((uint64)*c << 24) | ((uint64)*c << 16) | ((uint64)*c << 8) | *c; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'float' Conversions.
void ConvertDoubleToFloat	(float* pfDest, double* f) 				{ *pfDest = (float)*f; }
void ConvertFloatToFloat	(float* pfDest, float* f) 				{ *pfDest = *f; }
void ConvertLongToFloat		(float* pfDest, int64* i)			{ *pfDest = (float)*i / 0xffffffffffffffffLL; }
void ConvertIntToFloat		(float* pfDest, int* i)					{ *pfDest = (float)*i / 0xffffffff; }
void ConvertShortToFloat	(float* pfDest, int16* s) 				{ *pfDest = (float)*s / 0xffff; }
void ConvertCharToFloat		(float* pfDest, char* c)				{ *pfDest = (float)*c / 0xff; }
void ConvertULongToFloat	(float* pfDest, uint64* i)	{ *pfDest = (float)*i / 0xffffffffffffffffLL; }
void ConvertUIntToFloat		(float* pfDest, uint32* i)		{ *pfDest = (float)*i / 0xffffffff; }
void ConvertUShortToFloat	(float* pfDest, uint16* s) 		{ *pfDest = (float)*s / 0xffff; }
void ConvertUCharToFloat	(float* pfDest, uint8* c)		{ *pfDest = (float)*c / 0xff; }
void ConvertDoubleToDouble	(double* pfDest, double* f) 			{ *pfDest = *f; }
void ConvertFloatToDouble	(double* pfDest, float* f) 				{ *pfDest = *f; }
void ConvertLongToDouble	(double* pfDest, int64* i)			{ *pfDest = (double)*i / 0xffffffffffffffffLL; }
void ConvertIntToDouble		(double* pfDest, int* i)				{ *pfDest = (double)*i / 0xffffffff; }
void ConvertShortToDouble	(double* pfDest, int16* s) 				{ *pfDest = (double)*s / 0xffff; }
void ConvertCharToDouble	(double* pfDest, char* c)				{ *pfDest = (double)*c / 0xff; }
void ConvertULongToDouble	(double* pfDest, uint64* i)	{ *pfDest = (double)*i / 0xffffffffffffffffLL; }
void ConvertUIntToDouble	(double* pfDest, uint32* i)		{ *pfDest = (double)*i / 0xffffffff; }
void ConvertUShortToDouble	(double* pfDest, uint16* s)		{ *pfDest = (double)*s / 0xffff; }
void ConvertUCharToDouble	(double* pfDest, uint8* c)		{ *pfDest = (double)*c / 0xff; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Sixbits' Conversions.
void ConvertUCharToSixbits	(uint8* pcDest,  uint8* c)		{ *pcDest = *c / (0xff/0x3f); }
void ConvertUShortToSixbits	(uint8* pcDest,  uint16* s)		{ *pcDest = *s / (0xffff/0x3f); }
void ConvertUIntToSixbits	(uint8* pcDest,  uint32* i)		{ *pcDest = *i / (0xffffffff/0x3f); }
void ConvertDoubleToSixbits	(uint8* pcDest,  double* f)				{ *pcDest = (uint8)(*f * (double)0x3f); }
void ConvertFloatToSixbits	(uint8* pcDest,  float* f)				{ *pcDest = (uint8)(*f * (float)0x3f); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nickle' Conversions.
void ConvertUCharToNickle	(uint8* pcDest,  uint8* c)		{ *pcDest = *c / (0xff/0x1f); }
void ConvertUShortToNickle	(uint8* pcDest,  uint16* s)		{ *pcDest = *s / (0xffff/0x1f); }
void ConvertUIntToNickle	(uint8* pcDest,  uint32* i)		{ *pcDest = *i / (0xffffffff/0x1f); }
void ConvertDoubleToNickle	(uint8* pcDest,  double* f)				{ *pcDest = (uint8)(*f * (double)0x1f); }
void ConvertFloatToNickle	(uint8* pcDest,  float* f)				{ *pcDest = (uint8)(*f * (float)0x1f); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nybble' Conversions.
void ConvertUCharToNybble	(uint8* pcDest,  uint8* c)		{ *pcDest = *c / (0xff/0xf); }
void ConvertUShortToNybble	(uint8* pcDest,  uint16* s)		{ *pcDest = *s / (0xffff/0xf); }
void ConvertUIntToNybble	(uint8* pcDest,  uint32* i)		{ *pcDest = *i / (0xffffffff/0xf); }
void ConvertDoubleToNybble	(uint8* pcDest,  double* f)				{ *pcDest = (uint8)(*f * (double)0xf); }
void ConvertFloatToNybble	(uint8* pcDest,  float* f)				{ *pcDest = (uint8)(*f * (float)0xf); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Crumb' Conversions.
void ConvertUCharToCrumb	(uint8* pcDest,  uint8* c)		{ *pcDest = *c / (0xff/0x3); } 
void ConvertUShortToCrumb	(uint8* pcDest,  uint16* s)		{ *pcDest = *s / (0xffff/0x3); }
void ConvertUIntToCrumb		(uint8* pcDest,  uint32* i)		{ *pcDest = *i / (0xffffffff/0x3); }
void ConvertDoubleToCrumb	(uint8* pcDest,  double* f)				{ *pcDest = (uint8)(*f * (double)0x3); }
void ConvertFloatToCrumb	(uint8* pcDest,  float* f)				{ *pcDest = (uint8)(*f * (float)0x3); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Bit' Conversions.
void ConvertUCharToBit		(uint8* pcDest,  uint8* c)		{ *pcDest = *c / (0xff); }
void ConvertUShortToBit		(uint8* pcDest,  uint16* s)		{ *pcDest = *s / (0xffff); }
void ConvertUIntToBit		(uint8* pcDest,  uint32* i)		{ *pcDest = *i / (0xffffffff); }
void ConvertDoubleToBit		(uint8* pcDest,  double* f)				{ *pcDest = (uint8)*f; }
void ConvertFloatToBit		(uint8* pcDest,  float* f)				{ *pcDest = (uint8)*f; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
//Other U.  Clean up later
void ConvertSixbitsToUShort		(uint16* pDest,  uint8* c)		{ *pDest = (uint16)((float)*c * (65535.0f/63.0f)); }
void ConvertNickleToUShort		(uint16* pDest,  uint8* c)		{ *pDest = (uint16)((float)*c * (65535.0f/31.0f)); }
void ConvertNybbleToUShort		(uint16* pDest,  uint8* c)		{ *pDest = (uint16)((float)*c * (65535.0f/15.0f)); }
void ConvertThreeBitsToUShort	(uint16* pDest,  uint8* c)		{ *pDest = (uint16)((float)*c * (65535.0f/7.0f)); }
void ConvertCrumbToUShort		(uint16* pDest,  uint8* c)		{ *pDest = (uint16)*c * (0xffff/3); }
void ConvertBitToUShort			(uint16* pDest,  uint8* c)		{ *pDest = (uint16)*c * 0xffff; }

void ConvertSixbitsToUChar	(uint8* pDest,  uint8* c)		{ *pDest = (uint8)((float)*c * (255.0f/63.0f)); }
void ConvertNickleToUChar	(uint8* pDest,  uint8* c)		{ *pDest = (uint8)((float)*c * (255.0f/31.0f)); }
void ConvertNybbleToUChar	(uint8* pDest,  uint8* c)		{ *pDest = (uint8)((float)*c * (255.0f/15.0f)); }
void ConvertThreeBitsToUChar(uint8* pDest,  uint8* c)		{ *pDest = (uint8)((float)*c * (255.0f/7.0f)); }
void ConvertCrumbToUChar	(uint8* pDest,  uint8* c)		{ *pDest = (uint8)*c * (0xff/3); }
void ConvertBitToUChar		(uint8* pDest,  uint8* c)		{ *pDest = (uint8)*c * 0xff; }
									 
void ConvertSixbitsToUInt	(uint32* pDest,  	uint8* c)		{ *pDest = (uint32)((float)*c * (4294967295.0f/63.0f)); }
void ConvertNickleToUInt	(uint32* pDest,  	uint8* c)		{ *pDest = (uint32)((float)*c * (4294967295.0f/31.0f)); }
void ConvertNybbleToUInt	(uint32* pDest,  	uint8* c)		{ *pDest = (uint32)((float)*c * (4294967295.0f/15.0f)); }
void ConvertThreeBitsToUInt	(uint32* pDest,  	uint8* c)		{ *pDest = (uint32)((float)*c * (4294967295.0f/7.0f)); }
void ConvertCrumbToUInt		(uint32* pDest,  	uint8* c)		{ *pDest = (uint32)*c * (0xffffffff/3); }
void ConvertBitToUInt		(uint32* pDest,  	uint8* c)		{ *pDest = (uint32)*c * 0xffffffff; }
									 
void ConvertSixbitsToFloat	(float* pDest,  		uint8* c)		{ *pDest = (float)*c * (1.0f/63.0f); }
void ConvertNickleToFloat	(float* pDest,  		uint8* c)		{ *pDest = (float)*c * (1.0f/31.0f); }
void ConvertNybbleToFloat	(float* pDest,  		uint8* c)		{ *pDest = (float)*c * (1.0f/15.0f); }
void ConvertThreeBitsToFloat(float* pDest,  		uint8* c)		{ *pDest = (float)*c * (1.0f/7.0f); }
void ConvertCrumbToFloat	(float* pDest,  		uint8* c)		{ *pDest = (float)*c * (1.0f/3.0f); }
void ConvertBitToFloat		(float* pDest,  		uint8* c)		{ *pDest = (float)*c * (1.0f/1.0f); }

void ConvertIntToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertIntToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertIntToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertIntToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertIntToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertShortToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertShortToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertShortToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertShortToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertShortToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCharToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCharToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCharToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCharToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCharToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertFloatToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertFloatToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToBit(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x01; }
void ConvertBitToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToCrumb(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x03; }
void ConvertCrumbToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToNybble(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x0f; }
void ConvertNybbleToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToNickle(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x1f; }
void ConvertNickleToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToSixbits(void* pvDest, void* pvSource) { *(uint8*)pvDest = (*(uint8*)pvSource) & 0x3f; }
void ConvertLongToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertLongToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertLongToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertLongToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertLongToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertDoubleToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertDoubleToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertULongToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertULongToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertULongToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertULongToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertULongToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToLong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToDouble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToULong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToLong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToDouble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToULong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToLong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToDouble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToULong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToLong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToDouble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToULong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToLong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToDouble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToULong(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }

void ConversionInvalid(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types (invalid)."); }

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeConverter::Init(void)
{
	int		iDest;
	int		iSource;

	for (iDest = 0; iDest < PRIMTIVE_CONVERTER_END; iDest++)
	{
		for (iSource = 0; iSource < PRIMTIVE_CONVERTER_END; iSource++)
		{
			mapvConversion[iDest][iSource] = ConversionInvalid;
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
		mapvConversion[eDest][eSource](pvDest, pvSource);
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

