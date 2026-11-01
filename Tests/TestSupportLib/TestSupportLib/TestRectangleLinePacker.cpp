#include "SupportLib/RectangleLinePacker.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleLinePackerLeftToRight(void)
{
	CRectangleLinePacker	cPacker;
	CInt32Rect				cRect1;
	CInt32Rect				cRect2;
	char					sz1[] = "1";
	char					sz2[] = "2";
	CArrayPackedRectangle	macRects;
	CPackedRectangle*		pcPackedRect;
	SInt32Vec2				sImageSize;

	cRect1.Init(0, 0, 3, 2);
	cRect2.Init(0, 0, 4, 5);

	cPacker.Init(true, false, 2, 1);
	cPacker.AddRectangle(&cRect1, sz1);
	cPacker.AddRectangle(&cRect2, sz2);
	cPacker.Pack();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(2, macRects.NumElements());

	pcPackedRect = macRects.Get(0);
	AssertString(sz1, (char*)pcPackedRect->GetUserData());
	AssertInt((int32)2, pcPackedRect->msTopLeft.x);
	AssertInt((int32)2, pcPackedRect->msTopLeft.y);
	AssertInt((int32)5, pcPackedRect->msBottomRight.x);
	AssertInt((int32)4, pcPackedRect->msBottomRight.y);

	//One pixel gap between the rectangles.
	pcPackedRect = macRects.Get(1);
	AssertString(sz2, (char*)pcPackedRect->GetUserData());
	AssertInt((int32)6, pcPackedRect->msTopLeft.x);
	AssertInt((int32)2, pcPackedRect->msTopLeft.y);
	AssertInt((int32)10, pcPackedRect->msBottomRight.x);
	AssertInt((int32)7, pcPackedRect->msBottomRight.y);
	macRects.Kill();

	sImageSize = cPacker.GetPackedImageSize();
	AssertInt((int32)12, sImageSize.x);
	AssertInt((int32)9, sImageSize.y);

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleLinePackerTopToBottom(void)
{
	CRectangleLinePacker	cPacker;
	CInt32Rect				cRect1;
	CInt32Rect				cRect2;
	CArrayPackedRectangle	macRects;
	CPackedRectangle*		pcPackedRect;
	SInt32Vec2				sImageSize;

	cRect1.Init(0, 0, 3, 2);
	cRect2.Init(0, 0, 4, 5);

	cPacker.Init(false, false, 2, 1);
	cPacker.AddRectangle(&cRect1, NULL);
	cPacker.AddRectangle(&cRect2, NULL);
	cPacker.Pack();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(2, macRects.NumElements());

	pcPackedRect = macRects.Get(0);
	AssertInt((int32)2, pcPackedRect->msTopLeft.x);
	AssertInt((int32)2, pcPackedRect->msTopLeft.y);
	AssertInt((int32)5, pcPackedRect->msBottomRight.x);
	AssertInt((int32)4, pcPackedRect->msBottomRight.y);

	//One pixel gap between the rectangles.
	pcPackedRect = macRects.Get(1);
	AssertInt((int32)2, pcPackedRect->msTopLeft.x);
	AssertInt((int32)5, pcPackedRect->msTopLeft.y);
	AssertInt((int32)6, pcPackedRect->msBottomRight.x);
	AssertInt((int32)10, pcPackedRect->msBottomRight.y);
	macRects.Kill();

	sImageSize = cPacker.GetPackedImageSize();
	AssertInt((int32)8, sImageSize.x);
	AssertInt((int32)12, sImageSize.y);

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleLinePackerRejectsEmptyRectangles(void)
{
	CRectangleLinePacker	cPacker;
	CInt32Rect				cRect;
	CArrayPackedRectangle	macRects;

	cPacker.Init(true, false, 0, 0);

	cRect.Init(0, 0, 0, 5);
	AssertFalse(cPacker.AddRectangle(&cRect, NULL));
	cRect.Init(0, 0, 5, 0);
	AssertFalse(cPacker.AddRectangle(&cRect, NULL));
	cRect.Init(5, 0, 2, 3);
	AssertFalse(cPacker.AddRectangle(&cRect, NULL));
	cRect.Init(0, 5, 3, 2);
	AssertFalse(cPacker.AddRectangle(&cRect, NULL));

	cRect.Init(0, 0, 3, 3);
	AssertTrue(cPacker.AddRectangle(&cRect, NULL));
	cPacker.Pack();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(1, macRects.NumElements());
	macRects.Kill();

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleLinePackerPow2(void)
{
	CRectangleLinePacker	cPacker;
	CInt32Rect				cRect1;
	CInt32Rect				cRect2;
	CArrayPackedRectangle	macRects;
	CPackedRectangle*		pcPackedRect;
	SInt32Vec2				sImageSize;

	cRect1.Init(0, 0, 3, 2);
	cRect2.Init(0, 0, 4, 5);

	//12x9 rounds up to 16x16.  The rectangles are where they would be without rounding.
	cPacker.Init(true, true, 2, 1);
	cPacker.AddRectangle(&cRect1, NULL);
	cPacker.AddRectangle(&cRect2, NULL);
	cPacker.Pack();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(2, macRects.NumElements());
	pcPackedRect = macRects.Get(1);
	AssertInt((int32)6, pcPackedRect->msTopLeft.x);
	AssertInt((int32)2, pcPackedRect->msTopLeft.y);
	AssertInt((int32)10, pcPackedRect->msBottomRight.x);
	AssertInt((int32)7, pcPackedRect->msBottomRight.y);
	macRects.Kill();

	sImageSize = cPacker.GetPackedImageSize();
	AssertInt((int32)16, sImageSize.x);
	AssertInt((int32)16, sImageSize.y);
	cPacker.Kill();

	//8x12 rounds up to 8x16.
	cPacker.Init(false, true, 2, 1);
	cPacker.AddRectangle(&cRect1, NULL);
	cPacker.AddRectangle(&cRect2, NULL);
	cPacker.Pack();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(2, macRects.NumElements());
	pcPackedRect = macRects.Get(1);
	AssertInt((int32)2, pcPackedRect->msTopLeft.x);
	AssertInt((int32)5, pcPackedRect->msTopLeft.y);
	AssertInt((int32)6, pcPackedRect->msBottomRight.x);
	AssertInt((int32)10, pcPackedRect->msBottomRight.y);
	macRects.Kill();

	sImageSize = cPacker.GetPackedImageSize();
	AssertInt((int32)8, sImageSize.x);
	AssertInt((int32)16, sImageSize.y);
	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleLinePackerEmpty(void)
{
	CRectangleLinePacker	cPacker;
	CArrayPackedRectangle	macRects;
	SInt32Vec2				sImageSize;

	cPacker.Init(true, false, 2, 1);
	cPacker.Pack();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(0, macRects.NumElements());
	macRects.Kill();

	sImageSize = cPacker.GetPackedImageSize();
	AssertInt((int32)0, sImageSize.x);
	AssertInt((int32)0, sImageSize.y);
	cPacker.Kill();

	cPacker.Init(false, true, 2, 1);
	cPacker.Pack();
	sImageSize = cPacker.GetPackedImageSize();
	AssertInt((int32)0, sImageSize.x);
	AssertInt((int32)0, sImageSize.y);
	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleLinePacker(void)
{
	BeginTests();

	TestRectangleLinePackerLeftToRight();
	TestRectangleLinePackerTopToBottom();
	TestRectangleLinePackerRejectsEmptyRectangles();
	TestRectangleLinePackerPow2();
	TestRectangleLinePackerEmpty();

	TestStatistics();
}

