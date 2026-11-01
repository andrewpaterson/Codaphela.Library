#include <stdlib.h>
#include <string.h>
#include "BaseLib/IntegerHelper.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperFindFirstClearBit(void)
{
	uint8	ab[9];
	size	iIndex;
	size	iBit;

	ab[0] = ab[1] = ab[2] = ab[3] = ab[4] = ab[5] = ab[6] = ab[7] = ab[8] = 0xff;
	iIndex = FindFirstClearBit(ab, 64);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);
	
	ab[7] = 0xfe;
	iIndex = FindFirstClearBit(ab, 64);
	AssertSize(56, iIndex);
	ab[7] = 0x7f;
	iIndex = FindFirstClearBit(ab, 64);
	AssertSize(63, iIndex);

	ab[7] = 0xff;

	iBit = 72;
	do
	{
		iBit--;
		SetBit(iBit, ab, false);
		iIndex = FindFirstClearBit(ab, 72);
		AssertSize(iBit, iIndex);
	}
	while (iBit != 0);

	ab[0] = 0xff;
	iIndex = FindFirstClearBit(ab, 2);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);
	ab[0] = 0xfe;
	iIndex = FindFirstClearBit(ab, 2);
	AssertSize(0, iIndex);
	ab[0] = 0xfd;
	iIndex = FindFirstClearBit(ab, 2);
	AssertSize(1, iIndex);
	ab[0] = 0xfc;
	iIndex = FindFirstClearBit(ab, 2);
	AssertSize(0, iIndex);
	ab[0] = 0xfb;
	iIndex = FindFirstClearBit(ab, 2);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperFindFirstSetBit(void)
{
	uint8	ab[9];
	size	iIndex;
	size	iBit;

	ab[0] = ab[1] = ab[2] = ab[3] = ab[4] = ab[5] = ab[6] = ab[7] = ab[8] = 0;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);

	ab[0] = 1;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(0, iIndex);
	ab[0] = 2;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(1, iIndex);
	ab[0] = 4;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(2, iIndex);
	ab[0] = 8;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(3, iIndex);
	ab[0] = 16;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(4, iIndex);
	ab[0] = 32;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(5, iIndex);
	ab[0] = 64;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(6, iIndex);
	ab[0] = 128;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(7, iIndex);
	ab[0] = 0;
	ab[1] = 1;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(8, iIndex);
	ab[1] = 2;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(9, iIndex);
	
	ab[1] = 0;
	ab[7] = 1;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(56, iIndex);
	ab[7] = 0x80;
	iIndex = FindFirstSetBit(ab, 64);
	AssertSize(63, iIndex);

	ab[7] = 0;

	iBit = 72;
	do
	{
		iBit--;
		SetBit(iBit, ab, true);
		iIndex = FindFirstSetBit(ab, 72);
		AssertSize(iBit, iIndex);
	}
	while (iBit != 0);

	ab[0] = 0x00;
	iIndex = FindFirstSetBit(ab, 2);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);
	ab[0] = 0x01;
	iIndex = FindFirstSetBit(ab, 2);
	AssertSize(0, iIndex);
	ab[0] = 0x02;
	iIndex = FindFirstSetBit(ab, 2);
	AssertSize(1, iIndex);
	ab[0] = 0x03;
	iIndex = FindFirstSetBit(ab, 2);
	AssertSize(0, iIndex);
	ab[0] = 0x04;
	iIndex = FindFirstSetBit(ab, 2);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperFindLastClearBit(void)
{
	uint8	ab[9];
	size	iIndex;
	size	iBit;

	ab[0] = ab[1] = ab[2] = ab[3] = ab[4] = ab[5] = ab[6] = ab[7] = ab[8] = 0xff;
	iIndex = FindLastClearBit(ab, 64);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);

	ab[0] = 0x7f;
	iIndex = FindLastClearBit(ab, 64);
	AssertSize(7, iIndex);
	ab[0] = 0xfe;
	iIndex = FindLastClearBit(ab, 64);
	AssertSize(0, iIndex);

	ab[0] = 0xff;

	for (iBit = 0; iBit < 72; iBit++)
	{
		SetBit(iBit, ab, false);
		iIndex = FindLastClearBit(ab, 72);
		AssertSize(iBit, iIndex);
	}

	ab[0] = 0xff;
	iIndex = FindLastClearBit(ab, 2);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);
	ab[0] = 0xfd;
	iIndex = FindLastClearBit(ab, 2);
	AssertSize(1, iIndex);
	ab[0] = 0xfe;
	iIndex = FindLastClearBit(ab, 2);
	AssertSize(0, iIndex);
	ab[0] = 0xfc;
	iIndex = FindLastClearBit(ab, 2);
	AssertSize(1, iIndex);
	ab[0] = 0xfb;
	iIndex = FindLastClearBit(ab, 2);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperFindLastSetBit(void)
{
	uint8	ab[9];
	size	iIndex;
	size	iBit;

	ab[0] = ab[1] = ab[2] = ab[3] = ab[4] = ab[5] = ab[6] = ab[7] = ab[8] = 0;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iIndex);

	ab[7] = 128;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(63, iIndex);
	ab[7] = 64;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(62, iIndex);
	ab[7] = 32;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(61, iIndex);
	ab[7] = 16;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(60, iIndex);
	ab[7] = 8;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(59, iIndex);
	ab[7] = 4;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(58, iIndex);
	ab[7] = 2;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(57, iIndex);
	ab[7] = 1;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(56, iIndex);
	ab[7] = 0;
	ab[6] = 128;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(55, iIndex);
	ab[6] = 64;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(54, iIndex);

	ab[6] = 0;
	ab[0] = 128;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(7, iIndex);
	ab[0] = 2;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(1, iIndex);
	ab[0] = 1;
	iIndex = FindLastSetBit(ab, 64);
	AssertSize(0, iIndex);

	ab[0] = 0;

	for (iBit = 0; iBit < 72; iBit++)
	{
		SetBit(iBit, ab, true);
		iIndex = FindLastSetBit(ab, 72);
		AssertSize(iBit, iIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperReverseBytes(void)
{
	char szAuto[9] = "Autobody";
	char szAutoDash[10] = "Auto-Body";
	char szOne[2] = "1";

	ReverseBytes(szAuto, 8);
	AssertString("ydobotuA", szAuto);
	ReverseBytes(szAutoDash, 9);
	AssertString("ydoB-otuA", szAutoDash);
	ReverseBytes(szOne, 1);
	AssertString("1", szOne);
	ReverseBytes(szOne, 0);
	AssertString("1", szOne);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperCountBits(void)
{
	uint8 auc8a[] = { 0x6a, 0x83, 0x5d, 0x16, 0x42, 0x6e, 0x61, 0x80 };
	uint8 auc8b[] = { 0x54, 0xdc, 0x72, 0xf5, 0x78, 0xf9, 0x86, 0xa3 };
	uint8 auc80[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8 auc81[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	AssertSize(26, CountBits(auc8a, 64));
	AssertSize(26, CountBitsSingly(auc8a, 64));
	AssertSize(15, CountBits(auc8a, 32));
	AssertSize(15, CountBitsSingly(auc8a, 32));
	AssertSize(11, CountBits(&(auc8a[4]), 32));
	AssertSize(11, CountBitsSingly(&(auc8a[4]), 32));

	AssertSize(35, CountBits(auc8b, 64));
	AssertSize(35, CountBitsSingly(auc8b, 64));
	AssertSize(18, CountBits(auc8b, 32));
	AssertSize(18, CountBitsSingly(auc8b, 32));
	AssertSize(17, CountBits(&(auc8b[4]), 32));
	AssertSize(17, CountBitsSingly(&(auc8b[4]), 32));

	AssertSize(0, CountBits(auc80, 64));
	AssertSize(0, CountBitsSingly(auc80, 64));
	AssertSize(0, CountBits(auc80, 32));
	AssertSize(0, CountBitsSingly(auc80, 32));
	AssertSize(0, CountBits(&(auc80[4]), 32));
	AssertSize(0, CountBitsSingly(&(auc80[4]), 32));
	AssertSize(0, CountBits(auc80, 8));
	AssertSize(0, CountBits(auc80, 16));

	AssertSize(64, CountBits(auc81, 64));
	AssertSize(64, CountBitsSingly(auc81, 64));
	AssertSize(32, CountBits(auc81, 32));
	AssertSize(32, CountBitsSingly(auc81, 32));
	AssertSize(32, CountBits(&(auc81[4]), 32));
	AssertSize(32, CountBitsSingly(&(auc81[4]), 32));
	AssertSize(8, CountBits(auc81, 8));
	AssertSize(16, CountBits(auc81, 16));

	AssertSize(4, CountBits(auc8a, 8));
	AssertSize(4, CountBitsSingly(auc8a, 8));
	AssertSize(3, CountBits(auc8b, 8));
	AssertSize(3, CountBitsSingly(auc8b, 8));

	AssertSize(7, CountBits(auc8a, 16));
	AssertSize(7, CountBitsSingly(auc8a, 16));
	AssertSize(8, CountBits(&(auc8a[2]), 16));
	AssertSize(8, CountBitsSingly(&(auc8a[2]), 16));
	AssertSize(8, CountBits(auc8b, 16));
	AssertSize(8, CountBitsSingly(auc8b, 16));
	AssertSize(10, CountBits(&(auc8b[2]), 16));
	AssertSize(10, CountBitsSingly(&(auc8b[2]), 16));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperFindNextSetBit(void)
{
	uint8	ab[9];
	size		iBit;

	ab[0] = ab[1] = ab[2] = ab[3] = ab[4] = ab[5] = ab[6] = ab[7] = ab[8] = 0xff;
	ab[1] = 0b1110'1111;

	iBit = FindFirstClearBit(ab, 64);
	AssertSize(12, iBit);

	iBit = FindNextSetBit(ab, 64, iBit);
	AssertSize(13, iBit);

	iBit = FindFirstClearBit(&ab[1], 63);
	AssertSize(4, iBit);

	iBit = FindNextSetBit(&ab[1], 63, iBit);
	AssertSize(5, iBit);

	ab[0] = 0b1000'0000;
	iBit = FindFirstSetBit(ab, 7);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iBit);

	iBit = FindNextSetBit(ab, 7, 1);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iBit);

	ab[0] = ab[1] = ab[2] = ab[3] = ab[4] = ab[5] = ab[6] = ab[7] = ab[8] = 0;
	ab[7] = 0b0000'0010;
	ab[4] = 0b0010'0000;
	iBit = FindFirstSetBit(&ab[2], 44);
	AssertSize(21, iBit);

	iBit = FindNextSetBit(&ab[2], 44, 22);
	AssertSize(41, iBit);

	iBit = FindNextSetBit(&ab[2], 40, 22);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iBit);

	iBit = FindNextSetBit(&ab[2], 41, 22);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iBit);

	iBit = FindNextSetBit(&ab[2], 42, 22);
	AssertSize(41, iBit);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperFindNextClearBit(void)
{
	uint8	ab[9];
	size		iBit;

	ab[0] = ab[1] = ab[2] = ab[3] = ab[4] = ab[5] = ab[6] = ab[7] = ab[8] = 0;
	ab[1] = 0b0001'0000;

	iBit = FindFirstSetBit(ab, 64);
	AssertSize(12, iBit);

	iBit = FindNextClearBit(ab, 64, iBit);
	AssertSize(13, iBit);

	iBit = FindFirstSetBit(&ab[1], 63);
	AssertSize(4, iBit);

	iBit = FindNextClearBit(&ab[1], 63, iBit);
	AssertSize(5, iBit);

	ab[0] = 0b0111'1111;
	iBit = FindFirstClearBit(ab, 7);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iBit);

	iBit = FindNextClearBit(ab, 7, 1);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iBit);

	ab[0] = ab[1] = ab[2] = ab[3] = ab[4] = ab[5] = ab[6] = ab[7] = ab[8] = 0xff;
	ab[7] = 0b1111'1101;
	ab[4] = 0b1101'1111;
	iBit = FindFirstClearBit(&ab[2], 44);
	AssertSize(21, iBit);

	iBit = FindNextClearBit(&ab[2], 44, 22);
	AssertSize(41, iBit);

	iBit = FindNextClearBit(&ab[2], 40, 22);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iBit);

	iBit = FindNextClearBit(&ab[2], 41, 22);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, iBit);

	iBit = FindNextClearBit(&ab[2], 42, 22);
	AssertSize(41, iBit);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperSetBits(void)
{
	uint8	ab[9];
	
	memset(ab, 0, 9);
	SetBits(9, ab, true, 3);

	AssertSize(0xe00, *((size*)ab));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperBitFunctions(void)
{
	char	c[8];

	memset(c, 0, 8);

	SetBit(1, c, 1);
	AssertChar(c[0], 2);
	SetBit(7, c, 1);
	AssertChar(c[0], -126);
	SetBit(8, c, 1);
	AssertChar(c[1], 1);

	c[0] = 5;
	AssertBool(true, GetBit(0, &c));
	AssertBool(false, GetBit(1, &c));
	AssertBool(true, GetBit(2, &c));

	AssertSize(1, FindFirstClearBit(c, 8 * 8));
	AssertSize(0, FindFirstSetBit(c, 8 * 8));
	c[0] = 4;
	AssertSize(0, FindFirstClearBit(c, 8 * 8));
	AssertSize(2, FindFirstSetBit(c, 8 * 8));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperMisc(void)
{
	AssertSize(4, GetLogBase2(16));
	AssertSize(4, GetLogBase2(31));
	AssertSize(5, GetLogBase2(32));
	AssertSize(5, GetLogBase2(63));

	AssertSize(1024, TruncateLowBits(1024));
	AssertSize(1024, TruncateLowBits(1025));
	AssertSize(1024, TruncateLowBits(2047));
	AssertSize(1, TruncateLowBits(1));
	AssertSize(0, TruncateLowBits(0));

	AssertSize(1024, GetBestHighBit(1023));
	AssertSize(1024, GetBestHighBit(1024));
	AssertSize(2048, GetBestHighBit(1025));
	AssertSize(2048, GetBestHighBit(2047));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperEndianness(void)
{
	union
	{
		int64	i64;
		int8	ia8[8];
	};

	i64 = 0x0102030405060708LL;

	AssertChar(0x08, ia8[0]);
#ifdef _DEBUG
	AssertChar(0x07, ia8[1]);
#endif // _DEBUG
	AssertChar(0x06, ia8[2]);
	AssertChar(0x05, ia8[3]);
	AssertChar(0x04, ia8[4]);
#ifdef _DEBUG
	AssertChar(0x03, ia8[5]);
#endif // _DEBUG
	AssertChar(0x02, ia8[6]);
	AssertChar(0x01, ia8[7]);
}


//////////////////////////////////////////////////////////////////////////
//
// The bit finders checked against GetBit, for each count of bits from 0
// to 96 and patterns of bits: the size ones, the Large ones and the Next
// ones (Libc65's string816_16.asm and string816.asm on the W65C816).
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperFindBitsAgainstGetBit(void)
{
	uint8	auiBits[14];
	uint32	uiSeed;
	size	iPattern;
	size	iCount;
	size	i;
	size	iFirstSet;
	size	iFirstClear;
	size	iLastSet;
	size	iLastClear;
	size	iStart;
	size	iNextSet;
	size	iNextClear;

	uiSeed = 12345;
	for (iPattern = 0; iPattern < 24; iPattern++)
	{
		for (i = 0; i < 14; i++)
		{
			uiSeed = uiSeed * 1103515245 + 12345;
			if (iPattern == 0)
			{
				auiBits[i] = 0;
			}
			else if (iPattern == 1)
			{
				auiBits[i] = 0xFF;
			}
			else if (iPattern < 8)
			{
				//Sparse: mostly zero (or mostly ones), a bit here and there.
				auiBits[i] = (uint8)((((uiSeed >> 16) & 7) == 0) ? (1 << ((uiSeed >> 20) & 7)) : 0);
				if (iPattern >= 5)
				{
					auiBits[i] = (uint8)~auiBits[i];
				}
			}
			else
			{
				auiBits[i] = (uint8)(uiSeed >> 16);
			}
		}

		for (iCount = 0; iCount <= 96; iCount++)
		{
			iFirstSet = ARRAY_ELEMENT_NOT_FOUND;
			iFirstClear = ARRAY_ELEMENT_NOT_FOUND;
			iLastSet = ARRAY_ELEMENT_NOT_FOUND;
			iLastClear = ARRAY_ELEMENT_NOT_FOUND;
			for (i = 0; i < iCount; i++)
			{
				if (GetBit(i, auiBits))
				{
					if (iFirstSet == ARRAY_ELEMENT_NOT_FOUND)
					{
						iFirstSet = i;
					}
					iLastSet = i;
				}
				else
				{
					if (iFirstClear == ARRAY_ELEMENT_NOT_FOUND)
					{
						iFirstClear = i;
					}
					iLastClear = i;
				}
			}

			AssertSize(iFirstSet, FindFirstSetBit(auiBits, iCount));
			AssertSize(iFirstClear, FindFirstClearBit(auiBits, iCount));
			AssertSize(iLastSet, FindLastSetBit(auiBits, iCount));
			AssertSize(iLastClear, FindLastClearBit(auiBits, iCount));
			AssertLSize((iFirstSet == ARRAY_ELEMENT_NOT_FOUND) ? ARRAY_LARGE_ELEMENT_NOT_FOUND : iFirstSet, FindFirstSetBitLarge(auiBits, iCount));
			AssertLSize((iFirstClear == ARRAY_ELEMENT_NOT_FOUND) ? ARRAY_LARGE_ELEMENT_NOT_FOUND : iFirstClear, FindFirstClearBitLarge(auiBits, iCount));
			AssertLSize((iLastSet == ARRAY_ELEMENT_NOT_FOUND) ? ARRAY_LARGE_ELEMENT_NOT_FOUND : iLastSet, FindLastSetBitLarge(auiBits, iCount));
			AssertLSize((iLastClear == ARRAY_ELEMENT_NOT_FOUND) ? ARRAY_LARGE_ELEMENT_NOT_FOUND : iLastClear, FindLastClearBitLarge(auiBits, iCount));

			for (iStart = 0; iStart < iCount; iStart += 5)
			{
				iNextSet = ARRAY_ELEMENT_NOT_FOUND;
				iNextClear = ARRAY_ELEMENT_NOT_FOUND;
				for (i = iStart; i < iCount; i++)
				{
					if (GetBit(i, auiBits))
					{
						if (iNextSet == ARRAY_ELEMENT_NOT_FOUND)
						{
							iNextSet = i;
						}
					}
					else if (iNextClear == ARRAY_ELEMENT_NOT_FOUND)
					{
						iNextClear = i;
					}
				}
				AssertLSize((iNextSet == ARRAY_ELEMENT_NOT_FOUND) ? ARRAY_LARGE_ELEMENT_NOT_FOUND : iNextSet, FindNextSetBitLarge(auiBits, iCount, iStart));
				AssertLSize((iNextClear == ARRAY_ELEMENT_NOT_FOUND) ? ARRAY_LARGE_ELEMENT_NOT_FOUND : iNextClear, FindNextClearBitLarge(auiBits, iCount, iStart));
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// FindFirstClearBit(uint32) and FindFirstSetBit(uint64) for every bit,
// with the bits below it the other way and the ones above random.
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperFindBitInWord(void)
{
	size	i;
	uint32	uiSeed;
	uint32	ui;
	uint64	ulli;

	uiSeed = 54321;
	for (i = 0; i < 32; i++)
	{
		uiSeed = uiSeed * 1103515245 + 12345;
		ui = (uiSeed << i) | (((uint32)1 << i) - 1);
		ui &= ~((uint32)1 << i);
		AssertSize(i, FindFirstClearBit(ui));
	}
	AssertSize(32, FindFirstClearBit((uint32)0xFFFFFFFF));
	AssertSize(0, FindFirstClearBit((uint32)0));

	for (i = 0; i < 64; i++)
	{
		uiSeed = uiSeed * 1103515245 + 12345;
		ulli = (((uint64)uiSeed << 32) | uiSeed) << i;
		ulli |= (uint64)1 << i;
		AssertSize(i, FindFirstSetBit(ulli));
	}
	AssertSize(0, FindFirstSetBit((uint64)1));
}


//////////////////////////////////////////////////////////////////////////
//
// The Large bit finders past a size's count: 70000 bytes, so the words
// cross a 64KB bank and the count of words has a high word.
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperFindBitsLarge(void)
{
	uint8*	puiBits;
	lsize	uiBytes;
	lsize	uiBits;

	uiBytes = 70000;
	uiBits = uiBytes * 8;
	puiBits = (uint8*)malloc(uiBytes);
	AssertNotNull(puiBits);
	memset(puiBits, 0, uiBytes);

	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, FindFirstSetBitLarge(puiBits, uiBits));
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, FindLastSetBitLarge(puiBits, uiBits));
	AssertLSize(0, FindFirstClearBitLarge(puiBits, uiBits));
	AssertLSize(uiBits - 1, FindLastClearBitLarge(puiBits, uiBits));

	SetBitLarge(553333, puiBits, true);
	AssertLSize(553333, FindFirstSetBitLarge(puiBits, uiBits));
	AssertLSize(553333, FindLastSetBitLarge(puiBits, uiBits));
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, FindFirstSetBitLarge(puiBits, 553333));
	AssertLSize(553333, FindFirstSetBitLarge(puiBits, 553334));
	AssertLSize(553333, FindNextSetBitLarge(puiBits, uiBits, 70001));

	SetBitLarge(524290, puiBits, true);
	SetBitLarge(70001, puiBits, true);
	AssertLSize(70001, FindFirstSetBitLarge(puiBits, uiBits));
	AssertLSize(524290, FindNextSetBitLarge(puiBits, uiBits, 70002));
	AssertLSize(553333, FindLastSetBitLarge(puiBits, uiBits));
	AssertLSize(524290, FindLastSetBitLarge(puiBits, 553333));
	AssertLSize(70001, FindLastSetBitLarge(puiBits, 524290));

	memset(puiBits, 0xFF, uiBytes);
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, FindFirstClearBitLarge(puiBits, uiBits));
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, FindLastClearBitLarge(puiBits, uiBits));
	SetBitLarge(524287, puiBits, false);
	SetBitLarge(524288, puiBits, false);
	AssertLSize(524287, FindFirstClearBitLarge(puiBits, uiBits));
	AssertLSize(524288, FindLastClearBitLarge(puiBits, uiBits));
	AssertLSize(524288, FindNextClearBitLarge(puiBits, uiBits, 524288));
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, FindNextClearBitLarge(puiBits, uiBits, 524289));

	free(puiBits);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// i (0 to 9999) as a packed BCD word.
//
//////////////////////////////////////////////////////////////////////////
uint16 TestIntegerHelperToBCD(int32 i)
{
	return (uint16)(((i / 1000) << 12) | (((i / 100) % 10) << 8) | (((i / 10) % 10) << 4) | (i % 10));
}


//////////////////////////////////////////////////////////////////////////
//
// BCDAdd and BCDSubtract (the W65C816's decimal mode there) against
// binary arithmetic: one word, with and without a carry or borrow in, then
// carries and borrows going on through several words, and no words.
//
//////////////////////////////////////////////////////////////////////////
void TestIntegerHelperBCD(void)
{
	uint16	uiDest;
	uint16	uiSource;
	uint16	uiCarry;
	uint16	auiDest[3];
	uint16	auiSource[3];
	int32	a;
	int32	b;
	int32	c;
	int32	iSum;
	int32	iDifference;
	int32	iFailedA;
	int32	iFailedB;

	iFailedA = -1;
	iFailedB = -1;
	for (a = 0; (a < 10000) && (iFailedA < 0); a += 73)
	{
		for (b = 0; (b < 10000) && (iFailedA < 0); b += 89)
		{
			for (c = 0; c <= 1; c++)
			{
				iSum = a + b + c;
				uiDest = TestIntegerHelperToBCD(a);
				uiSource = TestIntegerHelperToBCD(b);
				uiCarry = BCDAdd(&uiDest, &uiSource, 1, (uint16)c);
				if ((uiDest != TestIntegerHelperToBCD(iSum % 10000)) || (uiCarry != ((iSum >= 10000) ? 1 : 0)))
				{
					iFailedA = a;
					iFailedB = b;
				}

				iDifference = a - b - c;
				uiDest = TestIntegerHelperToBCD(a);
				uiCarry = BCDSubtract(&uiDest, &uiSource, 1, (uint16)c);
				if ((uiDest != TestIntegerHelperToBCD((iDifference < 0) ? (iDifference + 10000) : iDifference)) || (uiCarry != ((iDifference < 0) ? 1 : 0)))
				{
					iFailedA = a;
					iFailedB = b;
				}
			}
		}
	}
	AssertInt((int32)-1, iFailedA);
	AssertInt((int32)-1, iFailedB);

	auiSource[0] = 0x0001;
	auiSource[1] = 0;
	auiSource[2] = 0;

	auiDest[0] = 0x9999;
	auiDest[1] = 0x9999;
	auiDest[2] = 0x0000;
	AssertShort((uint16)0, BCDAdd(auiDest, auiSource, 3, 0));
	AssertShortHex((uint16)0x0000, auiDest[0]);
	AssertShortHex((uint16)0x0000, auiDest[1]);
	AssertShortHex((uint16)0x0001, auiDest[2]);

	auiDest[0] = 0x9999;
	auiDest[1] = 0x9999;
	auiDest[2] = 0x9999;
	AssertShort((uint16)1, BCDAdd(auiDest, auiSource, 3, 0));
	AssertShortHex((uint16)0x0000, auiDest[0]);
	AssertShortHex((uint16)0x0000, auiDest[2]);

	auiDest[0] = 0x0000;
	auiDest[1] = 0x0000;
	auiDest[2] = 0x0001;
	AssertShort((uint16)0, BCDSubtract(auiDest, auiSource, 3, 0));
	AssertShortHex((uint16)0x9999, auiDest[0]);
	AssertShortHex((uint16)0x9999, auiDest[1]);
	AssertShortHex((uint16)0x0000, auiDest[2]);

	auiDest[0] = 0x0000;
	auiDest[1] = 0x0000;
	auiDest[2] = 0x0000;
	AssertShort((uint16)1, BCDSubtract(auiDest, auiSource, 3, 0));
	AssertShortHex((uint16)0x9999, auiDest[2]);

	auiDest[0] = 0x1234;
	auiDest[1] = 0x5678;
	auiSource[0] = 0x8766;
	auiSource[1] = 0x4321;
	AssertShort((uint16)1, BCDAdd(auiDest, auiSource, 2, 0));
	AssertShortHex((uint16)0x0000, auiDest[0]);
	AssertShortHex((uint16)0x0000, auiDest[1]);

	//No words: the carry (or borrow) goes straight out.
	AssertShort((uint16)1, BCDAdd(auiDest, auiSource, 0, 1));
	AssertShort((uint16)0, BCDAdd(auiDest, auiSource, 0, 0));
	AssertShort((uint16)1, BCDSubtract(auiDest, auiSource, 0, 1));
	AssertShortHex((uint16)0x0000, auiDest[0]);
}


void TestIntegerHelper(void)
{
	TestIntegerHelperFindFirstClearBit();
	TestIntegerHelperFindFirstSetBit();
	TestIntegerHelperFindLastClearBit();
	TestIntegerHelperFindLastSetBit();
	TestIntegerHelperReverseBytes();
	TestIntegerHelperCountBits();
	TestIntegerHelperFindNextSetBit();
	TestIntegerHelperFindNextClearBit();
	TestIntegerHelperSetBits();
	TestIntegerHelperBitFunctions();
	TestIntegerHelperMisc();
	TestIntegerHelperEndianness();
	TestIntegerHelperFindBitsAgainstGetBit();
	TestIntegerHelperFindBitInWord();
	TestIntegerHelperFindBitsLarge();
	TestIntegerHelperBCD();
}

