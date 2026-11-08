#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Timer.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/ToolTips.h"
#include "WindowLib/JewelTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"


Ptr<CFont> GenerateMenuFont(Ptr<CWindow> pWindow);


//A 300 x 200 window of three columns: a canvas with a tip, a container with a tip holding a canvas without one, and a
//canvas without one.
struct SToolTipsTest
{
	Ptr<CWindow>			pWindow;
	Ptr<CWindowTheme>		pTheme;
	Ptr<CGridContainer>		pMain;
	Ptr<CCanvas>			pTipped;
	Ptr<CFillContainer>		pParent;
	Ptr<CCanvas>			pChild;
	Ptr<CCanvas>			pPlain;
	Ptr<CToolTips>			pToolTips;

	void Init(CWinRefWindowFactory* pcFactory);
	void Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SToolTipsTest::Init(CWinRefWindowFactory* pcFactory)
{
	CPointer	pNull;

	pcFactory->Init(&gcMemoryAllocator, 300, 200, NULL);
	pWindow = OMalloc<CWindow>("Tool Tips Window", pcFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	ORoot()->Add(pWindow);
	pTheme = CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow));

	pTipped = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)200, (uint8)0, (uint8)0)));
	pTipped->SetToolTip("Tipped canvas");
	pParent = OMalloc<CFillContainer>(pWindow);
	pParent->SetToolTip("Parent");
	pChild = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0, (uint8)200, (uint8)0)));
	pParent->AddComponent(pChild);
	pPlain = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0, (uint8)0, (uint8)200)));

	pMain = OMalloc<CGridContainer>(pWindow);
	pMain->AddComponent(pTipped, 0, 0, 1, 1, 1, 1);
	pMain->AddComponent(pParent, 1, 0, 1, 1, 1, 1);
	pMain->AddComponent(pPlain, 2, 0, 1, 1, 1, 1);
	pWindow->SetContainer(pMain);

	pToolTips = OMalloc<CToolTips>(pWindow, pTheme);
	pWindow->SetToolTips(pToolTips);
	pWindow->Layout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SToolTipsTest::Kill(void)
{
	pWindow->SetToolTips(NULL);
	pToolTips = NULL;
	pPlain = NULL;
	pChild = NULL;
	pParent = NULL;
	pTipped = NULL;
	pMain = NULL;
	pTheme = NULL;
	ORoot()->Remove(pWindow);
	pWindow = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A tip shows once the pointer has rested on its component for the
// delay; moving on it before then waits again.  A component without one
// shows its nearest parent's, and one with no tip anywhere up shows
// none.
//
//////////////////////////////////////////////////////////////////////////
void TestToolTipsWait(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SToolTipsTest	sTest;
		int64			iNow;

		sTest.Init(&cNativeFactory);
		AssertInt(TOOL_TIPS_DELAY, sTest.pToolTips->GetDelay());
		AssertLong(-1, sTest.pToolTips->GetWait(0));

		iNow = 1000;
		sTest.pToolTips->PointerMoved(sTest.pTipped, iNow);
		AssertTrue(sTest.pToolTips->GetTipped() == sTest.pTipped);
		AssertLong(TOOL_TIPS_DELAY, sTest.pToolTips->GetWait(iNow));
		AssertLong(200, sTest.pToolTips->GetWait(iNow + TOOL_TIPS_DELAY - 200));
		AssertFalse(sTest.pToolTips->ShowIfDue(iNow + TOOL_TIPS_DELAY - 200));

		//Moving on it starts the wait again.
		iNow += 500;
		sTest.pToolTips->PointerMoved(sTest.pTipped, iNow);
		AssertFalse(sTest.pToolTips->ShowIfDue(iNow + TOOL_TIPS_DELAY - 1));
		AssertTrue(sTest.pToolTips->ShowIfDue(iNow + TOOL_TIPS_DELAY));
		AssertTrue(sTest.pToolTips->IsShown());
		AssertString("Tipped canvas", sTest.pToolTips->GetText());
		AssertSize(1, sTest.pWindow->NumPopups());
		AssertLong(-1, sTest.pToolTips->GetWait(iNow + TOOL_TIPS_DELAY));

		//Moving on it while it shows keeps it.
		iNow += TOOL_TIPS_DELAY + 100;
		sTest.pToolTips->PointerMoved(sTest.pTipped, iNow);
		AssertTrue(sTest.pToolTips->IsShown());

		//Onto a component without a tip of its own: its parent's, after the delay.
		sTest.pToolTips->PointerMoved(sTest.pChild, iNow);
		AssertFalse(sTest.pToolTips->IsShown());
		AssertNull(sTest.pToolTips->GetText());
		AssertSize(0, sTest.pWindow->NumPopups());
		AssertTrue(sTest.pToolTips->GetTipped() == sTest.pParent);
		AssertTrue(sTest.pToolTips->ShowIfDue(iNow + TOOL_TIPS_DELAY));
		AssertString("Parent", sTest.pToolTips->GetText());

		//Onto one without a tip anywhere up: none.
		iNow += TOOL_TIPS_DELAY;
		sTest.pToolTips->PointerMoved(sTest.pPlain, iNow);
		AssertFalse(sTest.pToolTips->IsShown());
		AssertTrue(sTest.pToolTips->GetTipped().IsNull());
		AssertLong(-1, sTest.pToolTips->GetWait(iNow));
		AssertFalse(sTest.pToolTips->ShowIfDue(iNow + 10 * TOOL_TIPS_DELAY));
		AssertSize(0, sTest.pWindow->NumPopups());

		//A shorter delay.
		sTest.pToolTips->SetDelay(100);
		sTest.pToolTips->PointerMoved(sTest.pTipped, iNow);
		AssertLong(100, sTest.pToolTips->GetWait(iNow));

		//A tip taken away while the pointer rests isn't shown.
		sTest.pTipped->SetToolTip(NULL);
		AssertFalse(sTest.pToolTips->ShowIfDue(iNow + 100));
		AssertLong(-1, sTest.pToolTips->GetWait(iNow + 100));
		AssertSize(0, sTest.pWindow->NumPopups());

		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A press, key or the wheel takes the tip away, and the component shows
// none again until the pointer moves onto another.  The pointer leaving
// the window, or the component leaving it, hides it.
//
//////////////////////////////////////////////////////////////////////////
void TestToolTipsDismiss(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SToolTipsTest	sTest;
		int64			iNow;

		sTest.Init(&cNativeFactory);

		iNow = 1000;
		sTest.pToolTips->PointerMoved(sTest.pTipped, iNow);
		AssertTrue(sTest.pToolTips->ShowIfDue(iNow + TOOL_TIPS_DELAY));
		sTest.pToolTips->Dismiss();
		AssertFalse(sTest.pToolTips->IsShown());
		AssertSize(0, sTest.pWindow->NumPopups());

		//Not again on the same component, however long the pointer rests.
		iNow += TOOL_TIPS_DELAY;
		sTest.pToolTips->PointerMoved(sTest.pTipped, iNow);
		AssertLong(-1, sTest.pToolTips->GetWait(iNow));
		AssertFalse(sTest.pToolTips->ShowIfDue(iNow + 10 * TOOL_TIPS_DELAY));

		//Off it and back: it waits again.
		sTest.pToolTips->PointerMoved(sTest.pPlain, iNow);
		sTest.pToolTips->PointerMoved(sTest.pTipped, iNow);
		AssertLong(TOOL_TIPS_DELAY, sTest.pToolTips->GetWait(iNow));

		//Dismissed before it showed: it doesn't.
		sTest.pToolTips->Dismiss();
		AssertFalse(sTest.pToolTips->ShowIfDue(iNow + TOOL_TIPS_DELAY));

		//The pointer leaving the window.
		sTest.pToolTips->PointerMoved(sTest.pPlain, iNow);
		sTest.pToolTips->PointerMoved(sTest.pChild, iNow);
		AssertTrue(sTest.pToolTips->ShowIfDue(iNow + TOOL_TIPS_DELAY));
		sTest.pToolTips->PointerLeft();
		AssertFalse(sTest.pToolTips->IsShown());
		AssertTrue(sTest.pToolTips->GetTipped().IsNull());
		AssertSize(0, sTest.pWindow->NumPopups());

		//The tipped component leaving the window: its tip goes with it.
		iNow += TOOL_TIPS_DELAY;
		sTest.pToolTips->PointerMoved(sTest.pChild, iNow);
		AssertTrue(sTest.pToolTips->ShowIfDue(iNow + TOOL_TIPS_DELAY));
		AssertTrue(sTest.pMain->RemoveComponent(sTest.pParent));
		AssertFalse(sTest.pToolTips->IsShown());
		AssertTrue(sTest.pToolTips->GetTipped().IsNull());
		AssertSize(0, sTest.pWindow->NumPopups());

		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Through the window: the pointer moving onto a component waits for its
// tip, which shows under the pointer in a popup the pointer passes
// through.  A press takes it away.  A window without tool tips shows none.
//
//////////////////////////////////////////////////////////////////////////
void TestToolTipsWindow(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		SToolTipsTest	sTest;
		Ptr<CToolTips>	pToolTips;
		int32			iBelow;

		sTest.Init(&cNativeFactory);
		iBelow = 50 + sTest.pTheme->GetMetric(TM_ToolTipOffset) + 4;

		sTest.pWindow->PointerMoved(50, 50);
		AssertTrue(sTest.pToolTips->GetTipped() == sTest.pTipped);
		AssertFalse(sTest.pWindow->ShowToolTip(GetDebugTickCount()));
		AssertTrue(sTest.pWindow->ShowToolTip(GetDebugTickCount() + TOOL_TIPS_DELAY));
		AssertSize(1, sTest.pWindow->NumPopups());
		sTest.pWindow->Layout();

		//The tip is under the pointer, but the pointer finds what is under it.
		AssertTrue(sTest.pWindow->FindComponentAt(50, iBelow) == sTest.pTipped);
		sTest.pWindow->PointerMoved(50, iBelow);
		AssertTrue(sTest.pToolTips->IsShown());

		//A press.
		sTest.pWindow->InputCommand(CIC_ActivateComponent, true);
		AssertFalse(sTest.pToolTips->IsShown());
		AssertSize(0, sTest.pWindow->NumPopups());
		sTest.pWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertFalse(sTest.pWindow->ShowToolTip(GetDebugTickCount() + TOOL_TIPS_DELAY));

		//Onto the next column, the parent's tip; out of the window, none.
		sTest.pWindow->PointerMoved(150, 50);
		AssertTrue(sTest.pToolTips->GetTipped() == sTest.pParent);
		AssertTrue(sTest.pWindow->ShowToolTip(GetDebugTickCount() + TOOL_TIPS_DELAY));
		sTest.pWindow->PointerLeft();
		AssertTrue(sTest.pToolTips->GetTipped().IsNull());
		AssertSize(0, sTest.pWindow->NumPopups());

		//Without tool tips.
		pToolTips = sTest.pToolTips;
		sTest.pWindow->SetToolTips(NULL);
		sTest.pWindow->PointerMoved(50, 50);
		AssertFalse(sTest.pWindow->ShowToolTip(GetDebugTickCount() + TOOL_TIPS_DELAY));
		AssertTrue(pToolTips->GetTipped().IsNull());
		AssertSize(0, sTest.pWindow->NumPopups());
		pToolTips = NULL;

		sTest.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestToolTips(void)
{
	BeginTests();
	ObjectsInit();

	TestToolTipsWait();
	TestToolTipsDismiss();
	TestToolTipsWindow();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

