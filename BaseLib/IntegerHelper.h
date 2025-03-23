/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
uint32 	GetBestHighBit(uint32 i);
size 	GetLogBase2(uint32 i);  //0x1 -> 0, 0x2 -> 1, 0x4 -> 2, 0x8 -> 3, 0x10 -> 4, 0x20 -> 5...
uint8	GetByte(uint32 iInt, uint16 iPos);
uint32 	SetByte(uint8 c, uint16 iPos);
bool 	GetBit(size iBit, void* pvBitArray);  //return 1 or 0 in char array of bits.
bool 	GetBitReverseHiLo(size iBit, void* pvBitArray);  //return 1 or 0 in char array of bits indexed in reverse.
size	CountBits(void* pvBitArray, size iBitLength);
size	CountBitsSingly(void* pvBitArray, size iBitLength);  //Slow, for testing.
void 	SetBit(size iBit, void* pvBitArray, bool bBit);
void 	SetBits(size iStart, void* pvBitArray, bool bBit, size iLength);
void 	SetFlagByte(uint8* psiDest, uint8 iFlag, bool iFlagValue);
void 	SetFlagShort(uint16* psiDest, uint16 iFlag, bool iFlagValue);
void 	SetFlagInt(uint32* piDest, uint32 iFlag, bool iFlagValue);
void 	SetFlagLong(uint64* psiDest, uint64 iFlag, bool iFlagValue); 
void 	SetFlagSize(size* piDest, size iFlag, bool iFlagValue);
size 	FindFirstClearBit(void* pvArray, size iArraySize);
size 	FindFirstSetBit(void* pvArray, size iArraySize);
size 	FindLastClearBit(void* pvArray, size iArraySize);
size 	FindLastSetBit(void* pvArray, size iArraySize);
size	FindNextSetBit(void* pvArray, size iArraySize, size iStartBit);
size	FindNextClearBit(void* pvArray, size iArraySize, size iStartBit);
bool 	FixBool(int32 i);
bool	FixBool(void* pv);
void 	SwapInt32(int32* pi1, int32* pi2);
void	SwapInt(int* pi1, int* pi2);
void 	SwapByte(uint8* pc1, uint8* pi2);
void	SwapSize(size* pui1, size* pui2);
size 	FindFirstInt(int32* piIntArray, int32 iSearch, size iMaxLength);
size	FindFirstByte(uint8* pcCharArray, uint8 uiSearch, size iMaxLength);
uint8	GetHighNybble(uint8 c);
uint8	GetLowNybble(uint8 c);
int 	CompareByte(const void* arg1, const void* arg2);
int		CompareByteReverse(const void* arg1, const void* arg2);
int 	CompareShort(const void* arg1, const void* arg2);
int		CompareShortReverse(const void* arg1, const void* arg2);
int 	CompareInt(const void* arg1, const void* arg2);
int		CompareIntReverse(const void* arg1, const void* arg2);
int 	CompareLong(const void* arg1, const void* arg2);
int		CompareLongReverse(const void* arg1, const void* arg2);
uint8	GetCrumb(uint16 iCrumb, void* pvArray);  //return 11, 10, 01 or 00.
uint8	GetNybble(uint16 iNybble, void* pvArray);
void 	CopyBits(void* pvDest, size iDestOffset, void* pvSource, size iSourceOffset, size iNumBits, bool bClear);  //Setting clear will make all unused bits zero.
void 	CopyBitsReverseHiLo(void* pvDest, size iDestOffset, void* pvSource, size iSourceOffset, size iNumBits, bool bClear);  //Setting clear will make all unused bits zero.
int16	ReverseShortEndianness(int16 s);
int32	ReverseIntEndianness(int32 i);
int64	ReverseLongEndianness(int64 i);
void	ReverseEndianness(void* pv, size iSize);
void	ReverseBytes(void* pv, size iSize);
size 	CalculateStride(size iElementSize, uint16 iAlignment);
uint8 	CalculateOffset(int16 iOffset, uint16 iAlignment);
int32	IntAbs(int32 i);
uint32	GetPowerOf2SizeDown(uint32 iInt);
uint32	GetPowerOf2SizeUp(uint32 iInt);
uint32	GetPowerOf2SizeAuto(uint32 iInt);
int32	LargestInt(int32* pInts, int32 iCount);


#endif // __INTEGER_HELPER_H__

