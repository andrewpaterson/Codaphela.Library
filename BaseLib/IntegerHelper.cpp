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
	return 1 << GetLogBase2(i);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsPowerOfTwo(uint32 i)
{
	int32	iPow2;

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
int32 GetBestHighBit(int32 i)
{
	int32 i2;

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
int32 GetLogBase2(int32 i)
{
	int32		c;

	for (c = 31; c >= 0; c--)
	{
		if (GetBit(c, &i))
		{
			return c;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//  Returns the byte at position ipos in a DWORD.  Useful for colours.
//	eg:  iInt = 0x00abcdef, iPos = 1 returns 0x000000cd.
//	eg:  iInt = 0x00abcdef, iPos = 2 returns 0x000000ab.
//
//////////////////////////////////////////////////////////////////////////
int32 GetByte(int32 iInt, int32 iPos)
{
	int32 c;

	iPos *= 8;
	iInt >>= iPos;
	c = (uint8)(iInt & 0xff);
	return c;
}


//////////////////////////////////////////////////////////////////////////
//
//	Same as above but sets the byte.
//
//////////////////////////////////////////////////////////////////////////
int32 SetByte(int32 c, int32 iPos)
{
	iPos *= 8;
	if (c > 256)
	{
		c = 256;
	}
	c <<= iPos;
	return c;
}


//////////////////////////////////////////////////////////////////////////
//
//	Gets the state of a bit in an array given the bit position.
//	No bounds checking is done.
//
//////////////////////////////////////////////////////////////////////////
bool GetBit(int32 iBit, void* pvBitArray)
{
	return ((uint8*)pvBitArray)[iBit / 8] & (1 << (iBit % 8));
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
int32 CountBits_PopCount64(uint64 x)
{
	const uint64 m1 = 0x5555555555555555;
	const uint64 m2 = 0x3333333333333333;
	const uint64 m4 = 0x0f0f0f0f0f0f0f0f;
	const uint64 m5 = 0x0101010101010101;

	x = x - ((x >> 1) & m1);        //put count of each 2 bits into those 2 bits
	x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
	x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
	return (x * m5) >> 56;
}


//////////////////////////////////////////////////////////////////////////
//
//  https://stackoverflow.com/questions/14780928/count-number-of-bits-in-an-unsigned-integer
//
//////////////////////////////////////////////////////////////////////////
int32 CountBits_PopCount32(uint32 i)
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
int32 CountBits_PopCount16(uint16 i)
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
//  https://stackoverflow.com/questions/30688465/how-to-check-the-number-of-set-bits-in-an-8-bit-unsigned-char
//
//////////////////////////////////////////////////////////////////////////
int32 CountBits_PopCount8(uint8 b)
{
	b = b - ((b >> 1) & 0x55);
	b = (b & 0x33) + ((b >> 2) & 0x33);
	return (((b + (b >> 4)) & 0x0F) * 0x01);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CountBitsSingly(void* pvBitArray, int32 iBitLength)
{
	int32 iCount;
	int32	i;

	iCount = 0;
	for (i = 0; i < iBitLength; i++)
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
int32	CountBits(void* pvBitArray, int32 iBitLength)
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
		//Do better here.
		return CountBitsSingly(pvBitArray, iBitLength);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	Gets the state of a bit in an array given the bit position.
//	No bounds checking is done.
//
//////////////////////////////////////////////////////////////////////////
bool GetBitReverseHiLo(int32 iBit, void* pvBitArray)
{
	return ((uint8*)pvBitArray)[iBit / 8] & (1 << (7 - (iBit % 8)));
}


//////////////////////////////////////////////////////////////////////////
//
//	Sets the state of a bit in an array given the bit position and state.
//	No bounds checking is done.
//
//////////////////////////////////////////////////////////////////////////
void SetBit(int32 iBit, void* pvBitArray, bool bBit)
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
//
//////////////////////////////////////////////////////////////////////////
void SetBits(int32 iStart, void* pvBitArray, bool bBit, int32 iLength)
{
	int32	i;

	for (i = 0; i < iLength; i++)
	{
		SetBit(iStart + i, pvBitArray, bBit);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	iArray size is in bits
//
//////////////////////////////////////////////////////////////////////////
int32 FindFirstSetBit(void* pvArray, int32 iArraySize)
{
	//Use _BitScanForward (MSVC) or __builtin_ffs (GCC) rather.

	int32				iIntSize;
	int32				i;
	int32				iRemainder;
	int32				iCharSize;
	int32				iStart;
	int32				iEnd;
	bool			bFound;
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
			return -1;
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
		return -1;
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
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	array size is in bits
//
//////////////////////////////////////////////////////////////////////////
int32 FindFirstClearBit(void* pvArray, int32 iArraySize)
{
	int32				iIntSize;
	int32				i;
	int32				iRemainder;
	int32				iCharSize;
	int32				iStart;
	int32				iEnd;
	bool			bFound;
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
			return -1;
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
		return -1;
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
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	iArray size is in bits
//
//////////////////////////////////////////////////////////////////////////
int32 FindNextSetBit(void* pvArray, int32 iArraySize, int32 iStartBit)
{
	int32				i;
	int32				iRemainder;
	int32				iStart;
	uint8	c;
	int32				iResult;
	int32				iStop;
	int32				ij;
	int32				iStartEight;

	iRemainder = iStartBit % 8;
	iStart = iStartBit / 8;

	if (iRemainder != 0)
	{
		iStop = iArraySize < 8 ? iStop = iArraySize : 8;

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
		iResult = FindFirstSetBit(&((uint8*)pvArray)[iStart + 1], iArraySize - iStartEight);
		if (iResult != -1)
		{
			return iResult + iStartEight;
		}
		return iResult;
	}
	else
	{
		iStartEight = iStartBit;
		iResult = FindFirstSetBit(&((uint8*)pvArray)[iStart], iArraySize - iStartEight);
		if (iResult != -1)
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
int32 FindNextClearBit(void* pvArray, int32 iArraySize, int32 iStartBit)
{
	int32				i;
	int32				iRemainder;
	int32				iStart;
	uint8	c;
	int32				iResult;
	int32				iStop;
	int32				ij;
	int32				iStartEight;

	iRemainder = iStartBit % 8;
	iStart = iStartBit / 8;

	if (iRemainder != 0)
	{
		iStop = iArraySize < 8 ? iStop = iArraySize : 8;

		c = ((uint8*)pvArray)[iStart];
		ij = iStart * 8;
		uint8 ucCmp = 1 << iRemainder;

		for (i = iRemainder; i < iStop; i++)
		{
			if (!(c & ucCmp))
				return ij + i;
			ucCmp <<= 1;
		}

		iStartEight = iStart * 8 + 8;
		iResult = FindFirstClearBit(&((uint8*)pvArray)[iStart + 1], iArraySize - iStartEight);
		if (iResult != -1)
		{
			return iResult + iStartEight;
		}
		return iResult;
	}
	else
	{
		iStartEight = iStartBit;
		iResult = FindFirstClearBit(&((uint8*)pvArray)[iStart], iArraySize - iStartEight);
		if (iResult != -1)
		{
			return iResult + iStartEight;
		}
		return iResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 FindLastClearBit(void* pvArray, int32 iArraySize)
{
	int32		i;
	int32		iBit;

	for (i = iArraySize-1; i >= 0; i--)
	{
		iBit = GetBit(i, pvArray);
		if (iBit == 0)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 FindLastSetBit(void* pvArray, int32 iArraySize)
{
	int32		i;
	int32		iBit;

	for (i = iArraySize-1; i >= 0; i--)
	{
		iBit = GetBit(i, pvArray);
		if (iBit)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FixBool(void* pv)
{
	return FixBool((int32)(size_t) pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FixBool(int32 i)
{
	if (i)
		return true;
	else
		return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlag(int32* piDest, int32 iFlag, int32 iFlagValue)
{
	//If the value is true then OR it with dest.
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
void SetFlag(uint32* piDest, int32 iFlag, int32 iFlagValue)
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
void SetFlag(int16* psiDest, int32 iFlag, int32 iFlagValue)
{
	//If the value is true then or it with dest.
	if (iFlagValue)
	{
		*psiDest |= iFlag;
	}
	//If the value is false then negate and and it with dest.
	else
	{
		*psiDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlag(uint16* psiDest, int32 iFlag, int32 iFlagValue)
{
	//If the value is true then or it with dest.
	if (iFlagValue)
	{
		*psiDest |= iFlag;
	}
	//If the value is false then negate and and it with dest.
	else
	{
		*psiDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlag(uint8* psiDest, int32 iFlag, int32 iFlagValue)
{
	//If the value is true then or it with dest.
	if (iFlagValue)
	{
		*psiDest |= iFlag;
	}
	//If the value is false then negate and and it with dest.
	else
	{
		*psiDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlag(int8* psiDest, int32 iFlag, int32 iFlagValue)
{
	//If the value is true then or it with dest.
	if (iFlagValue)
	{
		*psiDest |= iFlag;
	}
	//If the value is false then negate and and it with dest.
	else
	{
		*psiDest &= (~iFlag);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Swap(int32* pi1, int32* pi2)
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
void Swap(uint8* pc1, uint8* pc2)
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
int32 FindFirstInt(int32* piIntArray, int32 iSearch, int32 iMaxLength)
{
	for (int32 i = 0; i < iMaxLength; i++)
	{
		if (piIntArray[i] == iSearch)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 FindFirstByte(uint8* pcCharArray, uint8 uiSearch, int32 iMaxLength)
{
	int32		i;

	for (i = 0; i < iMaxLength; i++)
	{
		if (pcCharArray[i] == uiSearch)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 FindFirstByte(char* pcCharArray, char cSearch, int32 iMaxLength)
{
	return FindFirstByte((uint8*)pcCharArray, cSearch, iMaxLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 FindFirstByte(int8* pcCharArray, int8 iSearch, int32 iMaxLength)
{
	return FindFirstByte((uint8*)pcCharArray, iSearch, iMaxLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 GetHighNybble(char c)
{
	return ((int32)c >> 4) & 0xf;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 GetLowNybble(char c)
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
int32 CompareInt(const void* arg1, const void* arg2)
{
	if ((*((int32*)arg1)) < (*((int32*)arg2)))
	{
		return -1;
	}
	if ((*((int32*)arg1)) > (*((int32*)arg2)))
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 CompareLong(const void* arg1, const void* arg2)
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
int32 CompareIntReverse(const void* arg1, const void* arg2)
{
	if ((*((int32*)arg1)) < (*((int32*)arg2)))
	{
		return 1;
	}
	if ((*((int32*)arg1)) > (*((int32*)arg2)))
	{
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 CompareLongReverse(const void* arg1, const void* arg2)
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
char GetCrumb(int32 iCrumb, void* pvArray)
{
	int32 iBytePos;
	int32 iCrumbInByte;

	iBytePos = iCrumb / 4;
	iCrumbInByte = iCrumb % 4;

	return (((uint8*)pvArray)[iBytePos] >> (2 * iCrumbInByte)) & 0x3;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char GetNybble(int32 iNybble, void* pvArray)
{
	int32 iBytePos;
	int32 iNybbleInByte;

	iBytePos = iNybble / 2;
	iNybbleInByte = iNybble % 2;

	return (((uint8*)pvArray)[iBytePos] >> (4 * iNybbleInByte)) & 0xf;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CopyBits(void* pvDest, int32 iDestOffset, void* pvSource, int32 iSourceOffset, int32 iNumBits, int32 bClear)
{
	int32		i;
	int32		bBit;
	int32		iRemaining;
	int32		iLast;

	//Optimise this later...  Use bit shifting and friends.
	for (i = 0; i < iNumBits; i++)
	{
		bBit = GetBit(iSourceOffset + i, pvSource);
		SetBit(iDestOffset + i, pvDest, bBit);
	}

	if (bClear)
	{
		iLast = i;

		//Optimise this later...  Just AND the end off.
		iRemaining = (8 - ((iDestOffset + iNumBits) % 8)) % 8;
		for (i = 0; i < iRemaining; i++)
		{
			SetBit(iDestOffset + iLast + i, pvDest, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CopyBitsReverseHiLo(void* pvDest, int32 iDestOffset, void* pvSource, int32 iSourceOffset, int32 iNumBits, int32 bClear)
{
	int32		i;
	char	bBit;
	int32		iRemaining;
	int32		iLast;

	//This reverses bits in a byte.  Not the whole bitstream.  That's a much more difficult problem to solve.
	//This function is useless if it is not fully 8bits aligned.

	//Optimise this later...  Use bit shifting and friends.
	for (i = 0; i < iNumBits; i++)
	{
		bBit = GetBitReverseHiLo(iSourceOffset + i, pvSource);
		SetBit(iDestOffset + i, pvDest, bBit);
	}

	if (bClear)
	{
		iLast = i;

		//Optimise this later...  Just AND the end off.
		iRemaining = (8 - ((iDestOffset + iNumBits) % 8)) % 8;
		for (i = 0; i < iRemaining; i++)
		{
			SetBit(iDestOffset + iLast + i, pvDest, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int16 ReverseShortEndianness(int16 s)
{
	Swap((uint8*)&s, ((uint8*)&s) + 1);
	return s;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 ReverseIntEndianness(int32 i)
{
	Swap(((uint8*)&i + 0), ((uint8*)&i) + 3);
	Swap(((uint8*)&i + 1), ((uint8*)&i) + 2);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int64 ReverseLongEndianness(int64 i)
{
	Swap(((uint8*)&i + 0), ((uint8*)&i) + 7);
	Swap(((uint8*)&i + 1), ((uint8*)&i) + 6);
	Swap(((uint8*)&i + 2), ((uint8*)&i) + 5);
	Swap(((uint8*)&i + 3), ((uint8*)&i) + 4);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ReverseEndianness(void* pv, int32 iSize)
{
	if (iSize == 4)
	{
		*(int32*)pv = ReverseIntEndianness(*(int32*)pv);
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
void ReverseBytes(void* pv, int32 iSize)
{
	int32		i;
	int32		iHalf;
	
	iHalf = iSize / 2;
	for (i = 0; i < iHalf; i++)
	{
		Swap(&(((uint8*)pv)[i]), &(((uint8*)pv)[iSize - i - 1]));
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 CalculateStride(int32 iElementSize, int32 iAlignment)
{
	int32		iByteDiff;
	int32		iStride;

	iByteDiff = iElementSize % iAlignment;
	if (iByteDiff == 0)
	{
		return iElementSize;
	}
	else
	{
		iByteDiff = iAlignment - iByteDiff;
		iStride = iByteDiff + iElementSize;
		return iStride;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 CalculateOffset(char iOffset, int32 iAlignment)
{
	if ((iOffset == -8) && ((iAlignment == 4) || (iAlignment == 8)))
	{
		return 0;
	}

	if (iOffset < 0)
	{
		iOffset = (iAlignment + (iOffset % iAlignment)) % iAlignment;
	}
	else
	{
		iOffset = iOffset % iAlignment;
	}
	return iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 GetPowerOf2SizeDown(int32 iInt)
{
	return TruncateLowBits(iInt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 GetPowerOf2SizeUp(int32 iInt)
{
	return GetBestHighBit(iInt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 GetPowerOf2SizeAuto(int32 iInt)
{
	return TruncateLowBits((int32)(iInt * 1.5f));
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

