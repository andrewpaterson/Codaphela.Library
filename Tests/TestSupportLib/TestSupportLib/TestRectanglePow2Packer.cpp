#include "SupportLib/RectanglePow2Packer.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePow2PackerStuff(void)
{
	CRectanglePow2Packer	cPacker;
	CInt32Rect				cRect1;
	CInt32Rect				cRect2;
	CInt32Rect				cRect3;
	char					sz1[] = "1";
	char					sz2[] = "2";
	char					sz3[] = "3";
	CArrayPackedRectangle	macRects;
	CPackedRectangle*		pcPackedRect;
	SInt32Vec2					sImageSize;

	cRect1.Init(0, 0, 7, 5);
	cRect2.Init(0, 0, 3, 5);
	cRect3.Init(0, 0, 4, 5);

	cPacker.Init(0, 0);

	cPacker.AddRectangle(&cRect1, sz1);
	cPacker.AddRectangle(&cRect2, sz2);
	cPacker.AddRectangle(&cRect3, sz3);

	cPacker.Pack();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);

	AssertSize(3, macRects.NumElements());

	pcPackedRect = macRects.Get(0);
	AssertInt((int32)0, pcPackedRect->msTopLeft.x);
	AssertInt((int32)0, pcPackedRect->msTopLeft.y);
	AssertInt((int32)7, pcPackedRect->msBottomRight.x);
	AssertInt((int32)5, pcPackedRect->msBottomRight.y);

	pcPackedRect = macRects.Get(1);
	AssertInt((int32)7, pcPackedRect->msTopLeft.x);
	AssertInt((int32)0, pcPackedRect->msTopLeft.y);
	AssertInt((int32)11, pcPackedRect->msBottomRight.x);
	AssertInt((int32)5, pcPackedRect->msBottomRight.y);

	pcPackedRect = macRects.Get(2);
	AssertInt((int32)11, pcPackedRect->msTopLeft.x);
	AssertInt((int32)0, pcPackedRect->msTopLeft.y);
	AssertInt((int32)14, pcPackedRect->msBottomRight.x);
	AssertInt((int32)5, pcPackedRect->msBottomRight.y);

	sImageSize = cPacker.GetPackedImageSize();
	AssertInt((int32)16, sImageSize.x);
	AssertInt((int32)8, sImageSize.y);

	macRects.Kill();
	cPacker.Kill();

	/////
	
	cPacker.Init(2, 1);

	cPacker.AddRectangle(&cRect1, sz1);
	cPacker.AddRectangle(&cRect2, sz2);
	cPacker.AddRectangle(&cRect3, sz3);

	cPacker.Pack();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);

	AssertSize(3, macRects.NumElements());

	pcPackedRect = macRects.Get(0);
	AssertInt((int32)2, pcPackedRect->msTopLeft.x);
	AssertInt((int32)2, pcPackedRect->msTopLeft.y);
	AssertInt((int32)9, pcPackedRect->msBottomRight.x);
	AssertInt((int32)7, pcPackedRect->msBottomRight.y);

	pcPackedRect = macRects.Get(1);
	AssertInt((int32)10, pcPackedRect->msTopLeft.x);
	AssertInt((int32)2, pcPackedRect->msTopLeft.y);
	AssertInt((int32)14, pcPackedRect->msBottomRight.x);
	AssertInt((int32)7, pcPackedRect->msBottomRight.y);

	pcPackedRect = macRects.Get(2);
	AssertInt((int32)2, pcPackedRect->msTopLeft.x);
	AssertInt((int32)8, pcPackedRect->msTopLeft.y);
	AssertInt((int32)5, pcPackedRect->msBottomRight.x);
	AssertInt((int32)13, pcPackedRect->msBottomRight.y);

	sImageSize = cPacker.GetPackedImageSize();
	AssertInt((int32)16, sImageSize.x);
	AssertInt((int32)16, sImageSize.y);

	macRects.Kill();
	cPacker.Kill();
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePow2PackerEmpty(void)
{
	CRectanglePow2Packer	cPacker;
	CArrayPackedRectangle	macRects;
	SInt32Vec2				sImageSize;

	cPacker.Init(2, 1);
	cPacker.Pack();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(0, macRects.NumElements());
	macRects.Kill();

	sImageSize = cPacker.GetPackedImageSize();
	AssertInt((int32)0, sImageSize.x);
	AssertInt((int32)0, sImageSize.y);

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePow2Packer(void)
{
	BeginTests();

	TestRectanglePow2PackerStuff();
	TestRectanglePow2PackerEmpty();

	TestStatistics();
}
