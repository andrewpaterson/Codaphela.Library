#include "SupportLib/RectangleBestPacker.h"
#include "SupportLib/RectanglePow2Packer.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerHorizontalAllSameSize(void)
{
	CRectangleBestPacker	cPacker;
	CPackAttempt*			pcBestAttempt;
	CPackedLine*			pcLine;
	CPackRectangleAttempt*	pcRect;
	CInt32Rect				cRect1;
	CInt32Rect				cRect2;
	CInt32Rect				cRect3;
	CInt32Rect				cRect4;
	CInt32Rect				cRect5;
	CInt32Rect				cRect6;
	char					sz1[] = "1";
	char					sz2[] = "2";
	char					sz3[] = "3";
	char					sz4[] = "4";
	char					sz5[] = "5";
	char					sz6[] = "6";

	cRect1.Init(0, 0, 10, 10);
	cRect2.Init(0, 0, 10, 10);
	cRect3.Init(0, 0, 10, 10);
	cRect4.Init(0, 0, 10, 10);
	cRect5.Init(0, 0, 10, 10);
	cRect6.Init(0, 0, 10, 10);

	cPacker.Init(0, 0);

	cPacker.AddRectangle(&cRect1, sz1);
	cPacker.AddRectangle(&cRect2, sz2);
	cPacker.AddRectangle(&cRect3, sz3);
	cPacker.AddRectangle(&cRect4, sz4);
	cPacker.AddRectangle(&cRect5, sz5);
	cPacker.AddRectangle(&cRect6, sz6);

	cPacker.AddHorizontalAttempt();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertSize(1, pcBestAttempt->macLines.NumElements());
	pcLine = pcBestAttempt->macLines.Get(0);
	AssertSize(6, pcLine->macRectangles.NumElements());

	AssertInt((int32)0, pcLine->miy);
	AssertInt((int32)60, pcLine->miWidth);
	AssertInt((int32)10, pcLine->miHeight);

	pcRect = pcLine->Get(sz1);
	AssertNotNull(pcRect);
	AssertInt((int32)50, pcRect->mix);
	AssertInt((int32)0, pcRect->miy);

	pcRect = pcLine->Get(sz2);
	AssertNotNull(pcRect);
	AssertInt((int32)10, pcRect->mix);
	AssertInt((int32)0, pcRect->miy);

	pcRect = pcLine->Get(sz3);
	AssertNotNull(pcRect);
	AssertInt((int32)20, pcRect->mix);
	AssertInt((int32)0, pcRect->miy);

	pcRect = pcLine->Get(sz4);
	AssertNotNull(pcRect);
	AssertInt((int32)30, pcRect->mix);
	AssertInt((int32)0, pcRect->miy);

	pcRect = pcLine->Get(sz5);
	AssertNotNull(pcRect);
	AssertInt((int32)40, pcRect->mix);
	AssertInt((int32)0, pcRect->miy);

	pcRect = pcLine->Get(sz6);
	AssertNotNull(pcRect);
	AssertInt((int32)0, pcRect->mix);
	AssertInt((int32)0, pcRect->miy);

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerSquareAllSameSize(void)
{
	CRectangleBestPacker		cPacker;
	CPackAttempt*			pcBestAttempt;
	CPackedLine*			pcLine;
	CInt32Rect				cRect1;
	CInt32Rect				cRect2;
	CInt32Rect				cRect3;
	CInt32Rect				cRect4;
	CInt32Rect				cRect5;
	CInt32Rect				cRect6;
	CInt32Rect				cRect7;
	CInt32Rect				cRect8;
	char					sz1[] = "1";
	char					sz2[] = "2";
	char					sz3[] = "3";
	char					sz4[] = "4";
	char					sz5[] = "5";
	char					sz6[] = "6";
	char					sz7[] = "7";
	char					sz8[] = "8";

	cRect1.Init(0, 0, 1, 1);
	cRect2.Init(0, 0, 1, 1);
	cRect3.Init(0, 0, 1, 1);
	cRect4.Init(0, 0, 1, 1);
	cRect5.Init(0, 0, 1, 1);
	cRect6.Init(0, 0, 1, 1);
	cRect7.Init(0, 0, 1, 1);
	cRect8.Init(0, 0, 1, 1);

	cPacker.Init(0, 0);

	cPacker.AddRectangle(&cRect1, sz1);
	cPacker.AddRectangle(&cRect2, sz2);
	cPacker.AddRectangle(&cRect3, sz3);
	cPacker.AddRectangle(&cRect4, sz4);
	cPacker.AddRectangle(&cRect5, sz5);
	cPacker.AddRectangle(&cRect6, sz6);
	cPacker.AddRectangle(&cRect7, sz7);
	cPacker.AddRectangle(&cRect8, sz8);

	cPacker.AddSquareAttempt();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertSize(3, pcBestAttempt->macLines.NumElements());
	AssertInt((int32)3, pcBestAttempt->miHeight);
	AssertInt((int32)3, pcBestAttempt->miWidth);

	pcLine = pcBestAttempt->macLines.Get(0);
	AssertSize(3, pcLine->macRectangles.NumElements());
	AssertInt((int32)0, pcLine->miy);
	AssertInt((int32)3, pcLine->miWidth);
	AssertInt((int32)1, pcLine->miHeight);

	pcLine = pcBestAttempt->macLines.Get(1);
	AssertSize(3, pcLine->macRectangles.NumElements());
	AssertInt((int32)1, pcLine->miy);
	AssertInt((int32)3, pcLine->miWidth);
	AssertInt((int32)1, pcLine->miHeight);

	pcLine = pcBestAttempt->macLines.Get(2);
	AssertSize(2, pcLine->macRectangles.NumElements());
	AssertInt((int32)2, pcLine->miy);
	AssertInt((int32)2, pcLine->miWidth);
	AssertInt((int32)1, pcLine->miHeight);

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddPowerOf2Sizes(CRectangleBestPacker* pcPacker)
{
	CInt32Rect				cRect1;
	CInt32Rect				cRect2;
	CInt32Rect				cRect3;
	CInt32Rect				cRect4;
	CInt32Rect				cRect5;
	CInt32Rect				cRect6;
	CInt32Rect				cRect7;
	CInt32Rect				cRect8;

	cRect1.Init(0, 0, 8, 8);
	cRect2.Init(0, 0, 8, 8);
	cRect3.Init(0, 0, 4, 4);
	cRect4.Init(0, 0, 4, 4);
	cRect5.Init(0, 0, 4, 4);
	cRect6.Init(0, 0, 2, 2);
	cRect7.Init(0, 0, 2, 2);
	cRect8.Init(0, 0, 1, 1);

	pcPacker->AddRectangle(&cRect1, NULL);
	pcPacker->AddRectangle(&cRect2, NULL);
	pcPacker->AddRectangle(&cRect3, NULL);
	pcPacker->AddRectangle(&cRect4, NULL);
	pcPacker->AddRectangle(&cRect5, NULL);
	pcPacker->AddRectangle(&cRect6, NULL);
	pcPacker->AddRectangle(&cRect7, NULL);
	pcPacker->AddRectangle(&cRect8, NULL);

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerHorizontalPowerOf2Sizes(void)
{
	CRectangleBestPacker		cPacker;
	CPackAttempt*			pcBestAttempt;
	CChars					szRep;

	cPacker.Init(0, 0);
	AddPowerOf2Sizes(&cPacker);
	cPacker.AddHorizontalAttempt();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertSize(1, pcBestAttempt->macLines.NumElements());
	AssertInt((int32)24, pcBestAttempt->miWidth);
	AssertInt((int32)8, pcBestAttempt->miHeight);

	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[1] Size[24, 8]\n[(0, 0) 8:8], [(8, 0) 8:8], [(16, 0) 4:4], [(16, 4) 4:4], [(20, 0) 4:4], [(20, 4) 2:2], [(20, 6) 2:2], [(22, 4) 1:1]\n", szRep.Text());
	szRep.Kill();

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerSquarePowerOf2Sizes(void)
{
	CRectangleBestPacker		cPacker;
	CPackAttempt*			pcBestAttempt;
	CChars					szRep;

	cPacker.Init(0, 0);

	AddPowerOf2Sizes(&cPacker);

	cPacker.AddSquareAttempt();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertSize(2, pcBestAttempt->macLines.NumElements());
	AssertInt((int32)14, pcBestAttempt->miMaxWidth);
	AssertInt((int32)14, pcBestAttempt->miWidth);
	AssertInt((int32)16, pcBestAttempt->miHeight);

	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[2] Size[14, 16]\n[(0, 0) 8:8], [(8, 0) 4:4], [(8, 4) 4:4], [(12, 0) 2:2], [(12, 2) 2:2], [(12, 4) 1:1]\n[(0, 0) 8:8], [(8, 0) 4:4]\n", szRep.Text());
	szRep.Kill();

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerSquareThinRectangles(void)
{
	CRectangleBestPacker	cPacker;
	CPackAttempt*			pcBestAttempt;
	CInt32Rect				cRect;
	int						i;

	cPacker.Init(0, 0);
	for (i = 0; i < 10; i++)
	{
		cRect.Init(0, 0, 10, 1);
		cPacker.AddRectangle(&cRect, NULL);
	}

	//The side of a square with the rectangles' area is 10.
	cPacker.AddSquareAttempt();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	AssertInt((int32)10, pcBestAttempt->miMaxWidth);
	AssertInt((int32)10, pcBestAttempt->miWidth);
	AssertInt((int32)10, pcBestAttempt->miHeight);
	cPacker.Kill();

	//With a one pixel gap each rectangle covers 11x2 so the side is ceil(sqrt(220)) - 1 = 14, still one rectangle per row.
	cPacker.Init(0, 1);
	for (i = 0; i < 10; i++)
	{
		cRect.Init(0, 0, 10, 1);
		cPacker.AddRectangle(&cRect, NULL);
	}

	cPacker.AddSquareAttempt();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	AssertInt((int32)14, pcBestAttempt->miMaxWidth);
	AssertInt((int32)10, pcBestAttempt->miWidth);
	AssertInt((int32)19, pcBestAttempt->miHeight);
	cPacker.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerVerticalPowerOf2Sizes(void)
{
	CRectangleBestPacker		cPacker;
	CPackAttempt*			pcBestAttempt;
	CChars					szRep;

	cPacker.Init(0, 0);
	AddPowerOf2Sizes(&cPacker);
	cPacker.AddVerticalAttempt();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertSize(4, pcBestAttempt->macLines.NumElements());
	AssertInt((int32)8, pcBestAttempt->miWidth);
	AssertInt((int32)24, pcBestAttempt->miHeight);

	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[4] Size[8, 24]\n[(0, 0) 8:8]\n[(0, 0) 8:8]\n[(0, 0) 4:4], [(4, 0) 4:4]\n[(0, 0) 4:4], [(4, 0) 2:2], [(4, 2) 2:2], [(6, 0) 1:1]\n", szRep.Text());
	szRep.Kill();

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerDefault(void)
{
	CRectangleBestPacker		cPacker;
	CPackAttempt*			pcBestAttempt;
	CPackAttempt*			pcWorstAttempt;
	CChars					szRep;

	cPacker.Init(0, 0);
	AddPowerOf2Sizes(&cPacker);
	cPacker.AddDefaultAttempts();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertSize(1, pcBestAttempt->macLines.NumElements());
	AssertInt((int32)24, pcBestAttempt->miWidth);
	AssertInt((int32)8, pcBestAttempt->miHeight);
	AssertLong(7, pcBestAttempt->GetWastedArea());

	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[1] Size[24, 8]\n[(0, 0) 8:8], [(8, 0) 8:8], [(16, 0) 4:4], [(16, 4) 4:4], [(20, 0) 4:4], [(20, 4) 2:2], [(20, 6) 2:2], [(22, 4) 1:1]\n", szRep.Text());
	szRep.Kill();

	pcWorstAttempt = cPacker.GetWorstAttempt();
	AssertSize(2, pcWorstAttempt->macLines.NumElements());
	AssertInt((int32)22, pcWorstAttempt->miWidth);
	AssertInt((int32)12, pcWorstAttempt->miHeight);
	AssertLong(79, pcWorstAttempt->GetWastedArea());

	szRep.Init();
	pcWorstAttempt->Print(&szRep);
	AssertString("Lines[2] Size[22, 12]\n[(0, 0) 8:8], [(8, 0) 8:8], [(16, 0) 4:4], [(16, 4) 4:4], [(20, 0) 2:2], [(20, 2) 2:2], [(20, 4) 1:1]\n[(0, 0) 4:4]\n", szRep.Text());
	szRep.Kill();

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerGetPackedRectangles(void)
{
	CRectangleBestPacker	cPacker;
	CPackAttempt*			pcBestAttempt;
	CInt32Rect				cRect1;
	CInt32Rect				cRect2;
	CInt32Rect				cRect3;
	char					sz1[] = "1";
	char					sz2[] = "2";
	char					sz3[] = "3";
	CArrayPackedRectangle	macRects;
	CPackedRectangle*		pcPackedRect;

	cRect1.Init(0, 0, 7, 5);
	cRect2.Init(0, 0, 3, 5);
	cRect3.Init(0, 0, 4, 5);

	cPacker.Init(0, 0);

	cPacker.AddRectangle(&cRect1, sz1);
	cPacker.AddRectangle(&cRect2, sz2);
	cPacker.AddRectangle(&cRect3, sz3);

	cPacker.AddVerticalAttempt();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	macRects.Init();
	pcBestAttempt->GetPackedRectangles(&macRects);

	AssertSize(3, macRects.NumElements());

	pcPackedRect = macRects.Get(0);
	AssertString(sz1, (char*)pcPackedRect->GetUserData());
	AssertInt((int32)0, pcPackedRect->msTopLeft.x);
	AssertInt((int32)0, pcPackedRect->msTopLeft.y);
	AssertInt((int32)7, pcPackedRect->msBottomRight.x);
	AssertInt((int32)5, pcPackedRect->msBottomRight.y);

	pcPackedRect = macRects.Get(1);
	AssertString(sz3, (char*)pcPackedRect->GetUserData());
	AssertInt((int32)0, pcPackedRect->msTopLeft.x);
	AssertInt((int32)5, pcPackedRect->msTopLeft.y);
	AssertInt((int32)4, pcPackedRect->msBottomRight.x);
	AssertInt((int32)10, pcPackedRect->msBottomRight.y);

	pcPackedRect = macRects.Get(2);
	AssertString(sz2, (char*)pcPackedRect->GetUserData());
	AssertInt((int32)4, pcPackedRect->msTopLeft.x);
	AssertInt((int32)5, pcPackedRect->msTopLeft.y);
	AssertInt((int32)7, pcPackedRect->msBottomRight.x);
	AssertInt((int32)10, pcPackedRect->msBottomRight.y);

	macRects.Kill();
	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerGetPackedRectanglesWithEdges(void)
{
	CRectangleBestPacker		cPacker;
	CPackAttempt*			pcBestAttempt;
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

	cPacker.Init(2, 1);

	cPacker.AddRectangle(&cRect1, sz1);
	cPacker.AddRectangle(&cRect2, sz2);
	cPacker.AddRectangle(&cRect3, sz3);

	cPacker.AddVerticalAttempt();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	macRects.Init();
	pcBestAttempt->GetPackedRectangles(&macRects);

	AssertSize(3, macRects.NumElements());

	pcPackedRect = macRects.Get(0);
	AssertInt((int32)2, pcPackedRect->msTopLeft.x);
	AssertInt((int32)2, pcPackedRect->msTopLeft.y);
	AssertInt((int32)9, pcPackedRect->msBottomRight.x);
	AssertInt((int32)7, pcPackedRect->msBottomRight.y);

	pcPackedRect = macRects.Get(1);
	AssertInt((int32)2, pcPackedRect->msTopLeft.x);
	AssertInt((int32)8, pcPackedRect->msTopLeft.y);
	AssertInt((int32)6, pcPackedRect->msBottomRight.x);
	AssertInt((int32)13, pcPackedRect->msBottomRight.y);

	pcPackedRect = macRects.Get(2);
	AssertInt((int32)2, pcPackedRect->msTopLeft.x);
	AssertInt((int32)14, pcPackedRect->msTopLeft.y);
	AssertInt((int32)5, pcPackedRect->msBottomRight.x);
	AssertInt((int32)19, pcPackedRect->msBottomRight.y);

	sImageSize = pcBestAttempt->GetPackedImageSize();
	AssertInt((int32)11, sImageSize.x);
	AssertInt((int32)21, sImageSize.y);

	macRects.Kill();
	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ArePackedRectanglesSeparated(CArrayPackedRectangle* pacRects, int iInnerEdgeWidth)
{
	size				i;
	size				j;
	CPackedRectangle*	pcRect1;
	CPackedRectangle*	pcRect2;

	for (i = 0; i < pacRects->NumElements(); i++)
	{
		pcRect1 = pacRects->Get(i);
		for (j = i + 1; j < pacRects->NumElements(); j++)
		{
			pcRect2 = pacRects->Get(j);
			if (!((pcRect1->msBottomRight.x + iInnerEdgeWidth <= pcRect2->msTopLeft.x) ||
				  (pcRect2->msBottomRight.x + iInnerEdgeWidth <= pcRect1->msTopLeft.x) ||
				  (pcRect1->msBottomRight.y + iInnerEdgeWidth <= pcRect2->msTopLeft.y) ||
				  (pcRect2->msBottomRight.y + iInnerEdgeWidth <= pcRect1->msTopLeft.y)))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerInnerEdgeBelow(void)
{
	CRectangleBestPacker	cPacker;
	CPackAttempt*			pcBestAttempt;
	CInt32Rect				cRect;
	CChars					szRep;
	CArrayPackedRectangle	macRects;

	cPacker.Init(0, 1);

	cRect.Init(0, 0, 8, 1);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 7, 5);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 7, 7);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 12, 8);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 2, 2);
	cPacker.AddRectangle(&cRect, NULL);

	cPacker.AddAttempt(29);
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);

	//The 8:1 rectangle must not be placed below the 7:7 rectangle at (13, 8) where it would cover the gap column at x = 20.
	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[2] Size[28, 10]\n[(0, 0) 12:8], [(13, 0) 7:7], [(21, 0) 7:5], [(21, 6) 2:2]\n[(0, 0) 8:1]\n", szRep.Text());
	szRep.Kill();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(5, macRects.NumElements());
	AssertTrue(ArePackedRectanglesSeparated(&macRects, 1));
	macRects.Kill();

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerWidestNotFirst(void)
{
	CRectangleBestPacker	cPacker;
	CPackAttempt*			pcBestAttempt;
	CPackAttempt*			pcAttempt;
	CInt32Rect				cRect;
	CChars					szRep;
	size					i;

	cPacker.Init(0, 0);

	cRect.Init(0, 0, 2, 3);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 10, 2);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 4, 4);
	cPacker.AddRectangle(&cRect, NULL);

	AssertInt((int32)10, cPacker.GetSourcesMinWidth());
	AssertInt((int32)16, cPacker.GetSourcesMaxWidth());

	cPacker.AddVerticalAttempt();
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	AssertInt((int32)10, pcBestAttempt->miMaxWidth);

	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[2] Size[10, 6]\n[(0, 0) 10:2]\n[(0, 0) 4:4], [(4, 0) 2:3]\n", szRep.Text());
	szRep.Kill();
	cPacker.Kill();

	//Every default attempt is at least as wide as the widest rectangle so none of them can fail.
	cPacker.Init(0, 0);

	cRect.Init(0, 0, 2, 3);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 10, 2);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 4, 4);
	cPacker.AddRectangle(&cRect, NULL);

	cPacker.Pack();

	AssertTrue(cPacker.GetPackAttempts()->NumElements() > 0);
	for (i = 0; i < cPacker.GetPackAttempts()->NumElements(); i++)
	{
		pcAttempt = cPacker.GetPackAttempts()->Get(i);
		AssertTrue(pcAttempt->miMaxWidth >= 10);
		AssertTrue(pcAttempt->mbValid);
	}
	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerEmpty(void)
{
	CRectangleBestPacker	cPacker;
	CArrayPackedRectangle	macRects;
	SInt32Vec2				sImageSize;

	cPacker.Init(2, 1);
	cPacker.Pack();

	AssertNull(cPacker.GetBestAttempt());
	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(0, macRects.NumElements());
	macRects.Kill();

	sImageSize = cPacker.GetPackedImageSize();
	AssertInt((int32)0, sImageSize.x);
	AssertInt((int32)0, sImageSize.y);
	cPacker.Kill();

	cPacker.Init(2, 1);
	cPacker.AddAttempt(10);
	cPacker.Pack();
	AssertNull(cPacker.GetBestAttempt());
	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerNoAttemptFits(void)
{
	CRectangleBestPacker	cPacker;
	CInt32Rect				cRect;
	CArrayPackedRectangle	macRects;
	SInt32Vec2				sImageSize;

	cPacker.Init(0, 0);

	cRect.Init(0, 0, 10, 2);
	cPacker.AddRectangle(&cRect, NULL);

	cPacker.AddAttempt(5);
	cPacker.Pack();

	AssertNull(cPacker.GetBestAttempt());
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
bool ArePackedRectanglesInside(CArrayPackedRectangle* pacRects, SInt32Vec2 sImageSize, int iOutsideEdgeWidth)
{
	size				i;
	CPackedRectangle*	pcRect;

	for (i = 0; i < pacRects->NumElements(); i++)
	{
		pcRect = pacRects->Get(i);
		if ((pcRect->msTopLeft.x < iOutsideEdgeWidth) ||
			(pcRect->msTopLeft.y < iOutsideEdgeWidth) ||
			(pcRect->msBottomRight.x > sImageSize.x - iOutsideEdgeWidth) ||
			(pcRect->msBottomRight.y > sImageSize.y - iOutsideEdgeWidth))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int RectanglePackerRandom(uint32* puiSeed)
{
	//Own generator so every platform packs the same rectangles.
	*puiSeed = *puiSeed * 1103515245 + 12345;
	return (int)((*puiSeed >> 16) & 0x7fff);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddRandomRectangles(CRectanglePacker* pcPacker, uint32* puiSeed, size* puiNumRectangles)
{
	size		i;
	size		uiNumRectangles;
	CInt32Rect	cRect;

	uiNumRectangles = 2 + RectanglePackerRandom(puiSeed) % 20;
	for (i = 0; i < uiNumRectangles; i++)
	{
		cRect.Init(0, 0, 1 + RectanglePackerRandom(puiSeed) % 20, 1 + RectanglePackerRandom(puiSeed) % 20);
		pcPacker->AddRectangle(&cRect, (void*)(i + 1));
	}
	*puiNumRectangles = uiNumRectangles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerRandomGaps(void)
{
	CRectangleBestPacker	cBestPacker;
	CRectanglePow2Packer	cPow2Packer;
	CArrayPackedRectangle	macRects;
	SInt32Vec2				sImageSize;
	uint32					uiSeed;
	size					uiNumRectangles;
	int						iTrial;

	uiSeed = 1;
	for (iTrial = 0; iTrial < 300; iTrial++)
	{
		cBestPacker.Init(2, 1);
		AddRandomRectangles(&cBestPacker, &uiSeed, &uiNumRectangles);
		cBestPacker.Pack();
		AssertNotNull(cBestPacker.GetBestAttempt());

		macRects.Init();
		cBestPacker.GetPackedRectangles(&macRects);
		sImageSize = cBestPacker.GetPackedImageSize();
		AssertSize(uiNumRectangles, macRects.NumElements());
		AssertTrue(ArePackedRectanglesSeparated(&macRects, 1));
		AssertTrue(ArePackedRectanglesInside(&macRects, sImageSize, 2));
		macRects.Kill();
		cBestPacker.Kill();

		cPow2Packer.Init(2, 1);
		AddRandomRectangles(&cPow2Packer, &uiSeed, &uiNumRectangles);
		cPow2Packer.Pack();

		macRects.Init();
		cPow2Packer.GetPackedRectangles(&macRects);
		sImageSize = cPow2Packer.GetPackedImageSize();
		AssertSize(uiNumRectangles, macRects.NumElements());
		AssertTrue(ArePackedRectanglesSeparated(&macRects, 1));
		AssertTrue(ArePackedRectanglesInside(&macRects, sImageSize, 2));
		AssertInt((int32)0, sImageSize.x & (sImageSize.x - 1));
		AssertInt((int32)0, sImageSize.y & (sImageSize.y - 1));
		macRects.Kill();
		cPow2Packer.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerAttemptBeforeRectangles(void)
{
	CRectangleBestPacker	cPacker;
	CPackAttempt*			pcBestAttempt;
	CInt32Rect				cRect;
	CChars					szRep;
	CArrayPackedRectangle	macRects;
	SInt32Vec2				sImageSize;

	cPacker.Init(0, 0);

	//The attempt takes the rectangles from the packer when it packs, not when it is added.
	cPacker.AddAttempt(10);

	cRect.Init(0, 0, 2, 3);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 10, 2);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 4, 4);
	cPacker.AddRectangle(&cRect, NULL);

	cPacker.Pack();
	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[2] Size[10, 6]\n[(0, 0) 10:2]\n[(0, 0) 4:4], [(4, 0) 2:3]\n", szRep.Text());
	szRep.Kill();

	//Packing again gives the same result and no rectangle is packed twice.
	cPacker.Pack();
	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[2] Size[10, 6]\n[(0, 0) 10:2]\n[(0, 0) 4:4], [(4, 0) 2:3]\n", szRep.Text());
	szRep.Kill();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(3, macRects.NumElements());
	macRects.Kill();

	//A rectangle added between packs is included in the next pack.
	cRect.Init(0, 0, 6, 2);
	cPacker.AddRectangle(&cRect, NULL);
	cPacker.Pack();
	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	sImageSize = cPacker.GetPackedImageSize();
	AssertSize(4, macRects.NumElements());
	AssertTrue(ArePackedRectanglesSeparated(&macRects, 0));
	AssertTrue(ArePackedRectanglesInside(&macRects, sImageSize, 0));
	AssertTrue(sImageSize.x <= 10);
	macRects.Kill();

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerRejectsEmptyRectangles(void)
{
	CRectangleBestPacker	cPacker;
	CInt32Rect				cRect;

	cPacker.Init(0, 0);

	cRect.Init(0, 0, 0, 5);
	AssertFalse(cPacker.AddRectangle(&cRect, NULL));
	cRect.Init(0, 0, 5, 0);
	AssertFalse(cPacker.AddRectangle(&cRect, NULL));
	cRect.Init(5, 0, 2, 3);
	AssertFalse(cPacker.AddRectangle(&cRect, NULL));
	cRect.Init(0, 5, 3, 2);
	AssertFalse(cPacker.AddRectangle(&cRect, NULL));
	AssertSize(0, cPacker.GetSourceRectangles()->NumElements());
	AssertLong(0, cPacker.GetSourceArea());

	cRect.Init(0, 0, 3, 3);
	AssertTrue(cPacker.AddRectangle(&cRect, NULL));
	AssertSize(1, cPacker.GetSourceRectangles()->NumElements());
	AssertLong(9, cPacker.GetSourceArea());

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerDefaultAttemptRange(void)
{
	CRectangleBestPacker	cPacker;
	CInt32Rect				cRect;
	CArrayPackAttempt*		pacAttempts;

	cPacker.Init(0, 0);

	cRect.Init(0, 0, 2, 3);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 10, 2);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 4, 4);
	cPacker.AddRectangle(&cRect, NULL);

	//The attempts run from a single row down to the widest rectangle, one for each width in between.
	cPacker.AddDefaultAttempts();
	pacAttempts = cPacker.GetPackAttempts();
	AssertSize(7, pacAttempts->NumElements());
	AssertInt((int32)16, pacAttempts->Get(0)->miMaxWidth);
	AssertInt((int32)10, pacAttempts->Tail()->miMaxWidth);
	cPacker.Kill();

	//The single row includes a gap between each rectangle.
	cPacker.Init(0, 1);

	cRect.Init(0, 0, 2, 3);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 10, 2);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 4, 4);
	cPacker.AddRectangle(&cRect, NULL);

	cPacker.AddDefaultAttempts();
	pacAttempts = cPacker.GetPackAttempts();
	AssertInt((int32)18, pacAttempts->Get(0)->miMaxWidth);
	AssertInt((int32)10, pacAttempts->Tail()->miMaxWidth);

	//The single row attempt packs every rectangle on one line.
	cPacker.Pack();
	AssertNotNull(cPacker.GetBestAttempt());
	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerInvalidAttemptsSortLast(void)
{
	CRectangleBestPacker	cPacker;
	CPackAttempt*			pcBestAttempt;
	CPackAttempt*			pcAttempt;
	CInt32Rect				cRect;
	CArrayPackAttempt*		pacAttempts;

	cPacker.Init(0, 0);

	cRect.Init(0, 0, 10, 2);
	cPacker.AddRectangle(&cRect, NULL);
	cRect.Init(0, 0, 4, 4);
	cPacker.AddRectangle(&cRect, NULL);

	//5 and 3 are narrower than the 10 wide rectangle.  12 packs into 10x6 and 16 packs into 14x4.
	cPacker.AddAttempt(5);
	cPacker.AddAttempt(12);
	cPacker.AddAttempt(3);
	cPacker.AddAttempt(16);
	cPacker.Pack();

	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	AssertInt((int32)16, pcBestAttempt->miMaxWidth);
	AssertInt((int32)14, pcBestAttempt->miWidth);
	AssertInt((int32)4, pcBestAttempt->miHeight);

	pacAttempts = cPacker.GetPackAttempts();
	AssertSize(4, pacAttempts->NumElements());

	pcAttempt = pacAttempts->Get(1);
	AssertTrue(pcAttempt->mbValid);
	AssertInt((int32)12, pcAttempt->miMaxWidth);
	AssertLong(60, pcAttempt->miArea);

	pcAttempt = pacAttempts->Get(2);
	AssertFalse(pcAttempt->mbValid);
	pcAttempt = pacAttempts->Get(3);
	AssertFalse(pcAttempt->mbValid);

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerLargeAreas(void)
{
	CRectangleBestPacker	cPacker;
	CPackAttempt*			pcBestAttempt;
	CInt32Rect				cRect;

	//Boards in micrometres: two 50mm squares cover 5,000,000,000 square micrometres, more than 32 bits hold.
	cPacker.Init(0, 1600);
	cRect.Init(0, 0, 50000, 50000);
	cPacker.AddRectangle(&cRect, NULL);
	cPacker.AddRectangle(&cRect, NULL);
	AssertLong(5000000000LL, cPacker.GetSourceArea());

	cPacker.Pack();
	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	AssertLong((int64)pcBestAttempt->miWidth * (int64)pcBestAttempt->miHeight, pcBestAttempt->miArea);
	AssertLong(5000000000LL + 1600LL * 50000LL, pcBestAttempt->miArea);
	AssertLong(1600LL * 50000LL, pcBestAttempt->GetWastedArea());
	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleBestPacker(void)
{
	BeginTests();

	TestRectanglePackerHorizontalAllSameSize();
	TestRectanglePackerSquareAllSameSize();
	TestRectanglePackerHorizontalPowerOf2Sizes();
	TestRectanglePackerSquarePowerOf2Sizes();
	TestRectanglePackerSquareThinRectangles();
	TestRectanglePackerVerticalPowerOf2Sizes();
	TestRectanglePackerDefault();
	TestRectanglePackerGetPackedRectangles();
	TestRectanglePackerGetPackedRectanglesWithEdges();
	TestRectanglePackerInnerEdgeBelow();
	TestRectanglePackerWidestNotFirst();
	TestRectanglePackerEmpty();
	TestRectanglePackerNoAttemptFits();
	TestRectanglePackerRandomGaps();
	TestRectanglePackerAttemptBeforeRectangles();
	TestRectanglePackerRejectsEmptyRectangles();
	TestRectanglePackerDefaultAttemptRange();
	TestRectanglePackerInvalidAttemptsSortLast();
	TestRectanglePackerLargeAreas();

	TestStatistics();
}
