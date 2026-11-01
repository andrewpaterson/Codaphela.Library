#include "BaseLib/Chars.h"
#include "BaseLib/Line.h"
#include "BaseLib/LineBucket.h"
#include "BaseLib/LineMinimiser.h"
#include "BaseLib/LinePositionCache.h"
#include "BaseLib/LineSplitter.h"
#include "BaseLib/Positions.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertLine(const char* szExpected, CLine* pcLine)
{
	CChars	sz;

	sz.Init();
	pcLine->Print(&sz);
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// All the lines, separated by spaces.
//
//////////////////////////////////////////////////////////////////////////
void AssertLines(const char* szExpected, CArrayLine* pacLines)
{
	CChars	sz;
	size	i;

	sz.Init();
	for (i = 0; i < pacLines->NumElements(); i++)
	{
		if (i != 0)
		{
			sz.Append(" ");
		}
		pacLines->Get(i)->Print(&sz);
	}
	AssertString(szExpected, sz.Text());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Increasing y is North and increasing x is West, as in Logicim.
//
//////////////////////////////////////////////////////////////////////////
void TestLineDirection(void)
{
	CLine		cLine;
	SInt32Vec2	sStart;
	SInt32Vec2	sEnd;
	SInt32Vec2	sPosition;
	SInt32Vec2	sDimension;

	cLine.Init(0, 0, 0, 5);
	AssertInt(LD_North, cLine.GetDirection());
	AssertTrue(cLine.IsNorthSouth());
	AssertFalse(cLine.IsEastWest());
	AssertInt((int32)0, cLine.GetX());
	cLine.Init(0, 5, 0, 0);
	AssertInt(LD_South, cLine.GetDirection());
	cLine.Init(0, 0, 5, 0);
	AssertInt(LD_West, cLine.GetDirection());
	AssertTrue(cLine.IsEastWest());
	AssertFalse(cLine.IsNorthSouth());
	AssertInt((int32)0, cLine.GetY());
	cLine.Init(5, 0, 0, 0);
	AssertInt(LD_East, cLine.GetDirection());
	cLine.Init(0, 0, 1, 1);
	AssertInt(LD_Cannot, cLine.GetDirection());
	AssertFalse(cLine.IsEastWest());
	AssertFalse(cLine.IsNorthSouth());
	cLine.Init(2, 2, 2, 2);
	AssertInt(LD_Cannot, cLine.GetDirection());

	//Only lines that are horizontal or vertical and have a length.
	cLine.Init(7, 7, 8, 8);
	sStart.Init(3, 4);
	sEnd.Init(3, 4);
	AssertFalse(cLine.InitOrthogonal(&sStart, &sEnd));
	AssertLine("(7, 7) -> (8, 8)", &cLine);
	sEnd.Init(4, 5);
	AssertFalse(cLine.InitOrthogonal(&sStart, &sEnd));
	AssertFalse(cLine.InitOrthogonal(&sStart, NULL));
	sEnd.Init(3, -2);
	AssertTrue(cLine.InitOrthogonal(&sStart, &sEnd));
	AssertLine("(3, 4) -> (3, -2)", &cLine);
	AssertInt(LD_South, cLine.GetDirection());

	AssertInt((int32)3, cLine.GetMinimumX());
	AssertInt((int32)3, cLine.GetMaximumX());
	AssertInt((int32)-2, cLine.GetMinimumY());
	AssertInt((int32)4, cLine.GetMaximumY());
	AssertInt((int32)6, cLine.GetLength());
	cLine.GetCenter(&sPosition);
	AssertInt((int32)3, sPosition.x);
	AssertInt((int32)1, sPosition.y);
	cLine.GetBoundingBox(&sPosition, &sDimension);
	AssertInt((int32)3, sPosition.x);
	AssertInt((int32)-2, sPosition.y);
	AssertInt((int32)0, sDimension.x);
	AssertInt((int32)6, sDimension.y);

	cLine.Init(0, 0, 3, 4);
	AssertInt((int32)5, cLine.GetLength());
}


//////////////////////////////////////////////////////////////////////////
//
// Ends are on the line.  A diagonal line has no positions on it.
//
//////////////////////////////////////////////////////////////////////////
void TestLinePositionOn(void)
{
	CLine	cLine;

	cLine.Init(2, 1, 6, 1);
	AssertTrue(cLine.IsPositionOn(2, 1));
	AssertTrue(cLine.IsPositionOn(4, 1));
	AssertTrue(cLine.IsPositionOn(6, 1));
	AssertFalse(cLine.IsPositionOn(1, 1));
	AssertFalse(cLine.IsPositionOn(7, 1));
	AssertFalse(cLine.IsPositionOn(4, 2));

	cLine.Init(6, 1, 2, 1);
	AssertTrue(cLine.IsPositionOn(2, 1));
	AssertTrue(cLine.IsPositionOn(6, 1));
	AssertFalse(cLine.IsPositionOn(7, 1));

	cLine.Init(3, 5, 3, -5);
	AssertTrue(cLine.IsPositionOn(3, 0));
	AssertTrue(cLine.IsPositionOn(3, -5));
	AssertFalse(cLine.IsPositionOn(3, -6));
	AssertFalse(cLine.IsPositionOn(4, 0));

	cLine.Init(0, 0, 4, 4);
	AssertFalse(cLine.IsPositionOn(0, 0));
	AssertFalse(cLine.IsPositionOn(2, 2));
}


//////////////////////////////////////////////////////////////////////////
//
// How another line overlaps this one depends on this line's direction:
// Start is the part with this line's start.
//
//////////////////////////////////////////////////////////////////////////
void TestLineOverlap(void)
{
	CLine	cLine;
	CLine	cOther;

	cLine.Init(0, 0, 10, 0);
	cOther.Init(-2, 0, 12, 0);
	AssertInt(LO_Fully, cLine.GetOverlap(&cOther, true));
	cOther.Init(10, 0, 0, 0);
	AssertInt(LO_Fully, cLine.GetOverlap(&cOther, false));
	cOther.Init(2, 0, 5, 0);
	AssertInt(LO_Center, cLine.GetOverlap(&cOther, true));
	cOther.Init(-3, 0, 4, 0);
	AssertInt(LO_Start, cLine.GetOverlap(&cOther, true));
	cOther.Init(14, 0, 6, 0);
	AssertInt(LO_End, cLine.GetOverlap(&cOther, true));

	//Meeting at an end.
	cOther.Init(10, 0, 15, 0);
	AssertInt(LO_End, cLine.GetOverlap(&cOther, true));
	AssertInt(LO_None, cLine.GetOverlap(&cOther, false));
	cOther.Init(-5, 0, 0, 0);
	AssertInt(LO_Start, cLine.GetOverlap(&cOther, true));
	AssertInt(LO_None, cLine.GetOverlap(&cOther, false));
	cOther.Init(11, 0, 15, 0);
	AssertInt(LO_None, cLine.GetOverlap(&cOther, true));

	cOther.Init(0, 1, 5, 1);
	AssertInt(LO_None, cLine.GetOverlap(&cOther, true));
	cOther.Init(5, -5, 5, 5);
	AssertInt(LO_None, cLine.GetOverlap(&cOther, true));
	cOther.Init(0, 0, 5, 5);
	AssertInt(LO_None, cLine.GetOverlap(&cOther, true));

	//The same line the other way round.
	cLine.Init(10, 0, 0, 0);
	cOther.Init(-3, 0, 4, 0);
	AssertInt(LO_End, cLine.GetOverlap(&cOther, true));
	cOther.Init(6, 0, 14, 0);
	AssertInt(LO_Start, cLine.GetOverlap(&cOther, true));

	cLine.Init(4, 2, 4, 8);
	cOther.Init(4, 0, 4, 3);
	AssertInt(LO_Start, cLine.GetOverlap(&cOther, true));
	cOther.Init(4, 3, 4, 7);
	AssertInt(LO_Center, cLine.GetOverlap(&cOther, true));
	cOther.Init(5, 3, 5, 7);
	AssertInt(LO_None, cLine.GetOverlap(&cOther, true));

	cLine.Init(0, 0, 10, 0);
	cOther.Init(5, 0, 15, 0);
	AssertInt((int32)5, LineOverlapLength(&cLine, &cOther));
	cOther.Init(-5, 0, 3, 0);
	AssertInt((int32)3, LineOverlapLength(&cLine, &cOther));
	cOther.Init(2, 0, 4, 0);
	AssertInt((int32)2, LineOverlapLength(&cLine, &cOther));
	cOther.Init(2, 1, 4, 1);
	AssertInt((int32)0, LineOverlapLength(&cLine, &cOther));
	cOther.Init(2, 0, 2, 4);
	AssertInt((int32)0, LineOverlapLength(&cLine, &cOther));
}


//////////////////////////////////////////////////////////////////////////
//
// Lines touch if they overlap along the same axis or an end of one is on
// the other.  Crossing in the middle isn't touching.
//
//////////////////////////////////////////////////////////////////////////
void TestLineTouches(void)
{
	CLine	cLine;
	CLine	cOther;

	cLine.Init(0, 0, 10, 0);
	cOther.Init(10, 0, 12, 0);
	AssertInt(LO_End, cLine.Touches(&cOther));
	cOther.Init(5, 0, 5, 5);
	AssertInt(LO_Orthogonal, cLine.Touches(&cOther));
	cOther.Init(5, 5, 5, 0);
	AssertInt(LO_Orthogonal, cLine.Touches(&cOther));
	cOther.Init(0, -5, 0, 5);
	AssertInt(LO_Orthogonal, cLine.Touches(&cOther));
	AssertInt(LO_Orthogonal, cOther.Touches(&cLine));
	cOther.Init(5, -5, 5, 5);
	AssertInt(LO_None, cLine.Touches(&cOther));
	cOther.Init(11, -5, 11, 5);
	AssertInt(LO_None, cLine.Touches(&cOther));
	cOther.Init(0, 1, 10, 1);
	AssertInt(LO_None, cLine.Touches(&cOther));
}


//////////////////////////////////////////////////////////////////////////
//
// An extended line runs from the lowest to the highest of both lines.
//
//////////////////////////////////////////////////////////////////////////
void TestLineExtend(void)
{
	CLine	cLine;
	CLine	cOther;
	CLine	cCopy;

	cLine.Init(0, 0, 5, 0);
	cOther.Init(3, 0, 9, 0);
	cLine.Extend(&cOther);
	AssertLine("(0, 0) -> (9, 0)", &cLine);
	AssertInt(LD_West, cLine.GetDirection());

	cLine.Init(5, 0, 0, 0);
	cOther.Init(1, 0, -2, 0);
	cLine.Extend(&cOther);
	AssertLine("(-2, 0) -> (5, 0)", &cLine);
	AssertInt(LD_West, cLine.GetDirection());

	cLine.Init(1, 8, 1, 3);
	cOther.Init(1, 2, 1, 4);
	cLine.Extend(&cOther);
	AssertLine("(1, 2) -> (1, 8)", &cLine);
	AssertInt(LD_North, cLine.GetDirection());

	//Not on the same line.
	cOther.Init(2, 0, 2, 20);
	cLine.Extend(&cOther);
	AssertLine("(1, 2) -> (1, 8)", &cLine);
	cOther.Init(0, 5, 3, 5);
	cLine.Extend(&cOther);
	AssertLine("(1, 2) -> (1, 8)", &cLine);

	cCopy.Init(&cLine);
	AssertTrue(cCopy.Equals(&cLine));
	cCopy.Init(1, 8, 1, 2);
	AssertFalse(cCopy.Equals(&cLine));
}


//////////////////////////////////////////////////////////////////////////
//
// Lines on the same horizontal or vertical that overlap or meet end to end
// become one; a bucket grows until nothing after it overlaps.
//
//////////////////////////////////////////////////////////////////////////
void TestLineMinimiser(void)
{
	CArrayLine		acInput;
	CArrayLine		acOutput;
	CLineBucket		cBucket1;
	CLineBucket		cBucket2;
	CLine			cLine;

	acInput.Init();
	acInput.Add()->Init(0, 0, 4, 0);
	acInput.Add()->Init(10, 0, 6, 0);
	acInput.Add()->Init(0, 0, 0, 5);
	acInput.Add()->Init(4, 0, 6, 0);
	acInput.Add()->Init(0, 5, 0, 8);
	acInput.Add()->Init(2, -3, 2, 3);
	acInput.Add()->Init(20, 20, 20, 25);
	acInput.Add()->Init(20, 21, 20, 22);

	acOutput.Init();
	MinimiseLines(&acInput, &acOutput);
	AssertLines("(0, 0) -> (10, 0) (0, 0) -> (0, 8) (2, -3) -> (2, 3) (20, 20) -> (20, 25)", &acOutput);
	acOutput.Kill();

	acOutput.Init();
	acInput.ReInit();
	MinimiseLines(&acInput, &acOutput);
	AssertSize(0, acOutput.NumElements());
	acOutput.Kill();
	acInput.Kill();

	cLine.Init(0, 0, 4, 0);
	cBucket1.Init(&cLine);
	cLine.Init(6, 0, 4, 0);
	cBucket2.Init(&cLine);
	AssertTrue(cBucket1.Overlaps(&cBucket2));
	cBucket1.Merge(&cBucket2);
	AssertLine("(0, 0) -> (6, 0)", cBucket1.GetLine());
	AssertLines("(0, 0) -> (4, 0) (6, 0) -> (4, 0)", cBucket1.GetLines());
	cLine.Init(7, 0, 9, 0);
	cBucket2.Init(&cLine);
	AssertFalse(cBucket1.Overlaps(&cBucket2));
	cBucket2.Kill();
	cBucket1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Points are grouped by x in the order they were added, each once, and
// can be listed sorted.
//
//////////////////////////////////////////////////////////////////////////
void TestLinePositions(void)
{
	CPositions			cPositions;
	CArrayLine			acLines;
	CArrayInt32Vec2		asPositions;
	CPositionColumn*	pcColumn;

	acLines.Init();
	acLines.Add()->Init(0, 0, 4, 0);
	acLines.Add()->Init(4, 0, 4, 3);
	acLines.Add()->Init(0, 0, 0, 3);
	cPositions.Init(&acLines);
	acLines.Kill();

	AssertSize(2, cPositions.NumColumns());
	AssertSize(4, cPositions.NumPositions());
	pcColumn = cPositions.GetColumn(0);
	AssertInt((int32)0, pcColumn->GetX());
	AssertSize(2, pcColumn->NumYs());
	AssertInt((int32)0, pcColumn->GetY(0));
	AssertInt((int32)3, pcColumn->GetY(1));
	pcColumn = cPositions.GetColumn(1);
	AssertInt((int32)4, pcColumn->GetX());
	AssertInt((int32)0, pcColumn->GetY(0));
	AssertInt((int32)3, pcColumn->GetY(1));
	cPositions.Kill();

	cPositions.Init();
	cPositions.Add(5, 2);
	cPositions.Add(-1, 7);
	cPositions.Add(5, -4);
	cPositions.Add(-1, 7);
	AssertSize(2, cPositions.NumColumns());
	AssertSize(3, cPositions.NumPositions());
	AssertInt((int32)5, cPositions.GetColumn(0)->GetX());
	AssertInt((int32)2, cPositions.GetColumn(0)->GetY(0));
	AssertInt((int32)-4, cPositions.GetColumn(0)->GetY(1));

	asPositions.Init();
	cPositions.CalculatePositions(&asPositions);
	AssertSize(3, asPositions.NumElements());
	AssertInt((int32)-1, asPositions.Get(0)->x);
	AssertInt((int32)7, asPositions.Get(0)->y);
	AssertInt((int32)5, asPositions.Get(1)->x);
	AssertInt((int32)-4, asPositions.Get(1)->y);
	AssertInt((int32)5, asPositions.Get(2)->x);
	AssertInt((int32)2, asPositions.Get(2)->y);
	asPositions.Kill();
	cPositions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The cache finds the horizontal and vertical line through a point and
// splits them there.  Diagonal lines are dropped.
//
//////////////////////////////////////////////////////////////////////////
void TestLinePositionCache(void)
{
	CArrayLine			acLines;
	CArrayLine			acOutput;
	CLinePositionCache	cCache;

	acLines.Init();
	acLines.Add()->Init(0, 0, 10, 0);
	acLines.Add()->Init(5, -5, 5, 5);
	acLines.Add()->Init(0, 0, 3, 3);
	acLines.Add()->Init(10, 4, 0, 4);
	cCache.Init(&acLines);
	acLines.Kill();

	AssertLine("(0, 0) -> (10, 0)", cCache.GetHorizontalLine(3, 0));
	AssertLine("(10, 4) -> (0, 4)", cCache.GetHorizontalLine(10, 4));
	AssertNull(cCache.GetHorizontalLine(11, 0));
	AssertNull(cCache.GetHorizontalLine(3, 1));
	AssertLine("(5, -5) -> (5, 5)", cCache.GetVerticalLine(5, 0));
	AssertNull(cCache.GetVerticalLine(5, 6));
	AssertTrue(cCache.TouchesLine(5, 4));
	AssertTrue(cCache.TouchesLine(0, 0));
	AssertFalse(cCache.TouchesLine(1, 1));

	//At an end nothing is split; the crossing at 5, 0 splits both lines.
	cCache.Split(10, 0);
	cCache.Split(5, 5);
	cCache.Split(5, 0);
	acOutput.Init();
	cCache.GetLines(&acOutput);
	AssertLines("(0, 0) -> (5, 0) (5, 0) -> (10, 0) (10, 4) -> (0, 4) (5, -5) -> (5, 0) (5, 0) -> (5, 5)", &acOutput);
	acOutput.Kill();
	cCache.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Lines are split where one passes through a point at which another ends.
// Corners and crossings without a shared point aren't split unless the
// crossing is given as an extra junction.
//
//////////////////////////////////////////////////////////////////////////
void TestLineSplitter(void)
{
	CArrayLine			acLines;
	CArrayLine			acOutput;
	CPositions			cPositions;
	CLinePositionCache	cCache;
	CArrayInt32Vec2		asJunctions;

	acLines.Init();
	acLines.Add()->Init(0, 0, 10, 0);
	acLines.Add()->Init(5, 0, 5, 6);
	acLines.Add()->Init(8, -4, 8, 4);
	acLines.Add()->Init(0, 6, 5, 6);
	cPositions.Init(&acLines);

	acOutput.Init();
	SplitLines(&acLines, &cPositions, &acOutput);
	AssertLines("(0, 0) -> (5, 0) (5, 0) -> (10, 0) (0, 6) -> (5, 6) (5, 0) -> (5, 6) (8, -4) -> (8, 4)", &acOutput);
	acOutput.Kill();

	cCache.Init(&acLines);
	asJunctions.Init();
	CalculateLineJunctions(&cPositions, &cCache, &asJunctions);
	AssertSize(1, asJunctions.NumElements());
	AssertInt((int32)5, asJunctions.Get(0)->x);
	AssertInt((int32)0, asJunctions.Get(0)->y);

	asJunctions.ReInit();
	asJunctions.Add()->Init(8, 0);
	asJunctions.Add()->Init(5, 0);
	acOutput.Init();
	SplitLines(&cCache, &cPositions, &asJunctions, &acOutput);
	AssertLines("(0, 0) -> (5, 0) (5, 0) -> (8, 0) (8, 0) -> (10, 0) (0, 6) -> (5, 6) (5, 0) -> (5, 6) (8, -4) -> (8, 0) (8, 0) -> (8, 4)", &acOutput);
	acOutput.Kill();
	asJunctions.Kill();
	cCache.Kill();

	cPositions.Kill();
	acLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestLine(void)
{
	BeginTests();

	TestLineDirection();
	TestLinePositionOn();
	TestLineOverlap();
	TestLineTouches();
	TestLineExtend();
	TestLineMinimiser();
	TestLinePositions();
	TestLinePositionCache();
	TestLineSplitter();

	TestStatistics();
}

