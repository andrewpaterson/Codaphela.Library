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
uint32 GetBestHighBit(uint32 i)
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
bool GetBitReverseHiLo(size iBit, void* pvBitArray)
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
		//This can be optimised
		return CountBitsSingly(pvBitArray, iBitLength);
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
		iStop = iArraySize < 8 ? iStop = iArraySize : 8;

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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FixBool(void* pv)
{
	if (pv)
	{
		return true;
	}
	else
	{
		return false;
	}
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
size FindFirstByte(uint8* pcCharArray, uint8 uiSearch, size iMaxLength)
{
	size	i;

	for (i = 0; i < iMaxLength; i++)
	{
		if (pcCharArray[i] == uiSearch)
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
int CompareIntReverse(const void* arg1, const void* arg2)
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
uint8 GetCrumb(uint16 iCrumb, void* pvArray)
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
uint8 GetNybble(uint16 iNybble, void* pvArray)
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
void CopyBits(void* pvDest, size iDestOffset, void* pvSource, size iSourceOffset, size iNumBits, bool bClear)
{
	size	i;
	bool	bBit;
	size	iRemaining;
	size	iLast;

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
void CopyBitsReverseHiLo(void* pvDest, size iDestOffset, void* pvSource, size iSourceOffset, size iNumBits, bool bClear)
{
	size	i;
	bool	bBit;
	size	iRemaining;
	size	iLast;

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
	SwapByte((uint8*)&s, ((uint8*)&s) + 1);
	return s;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int32 ReverseIntEndianness(int32 i)
{
	SwapByte(((uint8*)&i + 0), ((uint8*)&i) + 3);
	SwapByte(((uint8*)&i + 1), ((uint8*)&i) + 2);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int64 ReverseLongEndianness(int64 i)
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
		*(size*)pv = ReverseIntEndianness(*(size*)pv);
	}
	if (iSize == 2)
	{
		*(int16*)pv = ReverseShortEndianness(*(int16*)pv);
	}
	if (iSize == 4)
	{
		*(size*)pv = ReverseIntEndianness(*(size*)pv);
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
uint8 CalculateOffset(int16 iOffset, uint16 iAlignment)
{
	if ((iOffset == -8) && ((iAlignment == 4) || (iAlignment == 8)))
	{
		return 0;
	}

	if (iOffset < 0)
	{
		return (iAlignment + (iOffset % iAlignment)) % iAlignment;
	}
	else
	{
		return iOffset % iAlignment;
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

