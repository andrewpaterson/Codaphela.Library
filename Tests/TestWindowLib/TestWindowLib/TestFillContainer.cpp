#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/ImageCanvasDraw.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "BorderCanvasDraw.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "DarkForest.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertFillContainerObjectsRemoved(void)
{
	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
struct SFillTestComponent
{
	SInt32Vec2					sDesiredSize;	//-1 on an axis fills the area on that axis.
	EContainerStyleHorizontal	eHorizontal;	//Unknown on both axes leaves the canvas's default alignment.
	EContainerStyleVertical		eVertical;
	SInt32Vec2					sExpectedPosition;
	SInt32Vec2					sExpectedSize;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerPositions(char* szDirectory, int32 iWidth, int32 iHeight, int* paiInsets, size uiNumComponents, SFillTestComponent* asComponents)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		size					i;
		Ptr<CCanvas>			apCanvas[4];
		Ptr<CBorderCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		Ptr<CFillContainer>		pFill;
		SInsets					sInsets;
		SFillTestComponent*		psComponent;
		SInt32Vec2				sVec;
		CPointer				pNull;

		AssertTrue(uiNumComponents <= 4);

		cNativeFactory.Init(&gcMemoryAllocator, iWidth, iHeight, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		if (paiInsets)
		{
			sInsets.Init(paiInsets);
			pFill->SetInsets(&sInsets);
		}
		pTestWindow->SetContainer(pFill);

		for (i = 0; i < uiNumComponents; i++)
		{
			psComponent = &asComponents[i];
			pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
			apCanvas[i] = OMalloc<CCanvas>(pTestWindow, pDraw);
			pFill->AddComponent(apCanvas[i]);
			apCanvas[i]->SetDesiredSize(psComponent->sDesiredSize.x, psComponent->sDesiredSize.y);
			if ((psComponent->eHorizontal != CSH_Unknown) || (psComponent->eVertical != CSV_Unknown))
			{
				apCanvas[i]->SetAlignment(psComponent->eHorizontal, psComponent->eVertical);
			}
		}
		pDraw = NULL;

		pTestWindow->Show();

		for (i = 0; i < uiNumComponents; i++)
		{
			psComponent = &asComponents[i];
			sVec = apCanvas[i]->GetPosition();
			AssertInt32Vec2(psComponent->sExpectedPosition.x, psComponent->sExpectedPosition.y, &sVec);
			sVec = apCanvas[i]->GetActualSize();
			AssertInt32Vec2(psComponent->sExpectedSize.x, psComponent->sExpectedSize.y, &sVec);
			apCanvas[i] = NULL;
		}

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pFill = NULL;
	}

	AssertFillContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerOverlayStretchInsets(void)
{
	int					aiInsets[4] = { 2, 3, 4, 5 };  //Top, Right, Bottom, Left.
	SFillTestComponent	asComponents[1] = {
		{ SInt32Vec2(-1, -1), CSH_Unknown, CSV_Unknown, SInt32Vec2(5, 2), SInt32Vec2(88, 24) } };

	TestFillContainerPositions("Output" _FS_ "FillContainerOverlayStretchInsets", 96, 30, aiInsets, 1, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerOverlayCenterBottom(void)
{
	SFillTestComponent	asComponents[1] = {
		{ SInt32Vec2(10, 10), CSH_Center, CSV_Bottom, SInt32Vec2(43, 20), SInt32Vec2(10, 10) } };

	TestFillContainerPositions("Output" _FS_ "FillContainerOverlayCenterBottom", 96, 30, NULL, 1, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerOverlayPartialDesiredSize(void)
{
	SFillTestComponent	asComponents[1] = {
		{ SInt32Vec2(10, -1), CSH_Left, CSV_Top, SInt32Vec2(0, 0), SInt32Vec2(10, 30) } };

	TestFillContainerPositions("Output" _FS_ "FillContainerOverlayPartialDesiredSize", 96, 30, NULL, 1, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerOverlayLargeInsets(void)
{
	int					aiInsets[4] = { 20, 0, 20, 0 };
	SFillTestComponent	asComponents[1] = {
		{ SInt32Vec2(-1, -1), CSH_Unknown, CSV_Unknown, SInt32Vec2(0, 20), SInt32Vec2(96, 0) } };

	TestFillContainerPositions("Output" _FS_ "FillContainerOverlayLargeInsets", 96, 30, aiInsets, 1, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerOverlayComponentAlignments(void)
{
	//Each component is placed by its own alignment.  A variable sized component fills the area whatever its alignment, and
	//a fixed size component stretched on an axis fills the area on that axis.
	SFillTestComponent	asComponents[4] = {
		{ SInt32Vec2(10, 10), CSH_Left,		CSV_Top,	SInt32Vec2(0, 0),	SInt32Vec2(10, 10) },
		{ SInt32Vec2(20, 20), CSH_Right,	CSV_Bottom,	SInt32Vec2(76, 10),	SInt32Vec2(20, 20) },
		{ SInt32Vec2(-1, -1), CSH_Right,	CSV_Bottom,	SInt32Vec2(0, 0),	SInt32Vec2(96, 30) },
		{ SInt32Vec2(10, 10), CSH_Stretch,	CSV_Center,	SInt32Vec2(0, 10),	SInt32Vec2(96, 10) } };

	TestFillContainerPositions("Output" _FS_ "FillContainerOverlayComponentAlignments", 96, 30, NULL, 4, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerOverlayAlignmentsInsets(void)
{
	//The insets leave an 88 x 24 area at 5, 2.  A component is centred unless it is given an alignment.  The last
	//component is wider than the area so it is centred over it and hangs out on both sides.
	int					aiInsets[4] = { 2, 3, 4, 5 };  //Top, Right, Bottom, Left.
	SFillTestComponent	asComponents[4] = {
		{ SInt32Vec2(10, 10),	CSH_Unknown,	CSV_Unknown,	SInt32Vec2(44, 9),	SInt32Vec2(10, 10) },
		{ SInt32Vec2(20, 6),	CSH_Center,		CSV_Top,		SInt32Vec2(39, 2),	SInt32Vec2(20, 6) },
		{ SInt32Vec2(8, 30),	CSH_Left,		CSV_Stretch,	SInt32Vec2(5, 2),	SInt32Vec2(8, 24) },
		{ SInt32Vec2(100, 4),	CSH_Center,		CSV_Bottom,		SInt32Vec2(-1, 22),	SInt32Vec2(100, 4) } };

	TestFillContainerPositions("Output" _FS_ "FillContainerOverlayAlignmentsInsets", 96, 30, aiInsets, 4, asComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerRequiredSize(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "FillContainerRequiredSize";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CCanvas>			pCanvas;
		Ptr<CBorderCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		Ptr<CFillContainer>		pFill;
		SInsets					sInsets;
		int						aiInsets[4] = { 1, 2, 3, 4 };
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 96, 30, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pCanvas->SetDesiredSize(10, 5);
		pFill->AddComponent(pCanvas);
		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(0, 1.0f, 0));
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pCanvas->SetDesiredSize(20, 8);
		pFill->AddComponent(pCanvas);
		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(0, 0, 1.0f));
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pCanvas->SetDesiredSize(6, 12);
		pFill->AddComponent(pCanvas);

		//The components are on top of each other so the widest and the tallest decide the required size.
		pFill->SetRequiredSize();
		sVec = pFill->GetRequiredSize();
		AssertInt32Vec2(20, 12, &sVec);

		//A component without a desired width adds nothing to the required width.
		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 1.0f, 0));
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pCanvas->SetDesiredSize(-1, 15);
		pFill->AddComponent(pCanvas);
		pCanvas = NULL;
		pDraw = NULL;

		pFill->SetRequiredSize();
		sVec = pFill->GetRequiredSize();
		AssertInt32Vec2(20, 15, &sVec);

		sInsets.Init(aiInsets);
		pFill->SetInsets(&sInsets);
		pFill->SetRequiredSize();
		sVec = pFill->GetRequiredSize();
		AssertInt32Vec2(26, 19, &sVec);

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pFill = NULL;
	}

	AssertFillContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerImageCanvasDesiredSize(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "FillContainerImageCanvasDesiredSize";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CCanvas>			pCanvas;
		Ptr<CImageCanvasDraw>	pDraw;
		Ptr<CImage>				pImage;
		Ptr<CImageCel>			pCel;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		Ptr<CFillContainer>		pFill;
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 96, 30, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pImage = OMalloc<CImage>(12, 7, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		pCel = OMalloc<CImageCel>(pImage);
		pDraw = OMalloc<CImageCanvasDraw>(pCel);
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pCanvas->SetAlignment(CSH_Left, CSV_Top);
		pFill->AddComponent(pCanvas);

		sVec = pDraw->GetDesiredSize();
		AssertInt32Vec2(12, 7, &sVec);
		sVec = pCanvas->GetDesiredSize();
		AssertInt32Vec2(12, 7, &sVec);

		pTestWindow->Show();
		sVec = pCanvas->GetActualSize();
		AssertInt32Vec2(12, 7, &sVec);

		pCanvas->SetDesiredSize(20, 10);
		sVec = pCanvas->GetDesiredSize();
		AssertInt32Vec2(20, 10, &sVec);

		pTestWindow->Layout();
		sVec = pCanvas->GetActualSize();
		AssertInt32Vec2(20, 10, &sVec);

		pCanvas->SetDesiredSize(-1, -1);
		sVec = pCanvas->GetDesiredSize();
		AssertInt32Vec2(12, 7, &sVec);

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pFill = NULL;
		pCanvas = NULL;
		pDraw = NULL;
		pCel = NULL;
		pImage = NULL;
	}

	AssertFillContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerDarkForest(char* szDirectory, int32 iWidth, int32 iHeight, int* paiInsets, size uiNumComponents, SDarkForestComponent* asComponents, SInt32Vec2* asExpectedPositions, bool bClipsChildren = true)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		size					i;
		CChars					szExpectedDirectory;
		Ptr<CCanvas>			apCanvas[16];
		Ptr<CCanvasDraw>		pDraw;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		Ptr<CFillContainer>		pFill;
		SInsets					sInsets;
		CColourFormatHelper		cHelper;
		SDarkForestComponent*	psComponent;
		SInt32Vec2				sDesired;
		SInt32Vec2				sVec;
		CPointer				pNull;

		AssertTrue(uiNumComponents <= 16);

		cNativeFactory.Init(&gcMemoryAllocator, iWidth, iHeight, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		if (paiInsets)
		{
			sInsets.Init(paiInsets);
			pFill->SetInsets(&sInsets);
		}
		pFill->SetClipsChildren(bClipsChildren);
		pTestWindow->SetContainer(pFill);

		for (i = 0; i < uiNumComponents; i++)
		{
			psComponent = &asComponents[i];
			pDraw = CreateDarkForestDraw(psComponent, &cHelper);
			apCanvas[i] = OMalloc<CCanvas>(pTestWindow, pDraw);
			apCanvas[i]->SetDesiredSize(psComponent->sDesiredSize.x, psComponent->sDesiredSize.y);
			if ((psComponent->eHorizontal != CSH_Unknown) || (psComponent->eVertical != CSV_Unknown))
			{
				apCanvas[i]->SetAlignment(psComponent->eHorizontal, psComponent->eVertical);
			}
			pFill->AddComponent(apCanvas[i]);
			pDraw = NULL;
		}

		pTestWindow->Show();

		//Where an axis has a desired size and isn't stretched the canvas must have been laid out at exactly that size.
		for (i = 0; i < uiNumComponents; i++)
		{
			sDesired = apCanvas[i]->GetDesiredSize();
			sVec = apCanvas[i]->GetActualSize();
			if ((sDesired.x > 0) && (apCanvas[i]->GetHorizontalAlignment() != CSH_Stretch))
			{
				AssertInt(sDesired.x, sVec.x);
			}
			if ((sDesired.y > 0) && (apCanvas[i]->GetVerticalAlignment() != CSV_Stretch))
			{
				AssertInt(sDesired.y, sVec.y);
			}
			if (asExpectedPositions)
			{
				sVec = apCanvas[i]->GetPosition();
				AssertInt32Vec2(asExpectedPositions[i].x, asExpectedPositions[i].y, &sVec);
			}
			apCanvas[i] = NULL;
		}

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pFill = NULL;

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");

		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertFillContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerDarkForestImagesCorners(void)
{
	//Each image sits where its own alignment puts it: three along the top and three along the bottom.
	SDarkForestComponent	asComponents[6] = {
		{ "Pine.png",		SInt32Vec2(-1, -1), 0, CSH_Left,	CSV_Top },
		{ "Moon.png",		SInt32Vec2(-1, -1), 0, CSH_Center,	CSV_Top },
		{ "Owl.png",		SInt32Vec2(-1, -1), 0, CSH_Right,	CSV_Top },
		{ "Bat.png",		SInt32Vec2(-1, -1), 0, CSH_Left,	CSV_Bottom },
		{ "Mushroom.png",	SInt32Vec2(-1, -1), 0, CSH_Center,	CSV_Bottom },
		{ "Wolf.png",		SInt32Vec2(-1, -1), 0, CSH_Right,	CSV_Bottom } };
	SInt32Vec2				asPosition[6] = { SInt32Vec2(0, 0), SInt32Vec2(31, 0), SInt32Vec2(63, 0), SInt32Vec2(0, 14), SInt32Vec2(32, 12), SInt32Vec2(60, 10) };

	TestFillContainerDarkForest("Output" _FS_ "FillContainerDarkForestImagesCorners", 72, 20, NULL, 6, asComponents, asPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerDarkForestImagesInsets(void)
{
	//The insets leave an 18 x 28 area at 4, 1 and the images are aligned inside it.
	int						aiInsets[4] = { 1, 2, 3, 4 };  //Top, Right, Bottom, Left.
	SDarkForestComponent	asComponents[3] = {
		{ "Bat.png",		SInt32Vec2(-1, -1), 0, CSH_Right,	CSV_Bottom },
		{ "Fern.png",		SInt32Vec2(-1, -1), 0, CSH_Left,	CSV_Center },
		{ "Fireflies.png",	SInt32Vec2(-1, -1), 0, CSH_Right,	CSV_Top } };
	SInt32Vec2				asPosition[3] = { SInt32Vec2(9, 23), SInt32Vec2(4, 9), SInt32Vec2(17, 1) };

	TestFillContainerDarkForest("Output" _FS_ "FillContainerDarkForestImagesInsets", 24, 32, aiInsets, 3, asComponents, asPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerDarkForestImagesStretch(void)
{
	//Stretched image canvases are larger than their cels so each cel sits in the top left of its canvas.  The mushroom's
	//canvas runs along the bottom and the owl's down the right.  The stump isn't stretched.
	SDarkForestComponent	asComponents[3] = {
		{ "Mushroom.png",	SInt32Vec2(-1, -1), 0, CSH_Stretch,	CSV_Bottom },
		{ "Owl.png",		SInt32Vec2(-1, -1), 0, CSH_Right,	CSV_Stretch },
		{ "Stump.png",		SInt32Vec2(-1, -1), 0, CSH_Left,	CSV_Top } };
	SInt32Vec2				asPosition[3] = { SInt32Vec2(0, 16), SInt32Vec2(23, 0), SInt32Vec2(0, 0) };

	TestFillContainerDarkForest("Output" _FS_ "FillContainerDarkForestImagesStretch", 32, 24, NULL, 3, asComponents, asPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerDarkForestMixed(void)
{
	//The green border has no desired size so it fills the window.  The yellow border has no desired width so it fills the
	//width and its alignment only places it vertically.  The red border is the other way round.  Later components are
	//drawn over earlier ones.
	SDarkForestComponent	asComponents[5] = {
		{ NULL,				SInt32Vec2(-1, -1), Set32BitColour((uint8)60, 110, 60),	CSH_Unknown,	CSV_Unknown },
		{ NULL,				SInt32Vec2(-1, 8),	Set32BitColour((uint8)236, 200, 70),	CSH_Center,		CSV_Top },
		{ NULL,				SInt32Vec2(20, -1), Set32BitColour((uint8)150, 30, 30),	CSH_Right,		CSV_Center },
		{ "Owl.png",		SInt32Vec2(-1, -1), 0,										CSH_Left,		CSV_Bottom },
		{ "Wolf.png",		SInt32Vec2(-1, -1), 0,										CSH_Center,		CSV_Bottom } };
	SInt32Vec2				asPosition[5] = { SInt32Vec2(0, 0), SInt32Vec2(0, 0), SInt32Vec2(64, 0), SInt32Vec2(0, 8), SInt32Vec2(36, 8) };

	TestFillContainerDarkForest("Output" _FS_ "FillContainerDarkForestMixed", 84, 18, NULL, 5, asComponents, asPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerDarkForestMixedOverlayCenter(void)
{
	//The border fills the inset area and the moon is drawn centred on top of it.
	int						aiInsets[4] = { 1, 1, 1, 1 };  //Top, Right, Bottom, Left.
	SDarkForestComponent	asComponents[2] = {
		{ NULL,				SInt32Vec2(-1, -1), Set32BitColour((uint8)78, 60, 96), CSH_Unknown, CSV_Unknown },
		{ "Moon.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown } };

	TestFillContainerDarkForest("Output" _FS_ "FillContainerDarkForestMixedOverlayCenter", 18, 16, aiInsets, 2, asComponents, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerDarkForestRowAlignments(void)
{
	//The area is larger than every cel so each is placed with space around it.
	SDarkForestComponent	asComponents[3] = {
		{ "Mushroom.png",	SInt32Vec2(-1, -1), 0, CSH_Left,	CSV_Center },
		{ "Fireflies.png",	SInt32Vec2(-1, -1), 0, CSH_Center,	CSV_Center },
		{ "Stump.png",		SInt32Vec2(-1, -1), 0, CSH_Right,	CSV_Center } };
	SInt32Vec2				asPosition[3] = { SInt32Vec2(0, 8), SInt32Vec2(27, 7), SInt32Vec2(50, 9) };

	TestFillContainerDarkForest("Output" _FS_ "FillContainerDarkForestRowAlignments", 60, 24, NULL, 3, asComponents, asPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerDarkForestLargerThanArea(void)
{
	//The 4 pixel insets leave a 34 x 8 area at 4, 4 and every cel is taller than it.  A centred cel overflows into the insets
	//above and below, with an odd overflow putting the extra pixel at the bottom.  A bottom aligned cel overflows only above.
	//The container clips its children so nothing is drawn in the insets.
	int						aiInsets[4] = { 4, 4, 4, 4 };  //Top, Right, Bottom, Left.
	SDarkForestComponent	asComponents[3] = {
		{ "Wolf.png",		SInt32Vec2(-1, -1), 0, CSH_Left,	CSV_Center },
		{ "Fireflies.png",	SInt32Vec2(-1, -1), 0, CSH_Center,	CSV_Center },
		{ "Moon.png",		SInt32Vec2(-1, -1), 0, CSH_Right,	CSV_Bottom } };
	SInt32Vec2				asPosition[3] = { SInt32Vec2(4, 3), SInt32Vec2(18, 4), SInt32Vec2(28, 2) };

	TestFillContainerDarkForest("Output" _FS_ "FillContainerDarkForestLargerThanArea", 42, 16, aiInsets, 3, asComponents, asPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerDarkForestLargerThanAreaUnclipped(void)
{
	//As TestFillContainerDarkForestLargerThanArea but with clipping turned off the cels are drawn over the insets.
	int						aiInsets[4] = { 4, 4, 4, 4 };  //Top, Right, Bottom, Left.
	SDarkForestComponent	asComponents[3] = {
		{ "Wolf.png",		SInt32Vec2(-1, -1), 0, CSH_Left,	CSV_Center },
		{ "Fireflies.png",	SInt32Vec2(-1, -1), 0, CSH_Center,	CSV_Center },
		{ "Moon.png",		SInt32Vec2(-1, -1), 0, CSH_Right,	CSV_Bottom } };
	SInt32Vec2				asPosition[3] = { SInt32Vec2(4, 3), SInt32Vec2(18, 4), SInt32Vec2(28, 2) };

	TestFillContainerDarkForest("Output" _FS_ "FillContainerDarkForestLargerThanAreaUnclipped", 42, 16, aiInsets, 3, asComponents, asPosition, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerClipFindComponent(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "FillContainerClipFindComponent";
	int						aiInsets[4] = { 4, 4, 4, 4 };  //Top, Right, Bottom, Left.

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CCanvas>			pCanvas;
		Ptr<CBorderCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>	pTick;
		SDataTestRefWindow		cData;
		Ptr<CFillContainer>		pFill;
		SInsets					sInsets;
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 42, 16, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		sInsets.Init(aiInsets);
		pFill->SetInsets(&sInsets);
		pTestWindow->SetContainer(pFill);

		//The canvas is 10 high in the 8 high area inside the insets so it overflows one pixel into the top and bottom insets.
		pDraw = OMalloc<CBorderCanvasDraw>(Set32BitColour(1.0f, 0, 0));
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pCanvas->SetDesiredSize(10, 10);
		pCanvas->SetAlignment(CSH_Left, CSV_Center);
		pFill->AddComponent(pCanvas);
		pDraw = NULL;

		pTestWindow->Show();

		sVec = pCanvas->GetPosition();
		AssertInt32Vec2(4, 3, &sVec);
		AssertTrue(pFill->ClipsChildren());

		//Where the canvas is cut off by the container it can't be found, the container is found instead.
		AssertTrue(pTestWindow->FindComponentAt(6, 8) == pCanvas);
		AssertTrue(pTestWindow->FindComponentAt(6, 3) == pFill);
		AssertTrue(pTestWindow->FindComponentAt(6, 12) == pFill);
		AssertTrue(pTestWindow->FindComponentAt(6, 11) == pCanvas);
		AssertTrue(pTestWindow->FindComponentAt(20, 8) == pFill);

		//Without clipping the canvas is drawn over the insets so it is found there.
		pFill->SetClipsChildren(false);
		AssertFalse(pFill->ClipsChildren());
		AssertTrue(pTestWindow->FindComponentAt(6, 8) == pCanvas);
		AssertTrue(pTestWindow->FindComponentAt(6, 3) == pCanvas);
		AssertTrue(pTestWindow->FindComponentAt(6, 12) == pCanvas);
		AssertTrue(pTestWindow->FindComponentAt(6, 13) == pFill);
		AssertTrue(pTestWindow->FindComponentAt(20, 8) == pFill);

		pCanvas = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pFill = NULL;
	}

	AssertFillContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerDarkForestCenterPastWindowEdges(void)
{
	//The 12x10 wolf is centred in an 8x6 window so it hangs 2 pixels off every edge and must be clipped to the window.
	SDarkForestComponent	asComponents[1] = {
		{ "Wolf.png",		SInt32Vec2(-1, -1), 0, CSH_Unknown, CSV_Unknown } };
	SInt32Vec2				asPosition[1] = { SInt32Vec2(-2, -2) };

	TestFillContainerDarkForest("Output" _FS_ "FillContainerDarkForestCenterPastWindowEdges", 8, 6, NULL, 1, asComponents, asPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainerImageCanvasPartialDesiredSize(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "FillContainerImageCanvasPartialDesiredSize";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CCanvas>			pCanvas;
		Ptr<CImageCanvasDraw>	pDraw;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		Ptr<CFillContainer>		pFill;
		CColourFormatHelper		cHelper;
		SInt32Vec2				sVec;
		CPointer				pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 30, 20, szDirectory);

		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Reference Test Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTestWindow->GetColourFormat(&cHelper);

		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);

		pDraw = OMalloc<CImageCanvasDraw>(ReadDarkForestCel("Wolf.png", &cHelper));
		pCanvas = OMalloc<CCanvas>(pTestWindow, pDraw);
		pCanvas->SetAlignment(CSH_Left, CSV_Top);
		pFill->AddComponent(pCanvas);

		//The canvas only takes its draw's size when its own desired size is -1, -1.  A partial desired size is the canvas's
		//own, so the axis left at -1 takes the whole area rather than the cel's size.
		pCanvas->SetDesiredSize(10, -1);
		sVec = pCanvas->GetDesiredSize();
		AssertInt32Vec2(10, -1, &sVec);
		pTestWindow->Show();
		sVec = pCanvas->GetActualSize();
		AssertInt32Vec2(10, 20, &sVec);

		pCanvas->SetDesiredSize(-1, 6);
		sVec = pCanvas->GetDesiredSize();
		AssertInt32Vec2(-1, 6, &sVec);
		pTestWindow->Layout();
		sVec = pCanvas->GetActualSize();
		AssertInt32Vec2(30, 6, &sVec);

		pCanvas->SetDesiredSize(-1, -1);
		sVec = pCanvas->GetDesiredSize();
		AssertInt32Vec2(12, 10, &sVec);
		pTestWindow->Layout();
		sVec = pCanvas->GetActualSize();
		AssertInt32Vec2(12, 10, &sVec);

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pFill = NULL;
		pCanvas = NULL;
		pDraw = NULL;
	}

	AssertFillContainerObjectsRemoved();

	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFillContainer(void)
{
	BeginTests();
	ObjectsInit();

	TestFillContainerOverlayStretchInsets();
	TestFillContainerOverlayCenterBottom();
	TestFillContainerOverlayPartialDesiredSize();
	TestFillContainerOverlayLargeInsets();
	TestFillContainerOverlayComponentAlignments();
	TestFillContainerOverlayAlignmentsInsets();
	TestFillContainerRequiredSize();
	TestFillContainerImageCanvasDesiredSize();
	TestFillContainerDarkForestImagesCorners();
	TestFillContainerDarkForestImagesInsets();
	TestFillContainerDarkForestImagesStretch();
	TestFillContainerDarkForestMixed();
	TestFillContainerDarkForestMixedOverlayCenter();
	TestFillContainerDarkForestRowAlignments();
	TestFillContainerDarkForestLargerThanArea();
	TestFillContainerDarkForestLargerThanAreaUnclipped();
	TestFillContainerClipFindComponent();
	TestFillContainerDarkForestCenterPastWindowEdges();
	TestFillContainerImageCanvasPartialDesiredSize();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

