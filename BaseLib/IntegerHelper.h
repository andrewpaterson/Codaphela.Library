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
#ifndef __INTEGER_HELPER_H__
#define __INTEGER_HELPER_H__
#include "Define.h"
#include "PrimitiveTypes.h"


uint32 	TruncateLowBits(uint32 i);
bool 	IsPowerOfTwo(uint32 i);
int32 	GetBestHighBit(int32 i);
int32 	GetLogBase2(int32 i);  //0x1 -> 0, 0x2 -> 1, 0x4 -> 2, 0x8 -> 3, 0x10 -> 4, 0x20 -> 5...
int32 	GetByte(int32 iInt, int32 iPos);
int32 	SetByte(int32 c, int32 iPos);
bool 	GetBit(int32 iBit, void* pvBitArray);  //return 1 or 0 in char array of bits.
bool 	GetBitReverseHiLo(int32 iBit, void* pvBitArray);  //return 1 or 0 in char array of bits indexed in reverse.
int32	CountBits(void* pvBitArray, int32 iBitLength); 
int32	CountBitsSingly(void* pvBitArray, int32 iBitLength);  //Slow, for testing.
void 	SetBit(int32 iBit, void* pvBitArray, bool bBit);
void 	SetBits(int32 iStart, void* pvBitArray, bool bBit, int32 iLength);
void 	SetFlag(int32* piDest, int32 iFlag, bool iFlagValue);
void 	SetFlag(uint32* piDest, uint32 iFlag, bool iFlagValue);
void 	SetFlag(int16* psiDest, int16 iFlag, bool iFlagValue);
void 	SetFlag(uint16* psiDest, uint16 iFlag, bool iFlagValue);
void 	SetFlag(int8* psiDest, int8 iFlag, bool iFlagValue);
void 	SetFlag(uint8* psiDest, uint8 iFlag, bool iFlagValue);
int32 	FindFirstClearBit(void* pvArray, int32 iArraySize);
int32 	FindFirstSetBit(void* pvArray, int32 iArraySize);
int32 	FindLastClearBit(void* pvArray, int32 iArraySize);
int32 	FindLastSetBit(void* pvArray, int32 iArraySize);
int32	FindNextSetBit(void* pvArray, int32 iArraySize, int32 iStartBit);
int32	FindNextClearBit(void* pvArray, int32 iArraySize, int32 iStartBit);
bool 	FixBool(int32 i);
bool	FixBool(void* pv);
void 	Swap(int32* pi1, int32* pi2);
void 	Swap(uint8* pc1, uint8* pi2);
int32 	FindFirstInt(int32* piIntArray, int32 iSearch, int32 iMaxLength);
int32	FindFirstByte(uint8* pcCharArray, uint8 uiSearch, int32 iMaxLength);
int32	FindFirstByte(int8* pcCharArray, int8 iSearch, int32 iMaxLength);
int32	FindFirstByte(char* pcCharArray, char cSearch, int32 iMaxLength);
int32	GetHighNybble(char c);
int32	GetLowNybble(char c);
int32 	CompareInt(const void* arg1, const void* arg2);
int32 	CompareLong(const void* arg1, const void* arg2);
int32	CompareIntReverse(const void* arg1, const void* arg2);
int32	CompareLongReverse(const void* arg1, const void* arg2);
char 	GetCrumb(int32 iCrumb, void* pvArray);  //return 11, 10, 01 or 00.
char 	GetNybble(int32 iNybble, void* pvArray);
void 	CopyBits(void* pvDest, int32 iDestOffset, void* pvSource, int32 iSourceOffset, int32 iNumBits, int32 bClear);  //Setting clear will make all unused bits zero.
void 	CopyBitsReverseHiLo(void* pvDest, int32 iDestOffset, void* pvSource, int32 iSourceOffset, int32 iNumBits, int32 bClear);  //Setting clear will make all unused bits zero.
int16	ReverseShortEndianness(int16 s);
int32	ReverseIntEndianness(int32 i);
int64	ReverseLongEndianness(int64 i);
void	ReverseEndianness(void* pv, int32 iSize);
void	ReverseBytes(void* pv, int32 iSize);
int32 	CalculateStride(int32 iElementSize, int32 iAlignment);
int32 	CalculateOffset(char iOffset, int32 iAlignment);
int32	IntAbs(int32 i);
int32	GetPowerOf2SizeDown(int32 iInt);
int32	GetPowerOf2SizeUp(int32 iInt);
int32	GetPowerOf2SizeAuto(int32 iInt);
int32	LargestInt(int32* pInts, int32 iCount);


#endif // __INTEGER_HELPER_H__

