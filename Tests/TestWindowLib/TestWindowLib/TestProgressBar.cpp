#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/ProgressBar.h"
#include "WindowLib/JewelColours.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


//////////////////////////////////////////////////////////////////////////
//
// A progress bar 200 wide fills from the left: a quarter done covers 50
// pixels, all done all 200, and nearly done never all of it.  What is
// done is kept between 0 and 1, and nothing to do is nothing done.
//
//////////////////////////////////////////////////////////////////////////
void TestProgressBarFraction(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pWindow;
		Ptr<CFillContainer>			pFill;
		Ptr<CProgressBar>			pBar;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		SInt32Vec2					sSize;

		cNativeFactory.Init(&gcMemoryAllocator, 200, 20, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>("Progress Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pFill = OMalloc<CFillContainer>(pWindow);
		pWindow->SetContainer(pFill);
		pBar = OMalloc<CProgressBar>(pWindow, JEWEL_SEPARATOR, JEWEL_BLUE, 6);
		pBar->SetAlignment(CSH_Stretch, CSV_Top);
		pFill->AddComponent(pBar);
		pWindow->Layout();

		sSize = pBar->GetActualSize();
		AssertInt(200, sSize.x);
		AssertInt(6, sSize.y);
		AssertFloat(0.0f, pBar->GetFraction(), 3);
		AssertInt(0, pBar->GetBarWidth());

		pBar->SetProgress(1, 4);
		AssertFloat(0.25f, pBar->GetFraction(), 3);
		AssertInt(50, pBar->GetBarWidth());
		pBar->SetProgress(399, 400);
		AssertInt(199, pBar->GetBarWidth());
		pBar->SetProgress(4, 4);
		AssertInt(200, pBar->GetBarWidth());
		pBar->SetProgress(3, 0);
		AssertInt(0, pBar->GetBarWidth());
		pBar->SetFraction(2.0f);
		AssertFloat(1.0f, pBar->GetFraction(), 3);
		pBar->SetFraction(-1.0f);
		AssertFloat(0.0f, pBar->GetFraction(), 3);

		//Its colour says how it went.
		AssertInt(JEWEL_BLUE, pBar->GetBarColour());
		pBar->SetBarColour(JEWEL_GREEN);
		AssertInt(JEWEL_GREEN, pBar->GetBarColour());
		AssertInt(JEWEL_SEPARATOR, pBar->GetTrackColour());

		pBar = NULL;
		pFill = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestProgressBar(void)
{
	BeginTests();
	ObjectsInit();

	TestProgressBarFraction();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

