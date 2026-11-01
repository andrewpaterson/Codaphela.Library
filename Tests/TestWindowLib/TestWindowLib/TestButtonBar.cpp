#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/ButtonBar.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


//////////////////////////////////////////////////////////////////////////
//
// The window is 200x60.  The grid's top row fills what the bar along the
// bottom doesn't need.  The bar's buttons are plain canvases, A is 40x20
// and B is 50x24, so the bar needs 40 + 8 + 50 plus 8 each side by 24
// plus 8 above and below.
//
//////////////////////////////////////////////////////////////////////////
void TestButtonBarLayout(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CGridContainer>		pGrid;
		Ptr<CCanvas>			pTop;
		Ptr<CButtonBar>			pBar;
		Ptr<CCanvas>			pA;
		Ptr<CCanvas>			pB;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow		cData;
		CPointer				pNull;
		SInt32Vec2				sVec;

		cNativeFactory.Init(&gcMemoryAllocator, 200, 60, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Button Bar Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);

		pTop = OMalloc<CCanvas>(pTestWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)30, (uint8)30, (uint8)30)));
		pBar = OMalloc<CButtonBar>(pTestWindow);
		pBar->SetAlignment(CSH_Stretch, CSV_Center);
		AssertTrue(pGrid->AddComponent(pTop, 0, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pBar, 0, 1));

		//An empty bar is only its insets.
		AssertInt(8, pBar->GetGap());
		AssertInt(CSH_Right, pBar->GetButtonAlignment());
		sVec = pBar->GetDesiredSize();
		AssertInt32Vec2(16, 16, &sVec);

		pA = OMalloc<CCanvas>(pTestWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)200, (uint8)0, (uint8)0)));
		pA->SetDesiredSize(40, 20);
		pB = OMalloc<CCanvas>(pTestWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0, (uint8)200, (uint8)0)));
		pB->SetDesiredSize(50, 24);
		AssertTrue(pBar->AddComponent(pA));
		AssertTrue(pBar->AddComponent(pB));
		sVec = pBar->GetDesiredSize();
		AssertInt32Vec2(114, 40, &sVec);

		//The bar is stretched across the bottom.  The buttons are aligned right inside the insets and centred in the row's height.
		pTestWindow->Layout();
		sVec = pTop->GetPosition();
		AssertInt32Vec2(0, 0, &sVec);
		sVec = pTop->GetActualSize();
		AssertInt32Vec2(200, 20, &sVec);
		sVec = pBar->GetPosition();
		AssertInt32Vec2(0, 20, &sVec);
		sVec = pBar->GetActualSize();
		AssertInt32Vec2(200, 40, &sVec);
		sVec = pA->GetPosition();
		AssertInt32Vec2(94, 30, &sVec);
		sVec = pA->GetActualSize();
		AssertInt32Vec2(40, 20, &sVec);
		sVec = pB->GetPosition();
		AssertInt32Vec2(142, 28, &sVec);
		sVec = pB->GetActualSize();
		AssertInt32Vec2(50, 24, &sVec);

		//A button's own alignment places it in the row's height.
		pA->SetAlignment(CSH_Center, CSV_Top);
		pTestWindow->Layout();
		sVec = pA->GetPosition();
		AssertInt32Vec2(94, 28, &sVec);
		pA->SetAlignment(CSH_Center, CSV_Center);

		pBar->SetButtonAlignment(CSH_Left);
		pTestWindow->Layout();
		sVec = pA->GetPosition();
		AssertInt32Vec2(8, 30, &sVec);
		sVec = pB->GetPosition();
		AssertInt32Vec2(56, 28, &sVec);

		pBar->SetButtonAlignment(CSH_Center);
		pBar->SetGap(4);
		pBar->SetInsets(2, 6, 2, 4);
		sVec = pBar->GetDesiredSize();
		AssertInt32Vec2(104, 28, &sVec);
		pTestWindow->Layout();
		sVec = pBar->GetPosition();
		AssertInt32Vec2(0, 32, &sVec);
		sVec = pA->GetPosition();
		AssertInt32Vec2(4 + (190 - 94) / 2, 36, &sVec);
		sVec = pB->GetPosition();
		AssertInt32Vec2(4 + (190 - 94) / 2 + 44, 34, &sVec);

		//Too narrow for the buttons, they keep their sizes and run off the right.
		pBar->SetButtonAlignment(CSH_Right);
		pBar->SetDesiredSize(60, -1);
		pBar->SetAlignment(CSH_Left, CSV_Center);
		pTestWindow->Layout();
		sVec = pBar->GetActualSize();
		AssertInt32Vec2(60, 28, &sVec);
		sVec = pA->GetPosition();
		AssertInt32Vec2(4 + (50 - 94), 36, &sVec);
		sVec = pA->GetActualSize();
		AssertInt32Vec2(40, 20, &sVec);

		pA = NULL;
		pB = NULL;
		pBar = NULL;
		pTop = NULL;
		pGrid = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestButtonBar(void)
{
	BeginTests();
	ObjectsInit();

	TestButtonBarLayout();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

