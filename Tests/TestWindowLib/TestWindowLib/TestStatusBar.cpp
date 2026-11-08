#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/StatusBar.h"
#include "WindowLib/JewelTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"


Ptr<CFont> GenerateMenuFont(Ptr<CWindow> pWindow);


//////////////////////////////////////////////////////////////////////////
//
// A status bar along the bottom of a window: a field 100 wide and a
// separator, the message field taking the space left over, then a field
// as wide as its text at the right end, a gap from the window's edge.
// The bar is the theme's height.
//
//////////////////////////////////////////////////////////////////////////
void TestStatusBarLayout(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pWindow;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CGridContainer>		pMain;
		Ptr<CStatusBar>			pStatus;
		Ptr<CLabel>				pCircuit;
		Ptr<CLabel>				pMessage;
		Ptr<CLabel>				pZoom;
		CPointer				pNull;
		CInt32Rect				cBar;
		CInt32Rect				cCircuit;
		CInt32Rect				cMessage;
		CInt32Rect				cZoom;
		int32					iGap;

		cNativeFactory.Init(&gcMemoryAllocator, 400, 100, NULL);
		pWindow = OMalloc<CWindow>("Status Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow));
		iGap = pTheme->GetMetric(TM_StatusFieldGap);

		pStatus = OMalloc<CStatusBar>(pWindow, pTheme, pTheme->GetFont());
		AssertNull(pStatus->GetMessage());
		pCircuit = pStatus->AddField(100);
		pStatus->AddSeparator();
		pMessage = pStatus->AddMessageField();
		AssertTrue(pStatus->IsStretched());
		AssertTrue(pStatus->AddMessageField() == pMessage);
		pZoom = pStatus->AddField(-1);
		pZoom->SetText("100%");

		pMain = OMalloc<CGridContainer>(pWindow);
		pMain->AddComponent(OMalloc<CFillContainer>(pWindow), 0, 0, 1, 1, 1, 1);  //The window's content above it.
		pMain->AddComponent(pStatus, 0, 1, 1, 1, 1, 0);
		pWindow->SetContainer(pMain);
		pWindow->Layout();

		pStatus->GetActualBounds(&cBar);
		AssertInt(400, cBar.GetWidth());
		AssertInt(100, cBar.GetBottom());
		AssertInt(pTheme->GetMetric(TM_StatusBarHeight), cBar.GetHeight());

		pCircuit->GetActualBounds(&cCircuit);
		pMessage->GetActualBounds(&cMessage);
		pZoom->GetActualBounds(&cZoom);
		AssertInt(iGap, cCircuit.GetLeft());
		AssertInt(100, cCircuit.GetWidth());
		AssertInt(cCircuit.GetRight() + 1 + iGap, cMessage.GetLeft());
		AssertInt(cZoom.GetLeft() - iGap, cMessage.GetRight());
		AssertInt(400 - iGap, cZoom.GetRight());

		pZoom = NULL;
		pMessage = NULL;
		pCircuit = NULL;
		pStatus = NULL;
		pMain = NULL;
		pTheme = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A message shows for the time given, then UpdateMessage clears it; one
// shown for 0 stays.  How long until it is cleared is how long the window
// may wait.  A bar without a message field shows none.
//
//////////////////////////////////////////////////////////////////////////
void TestStatusBarMessage(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pWindow;
		Ptr<CWindowTheme>		pTheme;
		Ptr<CStatusBar>			pStatus;
		Ptr<CStatusBar>			pFieldsOnly;
		CPointer				pNull;
		int64					iNow;

		cNativeFactory.Init(&gcMemoryAllocator, 400, 100, NULL);
		pWindow = OMalloc<CWindow>("Status Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pTheme = CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow));

		pStatus = OMalloc<CStatusBar>(pWindow, pTheme, pTheme->GetFont());
		pStatus->AddMessageField();
		AssertString("", pStatus->GetMessage());
		iNow = 1000;
		AssertLong(-1, pStatus->UpdateMessage(iNow));

		pStatus->ShowMessage("Saved.", 2000, iNow);
		AssertString("Saved.", pStatus->GetMessage());
		AssertLong(2000, pStatus->UpdateMessage(iNow));
		AssertLong(500, pStatus->UpdateMessage(iNow + 1500));
		AssertString("Saved.", pStatus->GetMessage());
		AssertLong(-1, pStatus->UpdateMessage(iNow + 2000));
		AssertString("", pStatus->GetMessage());

		//A message shown again before its time is up has its own time.
		pStatus->ShowMessage("First.", 2000, iNow);
		pStatus->ShowMessage("Second.", 3000, iNow + 1000);
		AssertLong(2000, pStatus->UpdateMessage(iNow + 2000));
		AssertString("Second.", pStatus->GetMessage());

		//Shown for 0, it stays.
		pStatus->ShowMessage("Stays.", 0, iNow);
		AssertLong(-1, pStatus->UpdateMessage(iNow + 100000));
		AssertString("Stays.", pStatus->GetMessage());

		pFieldsOnly = OMalloc<CStatusBar>(pWindow, pTheme, pTheme->GetFont());
		pFieldsOnly->AddField(50);
		pFieldsOnly->ShowMessage("Nowhere.", 1000, iNow);
		AssertNull(pFieldsOnly->GetMessage());
		AssertLong(-1, pFieldsOnly->UpdateMessage(iNow));

		pFieldsOnly = NULL;
		pStatus = NULL;
		pTheme = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestStatusBar(void)
{
	BeginTests();
	ObjectsInit();

	TestStatusBarLayout();
	TestStatusBarMessage();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

