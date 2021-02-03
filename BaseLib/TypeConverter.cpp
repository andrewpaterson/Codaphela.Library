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
void ConvertLongToChar		(char* pcDest, 			long long* i)			{ *pcDest = (char)(*i / 0x101010101010101LL); }
void ConvertIntToChar		(char* pcDest, 			int* i)					{ *pcDest = *i / (0xffffffff/0xff); }
void ConvertShortToChar		(char* pcDest, 			int16* s) 				{ *pcDest = *s / (0xffff/0xff); }
void ConvertCharToChar		(char* pcDest, 			char* c)				{ *pcDest = *c; }
void ConvertULongToChar		(char* pcDest, 			unsigned long long* i)	{ *pcDest = (char)(*i / 0x101010101010101LL); }
void ConvertUIntToChar		(char* pcDest, 			unsigned int* i)		{ *pcDest = *i / (0xffffffff/0xff); }
void ConvertUShortToChar	(char* pcDest, 			uint16* s) 		{ *pcDest = *s / (0xffff/0xff); }
void ConvertUCharToChar		(char* pcDest, 			unsigned char* c)		{ *pcDest = *c; }
void ConvertDoubleToUChar	(unsigned char* pcDest, double* f) 				{ *pcDest = (unsigned char)((double)0xff * *f); }
void ConvertFloatToUChar	(unsigned char* pcDest, float* f) 				{ *pcDest = (unsigned char)((float)0xff * *f); }
void ConvertLongToUChar		(unsigned char* pcDest, long long* i)			{ *pcDest = (unsigned char)(*i / 0x101010101010101LL); }
void ConvertIntToUChar		(unsigned char* pcDest, int* i)					{ *pcDest = *i / (0xffffffff/0xff); }
void ConvertShortToUChar	(unsigned char* pcDest, int16* s) 				{ *pcDest = *s / (0xffff/0xff); }
void ConvertCharToUChar		(unsigned char* pcDest, char* c)				{ *pcDest = *c; }
void ConvertULongToUChar	(unsigned char* pcDest, unsigned long long* i)	{ *pcDest = (unsigned char)(*i / 0x101010101010101LL); }
void ConvertUIntToUChar		(unsigned char* pcDest, unsigned int* i)		{ *pcDest = *i / (0xffffffff/0xff); }
void ConvertUShortToUChar	(unsigned char* pcDest, uint16* s)		{ *pcDest = *s / (0xffff/0xff); }
void ConvertUCharToUChar	(unsigned char* pcDest, unsigned char* c)		{ *pcDest = *c; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int16' Conversions.
void ConvertDoubleToShort	(int16* psDest,  			double* f) 				{ *psDest = (int16)((double)0xffff * *f); }
void ConvertFloatToShort	(int16* psDest,  			float* f) 				{ *psDest = (int16)((float)0xffff * *f); }
void ConvertLongToShort		(int16* psDest,  			long long* i)			{ *psDest = (int16)(*i / 0x1000100010001LL); }
void ConvertIntToShort		(int16* psDest,  			int* i)					{ *psDest = *i / (0xffffffff/0xffff); }
void ConvertShortToShort	(int16* psDest,  			int16* s) 				{ *psDest = *s; }
void ConvertCharToShort		(int16* psDest,  			char* c)				{ *psDest = ((int16)*c << 8) | *c; }
void ConvertULongToShort	(int16* psDest,  			unsigned long long* i)	{ *psDest = (int16)(*i / 0x1000100010001LL); }
void ConvertUIntToShort		(int16* psDest,  			unsigned int* i)		{ *psDest = *i / (0xffffffff/0xffff); }
void ConvertUShortToShort	(int16* psDest,  			uint16* s) 		{ *psDest = *s; }
void ConvertUCharToShort	(int16* psDest,  			unsigned char* c)		{ *psDest = ((int16)*c << 8) | *c; }
void ConvertDoubleToUShort	(uint16* psDest,  	double* f) 				{ *psDest = (uint16)((double)0xffff * *f); }
void ConvertFloatToUShort	(uint16* psDest,  	float* f) 				{ *psDest = (uint16)((float)0xffff * *f); }
void ConvertLongToUShort	(uint16* psDest,  	long long* i)			{ *psDest = (uint16)(*i / 0x1000100010001LL); }
void ConvertIntToUShort		(uint16* psDest,  	int* i)					{ *psDest = *i / (0xffffffff/0xffff); }
void ConvertShortToUShort	(uint16* psDest,  	int16* s) 				{ *psDest = *s; }
void ConvertCharToUShort	(uint16* psDest,  	char* c)				{ *psDest = ((uint16)*c << 8) | *c; }
void ConvertULongToUShort	(uint16* psDest,  	unsigned long long* i)	{ *psDest = (uint16)(*i / 0x1000100010001LL); }
void ConvertUIntToUShort	(uint16* psDest,  	unsigned int* i)		{ *psDest = *i / (0xffffffff/0xffff); }
void ConvertUShortToUShort	(uint16* psDest,  	uint16* s)		{ *psDest = *s; }
void ConvertUCharToUShort	(uint16* psDest,  	unsigned char* c)		{ *psDest = ((uint16)*c << 8) | *c; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'int' Conversions.
void ConvertDoubleToInt		(int* piDest,  			double* f) 				{ *piDest = (int)((double)0xffffffff * *f); }
void ConvertFloatToInt		(int* piDest,  			float* f) 				{ *piDest = (int)((float)0xffffffff * *f); }
void ConvertLongToInt		(int* piDest,  			long long* i)			{ *piDest = (int)(*i / 0x100000001LL); }
void ConvertIntToInt		(int* piDest,  			int* i)					{ *piDest = *i; }	   
void ConvertShortToInt		(int* piDest,  			int16* s) 				{ *piDest = ((int)*s << 16) | *s; }
void ConvertCharToInt		(int* piDest,  			char* c)				{ *piDest = ((int)*c << 24) | ((int)*c << 16) | ((int)*c << 8) | *c; }
void ConvertULongToInt		(int* piDest,  			unsigned long long* i)	{ *piDest = (int)(*i / 0x100000001LL); }
void ConvertUIntToInt		(int* piDest,  			unsigned int* i)		{ *piDest = *i; }
void ConvertUShortToInt		(int* piDest,  			uint16* s) 		{ *piDest = ((int)*s << 16) | *s; }
void ConvertUCharToInt		(int* piDest,  			unsigned char* c)		{ *piDest = ((int)*c << 24) | ((int)*c << 16) | ((int)*c << 8) | *c; }
void ConvertDoubleToUInt	(unsigned int* psDest,  double* f) 				{ *psDest = (unsigned int)((double)0xffffffff * *f); }
void ConvertFloatToUInt		(unsigned int* piDest,  float* f) 				{ *piDest = (unsigned int)((float)0xffffffff * *f); }
void ConvertLongToUInt		(unsigned int* piDest, 	long long* i)			{ *piDest = (unsigned int)(*i / 0x100000001LL); }
void ConvertIntToUInt		(unsigned int* piDest,  int* i)					{ *piDest = *i; }
void ConvertShortToUInt		(unsigned int* piDest,  int16* s) 				{ *piDest = ((unsigned int)*s << 16) | *s; }
void ConvertCharToUInt		(unsigned int* piDest,  char* c)				{ *piDest = ((unsigned int)*c << 24) | ((int)*c << 16) | ((int)*c << 8) | *c; }
void ConvertULongToUInt		(unsigned int* piDest, 	unsigned long long* i)	{ *piDest = (unsigned int)(*i / 0x100000001LL); }
void ConvertUIntToUInt		(unsigned int* piDest,  unsigned int* i)		{ *piDest = *i; }
void ConvertUShortToUInt	(unsigned int* piDest,  uint16* s)		{ *piDest = ((unsigned int)*s << 16) | *s; }
void ConvertUCharToUInt		(unsigned int* piDest,  unsigned char* c)		{ *piDest = ((unsigned int)*c << 24) | ((int)*c << 16) | ((int)*c << 8) | *c; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'long' Conversions.
void ConvertDoubleToLong	(long long* piDest,  			double* f) 				{ *piDest = (long long)((double)0xffffffffffffffffLL * *f); }
void ConvertFloatToLong		(long long* piDest,  			float* f) 				{ *piDest = (long long)((float)0xffffffffffffffffLL * *f); }
void ConvertLongToLong		(long long* piDest,  			long long* i)			{ *piDest = *i; }
void ConvertIntToLong		(long long* piDest,  			int* i)					{ *piDest = ((long long)*i << 32) | *i; }
void ConvertShortToLong		(long long* piDest,  			int16* s) 				{ *piDest = ((long long)*s << 48) | ((long long)*s << 32) | ((long long)*s << 16) | *s; }
void ConvertCharToLong		(long long* piDest,  			char* c)				{ *piDest = ((long long)*c << 56) | ((long long)*c << 48) | ((long long)*c << 40) | ((long long)*c << 32) | ((long long)*c << 24) | ((long long)*c << 16) | ((long long)*c << 8) | *c; }
void ConvertULongToLong		(long long* piDest,  			unsigned long long* i)	{ *piDest = *i; }
void ConvertUIntToLong		(long long* piDest,  			unsigned int* i)		{ *piDest = ((long long)*i << 32) | *i; }
void ConvertUShortToLong	(long long* piDest,  			uint16* s) 		{ *piDest = ((long long)*s << 48) | ((long long)*s << 32) | ((long long)*s << 16) | *s; }
void ConvertUCharToLong		(long long* piDest,  			unsigned char* c)		{ *piDest = ((long long)*c << 56) | ((long long)*c << 48) | ((long long)*c << 40) | ((long long)*c << 32) | ((long long)*c << 24) | ((long long)*c << 16) | ((long long)*c << 8) | *c; }
void ConvertDoubleToULong	(unsigned long long* psDest,	double* f) 				{ *psDest = (unsigned long long)((double)0xffffffffffffffffLL * *f); }
void ConvertFloatToULong	(unsigned long long* piDest,	float* f) 				{ *piDest = (unsigned long long)((float)0xffffffffffffffffLL * *f); }
void ConvertLongToULong		(unsigned long long* piDest,	long long* i)			{ *piDest = *i; }
void ConvertIntToULong		(unsigned long long* piDest,	int* i)					{ *piDest = ((unsigned long long)*i << 32) | *i; }
void ConvertShortToULong	(unsigned long long* piDest,	int16* s) 				{ *piDest = ((unsigned long long)*s << 48) | ((unsigned long long)*s << 32) | ((unsigned long long)*s << 16) | *s; }
void ConvertCharToULong		(unsigned long long* piDest,	char* c)				{ *piDest = ((unsigned long long)*c << 56) | ((unsigned long long)*c << 48) | ((unsigned long long)*c << 40) | ((unsigned long long)*c << 32) | ((unsigned long long)*c << 24) | ((unsigned long long)*c << 16) | ((unsigned long long)*c << 8) | *c; }
void ConvertULongToULong	(unsigned long long* piDest, 	unsigned long long* i)	{ *piDest = *i; }
void ConvertUIntToULong		(unsigned long long* piDest,	unsigned int* i)		{ *piDest = ((unsigned long long)*i << 32) | *i; }
void ConvertUShortToULong	(unsigned long long* piDest,	uint16* s)		{ *piDest = ((unsigned long long)*s << 48) | ((unsigned long long)*s << 32) | ((unsigned long long)*s << 16) | *s; }
void ConvertUCharToULong	(unsigned long long* piDest,	unsigned char* c)		{ *piDest = ((unsigned long long)*c << 56) | ((unsigned long long)*c << 48) | ((unsigned long long)*c << 40) | ((unsigned long long)*c << 32) | ((unsigned long long)*c << 24) | ((unsigned long long)*c << 16) | ((unsigned long long)*c << 8) | *c; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'float' Conversions.
void ConvertDoubleToFloat	(float* pfDest, double* f) 				{ *pfDest = (float)*f; }
void ConvertFloatToFloat	(float* pfDest, float* f) 				{ *pfDest = *f; }
void ConvertLongToFloat		(float* pfDest, long long* i)			{ *pfDest = (float)*i / 0xffffffffffffffffLL; }
void ConvertIntToFloat		(float* pfDest, int* i)					{ *pfDest = (float)*i / 0xffffffff; }
void ConvertShortToFloat	(float* pfDest, int16* s) 				{ *pfDest = (float)*s / 0xffff; }
void ConvertCharToFloat		(float* pfDest, char* c)				{ *pfDest = (float)*c / 0xff; }
void ConvertULongToFloat	(float* pfDest, unsigned long long* i)	{ *pfDest = (float)*i / 0xffffffffffffffffLL; }
void ConvertUIntToFloat		(float* pfDest, unsigned int* i)		{ *pfDest = (float)*i / 0xffffffff; }
void ConvertUShortToFloat	(float* pfDest, uint16* s) 		{ *pfDest = (float)*s / 0xffff; }
void ConvertUCharToFloat	(float* pfDest, unsigned char* c)		{ *pfDest = (float)*c / 0xff; }
void ConvertDoubleToDouble	(double* pfDest, double* f) 			{ *pfDest = *f; }
void ConvertFloatToDouble	(double* pfDest, float* f) 				{ *pfDest = *f; }
void ConvertLongToDouble	(double* pfDest, long long* i)			{ *pfDest = (double)*i / 0xffffffffffffffffLL; }
void ConvertIntToDouble		(double* pfDest, int* i)				{ *pfDest = (double)*i / 0xffffffff; }
void ConvertShortToDouble	(double* pfDest, int16* s) 				{ *pfDest = (double)*s / 0xffff; }
void ConvertCharToDouble	(double* pfDest, char* c)				{ *pfDest = (double)*c / 0xff; }
void ConvertULongToDouble	(double* pfDest, unsigned long long* i)	{ *pfDest = (double)*i / 0xffffffffffffffffLL; }
void ConvertUIntToDouble	(double* pfDest, unsigned int* i)		{ *pfDest = (double)*i / 0xffffffff; }
void ConvertUShortToDouble	(double* pfDest, uint16* s)		{ *pfDest = (double)*s / 0xffff; }
void ConvertUCharToDouble	(double* pfDest, unsigned char* c)		{ *pfDest = (double)*c / 0xff; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Sixbits' Conversions.
void ConvertUCharToSixbits	(unsigned char* pcDest,  unsigned char* c)		{ *pcDest = *c / (0xff/0x3f); }
void ConvertUShortToSixbits	(unsigned char* pcDest,  uint16* s)		{ *pcDest = *s / (0xffff/0x3f); }
void ConvertUIntToSixbits	(unsigned char* pcDest,  unsigned int* i)		{ *pcDest = *i / (0xffffffff/0x3f); }
void ConvertDoubleToSixbits	(unsigned char* pcDest,  double* f)				{ *pcDest = (unsigned char)(*f * (double)0x3f); }
void ConvertFloatToSixbits	(unsigned char* pcDest,  float* f)				{ *pcDest = (unsigned char)(*f * (float)0x3f); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nickle' Conversions.
void ConvertUCharToNickle	(unsigned char* pcDest,  unsigned char* c)		{ *pcDest = *c / (0xff/0x1f); }
void ConvertUShortToNickle	(unsigned char* pcDest,  uint16* s)		{ *pcDest = *s / (0xffff/0x1f); }
void ConvertUIntToNickle	(unsigned char* pcDest,  unsigned int* i)		{ *pcDest = *i / (0xffffffff/0x1f); }
void ConvertDoubleToNickle	(unsigned char* pcDest,  double* f)				{ *pcDest = (unsigned char)(*f * (double)0x1f); }
void ConvertFloatToNickle	(unsigned char* pcDest,  float* f)				{ *pcDest = (unsigned char)(*f * (float)0x1f); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Nybble' Conversions.
void ConvertUCharToNybble	(unsigned char* pcDest,  unsigned char* c)		{ *pcDest = *c / (0xff/0xf); }
void ConvertUShortToNybble	(unsigned char* pcDest,  uint16* s)		{ *pcDest = *s / (0xffff/0xf); }
void ConvertUIntToNybble	(unsigned char* pcDest,  unsigned int* i)		{ *pcDest = *i / (0xffffffff/0xf); }
void ConvertDoubleToNybble	(unsigned char* pcDest,  double* f)				{ *pcDest = (unsigned char)(*f * (double)0xf); }
void ConvertFloatToNybble	(unsigned char* pcDest,  float* f)				{ *pcDest = (unsigned char)(*f * (float)0xf); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Crumb' Conversions.
void ConvertUCharToCrumb	(unsigned char* pcDest,  unsigned char* c)		{ *pcDest = *c / (0xff/0x3); } 
void ConvertUShortToCrumb	(unsigned char* pcDest,  uint16* s)		{ *pcDest = *s / (0xffff/0x3); }
void ConvertUIntToCrumb		(unsigned char* pcDest,  unsigned int* i)		{ *pcDest = *i / (0xffffffff/0x3); }
void ConvertDoubleToCrumb	(unsigned char* pcDest,  double* f)				{ *pcDest = (unsigned char)(*f * (double)0x3); }
void ConvertFloatToCrumb	(unsigned char* pcDest,  float* f)				{ *pcDest = (unsigned char)(*f * (float)0x3); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
// To 'Bit' Conversions.
void ConvertUCharToBit		(unsigned char* pcDest,  unsigned char* c)		{ *pcDest = *c / (0xff); }
void ConvertUShortToBit		(unsigned char* pcDest,  uint16* s)		{ *pcDest = *s / (0xffff); }
void ConvertUIntToBit		(unsigned char* pcDest,  unsigned int* i)		{ *pcDest = *i / (0xffffffff); }
void ConvertDoubleToBit		(unsigned char* pcDest,  double* f)				{ *pcDest = (unsigned char)*f; }
void ConvertFloatToBit		(unsigned char* pcDest,  float* f)				{ *pcDest = (unsigned char)*f; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//
//Other U.  Clean up later
void ConvertSixbitsToUShort		(uint16* pDest,  unsigned char* c)		{ *pDest = (uint16)((float)*c * (65535.0f/63.0f)); }
void ConvertNickleToUShort		(uint16* pDest,  unsigned char* c)		{ *pDest = (uint16)((float)*c * (65535.0f/31.0f)); }
void ConvertNybbleToUShort		(uint16* pDest,  unsigned char* c)		{ *pDest = (uint16)((float)*c * (65535.0f/15.0f)); }
void ConvertThreeBitsToUShort	(uint16* pDest,  unsigned char* c)		{ *pDest = (uint16)((float)*c * (65535.0f/7.0f)); }
void ConvertCrumbToUShort		(uint16* pDest,  unsigned char* c)		{ *pDest = (uint16)*c * (0xffff/3); }
void ConvertBitToUShort			(uint16* pDest,  unsigned char* c)		{ *pDest = (uint16)*c * 0xffff; }

void ConvertSixbitsToUChar	(unsigned char* pDest,  unsigned char* c)		{ *pDest = (unsigned char)((float)*c * (255.0f/63.0f)); }
void ConvertNickleToUChar	(unsigned char* pDest,  unsigned char* c)		{ *pDest = (unsigned char)((float)*c * (255.0f/31.0f)); }
void ConvertNybbleToUChar	(unsigned char* pDest,  unsigned char* c)		{ *pDest = (unsigned char)((float)*c * (255.0f/15.0f)); }
void ConvertThreeBitsToUChar(unsigned char* pDest,  unsigned char* c)		{ *pDest = (unsigned char)((float)*c * (255.0f/7.0f)); }
void ConvertCrumbToUChar	(unsigned char* pDest,  unsigned char* c)		{ *pDest = (unsigned char)*c * (0xff/3); }
void ConvertBitToUChar		(unsigned char* pDest,  unsigned char* c)		{ *pDest = (unsigned char)*c * 0xff; }
									 
void ConvertSixbitsToUInt	(unsigned int* pDest,  	unsigned char* c)		{ *pDest = (unsigned int)((float)*c * (4294967295.0f/63.0f)); }
void ConvertNickleToUInt	(unsigned int* pDest,  	unsigned char* c)		{ *pDest = (unsigned int)((float)*c * (4294967295.0f/31.0f)); }
void ConvertNybbleToUInt	(unsigned int* pDest,  	unsigned char* c)		{ *pDest = (unsigned int)((float)*c * (4294967295.0f/15.0f)); }
void ConvertThreeBitsToUInt	(unsigned int* pDest,  	unsigned char* c)		{ *pDest = (unsigned int)((float)*c * (4294967295.0f/7.0f)); }
void ConvertCrumbToUInt		(unsigned int* pDest,  	unsigned char* c)		{ *pDest = (unsigned int)*c * (0xffffffff/3); }
void ConvertBitToUInt		(unsigned int* pDest,  	unsigned char* c)		{ *pDest = (unsigned int)*c * 0xffffffff; }
									 
void ConvertSixbitsToFloat	(float* pDest,  		unsigned char* c)		{ *pDest = (float)*c * (1.0f/63.0f); }
void ConvertNickleToFloat	(float* pDest,  		unsigned char* c)		{ *pDest = (float)*c * (1.0f/31.0f); }
void ConvertNybbleToFloat	(float* pDest,  		unsigned char* c)		{ *pDest = (float)*c * (1.0f/15.0f); }
void ConvertThreeBitsToFloat(float* pDest,  		unsigned char* c)		{ *pDest = (float)*c * (1.0f/7.0f); }
void ConvertCrumbToFloat	(float* pDest,  		unsigned char* c)		{ *pDest = (float)*c * (1.0f/3.0f); }
void ConvertBitToFloat		(float* pDest,  		unsigned char* c)		{ *pDest = (float)*c * (1.0f/1.0f); }

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
void ConvertBitToBit(void* pvDest, void* pvSource) { *(unsigned char*)pvDest = (*(unsigned char*)pvSource) & 0x01; }
void ConvertBitToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertBitToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToCrumb(void* pvDest, void* pvSource) { *(unsigned char*)pvDest = (*(unsigned char*)pvSource) & 0x03; }
void ConvertCrumbToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertCrumbToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToNybble(void* pvDest, void* pvSource) { *(unsigned char*)pvDest = (*(unsigned char*)pvSource) & 0x0f; }
void ConvertNybbleToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNybbleToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertNickleToNickle(void* pvDest, void* pvSource) { *(unsigned char*)pvDest = (*(unsigned char*)pvSource) & 0x1f; }
void ConvertNickleToSixbits(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToInt(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToShort(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToChar(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToBit(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToCrumb(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToNybble(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToNickle(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types"); }
void ConvertSixbitsToSixbits(void* pvDest, void* pvSource) { *(unsigned char*)pvDest = (*(unsigned char*)pvSource) & 0x3f; }
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

void ConversionInvalid(void* pvDest, void* pvSource) { gcUserError.Set("Conversion not defined for types (ConversionInvalid)"); }

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTypeConverter::Init(void)
{
	int		iDest;
	int		iSource;

	for (iDest = 0; iDest < PRIMTIVE_TYPE_END; iDest++)
	{
		for (iSource = 0; iSource < PRIMTIVE_TYPE_END; iSource++)
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
void CTypeConverter::Do(EPrimitiveTypes eDest, void* pvDest, EPrimitiveTypes eSource, void* pvSource)
{
	mapvConversion[eDest][eSource](pvDest, pvSource);
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

