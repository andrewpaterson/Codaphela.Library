#include "SupportLib/RectangleBestPackerWithRotation.h"
#include "TestLib/Assert.h"


bool ArePackedRectanglesSeparated(CArrayPackedRectangle* pacRects, int iInnerEdgeWidth);
bool ArePackedRectanglesInside(CArrayPackedRectangle* pacRects, SInt32Vec2 sImageSize, int iOutsideEdgeWidth);
int RectanglePackerRandom(uint32* puiSeed);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertPackedRectangle(CPackedRectangle* pcPackedRect, const char* szUserData, int iLeft, int iTop, int iRight, int iBottom, bool bRotated)
{
	AssertString(szUserData, (char*)pcPackedRect->GetUserData());
	AssertInt((int32)iLeft, pcPackedRect->msTopLeft.x);
	AssertInt((int32)iTop, pcPackedRect->msTopLeft.y);
	AssertInt((int32)iRight, pcPackedRect->msBottomRight.x);
	AssertInt((int32)iBottom, pcPackedRect->msBottomRight.y);
	AssertBool(bRotated, pcPackedRect->IsRotated());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerRotateToFit(void)
{
	CRectangleBestPacker	cPacker;
	CPackAttempt*			pcBestAttempt;
	CInt32Rect				cRect1;
	CInt32Rect				cRect2;
	char					sz1[] = "1";
	char					sz2[] = "2";
	CChars					szRep;
	CArrayPackedRectangle	macRects;

	cRect1.Init(0, 0, 10, 2);
	cRect2.Init(0, 0, 2, 10);

	//Without turning the 10:2 rectangle can't fit beside the 2:10 rectangle so it needs a second line.
	cPacker.Init(0, 0);
	cPacker.AddRectangle(&cRect1, sz1);
	cPacker.AddRectangle(&cRect2, sz2);
	cPacker.AddAttempt(10);
	cPacker.Pack();
	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[2] Size[10, 12]\n[(0, 0) 10:2]\n[(0, 0) 2:10]\n", szRep.Text());
	szRep.Kill();
	cPacker.Kill();

	//Turned it fits in the space to the right.
	cPacker.Init(0, 0);
	cPacker.AddRectangle(&cRect1, sz1);
	cPacker.AddRectangle(&cRect2, sz2);
	cPacker.AddAttempt(10, PR_AsGiven, true);
	cPacker.Pack();
	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[1] Size[4, 10]\n[(0, 0) 2:10], [(2, 0) 2:10 R]\n", szRep.Text());
	szRep.Kill();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(2, macRects.NumElements());
	AssertPackedRectangle(macRects.Get(0), sz2, 0, 0, 2, 10, false);
	AssertPackedRectangle(macRects.Get(1), sz1, 2, 0, 4, 10, true);
	macRects.Kill();

	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddWideAndTallRectangles(CRectangleBestPacker* pcPacker, char* sz1, char* sz2, char* sz3)
{
	CInt32Rect	cRect;

	cRect.Init(0, 0, 2, 9);
	pcPacker->AddRectangle(&cRect, sz1);
	cRect.Init(0, 0, 3, 8);
	pcPacker->AddRectangle(&cRect, sz2);
	cRect.Init(0, 0, 10, 2);
	pcPacker->AddRectangle(&cRect, sz3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerRotateWideAndTall(void)
{
	CRectangleBestPacker	cPacker;
	CPackAttempt*			pcBestAttempt;
	char					sz1[] = "1";
	char					sz2[] = "2";
	char					sz3[] = "3";
	CChars					szRep;
	CArrayPackedRectangle	macRects;

	//Wide turns the 2:9 and 3:8 rectangles.
	cPacker.Init(0, 0);
	AddWideAndTallRectangles(&cPacker, sz1, sz2, sz3);
	cPacker.AddAttempt(10, PR_Wide);
	cPacker.Pack();
	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[3] Size[10, 7]\n[(0, 0) 10:2]\n[(0, 0) 9:2 R]\n[(0, 0) 8:3 R]\n", szRep.Text());
	szRep.Kill();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(3, macRects.NumElements());
	AssertPackedRectangle(macRects.Get(0), sz3, 0, 0, 10, 2, false);
	AssertPackedRectangle(macRects.Get(1), sz1, 0, 2, 9, 4, true);
	AssertPackedRectangle(macRects.Get(2), sz2, 0, 4, 8, 7, true);
	macRects.Kill();
	cPacker.Kill();

	//Tall turns the 10:2 rectangle and all three fit on one line.
	cPacker.Init(0, 0);
	AddWideAndTallRectangles(&cPacker, sz1, sz2, sz3);
	cPacker.AddAttempt(10, PR_Tall);
	cPacker.Pack();
	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	szRep.Init();
	pcBestAttempt->Print(&szRep);
	AssertString("Lines[1] Size[7, 10]\n[(0, 0) 2:10 R], [(2, 0) 2:9], [(4, 0) 3:8]\n", szRep.Text());
	szRep.Kill();

	macRects.Init();
	cPacker.GetPackedRectangles(&macRects);
	AssertSize(3, macRects.NumElements());
	AssertPackedRectangle(macRects.Get(0), sz3, 0, 0, 2, 10, true);
	AssertPackedRectangle(macRects.Get(1), sz1, 2, 0, 4, 9, false);
	AssertPackedRectangle(macRects.Get(2), sz2, 4, 0, 7, 8, false);
	macRects.Kill();
	cPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddCrossedRectangles(CRectangleBestPacker* pcPacker)
{
	CInt32Rect	cRect;
	int			i;

	for (i = 0; i < 5; i++)
	{
		cRect.Init(0, 0, 10, 1);
		pcPacker->AddRectangle(&cRect, NULL);
		cRect.Init(0, 0, 1, 10);
		pcPacker->AddRectangle(&cRect, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerWithRotationDefault(void)
{
	CRectangleBestPacker				cPacker;
	CRectangleBestPackerWithRotation	cRotationPacker;
	CPackAttempt*						pcBestAttempt;
	int64								iArea;
	CArrayPackedRectangle				macRects;
	CPackedRectangle*					pcPackedRect;
	size								i;

	cPacker.Init(0, 0);
	AddCrossedRectangles(&cPacker);
	cPacker.Pack();
	pcBestAttempt = cPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	iArea = pcBestAttempt->miArea;
	cPacker.Kill();

	//Five 10:1 and five 1:10 rectangles turned the same way exactly fill 10x10.
	cRotationPacker.Init(0, 0);
	AddCrossedRectangles(&cRotationPacker);
	cRotationPacker.Pack();
	pcBestAttempt = cRotationPacker.GetBestAttempt();
	AssertNotNull(pcBestAttempt);
	AssertLong(100, pcBestAttempt->miArea);
	AssertTrue(iArea > 100);

	macRects.Init();
	cRotationPacker.GetPackedRectangles(&macRects);
	AssertSize(10, macRects.NumElements());
	AssertTrue(ArePackedRectanglesSeparated(&macRects, 0));
	pcPackedRect = macRects.Get(0);
	for (i = 1; i < macRects.NumElements(); i++)
	{
		AssertInt((int32)pcPackedRect->GetWidth(), macRects.Get(i)->GetWidth());
		AssertInt((int32)pcPackedRect->GetHeight(), macRects.Get(i)->GetHeight());
	}
	macRects.Kill();
	cRotationPacker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerWithRotationRandom(void)
{
	CRectangleBestPacker				cPacker;
	CRectangleBestPackerWithRotation	cRotationPacker;
	CArrayPackedRectangle				macRects;
	CPackedRectangle*					pcPackedRect;
	SInt32Vec2							sImageSize;
	CInt32Rect							cRect;
	uint32								uiSeed;
	int									iTrial;
	size								i;
	size								uiNumRectangles;
	size								uiIndex;
	int									aiWidths[32];
	int									aiHeights[32];
	int64								iArea;
	bool								bAnyRotated;

	bAnyRotated = false;
	uiSeed = 7;
	for (iTrial = 0; iTrial < 200; iTrial++)
	{
		uiNumRectangles = 2 + RectanglePackerRandom(&uiSeed) % 20;
		for (i = 0; i < uiNumRectangles; i++)
		{
			aiWidths[i] = 1 + RectanglePackerRandom(&uiSeed) % 20;
			aiHeights[i] = 1 + RectanglePackerRandom(&uiSeed) % 20;
		}

		cPacker.Init(2, 1);
		for (i = 0; i < uiNumRectangles; i++)
		{
			cRect.Init(0, 0, aiWidths[i], aiHeights[i]);
			cPacker.AddRectangle(&cRect, (void*)(i + 1));
		}
		cPacker.Pack();
		AssertNotNull(cPacker.GetBestAttempt());
		iArea = cPacker.GetBestAttempt()->miArea;

		macRects.Init();
		cPacker.GetPackedRectangles(&macRects);
		for (i = 0; i < macRects.NumElements(); i++)
		{
			AssertFalse(macRects.Get(i)->IsRotated());
		}
		macRects.Kill();
		cPacker.Kill();

		cRotationPacker.Init(2, 1);
		for (i = 0; i < uiNumRectangles; i++)
		{
			cRect.Init(0, 0, aiWidths[i], aiHeights[i]);
			cRotationPacker.AddRectangle(&cRect, (void*)(i + 1));
		}
		cRotationPacker.Pack();
		AssertNotNull(cRotationPacker.GetBestAttempt());

		//Never worse than packing without turning.
		AssertTrue(cRotationPacker.GetBestAttempt()->miArea <= iArea);

		macRects.Init();
		cRotationPacker.GetPackedRectangles(&macRects);
		sImageSize = cRotationPacker.GetPackedImageSize();
		AssertSize(uiNumRectangles, macRects.NumElements());
		AssertTrue(ArePackedRectanglesSeparated(&macRects, 1));
		AssertTrue(ArePackedRectanglesInside(&macRects, sImageSize, 2));

		//A turned rectangle swaps its width and height.
		for (i = 0; i < macRects.NumElements(); i++)
		{
			pcPackedRect = macRects.Get(i);
			uiIndex = (size)(ptr)pcPackedRect->GetUserData() - 1;
			if (pcPackedRect->IsRotated())
			{
				bAnyRotated = true;
				AssertInt((int32)aiHeights[uiIndex], pcPackedRect->GetWidth());
				AssertInt((int32)aiWidths[uiIndex], pcPackedRect->GetHeight());
			}
			else
			{
				AssertInt((int32)aiWidths[uiIndex], pcPackedRect->GetWidth());
				AssertInt((int32)aiHeights[uiIndex], pcPackedRect->GetHeight());
			}
		}
		macRects.Kill();
		cRotationPacker.Kill();
	}
	AssertTrue(bAnyRotated);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectanglePackerWithRotationEmpty(void)
{
	CRectangleBestPackerWithRotation	cPacker;
	CArrayPackedRectangle				macRects;
	SInt32Vec2							sImageSize;

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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRectangleBestPackerWithRotation(void)
{
	BeginTests();

	TestRectanglePackerRotateToFit();
	TestRectanglePackerRotateWideAndTall();
	TestRectanglePackerWithRotationDefault();
	TestRectanglePackerWithRotationRandom();
	TestRectanglePackerWithRotationEmpty();

	TestStatistics();
}

