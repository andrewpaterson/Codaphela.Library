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
#include "IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//  Returns the Nearest power of two to this number rounded down.
//	eg: 1025 become 1024 but 1024 becomes 1024.  (also 2047 becomes 1024).
//
//////////////////////////////////////////////////////////////////////////
int TruncateLowBits(int i)
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
int IsPowerOfTwo(int i)
{
	int	iPow2;

	iPow2 = TruncateLowBits(i);
	if (iPow2 == i)
	{
		return iPow2;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//  Returns the Nearest power of two to this number rounded up.
//	eg: 1025 become 2048 but 1024 becomes 1024.  (also 1023 becomes 1024).
//
//////////////////////////////////////////////////////////////////////////
int GetBestHighBit(int i)
{
	int i2;

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
int GetLogBase2(int i)
{
	int		c;

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
int GetByte(int iInt, int iPos)
{
	int c;

	iPos *= 8;
	iInt >>= iPos;
	c = (unsigned char)(iInt & 0xff);
	return c;
}


//////////////////////////////////////////////////////////////////////////
//
//	Same as above but sets the byte.
//
//////////////////////////////////////////////////////////////////////////
int SetByte(int c, int iPos)
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
int GetBit(int iBit, void* pvArray)
{
	return ((char*)pvArray)[iBit / 8] & (1 << (iBit % 8));
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
int CountBits_PopCount64(unsigned long long int x)
{
	const unsigned long long int m1 = 0x5555555555555555;
	const unsigned long long int m2 = 0x3333333333333333;
	const unsigned long long int m4 = 0x0f0f0f0f0f0f0f0f;
	const unsigned long long int m5 = 0x0101010101010101;

	x = x - ((x >> 1) & m1);          //put count of each 2 bits into those 2 bits
	x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
	x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
	return (x * m5) >> 56;
}


//////////////////////////////////////////////////////////////////////////
//
//  https://stackoverflow.com/questions/14780928/count-number-of-bits-in-an-unsigned-integer
//
//////////////////////////////////////////////////////////////////////////
int CountBits_PopCount32(unsigned int i)
{
	const unsigned int m1 = 0x55555555;
	const unsigned int m2 = 0x33333333;
	const unsigned int m4 = 0x0f0f0f0f;

	i = i - ((i >> 1) & m1);
	i = (i & m2) + ((i >> 2) & m2);
	i = ((i + (i >> 4)) & m4);
	return (i * 0x01010101) >> 24;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountBits_PopCount16(unsigned short i)
{
	const unsigned short m1 = 0x5555;
	const unsigned short m2 = 0x3333;
	const unsigned short m4 = 0x0f0f;

	i = i - ((i >> 1) & m1);
	i = (i & m2) + ((i >> 2) & m2);
	i = ((i + (i >> 4)) & m4);
	return (unsigned short)(i * 0x0101) >> 8;
}


//////////////////////////////////////////////////////////////////////////
//
//  https://stackoverflow.com/questions/30688465/how-to-check-the-number-of-set-bits-in-an-8-bit-unsigned-char
//
//////////////////////////////////////////////////////////////////////////
int CountBits_PopCount8(unsigned char b)
{
	b = b - ((b >> 1) & 0x55);
	b = (b & 0x33) + ((b >> 2) & 0x33);
	return (((b + (b >> 4)) & 0x0F) * 0x01);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountBitsSingly(void* pvBitArray, int iBitLength)
{
	int iCount;
	int	i;

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
int	CountBits(void* pvBitArray, int iBitLength)
{
	if (iBitLength == 32)
	{
		return CountBits_PopCount32(*((unsigned int*)pvBitArray));
	}
	else if (iBitLength == 64)
	{
		return CountBits_PopCount64(*((unsigned long long int*)pvBitArray));
	}
	else if (iBitLength == 128)
	{
		return CountBits_PopCount64(*((unsigned long long int*)pvBitArray)) + 
		       CountBits_PopCount64(((unsigned long long int*)pvBitArray)[1]);
	}
	else if (iBitLength == 8)
	{
		return CountBits_PopCount8(*((unsigned char*)pvBitArray));
	}
	else if (iBitLength == 16)
	{
		return CountBits_PopCount16(*((unsigned short int*)pvBitArray));
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
int GetBitReverseHiLo(int iBit, void* pvArray)
{
	return ((char*)pvArray)[iBit / 8] & (1 << (7 - (iBit % 8)));
}


//////////////////////////////////////////////////////////////////////////
//
//	Sets the state of a bit in an array given the bit position and state.
//	No bounds checking is done.
//
//////////////////////////////////////////////////////////////////////////
void SetBit(int iBit, void* pvArray, int bBit)
{
	if (bBit)
	{
		((char*)pvArray)[iBit / 8] |= (1 << (iBit % 8));
	}
	else
	{
		((char*)pvArray)[iBit / 8] &= ~(1 << (iBit % 8));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	iArray size is in bits
//
//////////////////////////////////////////////////////////////////////////
int FindFirstSetBit(void* pvArray, int iArraySize)
{
	//Use _BitScanForward (MSVC) or __builtin_ffs (GCC) rather.

	int				iIntSize;
	int				i;
	int				iRemainder;
	int				iCharSize;
	int				iStart;
	int				iEnd;
	BOOL			bFound;
	unsigned char	c;

	iIntSize = iArraySize / 32;
	iRemainder = iArraySize % 32;

	for (i = 0; i < iIntSize; i++)
	{
		if (((unsigned int*)pvArray)[i] != 0x00000000)
		{
			break;
		}
	}


	iCharSize = iRemainder / 8;
	iRemainder = iRemainder % 8;
	iStart = i * 4;
	iEnd = iIntSize * 4 + iCharSize;

	bFound = FALSE;
	for (i = iStart; i < iEnd; i++)
	{
		c = ((char*)pvArray)[i];
		if (c != (unsigned char)0x00)
		{
			bFound = TRUE;
			break;
		}
	}

	if (iRemainder == 0)
	{
		if (!bFound)
		{
			return -1;
		}

		c = ((char*)pvArray)[i];
		iStart = i * 8;
		unsigned char ucCmp = 1;

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
		c = ((char*)pvArray)[i];
		iStart = i * 8;
		unsigned char ucCmp = 1;

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
//	iArray size is in bits
//
//////////////////////////////////////////////////////////////////////////
int FindFirstClearBit(void* pvArray, int iArraySize)
{
	int				iIntSize;
	int				i;
	int				iRemainder;
	int				iCharSize;
	int				iStart;
	int				iEnd;
	BOOL			bFound;
	unsigned char	c;

	iIntSize = iArraySize / 32;
	iRemainder = iArraySize % 32;

	for (i = 0; i < iIntSize; i++)
	{
		if (((unsigned int*)pvArray)[i] != 0xffffffff)
		{
			break;
		}
	}

	iCharSize = iRemainder / 8;
	iRemainder = iRemainder % 8;
	iStart = i * 4;
	iEnd = iIntSize * 4 + iCharSize;

	bFound = FALSE;
	for (i = iStart; i < iEnd; i++)
	{
		c = ((char*)pvArray)[i];
		if (c != (unsigned char)0xff)
		{
			bFound = TRUE;
			break;
		}
	}

	if (iRemainder == 0)
	{
		if (!bFound)
		{
			return -1;
		}

		c = ((char*)pvArray)[i];
		iStart = i * 8;
		unsigned char ucCmp = 1;

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
		c = ((char*)pvArray)[i];
		iStart = i * 8;
		unsigned char ucCmp = 1;

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
//
//////////////////////////////////////////////////////////////////////////
int FindLastClearBit(void* pvArray, int iArraySize)
{
	int		i;
	int		iBit;

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
int FindLastSetBit(void* pvArray, int iArraySize)
{
	int		i;
	int		iBit;

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
int FixBool(void* pv)
{
	return FixBool((int)(size_t) pv);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL FixBool(int i)
{
	if (i)
		return TRUE;
	else
		return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SetFlag(int* piDest, int iFlag, int iFlagValue)
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
void SetFlag(unsigned int* piDest, int iFlag, int iFlagValue)
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
void SetFlag(short int* psiDest, int iFlag, int iFlagValue)
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
void SetFlag(unsigned short int* psiDest, int iFlag, int iFlagValue)
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
void SetFlag(char* psiDest, int iFlag, int iFlagValue)
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
void SetFlag(unsigned char* psiDest, int iFlag, int iFlagValue)
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
void Swap(int* pi1, int* pi2)
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
void Swap(char* pc1, char* pc2)
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
int FindFirstInt(int* piIntArray, int iSearch, int iMaxLength)
{
	for (int i = 0; i < iMaxLength; i++)
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
int FindFirstByte(char* pcCharArray, char cSearch, int iMaxLength)
{
	int		i;

	for (i = 0; i < iMaxLength; i++)
	{
		if (pcCharArray[i] == cSearch)
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
int GetHighNybble(char c)
{
	return ((int)c >> 4) & 0xf;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetLowNybble(char c)
{
	return (int)c & 0xf;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int IntAbs(int i)
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
//																		//
//////////////////////////////////////////////////////////////////////////
char GetCrumb(int iCrumb, void* pvArray)
{
	int iBytePos;
	int iCrumbInByte;

	iBytePos = iCrumb / 4;
	iCrumbInByte = iCrumb % 4;

	return (((char*)pvArray)[iBytePos] >> (2 * iCrumbInByte)) & 0x3;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char GetNybble(int iNybble, void* pvArray)
{
	int iBytePos;
	int iNybbleInByte;

	iBytePos = iNybble / 2;
	iNybbleInByte = iNybble % 2;

	return (((char*)pvArray)[iBytePos] >> (4 * iNybbleInByte)) & 0xf;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CopyBits(void* pvDest, int iDestOffset, void* pvSource, int iSourceOffset, int iNumBits, int bClear)
{
	int		i;
	int		bBit;
	int		iRemaining;
	int		iLast;

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
void CopyBitsReverseHiLo(void* pvDest, int iDestOffset, void* pvSource, int iSourceOffset, int iNumBits, int bClear)
{
	int		i;
	char	bBit;
	int		iRemaining;
	int		iLast;

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
short ReverseShortEndianness(short s)
{
	Swap((char*)&s, ((char*)&s) + 1);
	return s;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int ReverseIntEndianness(int i)
{
	Swap(((char*)&i + 0), ((char*)&i) + 3);
	Swap(((char*)&i + 1), ((char*)&i) + 2);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
long long int ReverseLongEndianness(long long int i)
{
	Swap(((char*)&i + 0), ((char*)&i) + 7);
	Swap(((char*)&i + 1), ((char*)&i) + 6);
	Swap(((char*)&i + 2), ((char*)&i) + 5);
	Swap(((char*)&i + 3), ((char*)&i) + 4);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ReverseEndianness(void* pv, int iSize)
{
	if (iSize == 4)
	{
		*(int*)pv = ReverseIntEndianness(*(int*)pv);
	}
	if (iSize == 2)
	{
		*(short*)pv = ReverseShortEndianness(*(short*)pv);
	}
	if (iSize == 8)
	{
		*(long long int*)pv = ReverseLongEndianness(*(long long int*)pv);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ReverseBytes(void* pv, int iSize)
{
	int		i;
	int		iHalf;
	
	iHalf = iSize / 2;
	for (i = 0; i < iHalf; i++)
	{
		Swap(&(((char*)pv)[i]), &(((char*)pv)[iSize - i - 1]));
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CalculateStride(int iElementSize, int iAlignment)
{
	int		iByteDiff;
	int		iStride;

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
int CalculateOffset(char iOffset, int iAlignment)
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
int GetPowerOf2SizeDown(int iInt)
{
	return TruncateLowBits(iInt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetPowerOf2SizeUp(int iInt)
{
	return GetBestHighBit(iInt);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetPowerOf2SizeAuto(int iInt)
{
	return TruncateLowBits((int)(iInt * 1.5f));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int LargestInt(int* pInts, int iCount)
{
	int	iLargest;
	int i;

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

