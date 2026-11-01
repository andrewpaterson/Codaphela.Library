#include "BaseLib/Int16Vec2.h"
#include "SupportLib/Int32Rect.h"
#include "SupportLib/Float64Rect.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertRectangle(int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, CInt32Rect* pcRectangle)
{
	AssertInt(iLeft, pcRectangle->GetLeft());
	AssertInt(iTop, pcRectangle->GetTop());
	AssertInt(iRight, pcRectangle->GetRight());
	AssertInt(iBottom, pcRectangle->GetBottom());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleClipTo(void)
{
	CInt32Rect	cBounds;
	CInt32Rect	cRectangle;

	cBounds.Init(0, 0, 10, 10);

	//A rectangle already inside its bounds is not changed.
	cRectangle.Init(2, 3, 8, 9);
	cRectangle.ClipTo(&cBounds);
	AssertRectangle(2, 3, 8, 9, &cRectangle);

	//Overhanging every side it is cut back to the bounds.
	cRectangle.Init(-5, -4, 15, 14);
	cRectangle.ClipTo(&cBounds);
	AssertRectangle(0, 0, 10, 10, &cRectangle);

	//Only the sides that overhang are cut back.
	cRectangle.Init(-3, 2, 6, 12);
	cRectangle.ClipTo(&cBounds);
	AssertRectangle(0, 2, 6, 10, &cRectangle);

	//Bounds smaller than the rectangle leave just the bounds.
	cRectangle.Init(0, 0, 20, 20);
	cBounds.Init(5, 6, 7, 8);
	cRectangle.ClipTo(&cBounds);
	AssertRectangle(5, 6, 7, 8, &cRectangle);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleGetArea(void)
{
	CInt32Rect	cRect;

	cRect.Init(2, 3, 12, 8);
	AssertLong(50, cRect.GetArea());

	//A 50mm square in micrometres is more than 32 bits hold.
	cRect.Init(0, 0, 50000, 50000);
	AssertLong(2500000000LL, cRect.GetArea());

	cRect.Init(-60000, -70000, 60000, 70000);
	AssertLong(16800000000LL, cRect.GetArea());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleInt16(void)
{
	CRectangle<SInt16Vec2, int16, int32>	cRect;

	//300 x 300 doesn't fit in 16 bits; the area is the double wide type.
	cRect.Init(-100, -50, 200, 250);
	AssertInt(300, cRect.GetWidth());
	AssertInt(300, cRect.GetHeight());
	AssertInt(90000, cRect.GetArea());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleFloat64(void)
{
	CFloat64Rect	cRect;
	CFloat64Rect	cOther;
	SFloat64Vec2	sTopLeft;
	SFloat64Vec2	sBottomRight;

	//The Clock Doubler's edge cuts in millimetres.
	sTopLeft.Init(-1.778, -30.553733);
	sBottomRight.Init(17.018003, 2.6162);
	cRect.Init(sTopLeft, sBottomRight);
	AssertDouble(18.796003, cRect.GetWidth(), 9);
	AssertDouble(33.169933, cRect.GetHeight(), 9);
	AssertDouble(18.796003 * 33.169933, (float64)cRect.GetArea(), 6);
	AssertTrue(cRect.IsValid());
	AssertTrue(cRect.IsIn(0.5, -10.25));
	AssertFalse(cRect.IsIn(17.5, 0.0));

	//Moved so its top left is the origin, the size stays.
	cRect.SetPos(0.0, 0.0);
	AssertDouble(0.0, cRect.GetLeft(), 9);
	AssertDouble(18.796003, cRect.GetRight(), 9);
	AssertDouble(33.169933, cRect.GetBottom(), 9);

	cOther.Init(10.0, 20.0, 30.0, 40.0);
	AssertTrue(cRect.Intersect(&cOther));
	cRect.ClipTo(&cOther);
	AssertDouble(10.0, cRect.GetLeft(), 9);
	AssertDouble(20.0, cRect.GetTop(), 9);
	AssertDouble(18.796003, cRect.GetRight(), 9);
	AssertDouble(33.169933, cRect.GetBottom(), 9);
	AssertTrue(cRect.Inside(&cOther));

	cOther.Init(0.25, 0.5);
	AssertDouble(0.125, (float64)cOther.GetArea(), 9);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangle(void)
{
	BeginTests();

	TestRectangleClipTo();
	TestRectangleGetArea();
	TestRectangleInt16();
	TestRectangleFloat64();

	TestStatistics();
}

