#include "BaseLib/ArrayBit.h"
#include "BaseLib/ArrayElementNotFound.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/GlobalMemory.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayBitAdd(void)
{
	CArrayBit		cArrayBit;
	uint32	ui[3];

	cArrayBit.Init();

	cArrayBit.Add(true);
	cArrayBit.Add(false);

	AssertLSize(2, cArrayBit.NumElements());

	AssertTrue(cArrayBit.Get(0));
	AssertFalse(cArrayBit.Get(1));

	cArrayBit.Add(true);
	cArrayBit.Add(false);
	cArrayBit.Add(true);
	cArrayBit.Add(false);
	cArrayBit.Add(true);
	cArrayBit.Add(false);

	AssertLSize(8, cArrayBit.NumElements());

	AssertTrue(cArrayBit.Get(2));
	AssertFalse(cArrayBit.Get(3));
	AssertTrue(cArrayBit.Get(4));
	AssertFalse(cArrayBit.Get(5));
	AssertTrue(cArrayBit.Get(6));
	AssertFalse(cArrayBit.Get(7));

	ui[0] = 0xffcbb78e;
	ui[1] = 0x18011861;
	ui[2] = 0x0082d97e;

	cArrayBit.Add(ui, 0, 88);

	AssertLSize(96, cArrayBit.NumElements());

	AssertFalse(cArrayBit.Get(8));
	AssertTrue(cArrayBit.Get(9));
	AssertTrue(cArrayBit.Get(10));
	AssertTrue(cArrayBit.Get(11));
	AssertFalse(cArrayBit.Get(12));
	AssertFalse(cArrayBit.Get(13));
	AssertFalse(cArrayBit.Get(14));
	AssertTrue(cArrayBit.Get(15));

	AssertIntHex((uint32)0xcbb78e55, ((uint32*)cArrayBit.GetData())[0]);
	AssertIntHex((uint32)0x011861ff, ((uint32*)cArrayBit.GetData())[1]);
	AssertIntHex((uint32)0x82d97e18, ((uint32*)cArrayBit.GetData())[2]);

	cArrayBit.Add(true);

	AssertLSize(97, cArrayBit.NumElements());
	AssertTrue(cArrayBit.Get(96));

	AssertIntHex((uint32)0xcbb78e55, ((uint32*)cArrayBit.GetData())[0]);
	AssertIntHex((uint32)0x011861ff, ((uint32*)cArrayBit.GetData())[1]);
	AssertIntHex((uint32)0x82d97e18, ((uint32*)cArrayBit.GetData())[2]);

	cArrayBit.Add(false);
	cArrayBit.Add(true);
	cArrayBit.Add(false);
	cArrayBit.Add(true);
	cArrayBit.Add(false);
	cArrayBit.Add(true);
	cArrayBit.Add(false);

	AssertLSize(104, cArrayBit.NumElements());

	cArrayBit.Add(true);

	AssertLSize(105, cArrayBit.NumElements());
	AssertTrue(cArrayBit.Get(104));

	AssertFalse(cArrayBit.Get(97));
	AssertTrue(cArrayBit.Get(98));
	AssertFalse(cArrayBit.Get(99));
	AssertTrue(cArrayBit.Get(100));
	AssertFalse(cArrayBit.Get(101));
	AssertTrue(cArrayBit.Get(102));
	AssertFalse(cArrayBit.Get(103));
	AssertTrue(cArrayBit.Get(104));

	cArrayBit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayBitReadWrite(void)
{
	CFileBasic		cFile;
	CArrayBit		cArrayBit;
	CArrayBit		cArrayBitIn;
	uint32	ui[3];
	cArrayBit.Init();

	cArrayBit.Add(true);
	cArrayBit.Add(false);

	cFile.Init(MemoryFile());
	cFile.Open(EFM_Write_Create);
	AssertTrue(cArrayBit.Write(&cFile));

	cFile.Close();
	cFile.Open(EFM_Read);
	
	cArrayBitIn.Read(&cFile);

	AssertLSize(2, cArrayBitIn.NumElements());

	AssertTrue(cArrayBitIn.Get(0));
	AssertFalse(cArrayBitIn.Get(1));
	cArrayBitIn.Kill();

	cArrayBit.Add(true);
	cArrayBit.Add(false);
	cArrayBit.Add(true);
	cArrayBit.Add(false);
	cArrayBit.Add(true);
	cArrayBit.Add(false);
	ui[0] = 0xffcbb78e;
	ui[1] = 0x18011861;
	ui[2] = 0x0082d97e;
	cArrayBit.Add(ui, 0, 88);
	cArrayBit.Add(true);
	cArrayBit.Add(false);
	cArrayBit.Add(true);
	cArrayBit.Add(false);
	cArrayBit.Add(true);
	cArrayBit.Add(false);
	cArrayBit.Add(true);
	cArrayBit.Add(false);
	cArrayBit.Add(true);
	AssertLSize(105, cArrayBit.NumElements());

	cFile.Open(EFM_Write_Create);
	AssertTrue(cArrayBit.Write(&cFile));

	cFile.Close();
	cArrayBit.Kill();

	cFile.Open(EFM_Read);
	cArrayBitIn.Read(&cFile);

	AssertLSize(105, cArrayBitIn.NumElements());
	AssertTrue(cArrayBitIn.Get(0));
	AssertFalse(cArrayBitIn.Get(1));
	AssertIntHex((uint32)0xcbb78e55, ((uint32*)cArrayBitIn.GetData())[0]);
	AssertIntHex((uint32)0x011861ff, ((uint32*)cArrayBitIn.GetData())[1]);
	AssertIntHex((uint32)0x82d97e18, ((uint32*)cArrayBitIn.GetData())[2]);
	AssertTrue(cArrayBitIn.Get(96));
	AssertFalse(cArrayBitIn.Get(97));
	AssertTrue(cArrayBitIn.Get(98));
	AssertFalse(cArrayBitIn.Get(99));
	AssertTrue(cArrayBitIn.Get(100));
	AssertFalse(cArrayBitIn.Get(101));
	AssertTrue(cArrayBitIn.Get(102));
	AssertFalse(cArrayBitIn.Get(103));
	AssertTrue(cArrayBitIn.Get(104));

	cArrayBitIn.Kill();
	cFile.Close();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// More bits than a natural size counts on a 16 bit size.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayBitLarge(void)
{
	CArrayBit	cArrayBit;
	uint8		auiBits[2];

	cArrayBit.Init();
	cArrayBit.GrowTo(70000);
	cArrayBit.Zero();
	AssertLSize(70000, cArrayBit.NumElements());

	cArrayBit.Set(69999, true);
	cArrayBit.Set(65536, true);
	AssertTrue(cArrayBit.Get(69999));
	AssertTrue(cArrayBit.Get(65536));
	AssertFalse(cArrayBit.Get(65535));
	AssertFalse(cArrayBit.Get(69998));
	AssertFalse(cArrayBit.Get(464));

	auiBits[0] = 0;
	auiBits[1] = 0;
	cArrayBit.Get(auiBits, 0, 69992, 8);
	AssertByteHex((uint8)0x80, auiBits[0]);

	cArrayBit.Add(true);
	AssertLSize(70001, cArrayBit.NumElements());
	AssertTrue(cArrayBit.Get(70000));
	AssertTrue(cArrayBit.Get(69999));

	cArrayBit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A bit past the end is false and setting it does nothing, in the bits
// kept in the array and in those it allocates.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayBitPastEnd(void)
{
	CArrayBit	cArrayBit;

	cArrayBit.Init();
	cArrayBit.GrowTo(10);
	cArrayBit.Zero();
	AssertFalse(cArrayBit.IsLarge());
	cArrayBit.Set(10, true);
	cArrayBit.Set(95, true);
	AssertFalse(cArrayBit.Get(10));
	AssertFalse(cArrayBit.Get(95));
	cArrayBit.GrowTo(11);
	AssertFalse(cArrayBit.Get(10));

	cArrayBit.GrowTo(200);
	cArrayBit.Zero();
	AssertTrue(cArrayBit.IsLarge());
	cArrayBit.Set(200, true);
	AssertFalse(cArrayBit.Get(200));
	cArrayBit.Set(199, true);
	AssertTrue(cArrayBit.Get(199));

	cArrayBit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// CountSet, CountClear, FindSet and FindClear: empty, small (in the array
// itself) and large arrays, the first bit and the last, none set and all.
//
//////////////////////////////////////////////////////////////////////////
void TestArrayBitScans(void)
{
	CArrayBit	ab;
	lsize		i;
	lsize		uiCount;
	lsize		uiBit;
	bool		bResult;

	ab.Init();
	AssertLSize(0, ab.CountSet());
	AssertLSize(0, ab.CountClear());
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, ab.FindSet(0));
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, ab.FindClear(0));

	//Small: 70 bits, held in the array itself.
	for (i = 0; i < 70; i++)
	{
		ab.Add((i % 7) == 0);
	}
	AssertFalse(ab.IsLarge());
	AssertLSize(10, ab.CountSet());
	AssertLSize(60, ab.CountClear());
	AssertLSize(0, ab.FindSet(0));
	AssertLSize(7, ab.FindSet(1));
	AssertLSize(63, ab.FindSet(57));
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, ab.FindSet(64));
	AssertLSize(1, ab.FindClear(0));
	AssertLSize(8, ab.FindClear(7));
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, ab.FindClear(70));
	ab.Set(69, true);
	AssertLSize(69, ab.FindSet(64));
	AssertLSize(11, ab.CountSet());
	ab.Kill();

	//Large: more bits than ARRAY_BIT_COUNT_CHUNK counts at a time, every 3rd set.
	ab.Init();
	for (i = 0; i < 100003; i++)
	{
		ab.Add((i % 3) == 0);
	}
	AssertTrue(ab.IsLarge());
	AssertLSize(33335, ab.CountSet());
	AssertLSize(100003 - 33335, ab.CountClear());
	uiCount = 0;
	bResult = true;
	for (uiBit = ab.FindSet(0); uiBit != ARRAY_LARGE_ELEMENT_NOT_FOUND; uiBit = ab.FindSet(uiBit + 1))
	{
		bResult &= ((uiBit % 3) == 0);
		uiCount++;
	}
	AssertTrue(bResult);
	AssertLSize(33335, uiCount);
	AssertLSize(100002, ab.FindSet(99999 + 1));
	AssertLSize(100000, ab.FindClear(100000));
	AssertLSize(100000, ab.FindClear(99999));

	//None set, then all.
	ab.Zero();
	AssertLSize(0, ab.CountSet());
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, ab.FindSet(0));
	for (i = 0; i < 100003; i++)
	{
		ab.Set(i, true);
	}
	AssertLSize(100003, ab.CountSet());
	AssertLSize(ARRAY_LARGE_ELEMENT_NOT_FOUND, ab.FindClear(0));
	ab.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestArrayBit(void)
{
	BeginTests();
	MemoryInit();

	TestArrayBitAdd();
	TestArrayBitReadWrite();
	TestArrayBitLarge();
	TestArrayBitPastEnd();
	TestArrayBitScans();

	MemoryKill();
	TestStatistics();
}

