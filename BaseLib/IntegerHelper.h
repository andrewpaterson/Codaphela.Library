/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
bool 	GetBitReverseHiLo(lsize iBit, void* pvBitArray);  //return 1 or 0 in char array of bits indexed in reverse.
size	CountBits(void* pvBitArray, size iBitLength);
size	CountBitsSingly(void* pvBitArray, size iBitLength);  //Slow, for testing.
size	CountDigits(int16 i);
size	CountDigits(int32 i);
size	CountDigits(int64 i);
void 	SetBit(size iBit, void* pvBitArray, bool bBit);
bool 	GetBitLarge(lsize iBit, void* pvBitArray);
void 	SetBitLarge(lsize iBit, void* pvBitArray, bool bBit);
void 	SetBits(size iStart, void* pvBitArray, bool bBit, size iLength);
void 	SetFlagByte(uint8* psiDest, uint8 iFlag, bool iFlagValue);
void 	SetFlagByte(int8* psiDest, int8 iFlag, bool iFlagValue);
void 	SetFlagShort(uint16* psiDest, uint16 iFlag, bool iFlagValue);
void 	SetFlagShort(int16* psiDest, int16 iFlag, bool iFlagValue);
void 	SetFlagInt(uint32* piDest, uint32 iFlag, bool iFlagValue);
void 	SetFlagInt(int32* piDest, int32 iFlag, bool iFlagValue);
void 	SetFlagLong(uint64* psiDest, uint64 iFlag, bool iFlagValue); 
void 	SetFlagLong(int64* psiDest, int64 iFlag, bool iFlagValue); 
void 	SetFlagSize(size* piDest, size iFlag, bool iFlagValue);
size 	FindFirstClearBit(void* pvArray, size iArraySize);  //Libc65's string816_16.asm on the W65C816 (and the Last, uint32 and uint64 ones).
size	FindFirstClearBit(uint32 uiBits);  //uiBits must have a clear bit.
size 	FindFirstSetBit(void* pvArray, size iArraySize);
size	FindFirstSetBit(uint64 uiBits);  //uiBits must have a set bit.
size 	FindLastClearBit(void* pvArray, size iArraySize);
size 	FindLastSetBit(void* pvArray, size iArraySize);
size	FindNextSetBit(void* pvArray, size iArraySize, size iStartBit);
size	FindNextClearBit(void* pvArray, size iArraySize, size iStartBit);
lsize 	FindFirstClearBitLarge(void* pvArray, lsize iArraySize);  //The bit array functions for more bits than a size counts, ARRAY_LARGE_ELEMENT_NOT_FOUND if there is none.
lsize 	FindFirstSetBitLarge(void* pvArray, lsize iArraySize);
lsize 	FindLastClearBitLarge(void* pvArray, lsize iArraySize);
lsize 	FindLastSetBitLarge(void* pvArray, lsize iArraySize);
lsize	FindNextSetBitLarge(void* pvArray, lsize iArraySize, lsize iStartBit);
lsize	FindNextClearBitLarge(void* pvArray, lsize iArraySize, lsize iStartBit);
void 	SwapInt16(int16* pi1, int16* pi2);
void 	SwapUint16(uint16* pi1, uint16* pi2);
void 	SwapInt32(int32* pi1, int32* pi2);
void 	SwapUint32(uint32* pi1, uint32* pi2);
void	SwapInt(int* pi1, int* pi2);
void	SwapInt(uint* pi1, uint* pi2);
void 	SwapByte(uint8* pc1, uint8* pi2);
void	SwapSize(size* pui1, size* pui2);
void	SwapBit(uint8* puiByte, size uiBit1, size uiBit2);
void	SwapBit(uint16* puiByte, size uiBit1, size uiBit2);
size 	FindFirstInt(int32* piIntArray, int32 iSearch, size iMaxLength);
lsize	FindFirstByte(uint8* pcCharArray, uint8 uiSearch, lsize iMaxLength);  //ARRAY_LARGE_ELEMENT_NOT_FOUND if it isn't there.
uint8	GetHighNybble(uint8 c);
uint8	GetLowNybble(uint8 c);
uint8	GetLowByte(uint16 uiValue);
uint8	GetHighByte(uint16 uiValue);
uint16	SetLowByte(uint16 uiVariable, uint8 uiData);
uint16	SetHighByte(uint16 uiVariable, uint8 uiData);
int 	CompareByte(const void* arg1, const void* arg2);
int		CompareByteReverse(const void* arg1, const void* arg2);
int 	CompareShort(const void* arg1, const void* arg2);
int		CompareShortReverse(const void* arg1, const void* arg2);
int 	CompareInt(const void* arg1, const void* arg2);
int		CompareIntReverse(const void* arg1, const void* arg2);
int 	CompareSize(const void* arg1, const void* arg2);
int 	CompareLong(const void* arg1, const void* arg2);
int		CompareLongReverse(const void* arg1, const void* arg2);
uint8	GetCrumb(size iCrumb, void* pvArray);  //return 11, 10, 01 or 00.
uint8	GetNybble(size iNybble, void* pvArray);
void 	CopyBits(void* pvDest, lsize iDestOffset, void* pvSource, lsize iSourceOffset, lsize iNumBits, bool bClear);  //Setting clear will make all unused bits zero.
void 	CopyBitsReverseHiLo(void* pvDest, lsize iDestOffset, void* pvSource, lsize iSourceOffset, lsize iNumBits, bool bClear);  //Setting clear will make all unused bits zero.
int16	ReverseShortEndianness(int16 s);
int32	ReverseIntEndianness(int32 i);
int64	ReverseLongEndianness(int64 i);
uint16	ReverseShortEndianness(uint16 s);
uint32	ReverseIntEndianness(uint32 i);
uint64	ReverseLongEndianness(uint64 i);
void	ReverseEndianness(void* pv, size iSize);
void	ReverseBytes(void* pv, size iSize);
size 	CalculateStride(size iElementSize, uint16 iAlignment);
uint8 	CalculateOffset(int16 iOffset, uint16 iAlignment);
int32	IntAbs(int32 i);
uint32	GetPowerOf2SizeDown(uint32 iInt);
uint32	GetPowerOf2SizeUp(uint32 iInt);
uint32	GetPowerOf2SizeAuto(uint32 iInt);
int32	LargestInt(int32* pInts, int32 iCount);
size	Min(size ui1, size ui2);
size	Max(size ui1, size ui2);
int8	Absolute(int8 i);
int16	Absolute(int16 i);
int32	Absolute(int32 i);
int64	Absolute(int64 i);
int16	SquareRoot(int16 i);
int32	SquareRoot(int32 i);
int64	SquareRoot(int64 i);


//Packed BCD words: four decimal digits a word, the least significant in the low nibble.  BCDAdd adds uiWords words
//of pauiSource to pauiDest (the least significant word first) with a carry in of 0 or 1 and returns the carry out;
//BCDSubtract subtracts them with a borrow.  The W65C816 does them in its decimal mode (its compiler's
//__builtin_bcd_add and __builtin_bcd_subtract).
#ifdef __W65C816__
#define BCDAdd(pauiDest, pauiSource, uiWords, uiCarry)			__builtin_bcd_add(pauiDest, pauiSource, uiWords, uiCarry)
#define BCDSubtract(pauiDest, pauiSource, uiWords, uiBorrow)	__builtin_bcd_subtract(pauiDest, pauiSource, uiWords, uiBorrow)
#else
uint16	BCDAdd(uint16* pauiDest, uint16* pauiSource, size uiWords, uint16 uiCarry);
uint16	BCDSubtract(uint16* pauiDest, uint16* pauiSource, size uiWords, uint16 uiBorrow);
#endif // __W65C816__



//Inline: called often (flag tests) and too small to be worth a call.
inline bool FixBool(int32 i)
{
	return i != 0;
}


inline bool FixBool(void* pv)
{
	return pv != NULL;
}


#endif // __INTEGER_HELPER_H__

