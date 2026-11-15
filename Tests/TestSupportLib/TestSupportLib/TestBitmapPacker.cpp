#include "SupportLib/BitmapPacker.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Bits set and read, 32 of them from any bit (across two words), counted,
// grown (one bit is nine, a bit wider all round) and turned a quarter
// anticlockwise (an L's foot, along the bottom, goes up its right).
//
//////////////////////////////////////////////////////////////////////////
void TestBitmapPackerBitRows(void)
{
	CBitRows	cBits;
	CBitRows	cGrown;
	CBitRows	cTurned;
	int			x;

	cBits.Init(40, 3);
	AssertInt(2, cBits.miWords);
	for (x = 30; x < 36; x++)
	{
		cBits.Set(x, 1);
	}
	AssertTrue(cBits.Get(31, 1));
	AssertTrue(cBits.Get(32, 1));
	AssertFalse(cBits.Get(36, 1));
	AssertFalse(cBits.Get(31, 0));
	AssertFalse(cBits.Get(-1, 1));
	AssertInt((uint32)0xFC, cBits.GetWord(1, 28));
	AssertInt((uint32)0x0F, cBits.GetWord(1, 32));
	AssertInt((uint32)0, cBits.GetWord(0, 28));
	AssertInt(6, cBits.Count());
	cBits.Kill();

	cBits.Init(3, 3);
	cBits.Set(1, 1);
	cBits.Grow(&cGrown);
	AssertInt(5, cGrown.miWidth);
	AssertInt(5, cGrown.miHeight);
	AssertInt(9, cGrown.Count());
	AssertTrue(cGrown.Get(1, 1));
	AssertTrue(cGrown.Get(3, 3));
	AssertFalse(cGrown.Get(0, 2));
	AssertFalse(cGrown.Get(4, 2));
	cGrown.Kill();
	cBits.Kill();

	cBits.Init(3, 2);
	cBits.Set(0, 0);
	cBits.Set(1, 0);
	cBits.Set(2, 0);
	cBits.Set(0, 1);
	cBits.Turn(&cTurned);
	AssertInt(2, cTurned.miWidth);
	AssertInt(3, cTurned.miHeight);
	AssertInt(4, cTurned.Count());
	AssertTrue(cTurned.Get(1, 0));
	AssertTrue(cTurned.Get(1, 1));
	AssertTrue(cTurned.Get(1, 2));
	AssertTrue(cTurned.Get(0, 0));
	cTurned.Kill();
	cBits.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Into a space four wide: an L three by three in the corner, then a two
// by two square in the L's inside (where it first fits, lowest and then
// leftmost), then a row four wide above them both.  A shape five wide
// doesn't fit.
//
//////////////////////////////////////////////////////////////////////////
void TestBitmapPackerPack(void)
{
	CBitmapPacker	cPacker;
	CBitRows		cL;
	CBitRows		cSquare;
	CBitRows		cRow;
	CBitRows		cWide;
	int				x;
	int				y;

	cL.Init(3, 3);
	cL.Set(0, 0);
	cL.Set(1, 0);
	cL.Set(2, 0);
	cL.Set(0, 1);
	cL.Set(0, 2);
	cSquare.Init(2, 2);
	cSquare.Set(0, 0);
	cSquare.Set(1, 0);
	cSquare.Set(0, 1);
	cSquare.Set(1, 1);
	cRow.Init(4, 1);
	for (x = 0; x < 4; x++)
	{
		cRow.Set(x, 0);
	}
	cWide.Init(5, 1);

	cPacker.Init(4);
	AssertInt(0, cPacker.GetHeight());
	AssertTrue(cPacker.Find(&cL, &x, &y));
	AssertInt(0, x);
	AssertInt(0, y);
	cPacker.Add(&cL, x, y);
	AssertInt(3, cPacker.GetHeight());

	AssertTrue(cPacker.Find(&cSquare, &x, &y));
	AssertInt(1, x);
	AssertInt(1, y);
	AssertTrue(cPacker.Fits(&cSquare, 2, 1));
	AssertFalse(cPacker.Fits(&cSquare, 0, 1));
	AssertFalse(cPacker.Fits(&cSquare, 3, 1));
	cPacker.Add(&cSquare, x, y);
	AssertInt(3, cPacker.GetHeight());

	AssertTrue(cPacker.Find(&cRow, &x, &y));
	AssertInt(0, x);
	AssertInt(3, y);
	cPacker.Add(&cRow, x, y);
	AssertInt(4, cPacker.GetHeight());
	AssertInt(5 + 4 + 4, cPacker.mcSpace.Count());

	AssertFalse(cPacker.Find(&cWide, &x, &y));

	cPacker.Kill();
	cWide.Kill();
	cRow.Kill();
	cSquare.Kill();
	cL.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestBitmapPacker(void)
{
	BeginTests();

	TestBitmapPackerBitRows();
	TestBitmapPackerPack();

	TestStatistics();
}

