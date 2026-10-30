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
#ifdef _MSC_VER
#include <intrin.h>
#endif // _MSC_VER
#include "ArrayElementNotFound.h"
#include "IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//  Returns the Nearest power of two to this number rounded down.
//	eg: 1025 become 1024 but 1024 becomes 1024.  (also 2047 becomes 1024).
//
//////////////////////////////////////////////////////////////////////////
uint32 TruncateLowBits(uint32 i)
{
	if (i == 0)
	{
		return 0;
	}
	return (uint32)1 << GetLogBase2(i);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsPowerOfTwo(uint32 i)
{
	uint32	iPow2;

	iPow2 = TruncateLowBits(i);
	if (iPow2 == i)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//  Returns the Nearest power of two to this number rounded up.
//	eg: 1025 become 2048 but 1024 becomes 1024.  (also 1023 becomes 1024).
//
//////////////////////////////////////////////////////////////////////////
uint32 GetBestHighBit(uint32 i)
{
	uint32 i2;

	i2 = i;
	i = TruncateLowBits(i);
	if (i != i2)
	{
		return i << 1;
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//	Count how many bits are not used in zero extending a DWORD.
//	eg: 00011111 becomes 4 and 00010000 also becomes 4.
//
//////////////////////////////////////////////////////////////////////////
size GetLogBase2(uint32 i)
{
	size c;

	c = 32;
	do
	{
		c--;
		if (GetBit(c, &i))
		{
			return c;
		}
	}
	while (c != 0);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//  Returns the byte at position ipos in a DWORD.  Useful for colours.
//	eg:  iInt = 0x00abcdef, iPos = 1 returns 0x000000cd.
//	eg:  iInt = 0x00abcdef, iPos = 2 returns 0x000000ab.
//
//////////////////////////////////////////////////////////////////////////
uint8 GetByte(uint32 iInt, uint16 uiPos)
{
	//This can be optimised

	uiPos <<= 3;
	iInt >>= uiPos;
	return iInt;
}


//////////////////////////////////////////////////////////////////////////
//
//	Same as above but sets the byte.
//
//////////////////////////////////////////////////////////////////////////
uint32 SetByte(uint8 c, uint16 iPos)
{
	//This can be optimised

	uint32 uiC;

	uiC = c;
	iPos <<= 8;
	uiC <<= iPos;
	return uiC;
}


//////////////////////////////////////////////////////////////////////////
//
//	Gets the state of a bit in an array given the bit position.
//	No bounds checking is done.
//
//////////////////////////////////////////////////////////////////////////
bool GetBit(size iBit, void* pvBitArray)
{
	//This can be optimised

	return ((uint8*)pvBitArray)[iBit / 8] & (1 << (iBit % 8));
}


//////////////////////////////////////////////////////////////////////////
//
//	Gets the state of a bit in an array given the bit position.
//	No bounds checking is done.
//
//////////////////////////////////////////////////////////////////////////
bool GetBitReverseHiLo(lsize iBit, void* pvBitArray)
{
	//This can be optimised

	return ((uint8*)pvBitArray)[iBit / 8] & (1 << (7 - (iBit % 8)));
}


//////////////////////////////////////////////////////////////////////////
//
//	Sets the state of a bit in an array given the bit position and state.
//	No bounds checking is done.
//
//////////////////////////////////////////////////////////////////////////
void SetBit(size iBit, void* pvBitArray, bool bBit)
{
	//This can be optimised

	if (bBit)
	{
		((uint8*)pvBitArray)[iBit / 8] |= (1 << (iBit % 8));
	}
	else
	{
		((uint8*)pvBitArray)[iBit / 8] &= ~(1 << (iBit % 8));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	GetBit for a bit position past what a natural size counts.
//
//////////////////////////////////////////////////////////////////////////
bool GetBitLarge(lsize iBit, void* pvBitArray)
{
	return ((uint8*)pvBitArray)[iBit / 8] & (1 << (iBit % 8));
}


//////////////////////////////////////////////////////////////////////////
//
//	SetBit for a bit position past what a natural size counts.
//
//////////////////////////////////////////////////////////////////////////
void SetBitLarge(lsize iBit, void* pvBitArray, bool bBit)
{
	if (bBit)
	{
		((uint8*)pvBitArray)[iBit / 8] |= (1 << (iBit % 8));
	}
	else
	{
		((uint8*)pvBitArray)[iBit / 8] &= ~(1 << (iBit % 8));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//  https://en.wikipedia.org/wiki/Hamming_weight
//
//  This uses fewer arithmetic operations than any other known  
//  implementation on machines with slow multiplication.
//  This algorithm uses 17 arithmetic operations.
//
//////////////////////////////////////////////////////////////////////////
size CountBits_PopCount64(uint64 x)
{
	const uint64 m1 = 0x5555555555555555LL;
	const uint64 m2 = 0x3333333333333333LL;
	const uint64 m4 = 0x0f0f0f0f0f0f0f0fLL;
	const uint64 m5 = 0x0101010101010101LL;

	x = x - ((x >> 1) & m1);        //put count of each 2 bits into those 2 bits
	x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
	x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
	return (x * m5) >> 56;
}


//////////////////////////////////////////////////////////////////////////
//
//  https://stackoverflow.com/questions/14780928/count-number-of-bits-in-an-uint32-integer
//
//////////////////////////////////////////////////////////////////////////
size CountBits_PopCount32(uint32 i)
{
	const uint32 m1 = 0x55555555;
	const uint32 m2 = 0x33333333;
	const uint32 m4 = 0x0f0f0f0f;

	i = i - ((i >> 1) & m1);
	i = (i & m2) + ((i >> 2) & m2);
	i = ((i + (i >> 4)) & m4);
	return (i * 0x01010101) >> 24;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CountBits_PopCount16(uint16 i)
{
	const uint16 m1 = 0x5555;
	const uint16 m2 = 0x3333;
	const uint16 m4 = 0x0f0f;

	i = i - ((i >> 1) & m1);
	i = (i & m2) + ((i >> 2) & m2);
	i = ((i + (i >> 4)) & m4);
	return (uint16)(i * 0x0101) >> 8;
}


//////////////////////////////////////////////////////////////////////////
//
//  https://stackoverflow.com/questions/30688465/how-to-check-the-number-of-set-bits-in-an-8-bit-uint32-char
//
//////////////////////////////////////////////////////////////////////////
size CountBits_PopCount8(uint8 b)
{
	b = b - ((b >> 1) & 0x55);
	b = (b & 0x33) + ((b >> 2) & 0x33);
	return (((b + (b >> 4)) & 0x0f) * 0x01);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CountBitsSingly(void* pvBitArray, size iBitLength)
{
	size	iCount;
	size	i;

	for (i = 0, iCount = 0; i < iBitLength; i++)
	{
		if (GetBit(i, pvBitArray))
		{
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CountDigits(int16 i)
{
	int16	iCmp;
	size	iDigits;

	i = Absolute(i);
	if (i >= 10000)
	{
		return 5;
	}

	iCmp = 1;
	for (iDigits = 0; iDigits < 5; iDigits++)
	{
		if (i < iCmp)
		{
			return iDigits;
		}
		iCmp *= 10;
	}
	return iDigits;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CountDigits(int32 i)
{
	int32	iCmp;
	size	iDigits;

	i = Absolute(i);
	if (i >= 1000000000)
	{
		return 10;
	}

	iCmp = 1;
	for (iDigits = 0; iDigits < 10; iDigits++)
	{
		if (i < iCmp)
		{
			return iDigits;
		}
		iCmp *= 10;
	}
	return iDigits;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CountDigits(int64 i)
{
	int64	iCmp;
	size	iDigits;

	i = Absolute(i);
	if (i >= 1000000000000000000LL)
	{
		return 19;
	}

	iCmp = 1;
	for (iDigits = 0; iDigits < 19; iDigits++)
	{
		if (i < iCmp)
		{
			return iDigits;
		}
		iCmp *= 10;
	}
	return iDigits;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CountBits(void* pvBitArray, size iBitLength)
{
	if (iBitLength == 32)
	{
		return CountBits_PopCount32(*((uint32*)pvBitArray));
	}
	else if (iBitLength == 64)
	{
		return CountBits_PopCount64(*((uint64*)pvBitArray));
	}
	else if (iBitLength == 128)
	{
		return CountBits_PopCount64(*((uint64*)pvBitArray)) + 
		       CountBits_PopCount64(((uint64*)pvBitArray)[1]);
	}
	else if (iBitLength == 8)
	{
		return CountBits_PopCount8(*((uint8*)pvBitArray));
	}
	else if (iBitLength == 16)
	{
		return CountBits_PopCount16(*((uint16*)pvBitArray));
	}
	else
	{
		size	iCount;
		size	iWords;
		size	i;

		//Count whole 32 bit words then the left over bits.
		iWords = iBitLength / 32;
		iCount = 0;
		for (i = 0; i < iWords; i++)
		{
			iCount += CountBits_PopCount32(((uint32*)pvBitArray)[i]);
		}
		return iCount + CountBitsSingly(&((uint32*)pvBitArray)[iWords], iBitLength % 32);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetBits(size iStart, void* pvBitArray, bool bBit, size iLength)
{
	size	i;
	size	j;

	for (i = 0, j = iStart; i < iLength; i++, j++)
	{
		SetBit(j, pvBitArray, bBit);
	}
}


#ifndef __W65C816__
//////////////////////////////////////////////////////////////////////////
//
//	iArray size is in bits
//
//////////////////////////////////////////////////////////////////////////
size FindFirstSetBit(void* pvArray, size iArraySize)
{
	//Use _BitScanForward (MSVC) or __builtin_ffs (GCC) rather.

	size	iIntSize;
	size	i;
	size	iRemainder;
	size	iCharSize;
	size	iStart;
	size	iEnd;
	bool	bFound;
	uint8	c;

	iIntSize = iArraySize / 32;
	iRemainder = iArraySize % 32;

	for (i = 0; i < iIntSize; i++)
	{
		if (((uint32*)pvArray)[i] != 0x00000000)
		{
			break;
		}
	}


	iCharSize = iRemainder / 8;
	iRemainder = iRemainder % 8;
	iStart = i * 4;
	iEnd = iIntSize * 4 + iCharSize;

	bFound = false;
	for (i = iStart; i < iEnd; i++)
	{
		c = ((uint8*)pvArray)[i];
		if (c != (uint8)0x00)
		{
			bFound = true;
			break;
		}
	}

	if ((i < iEnd) || (iRemainder == 0))
	{
		if (!bFound)
		{
			return ARRAY_ELEMENT_NOT_FOUND;
		}

		c = ((uint8*)pvArray)[i];
		iStart = i * 8;
		uint8 ucCmp = 1;

		if ((c & ucCmp))
			return iStart;
		ucCmp <<= 1;
		if ((c & ucCmp))
			return iStart + 1;
		ucCmp <<= 1;
		if ((c & ucCmp))
			return iStart + 2;
		ucCmp <<= 1;
		if ((c & ucCmp))
			return iStart + 3;
		ucCmp <<= 1;
		if ((c & ucCmp))
			return iStart + 4;
		ucCmp <<= 1;
		if ((c & ucCmp))
			return iStart + 5;
		ucCmp <<= 1;
		if ((c & ucCmp))
			return iStart + 6;
		ucCmp <<= 1;
		if ((c & ucCmp))
			return iStart + 7;
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	else
	{
		c = ((uint8*)pvArray)[i];
		iStart = i * 8;
		uint8 ucCmp = 1;

		for (i = 0; i < iRemainder; i++)
		{
			if ((c & ucCmp))
				return iStart + i;
			ucCmp <<= 1;
		}
		return ARRAY_ELEMENT_NOT_FOUND;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	array size is in bits
//
//////////////////////////////////////////////////////////////////////////
size FindFirstClearBit(void* pvArray, size iArraySize)
{
	size	iIntSize;
	size	i;
	size	iRemainder;
	size	iCharSize;
	size	iStart;
	size	iEnd;
	bool	bFound;
	uint8	c;

	iIntSize = iArraySize / 32;
	iRemainder = iArraySize % 32;

	for (i = 0; i < iIntSize; i++)
	{
		if (((uint32*)pvArray)[i] != 0xffffffff)
		{
			break;
		}
	}

	iCharSize = iRemainder / 8;
	iRemainder = iRemainder % 8;
	iStart = i * 4;
	iEnd = iIntSize * 4 + iCharSize;

	bFound = false;
	for (i = iStart; i < iEnd; i++)
	{
		c = ((uint8*)pvArray)[i];
		if (c != (uint8)0xff)
		{
			bFound = true;
			break;
		}
	}

	if ((i < iEnd) || (iRemainder == 0))
	{
		if (!bFound)
		{
			return ARRAY_ELEMENT_NOT_FOUND;
		}

		c = ((uint8*)pvArray)[i];
		iStart = i * 8;
		uint8 ucCmp = 1;

		if (!(c & ucCmp))
			return iStart;
		ucCmp <<= 1;
		if (!(c & ucCmp))
			return iStart + 1;
		ucCmp <<= 1;
		if (!(c & ucCmp))
			return iStart + 2;
		ucCmp <<= 1;
		if (!(c & ucCmp))
			return iStart + 3;
		ucCmp <<= 1;
		if (!(c & ucCmp))
			return iStart + 4;
		ucCmp <<= 1;
		if (!(c & ucCmp))
			return iStart + 5;
		ucCmp <<= 1;
		if (!(c & ucCmp))
			return iStart + 6;
		ucCmp <<= 1;
		if (!(c & ucCmp))
			return iStart + 7;
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	else
	{
		c = ((uint8*)pvArray)[i];
		iStart = i * 8;
		uint8 ucCmp = 1;

		for (i = 0; i < iRemainder; i++)
		{
			if (!(c & ucCmp))
				return iStart + i;
			ucCmp <<= 1;
		}
		return ARRAY_ELEMENT_NOT_FOUND;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size FindFirstClearBit(uint32 uiBits)
{
#ifdef _MSC_VER
	unsigned long	uiIndex;

	_BitScanForward(&uiIndex, ~uiBits);
	return uiIndex;
#else
	return __builtin_ctz(~uiBits);
#endif // _MSC_VER
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size FindFirstSetBit(uint64 uiBits)
{
#if defined(_MSC_VER) && defined(BUILD_64)
	unsigned long	uiIndex;

	_BitScanForward64(&uiIndex, uiBits);
	return uiIndex;
#elif defined(_MSC_VER)
	unsigned long	uiIndex;

	if ((uint32)uiBits != 0)
	{
		_BitScanForward(&uiIndex, (uint32)uiBits);
		return uiIndex;
	}
	_BitScanForward(&uiIndex, (uint32)(uiBits >> 32));
	return uiIndex + 32;
#else
	return __builtin_ctzll(uiBits);
#endif // _MSC_VER
}
#endif // __W65C816__ (Libc65's string816_16.asm has them)


//////////////////////////////////////////////////////////////////////////
//
//	iArray size is in bits
//
//////////////////////////////////////////////////////////////////////////
size FindNextSetBit(void* pvArray, size iArraySize, size iStartBit)
{
	size	i;
	size	iRemainder;
	size	iStart;
	uint8	c;
	size	iResult;
	size	iStop;
	size	ij;
	size	iStartEight;

	iRemainder = iStartBit % 8;
	iStart = iStartBit / 8;

	if (iRemainder != 0)
	{
		iStop = iArraySize < 8 ? iArraySize : 8;

		c = ((uint8*)pvArray)[iStart];
		ij = iStart * 8;
		uint8 ucCmp = 1 << iRemainder;

		for (i = iRemainder; i < iStop; i++)
		{
			if ((c & ucCmp))
				return ij + i;
			ucCmp <<= 1;
		}

		iStartEight = iStart * 8 + 8;
		if (iStartEight < iArraySize)
		{
			iResult = FindFirstSetBit(&((uint8*)pvArray)[iStart + 1], iArraySize - iStartEight);
			if (iResult != ARRAY_ELEMENT_NOT_FOUND)
			{
				return iResult + iStartEight;
			}
		}
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	else
	{
		iStartEight = iStartBit;
		iResult = FindFirstSetBit(&((uint8*)pvArray)[iStart], iArraySize - iStartEight);
		if (iResult != ARRAY_ELEMENT_NOT_FOUND)
		{
			return iResult + iStartEight;
		}
		return iResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	Array size is in bits
//
//////////////////////////////////////////////////////////////////////////
size FindNextClearBit(void* pvArray, size iArraySize, size iStartBit)
{
	size	i;
	size	iRemainder;
	size	iStart;
	uint8	c;
	size	iResult;
	size	iStop;
	size	ij;
	size	iStartEight;

	iRemainder = iStartBit % 8;
	iStart = iStartBit / 8;

	if (iRemainder != 0)
	{
		iStop = iArraySize < 8 ? iArraySize : 8;

		c = ((uint8*)pvArray)[iStart];
		ij = iStart * 8;
		uint8 ucCmp = 1 << iRemainder;

		for (i = iRemainder; i < iStop; i++)
		{
			if (!(c & ucCmp))
			{
				return ij + i;
			}
			ucCmp <<= 1;
		}

		iStartEight = iStart * 8 + 8;
		if (iStartEight < iArraySize)
		{
			iResult = FindFirstClearBit(&((uint8*)pvArray)[iStart + 1], iArraySize - iStartEight);
			if (iResult != ARRAY_ELEMENT_NOT_FOUND)
			{
				return iResult + iStartEight;
			}
		}
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	else
	{
		iStartEight = iStartBit;
		iResult = FindFirstClearBit(&((uint8*)pvArray)[iStart], iArraySize - iStartEight);
		if (iResult != ARRAY_ELEMENT_NOT_FOUND)
		{
			return iResult + iStartEight;
		}
		return iResult;
	}
}


#ifndef __W65C816__
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size FindLastClearBit(void* pvArray, size iArraySize)
{
	size	i;
	bool	iBit;
	
	if (iArraySize != 0)
	{
		i = iArraySize;
		do
		{
			i--;
			iBit = GetBit(i, pvArray);
			if (iBit == 0)
			{
				return i;
			}
		}
		while (i != 0);
	}

	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size FindLastSetBit(void* pvArray, size iArraySize)
{
	size	i;
	bool	iBit;

	if (iArraySize != 0)
	{
		i = iArraySize;
		do
		{
			i--;
			iBit = GetBit(i, pvArray);
			if (iBit)
			{
				return i;
			}
		}
		while (i != 0);
	}

	return ARRAY_ELEMENT_NOT_FOUND;
}
#endif // __W65C816__ (Libc65's string816_16.asm has them)


#ifndef __W65C816__
//////////////////////////////////////////////////////////////////////////
//
//	The lowest bit of the first iArraySize bits of pvArray that is set
//	after the bytes are xored with uiInvert.
//
//////////////////////////////////////////////////////////////////////////
static lsize FindFirstBitLarge(void* pvArray, lsize iArraySize, uint8 uiInvert)
{
	lsize	iBytes;
	lsize	i;
	uint8	c;
	uint8	uiRemainder;

	iBytes = iArraySize / 8;
	for (i = 0; i < iBytes; i++)
	{
		c = (uint8)(((uint8*)pvArray)[i] ^ uiInvert);
		if (c != 0)
		{
			return i * 8 + FindFirstSetBit((uint64)c);
		}
	}

	uiRemainder = (uint8)(iArraySize % 8);
	if (uiRemainder != 0)
	{
		c = (uint8)((((uint8*)pvArray)[i] ^ uiInvert) & ((1 << uiRemainder) - 1));
		if (c != 0)
		{
			return i * 8 + FindFirstSetBit((uint64)c);
		}
	}
	return ARRAY_LARGE_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//	The highest bit of the first iArraySize bits of pvArray that is set
//	after the bytes are xored with uiInvert.
//
//////////////////////////////////////////////////////////////////////////
static lsize FindLastBitLarge(void* pvArray, lsize iArraySize, uint8 uiInvert)
{
	lsize	i;
	uint8	c;
	uint8	uiRemainder;
	uint8	uiBit;

	i = iArraySize / 8;
	uiRemainder = (uint8)(iArraySize % 8);
	if (uiRemainder != 0)
	{
		c = (uint8)((((uint8*)pvArray)[i] ^ uiInvert) & ((1 << uiRemainder) - 1));
	}
	else
	{
		c = 0;
	}

	for (;;)
	{
		if (c != 0)
		{
			uiBit = 7;
			while ((c & (1 << uiBit)) == 0)
			{
				uiBit--;
			}
			return i * 8 + uiBit;
		}
		if (i == 0)
		{
			return ARRAY_LARGE_ELEMENT_NOT_FOUND;
		}
		i--;
		c = (uint8)(((uint8*)pvArray)[i] ^ uiInvert);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	Array size is in bits
//
//////////////////////////////////////////////////////////////////////////
lsize FindFirstSetBitLarge(void* pvArray, lsize iArraySize)
{
	return FindFirstBitLarge(pvArray, iArraySize, 0x00);
}


//////////////////////////////////////////////////////////////////////////
//
//	Array size is in bits
//
//////////////////////////////////////////////////////////////////////////
lsize FindFirstClearBitLarge(void* pvArray, lsize iArraySize)
{
	return FindFirstBitLarge(pvArray, iArraySize, 0xff);
}


//////////////////////////////////////////////////////////////////////////
//
//	Array size is in bits
//
//////////////////////////////////////////////////////////////////////////
lsize FindLastSetBitLarge(void* pvArray, lsize iArraySize)
{
	return FindLastBitLarge(pvArray, iArraySize, 0x00);
}


//////////////////////////////////////////////////////////////////////////
//
//	Array size is in bits
//
//////////////////////////////////////////////////////////////////////////
lsize FindLastClearBitLarge(void* pvArray, lsize iArraySize)
{
	return FindLastBitLarge(pvArray, iArraySize, 0xff);
}
#endif // __W65C816__ (Libc65's string816.asm has them)


//////////////////////////////////////////////////////////////////////////
//
//	Array size is in bits
//
//////////////////////////////////////////////////////////////////////////
lsize FindNextSetBitLarge(void* pvArray, lsize iArraySize, lsize iStartBit)
{
	lsize	iResult;

	if (iStartBit >= iArraySize)
	{
		return ARRAY_LARGE_ELEMENT_NOT_FOUND;
	}
	while ((iStartBit % 8) != 0)
	{
		if (GetBitLarge(iStartBit, pvArray))
		{
			return iStartBit;
		}
		iStartBit++;
		if (iStartBit == iArraySize)
		{
			return ARRAY_LARGE_ELEMENT_NOT_FOUND;
		}
	}

	iResult = FindFirstSetBitLarge(&((uint8*)pvArray)[iStartBit / 8], iArraySize - iStartBit);
	if (iResult != ARRAY_LARGE_ELEMENT_NOT_FOUND)
	{
		return iResult + iStartBit;
	}
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//	Array size is in bits
//
//////////////////////////////////////////////////////////////////////////
lsize FindNextClearBitLarge(void* pvArray, lsize iArraySize, lsize iStartBit)
{
	lsize	iResult;

	if (iStartBit >= iArraySize)
	{
		return ARRAY_LARGE_ELEMENT_NOT_FOUND;
	}
	while ((iStartBit % 8) != 0)
	{
		if (!GetBitLarge(iStartBit, pvArray))
		{
			return iStartBit;
		}
		iStartBit++;
		if (iStartBit == iArraySize)
		{
			return ARRAY_LARGE_ELEMENT_NOT_FOUND;
		}
	}

	iResult = FindFirstClearBitLarge(&((uint8*)pvArray)[iStartBit / 8], iArraySize - iStartBit);
	if (iResult != ARRAY_LARGE_ELEMENT_NOT_FOUND)
	{
		return iResult + iStartBit;
	}
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlagInt(uint32* piDest, uint32 iFlag, bool iFlagValue)
{
	//If the value is true then or it with dest.
	if (iFlagValue)
	{
		*piDest |= iFlag;
	}
	//If the value is false then negate and and it with dest.
	else
	{
		*piDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlagShort(uint16* psiDest, uint16 iFlag, bool iFlagValue)
{
	if (iFlagValue)
	{
		*psiDest |= iFlag;
	}
	else
	{
		*psiDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlagByte(uint8* psiDest, uint8 iFlag, bool iFlagValue)
{
	if (iFlagValue)
	{
		*psiDest |= iFlag;
	}
	else
	{
		*psiDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlagLong(uint64* piDest, uint64 iFlag, bool iFlagValue)
{
	if (iFlagValue)
	{
		*piDest |= iFlag;
	}
	else
	{
		*piDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlagInt(int32* piDest, int32 iFlag, bool iFlagValue)
{
	//If the value is true then or it with dest.
	if (iFlagValue)
	{
		*piDest |= iFlag;
	}
	//If the value is false then negate and and it with dest.
	else
	{
		*piDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlagShort(int16* psiDest, int16 iFlag, bool iFlagValue)
{
	if (iFlagValue)
	{
		*psiDest |= iFlag;
	}
	else
	{
		*psiDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlagByte(int8* psiDest, int8 iFlag, bool iFlagValue)
{
	if (iFlagValue)
	{
		*psiDest |= iFlag;
	}
	else
	{
		*psiDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlagLong(int64* piDest, int64 iFlag, bool iFlagValue)
{
	if (iFlagValue)
	{
		*piDest |= iFlag;
	}
	else
	{
		*piDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlagSize(size* piDest, size iFlag, bool iFlagValue)
{
	if (iFlagValue)
	{
		*piDest |= iFlag;
	}
	else
	{
		*piDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapInt16(int16* pi1, int16* pi2)
{
	int16 i3;

	i3 = *pi1;
	*pi1 = *pi2;
	*pi2 = i3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapUint16(uint16* pi1, uint16* pi2)
{
	uint16 i3;

	i3 = *pi1;
	*pi1 = *pi2;
	*pi2 = i3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapInt32(int32* pi1, int32* pi2)
{
	int32 i3;

	i3 = *pi1;
	*pi1 = *pi2;
	*pi2 = i3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapUint32(uint32* pi1, uint32* pi2)
{
	uint32 i3;

	i3 = *pi1;
	*pi1 = *pi2;
	*pi2 = i3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapInt(int* pi1, int* pi2)
{
	int i3;

	i3 = *pi1;
	*pi1 = *pi2;
	*pi2 = i3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapInt(uint* pi1, uint* pi2)
{
	uint i3;

	i3 = *pi1;
	*pi1 = *pi2;
	*pi2 = i3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapByte(uint8* pc1, uint8* pc2)
{
	char c3;

	c3 = *pc1;
	*pc1= *pc2;
	*pc2 = c3;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapSize(size* pui1, size* pui2)
{
	size temp;

	temp = *pui2;
	*pui2 = *pui1;
	*pui1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapBit(uint8* puiByte, size uiBit1, size uiBit2)
{
	bool	bBit1;
	bool	bBit2;

	bBit1 = GetBit(uiBit1, puiByte);
	bBit2 = GetBit(uiBit2, puiByte);
	SetBit(uiBit1, puiByte, bBit2);
	SetBit(uiBit2, puiByte, bBit1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SwapBit(uint16* puiByte, size uiBit1, size uiBit2)
{
	bool	bBit1;
	bool	bBit2;

	bBit1 = GetBit(uiBit1, puiByte);
	bBit2 = GetBit(uiBit2, puiByte);
	SetBit(uiBit1, puiByte, bBit2);
	SetBit(uiBit2, puiByte, bBit1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size FindFirstInt(int32* piIntArray, int32 iSearch, size iMaxLength)
{
	size	i;

	for (i = 0; i < iMaxLength; i++)
	{
		if (piIntArray[i] == iSearch)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize FindFirstByte(uint8* pcCharArray, uint8 uiSearch, lsize iMaxLength)
{
	lsize	i;

	for (i = 0; i < iMaxLength; i++)
	{
		if (pcCharArray[i] == uiSearch)
		{
			return i;
		}
	}
	return ARRAY_LARGE_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 GetHighNybble(uint8 c)
{
	return ((int32)c >> 4) & 0xf;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 GetLowNybble(uint8 c)
{
	return (int32)c & 0xf;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 IntAbs(int32 i)
{
	if (i >= 0)
	{
		return i;
	}
	return -i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareByte(const void* arg1, const void* arg2)
{
	if ((*((int8*)arg1)) < (*((int8*)arg2)))
	{
		return -1;
	}
	if ((*((int8*)arg1)) > (*((int8*)arg2)))
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareByteReverse(const void* arg1, const void* arg2)
{
	if ((*((int16*)arg1)) < (*((int16*)arg2)))
	{
		return 1;
	}
	if ((*((int16*)arg1)) > (*((int16*)arg2)))
	{
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareShort(const void* arg1, const void* arg2)
{
	if ((*((int16*)arg1)) < (*((int16*)arg2)))
	{
		return -1;
	}
	if ((*((int16*)arg1)) > (*((int16*)arg2)))
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareShortReverse(const void* arg1, const void* arg2)
{
	if ((*((int16*)arg1)) < (*((int16*)arg2)))
	{
		return 1;
	}
	if ((*((int16*)arg1)) > (*((int16*)arg2)))
	{
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareInt(const void* arg1, const void* arg2)
{
	if ((*((int*)arg1)) < (*((int*)arg2)))
	{
		return -1;
	}
	if ((*((int*)arg1)) > (*((int*)arg2)))
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
// size keys: not the same width as an int everywhere.					//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareSize(const void* arg1, const void* arg2)
{
	if ((*((size*)arg1)) < (*((size*)arg2)))
	{
		return -1;
	}
	if ((*((size*)arg1)) > (*((size*)arg2)))
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareIntReverse(const void* arg1, const void* arg2)
{
	if ((*((int*)arg1)) < (*((int*)arg2)))
	{
		return 1;
	}
	if ((*((int*)arg1)) > (*((int*)arg2)))
	{
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareLong(const void* arg1, const void* arg2)
{
	if ((*((int64*)arg1)) < (*((int64*)arg2)))
	{
		return -1;
	}
	if ((*((int64*)arg1)) > (*((int64*)arg2)))
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CompareLongReverse(const void* arg1, const void* arg2)
{
	if ((*((int64*)arg1)) < (*((int64*)arg2)))
	{
		return 1;
	}
	if ((*((int64*)arg1)) > (*((int64*)arg2)))
	{
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint8 GetCrumb(size iCrumb, void* pvArray)
{
	size iBytePos;
	size iCrumbInByte;

	iBytePos = iCrumb / 4;
	iCrumbInByte = iCrumb % 4;

	return (((uint8*)pvArray)[iBytePos] >> (2 * iCrumbInByte)) & 0x3;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint8 GetNybble(size iNybble, void* pvArray)
{
	size iBytePos;
	size iNybbleInByte;

	iBytePos = iNybble / 2;
	iNybbleInByte = iNybble % 2;

	return (((uint8*)pvArray)[iBytePos] >> (4 * iNybbleInByte)) & 0xf;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CopyBits(void* pvDest, lsize iDestOffset, void* pvSource, lsize iSourceOffset, lsize iNumBits, bool bClear)
{
	lsize	i;
	bool	bBit;
	lsize	iRemaining;
	lsize	iLast;

	//Optimise this later...  Use bit shifting and friends.
	for (i = 0; i < iNumBits; i++)
	{
		bBit = GetBitLarge(iSourceOffset + i, pvSource);
		SetBitLarge(iDestOffset + i, pvDest, bBit);
	}

	if (bClear)
	{
		iLast = i;

		//Optimise this later...  Just AND the end off.
		iRemaining = (8 - ((iDestOffset + iNumBits) % 8)) % 8;
		for (i = 0; i < iRemaining; i++)
		{
			SetBitLarge(iDestOffset + iLast + i, pvDest, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CopyBitsReverseHiLo(void* pvDest, lsize iDestOffset, void* pvSource, lsize iSourceOffset, lsize iNumBits, bool bClear)
{
	lsize	i;
	bool	bBit;
	lsize	iRemaining;
	lsize	iLast;

	//This reverses bits in a byte.  Not the whole bitstream.  That's a much more difficult problem to solve.
	//This function is useless if it is not fully 8bits aligned.

	//Optimise this later...  Use bit shifting and friends.
	for (i = 0; i < iNumBits; i++)
	{
		bBit = GetBitReverseHiLo(iSourceOffset + i, pvSource);
		SetBitLarge(iDestOffset + i, pvDest, bBit);
	}

	if (bClear)
	{
		iLast = i;

		//Optimise this later...  Just AND the end off.
		iRemaining = (8 - ((iDestOffset + iNumBits) % 8)) % 8;
		for (i = 0; i < iRemaining; i++)
		{
			SetBitLarge(iDestOffset + iLast + i, pvDest, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int16 ReverseShortEndianness(int16 s)
{
	return (int16)ReverseShortEndianness((uint16)s);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 ReverseIntEndianness(int32 i)
{
	return (int32)ReverseIntEndianness((uint32)i);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int64 ReverseLongEndianness(int64 i)
{
	return (int64)ReverseLongEndianness((uint64)i);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint16 ReverseShortEndianness(uint16 s)
{
	SwapByte((uint8*)&s, ((uint8*)&s) + 1);
	return s;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint32 ReverseIntEndianness(uint32 i)
{
	SwapByte(((uint8*)&i + 0), ((uint8*)&i) + 3);
	SwapByte(((uint8*)&i + 1), ((uint8*)&i) + 2);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint64 ReverseLongEndianness(uint64 i)
{
	SwapByte(((uint8*)&i + 0), ((uint8*)&i) + 7);
	SwapByte(((uint8*)&i + 1), ((uint8*)&i) + 6);
	SwapByte(((uint8*)&i + 2), ((uint8*)&i) + 5);
	SwapByte(((uint8*)&i + 3), ((uint8*)&i) + 4);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ReverseEndianness(void* pv, size iSize)
{
	if (iSize == 4)
	{
		*(uint32*)pv = ReverseIntEndianness(*(uint32*)pv);
	}
	if (iSize == 2)
	{
		*(int16*)pv = ReverseShortEndianness(*(int16*)pv);
	}
	if (iSize == 8)
	{
		*(int64*)pv = ReverseLongEndianness(*(int64*)pv);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ReverseBytes(void* pv, size iSize)
{
	size	i;
	size	iHalf;
	
	iHalf = iSize / 2;
	for (i = 0; i < iHalf; i++)
	{
		SwapByte(&(((uint8*)pv)[i]), &(((uint8*)pv)[iSize - i - 1]));
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CalculateStride(size iElementSize, uint16 iAlignment)
{
	int32		iByteDiff;
	int32		iStride;

	iByteDiff = (int)(iElementSize % iAlignment);
	if (iByteDiff == 0)
	{
		return iElementSize;
	}
	else
	{
		iByteDiff = iAlignment - iByteDiff;
		iStride = iByteDiff + (int)iElementSize;
		return iStride;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint8 CalculateOffset(int16 iOffset, uint16 iAlignment)
{
	if ((iOffset == -8) && ((iAlignment == 4) || (iAlignment == 8)))
	{
		return 0;
	}

	//Signed (where an int is 16 bits a uint16 would make the remainder unsigned).
	if (iOffset < 0)
	{
		return (uint8)(((int32)iAlignment + ((int32)iOffset % (int32)iAlignment)) % (int32)iAlignment);
	}
	else
	{
		return (uint8)((int32)iOffset % (int32)iAlignment);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 GetPowerOf2SizeDown(uint32 iInt)
{
	return TruncateLowBits(iInt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 GetPowerOf2SizeUp(uint32 iInt)
{
	return GetBestHighBit(iInt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 GetPowerOf2SizeAuto(uint32 iInt)
{
	return TruncateLowBits((uint32)(iInt * 1.5f));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 LargestInt(int32* pInts, int32 iCount)
{
	int32	iLargest;
	int32 i;

	iLargest = pInts[0];
	for (i = 1; i < iCount; i++)
	{	
		if (pInts[i] > iLargest)
		{
			iLargest = pInts[i];
		}
	}
	return iLargest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 GetLowByte(uint16 uiValue)
{
	return (uint8)uiValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8 GetHighByte(uint16 uiValue)
{
	return (uint8)((uiValue & 0xFF00) >> 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 SetLowByte(uint16 uiVariable, uint8 uiData)
{
	return (uiVariable & 0xFF00) | uiData;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 SetHighByte(uint16 uiVariable, uint8 uiData)
{
	return (uiVariable & 0xFF) | (uiData << 8);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size Min(size ui1, size ui2)
{
	if (ui1 < ui2)
	{
		return ui1;
	}
	else
	{
		return ui2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size Max(size ui1, size ui2)
{
	if (ui1 > ui2)
	{
		return ui1;
	}
	else
	{
		return ui2;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int8 Absolute(int8 i)
{
	if (i < 0)
	{
		i *= -1;
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 Absolute(int16 i)
{
	if (i < 0)
	{
		i *= -1;
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 Absolute(int32 i)
{
	if (i < 0)
	{
		i *= -1;
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 Absolute(int64 i)
{
	if (i < 0)
	{
		i *= -1;
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int16 SquareRoot(int16 i)
{
	int16 iLeft = 0;
	int16 iRight = i;
	int16 iAns = 0;
	int16 iMid;

	if (i < 0)
	{
		return -1;
	}
	if (i < 2)
	{
		return i;
	}

	iLeft = 0;
	iRight = i;
	iAns = 0;

	while (iLeft <= iRight)
	{
		iMid = iLeft + (iRight - iLeft) / 2;

		if (iMid <= i / iMid)
		{
			iAns = iMid;
			iLeft = iMid + 1;
		}
		else 
		{
			iRight = iMid - 1;
		}
	}
	return iAns;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 SquareRoot(int32 i)
{
	int32 iLeft = 0;
	int32 iRight = i;
	int32 iAns = 0;
	int32 iMid;

	if (i < 0)
	{
		return -1;
	}
	if (i < 2)
	{
		return i;
	}

	iLeft = 0;
	iRight = i;
	iAns = 0;

	while (iLeft <= iRight)
	{
		iMid = iLeft + (iRight - iLeft) / 2;

		if (iMid <= i / iMid)
		{
			iAns = iMid;
			iLeft = iMid + 1;
		}
		else 
		{
			iRight = iMid - 1;
		}
	}
	return iAns;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 SquareRoot(int64 i)
{
	int64 iLeft = 0;
	int64 iRight = i;
	int64 iAns = 0;
	int64 iMid;

	if (i < 0)
	{
		return -1;
	}
	if (i < 2)
	{
		return i;
	}

	iLeft = 0;
	iRight = i;
	iAns = 0;

	while (iLeft <= iRight)
	{
		iMid = iLeft + (iRight - iLeft) / 2;

		if (iMid <= i / iMid)
		{
			iAns = iMid;
			iLeft = iMid + 1;
		}
		else 
		{
			iRight = iMid - 1;
		}
	}
	return iAns;
}


#ifndef __W65C816__
//////////////////////////////////////////////////////////////////////////
//
// Each word is added in binary with 6 more in every nibble (x), so a
// nibble's digit sum of 10 or more carries out of it.  The nibbles that
// didn't carry (their carry in bit, found from x ^ y ^ the sum) have the
// 6 taken off again.
//
//////////////////////////////////////////////////////////////////////////
uint16 BCDAdd(uint16* pauiDest, uint16* pauiSource, size uiWords, uint16 uiCarry)
{
	size	i;
	uint32	uiX;
	uint32	uiY;
	uint32	uiSum;
	uint32	uiCarries;
	uint32	uiNone;

	for (i = 0; i < uiWords; i++)
	{
		uiX = (uint32)pauiDest[i] + 0x6666;
		uiY = (uint32)pauiSource[i] + uiCarry;
		uiSum = uiX + uiY;
		uiCarries = (uiSum ^ uiX ^ uiY) & 0x11110;
		uiNone = ~uiCarries & 0x11110;
		pauiDest[i] = (uint16)(uiSum - ((uiNone >> 2) | (uiNone >> 3)));
		uiCarry = (uint16)(uiCarries >> 16);
	}
	return uiCarry;
}


//////////////////////////////////////////////////////////////////////////
//
// Each word is subtracted in binary; the nibbles that borrowed (their
// borrow in bit, found from x ^ y ^ the difference) went up by 16, not 10,
// so 6 is taken off them.
//
//////////////////////////////////////////////////////////////////////////
uint16 BCDSubtract(uint16* pauiDest, uint16* pauiSource, size uiWords, uint16 uiBorrow)
{
	size	i;
	uint32	uiX;
	uint32	uiY;
	uint32	uiDifference;
	uint32	uiBorrows;

	for (i = 0; i < uiWords; i++)
	{
		uiX = pauiDest[i];
		uiY = (uint32)pauiSource[i] + uiBorrow;
		uiDifference = uiX - uiY;
		uiBorrows = (uiDifference ^ uiX ^ uiY) & 0x11110;
		pauiDest[i] = (uint16)(uiDifference - ((uiBorrows >> 2) | (uiBorrows >> 3)));
		uiBorrow = (uint16)(uiBorrows >> 16);
	}
	return uiBorrow;
}
#endif // __W65C816__
