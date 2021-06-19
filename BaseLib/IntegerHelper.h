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
#ifndef __INTEGER_HELPER_H__
#define __INTEGER_HELPER_H__
#include "Define.h"
#include "PrimitiveTypes.h"


#ifdef MAXINT
#undef MAXINT
#endif //MAXINT
#define MAXINT 0x7fffffff

#ifdef MININT
#undef MININT
#endif //MININT
#define MININT 0x80000000


int 	TruncateLowBits(int i);
int 	IsPowerOfTwo(int i);
int 	GetBestHighBit(int i);
int 	GetLogBase2(int i);  //0x1 -> 0, 0x2 -> 1, 0x4 -> 2, 0x8 -> 3, 0x10 -> 4, 0x20 -> 5...
int 	GetByte(int iInt, int iPos);
int 	SetByte(int c, int iPos);
int 	GetBit(int iBit, void* pvBitArray);  //return 1 or 0 in char array of bits.
int 	GetBitReverseHiLo(int iBit, void* pvBitArray);  //return 1 or 0 in char array of bits indexed in reverse.
int		CountBits(void* pvBitArray, int iBitLength); 
int		CountBitsSingly(void* pvBitArray, int iBitLength);  //Slow, for testing.
void 	SetBit(int iBit, void* pvBitArray, int bBit);  //bBit can only take 1 or 0.
void 	SetFlag(int* piDest, int iFlag, int iFlagValue);  //Identically to set bit but only works on 4bytes.
void 	SetFlag(unsigned int* piDest, int iFlag, int iFlagValue);  //Identically to set bit but only works on 4bytes.
void 	SetFlag(int16* psiDest, int iFlag, int iFlagValue);  //Identically to set bit but only works on 4bytes.
void 	SetFlag(uint16* psiDest, int iFlag, int iFlagValue);  //Identically to set bit but only works on 2bytes.
void 	SetFlag(char* psiDest, int iFlag, int iFlagValue);  //Identically to set bit but only works on 4bytes.
void 	SetFlag(unsigned char* psiDest, int iFlag, int iFlagValue);  //Identically to set bit but only works on 2bytes.
int 	FindFirstClearBit(void* pvArray, int iArraySize);
int 	FindFirstSetBit(void* pvArray, int iArraySize);
int 	FindLastClearBit(void* pvArray, int iArraySize);
int 	FindLastSetBit(void* pvArray, int iArraySize);
int 	FixBool(int i);
int 	FixBool(void* pv);
void 	Swap(int* pi1, int* pi2);
void 	Swap(char* pc1, char* pi2);
int 	FindFirstInt(int* piIntArray, int iSearch, int iMaxLength);
int		FindFirstByte(char* pcCharArray, char cSearch, int iMaxLength);
int		GetHighNybble(char c);
int		GetLowNybble(char c);
int 	CompareInt(const void* arg1, const void* arg2);
int 	CompareLong(const void* arg1, const void* arg2);
int		CompareIntReverse(const void* arg1, const void* arg2);
int		CompareLongReverse(const void* arg1, const void* arg2);
char 	GetCrumb(int iCrumb, void* pvArray);  //return 11, 10, 01 or 00 in char.
char 	GetNybble(int iNybble, void* pvArray);
void 	CopyBits(void* pvDest, int iDestOffset, void* pvSource, int iSourceOffset, int iNumBits, int bClear);  //Setting clear will make all unused bits zero.
void 	CopyBitsReverseHiLo(void* pvDest, int iDestOffset, void* pvSource, int iSourceOffset, int iNumBits, int bClear);  //Setting clear will make all unused bits zero.
int16	ReverseShortEndianness(int16 s);
int		ReverseIntEndianness(int i);
int64	ReverseLongEndianness(int64 i);
void	ReverseEndianness(void* pv, int iSize);
void	ReverseBytes(void* pv, int iSize);
int 	CalculateStride(int iElementSize, int iAlignment);
int 	CalculateOffset(char iOffset, int iAlignment);
int		IntAbs(int i);
int		GetPowerOf2SizeDown(int iInt);
int		GetPowerOf2SizeUp(int iInt);
int		GetPowerOf2SizeAuto(int iInt);
int		LargestInt(int* pInts, int iCount);


#endif // !__INTEGER_HELPER_H__

