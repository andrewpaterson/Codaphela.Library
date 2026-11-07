#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Font.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/Label.h"
#include "WindowLib/WindowInputSource.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertGeometric.h"
#include "WaitTestRefWindow.h"


Ptr<CFont> ReadLabelFont(void);


//What the scripts check and do, shared with the test that shows the window.
struct SRedrawTestData
{
	Ptr<CLayoutCountComponent>	pCounter;
	int64						aiWaits[8];  //What each wait asked for.
	int32						aiDraws[8];  //How many times the component had been drawn at each wait.
	int32						aiLayouts[8];
	int32						aiTicks[8];
	Ptr<CCountingTick>			pTick;
};


//////////////////////////////////////////////////////////////////////////
//
// Notes what was done before each wait in the test's data.
//
//////////////////////////////////////////////////////////////////////////
static void RecordWait(SRedrawTestData* psData, size uiWait, int64 iMilliseconds)
{
	if (uiWait < 8)
	{
		psData->aiWaits[uiWait] = iMilliseconds;
		psData->aiDraws[uiWait] = psData->pCounter->miDraws;
		psData->aiLayouts[uiWait] = psData->pCounter->miLayouts;
		psData->aiTicks[uiWait] = psData->pTick->miTicks;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A window with a counting component filling it, shown by fScript's
// waiting reference window.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CWindow> NewRedrawTestWindow(CWaitTestRefWindowFactory* pcFactory, SRedrawTestData* psData, int32 iRequests, uint32 uiRequestMilliseconds, uint32 uiFramesPerSecond)
{
	Ptr<CWindow>			pWindow;
	Ptr<CFillContainer>		pFill;
	CPointer				pNull;
	size					i;

	for (i = 0; i < 8; i++)
	{
		psData->aiWaits[i] = -2;
		psData->aiDraws[i] = -1;
		psData->aiLayouts[i] = -1;
		psData->aiTicks[i] = -1;
	}
	psData->pTick = OMalloc<CCountingTick>(iRequests, uiRequestMilliseconds, -1);
	pWindow = OMalloc<CWindow>("Redraw Window", pcFactory, psData->pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	ORoot()->Add(pWindow);
	pWindow->SetMaxFramesPerSecond(uiFramesPerSecond);
	pWindow->SetCheckLayoutCache(false);  //It would lay out every paint.
	pFill = OMalloc<CFillContainer>(pWindow);
	pWindow->SetContainer(pFill);
	psData->pCounter = OMalloc<CLayoutCountComponent>(pWindow);
	pFill->AddComponent(psData->pCounter);
	return pWindow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void KillRedrawTestWindow(Ptr<CWindow> pWindow, SRedrawTestData* psData)
{
	psData->pCounter = NULL;
	psData->pTick = NULL;
	ORoot()->Remove(pWindow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static CWaitTestRefWindow* GetWaitWindow(Ptr<CWindow> pWindow)
{
	return (CWaitTestRefWindow*)pWindow->GetNativeWindow();
}


//////////////////////////////////////////////////////////////////////////
//
// Records the first wait and stops.
//
//////////////////////////////////////////////////////////////////////////
static bool StopAtFirstWait(Ptr<CWindow> pWindow, size uiWait, int64 iMilliseconds, void* pvContext)
{
	RecordWait((SRedrawTestData*)pvContext, uiWait, iMilliseconds);
	pWindow->Stop();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Shown, the window is laid out, painted and ticked once.  With nothing
// changing and nothing asked for it waits for events for ever.
//
//////////////////////////////////////////////////////////////////////////
void TestWindowRedrawIdle(void)
{
	CWaitTestRefWindowFactory	cFactory;
	SRedrawTestData				sData;
	char						szDirectory[] = "Output" _FS_ "WindowRedraw";

	cFactory.Init(&gcMemoryAllocator, 64, 32, szDirectory, StopAtFirstWait, &sData);
	{
		Ptr<CWindow>	pWindow;

		pWindow = NewRedrawTestWindow(&cFactory, &sData, 0, 0, 30);
		AssertTrue(pWindow->Show());
		AssertSize(1, GetWaitWindow(pWindow)->NumWaits());
		AssertLong(-1, sData.aiWaits[0]);
		AssertInt(1, sData.aiDraws[0]);
		AssertInt(1, sData.aiLayouts[0]);
		AssertInt(1, sData.aiTicks[0]);
		AssertFalse(pWindow->IsContinuous());
		KillRedrawTestWindow(pWindow, &sData);
	}
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Invalidated at the first wait, then left alone.
//
//////////////////////////////////////////////////////////////////////////
static bool InvalidateThenWait(Ptr<CWindow> pWindow, size uiWait, int64 iMilliseconds, void* pvContext)
{
	RecordWait((SRedrawTestData*)pvContext, uiWait, iMilliseconds);
	if (uiWait == 0)
	{
		pWindow->Invalidate();
		return true;
	}
	if (iMilliseconds < 0)
	{
		pWindow->Stop();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A change is painted no sooner than the frame rate allows: the window
// sleeps until then.  Without a frame rate it is painted straight away.
// Painting again doesn't lay the window out again.
//
//////////////////////////////////////////////////////////////////////////
void TestWindowRedrawFrameRate(void)
{
	CWaitTestRefWindowFactory	cFactory;
	SRedrawTestData				sData;
	char						szDirectory[] = "Output" _FS_ "WindowRedraw";

	cFactory.Init(&gcMemoryAllocator, 64, 32, szDirectory, InvalidateThenWait, &sData);
	{
		Ptr<CWindow>	pWindow;

		//Four frames a second: the paint waits for the rest of its 250ms.
		pWindow = NewRedrawTestWindow(&cFactory, &sData, 0, 0, 4);
		AssertTrue(pWindow->Show());
		AssertLong(-1, sData.aiWaits[0]);
		AssertInt(1, sData.aiDraws[1]);
		AssertTrue((sData.aiWaits[1] > 0) && (sData.aiWaits[1] <= 250));
		AssertInt(2, sData.aiDraws[2]);
		AssertInt(1, sData.aiLayouts[2]);
		AssertLong(-1, sData.aiWaits[2]);
		AssertSize(3, GetWaitWindow(pWindow)->NumWaits());
		KillRedrawTestWindow(pWindow, &sData);

		//No frame rate.
		pWindow = NewRedrawTestWindow(&cFactory, &sData, 0, 0, 0);
		AssertTrue(pWindow->Show());
		AssertInt(2, sData.aiDraws[1]);
		AssertInt(1, sData.aiLayouts[1]);
		AssertLong(-1, sData.aiWaits[1]);
		AssertSize(2, GetWaitWindow(pWindow)->NumWaits());
		KillRedrawTestWindow(pWindow, &sData);
	}
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Lets the window sleep as long as it asks and stops it when it asks to
// wait for ever.
//
//////////////////////////////////////////////////////////////////////////
static bool SleepUntilIdle(Ptr<CWindow> pWindow, size uiWait, int64 iMilliseconds, void* pvContext)
{
	RecordWait((SRedrawTestData*)pvContext, uiWait, iMilliseconds);
	if (iMilliseconds < 0)
	{
		pWindow->Stop();
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A tick that asks for another is ticked again that much later, without
// being painted, as a running simulation is; then the window waits for
// events again.
//
//////////////////////////////////////////////////////////////////////////
void TestWindowRedrawRequestTick(void)
{
	CWaitTestRefWindowFactory	cFactory;
	SRedrawTestData				sData;
	char						szDirectory[] = "Output" _FS_ "WindowRedraw";

	cFactory.Init(&gcMemoryAllocator, 64, 32, szDirectory, SleepUntilIdle, &sData);
	{
		Ptr<CWindow>	pWindow;

		pWindow = NewRedrawTestWindow(&cFactory, &sData, 2, 20, 30);
		AssertTrue(pWindow->Show());
		AssertLong(20, sData.aiWaits[0]);
		AssertInt(1, sData.aiTicks[0]);
		AssertLong(20, sData.aiWaits[1]);
		AssertInt(2, sData.aiTicks[1]);
		AssertLong(-1, sData.aiWaits[2]);
		AssertInt(3, sData.aiTicks[2]);
		AssertInt(1, sData.aiDraws[2]);
		AssertInt(1, sData.aiLayouts[2]);
		AssertSize(3, GetWaitWindow(pWindow)->NumWaits());
		KillRedrawTestWindow(pWindow, &sData);
	}
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each wait gives the window a different input, as if it had arrived
// while the window slept.
//
//////////////////////////////////////////////////////////////////////////
static bool GiveInputs(Ptr<CWindow> pWindow, size uiWait, int64 iMilliseconds, void* pvContext)
{
	SWindowInputEvent	sEvent;

	RecordWait((SRedrawTestData*)pvContext, uiWait, iMilliseconds);
	memset(&sEvent, 0, sizeof(SWindowInputEvent));
	switch (uiWait)
	{
	case 0:
		pWindow->PointerMoved(5, 5);
		return true;
	case 1:
		//Movement, as the mouse sends.  The window's focus isn't active (see TestWindowRedrawInput) so it goes no further.
		sEvent.eType = WIET_Delta;
		sEvent.fValue = 3.0f;
		pWindow->Input(&sEvent, NULL);
		return true;
	case 2:
		//A key or button.
		sEvent.eType = WIET_State;
		sEvent.fValue = 1.0f;
		pWindow->Input(&sEvent, NULL);
		return true;
	case 3:
		pWindow->InputCommand(CIC_Hover, true);
		return true;
	case 4:
		pWindow->PointerLeft();
		return true;
	case 5:
		GetWaitWindow(pWindow)->SetSize(80, 40);
		return true;
	default:
		pWindow->Stop();
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer moving and movement paint the window again but keep its
// layout; keys, buttons and commands lay it out again, as a new size does
// (a key typing a character doesn't, see TestWindowRedrawLayoutCache).
//
//////////////////////////////////////////////////////////////////////////
void TestWindowRedrawInput(void)
{
	CWaitTestRefWindowFactory	cFactory;
	SRedrawTestData				sData;
	char						szDirectory[] = "Output" _FS_ "WindowRedraw";
	SInt32Vec2					sSize;

	cFactory.Init(&gcMemoryAllocator, 64, 32, szDirectory, GiveInputs, &sData);
	{
		Ptr<CWindow>	pWindow;

		pWindow = NewRedrawTestWindow(&cFactory, &sData, 0, 0, 0);

		//The events go no further than the window: there is no component input to translate them.
		pWindow->Deactivate();
		AssertTrue(pWindow->Show());

		AssertInt(1, sData.aiDraws[0]);
		AssertInt(1, sData.aiLayouts[0]);

		//Pointer moved.
		AssertInt(2, sData.aiDraws[1]);
		AssertInt(1, sData.aiLayouts[1]);
		AssertLong(-1, sData.aiWaits[1]);

		//Movement.
		AssertInt(3, sData.aiDraws[2]);
		AssertInt(1, sData.aiLayouts[2]);

		//A key or button.
		AssertInt(4, sData.aiDraws[3]);
		AssertInt(2, sData.aiLayouts[3]);

		//A command.
		AssertInt(5, sData.aiDraws[4]);
		AssertInt(3, sData.aiLayouts[4]);

		//The pointer left.
		AssertInt(6, sData.aiDraws[5]);
		AssertInt(3, sData.aiLayouts[5]);

		//Resized.
		AssertInt(7, sData.aiDraws[6]);
		AssertInt(4, sData.aiLayouts[6]);
		sSize = sData.pCounter->GetActualSize();
		AssertInt32Vec2(80, 40, &sSize);
		AssertFalse(GetWaitWindow(pWindow)->IsStuck());

		KillRedrawTestWindow(pWindow, &sData);
	}
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A continuous window never waits: it is painted and ticked every pass,
// though its layout is still kept.
//
//////////////////////////////////////////////////////////////////////////
void TestWindowRedrawContinuous(void)
{
	CWaitTestRefWindowFactory	cFactory;
	SRedrawTestData				sData;
	char						szDirectory[] = "Output" _FS_ "WindowRedraw";

	cFactory.Init(&gcMemoryAllocator, 64, 32, szDirectory, SleepUntilIdle, &sData);
	{
		Ptr<CWindow>	pWindow;

		pWindow = NewRedrawTestWindow(&cFactory, &sData, 0, 0, 30);
		sData.pTick->miStop = 3;
		pWindow->SetContinuous(true);
		AssertTrue(pWindow->IsContinuous());
		AssertTrue(pWindow->Show());
		AssertSize(0, GetWaitWindow(pWindow)->NumWaits());
		AssertInt(3, sData.pTick->miTicks);
		AssertInt(3, sData.pCounter->miDraws);
		AssertInt(1, sData.pCounter->miLayouts);
		KillRedrawTestWindow(pWindow, &sData);
	}
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A native window has nothing to present until the window is first
// painted; an on screen one must still validate the paint it was asked
// for (see CWinGDIWindow's WM_PAINT).
//
//////////////////////////////////////////////////////////////////////////
void TestWindowRedrawPresentBeforePaint(void)
{
	CWinRefWindowFactory	cFactory;
	char					szDirectory[] = "Output" _FS_ "WindowRedraw";

	cFactory.Init(&gcMemoryAllocator, 64, 32, szDirectory);
	{
		Ptr<CWindow>			pWindow;
		Ptr<CCountingTick>		pTick;
		CPointer				pNull;

		pTick = OMalloc<CCountingTick>(0, 0, -1);
		pWindow = OMalloc<CWindow>("Present Window", &cFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		AssertFalse(pWindow->GetNativeWindow()->Present());
		pWindow->Paint();
		AssertTrue(pWindow->GetNativeWindow()->Present());
		ORoot()->Remove(pWindow);
		pTick = NULL;
	}
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Painting keeps the layout unless something it depends on changed.
// Layout always lays the window out.  Input lays it out, but for a key
// typing a character.
//
//////////////////////////////////////////////////////////////////////////
void TestWindowRedrawLayoutCache(void)
{
	CWaitTestRefWindowFactory	cFactory;
	SRedrawTestData				sData;
	char						szDirectory[] = "Output" _FS_ "WindowRedraw";

	cFactory.Init(&gcMemoryAllocator, 64, 32, szDirectory, NULL, NULL);
	{
		Ptr<CWindow>					pWindow;
		Ptr<CLayoutCountComponent>		pCounter;
		Ptr<CLayoutCountComponent>		pOther;
		Ptr<CFont>						pFont;
		Ptr<CLabel>						pLabel;
		Ptr<CComponent>					pFill;

		pWindow = NewRedrawTestWindow(&cFactory, &sData, 0, 0, 30);
		pCounter = sData.pCounter;
		pFill = pCounter->GetParent();

		pWindow->Paint();
		AssertInt(1, pCounter->miLayouts);
		AssertInt(1, pCounter->miDraws);
		pWindow->Paint();
		AssertInt(1, pCounter->miLayouts);
		AssertInt(2, pCounter->miDraws);
		pWindow->Layout();
		AssertInt(2, pCounter->miLayouts);

		//Painted again, but not laid out.
		pWindow->Invalidate();
		pWindow->Paint();
		AssertInt(2, pCounter->miLayouts);

		//Asked for directly.
		pCounter->InvalidateLayout();
		pWindow->Paint();
		AssertInt(3, pCounter->miLayouts);

		//Children added and removed.
		pOther = OMalloc<CLayoutCountComponent>(pWindow);
		pFill->AddComponent(pOther);
		pWindow->Paint();
		AssertInt(4, pCounter->miLayouts);
		AssertTrue(pFill->RemoveComponent(pOther));
		pWindow->Paint();
		AssertInt(5, pCounter->miLayouts);

		//Sizes and alignments.
		pCounter->SetDesiredSize(10, 10);
		pWindow->Paint();
		AssertInt(6, pCounter->miLayouts);
		pCounter->SetAlignment(CSH_Left, CSV_Top);
		pWindow->Paint();
		AssertInt(7, pCounter->miLayouts);

		//A component's setter: a label's text changes its size.
		pFont = ReadLabelFont();
		pLabel = OMalloc<CLabel>(pWindow, pFont, "Hello");
		pFill->AddComponent(pLabel);
		pWindow->Paint();
		AssertInt(8, pCounter->miLayouts);
		pLabel->SetText("Hello World");
		pWindow->Paint();
		AssertInt(9, pCounter->miLayouts);
		AssertTrue(pFill->RemoveComponent(pLabel));
		pWindow->Paint();
		AssertInt(10, pCounter->miLayouts);

		//Popups opened and closed.
		pOther = OMalloc<CLayoutCountComponent>(pWindow);
		pWindow->OpenPopup(pOther, SInt32Vec2(2, 2), pCounter);
		pWindow->Paint();
		AssertInt(11, pCounter->miLayouts);
		AssertTrue(pWindow->ClosePopup(pOther));
		pWindow->Paint();
		AssertInt(12, pCounter->miLayouts);

		//The pointer moving over it doesn't, unless something is being dragged: a drag is a command.
		pCounter->SetDesiredSize(-1, -1);
		pCounter->SetAlignment(CSH_Stretch, CSV_Stretch);
		pWindow->Paint();
		AssertInt(13, pCounter->miLayouts);
		pWindow->PointerMoved(10, 10);
		pWindow->InputCommand(CIC_ActivateComponent, true);
		AssertTrue(pWindow->GetFocus()->GetFocussedComponent() == pCounter);
		pWindow->Paint();
		AssertInt(14, pCounter->miLayouts);
		pWindow->PointerMoved(12, 10);
		pWindow->Paint();
		AssertInt(14, pCounter->miLayouts);
		pWindow->GetFocus()->StartDrag(FD_Select, 12, 10);
		pWindow->PointerMoved(14, 10);
		pWindow->Paint();
		AssertInt(15, pCounter->miLayouts);
		pWindow->GetFocus()->StopDrag(FD_Select);
		pWindow->PointerMoved(16, 10);
		pWindow->Paint();
		AssertInt(15, pCounter->miLayouts);

		//A character typed only paints: what shows it lays itself out if its size changed.  A command lays out.
		pWindow->InputPrintable('a', true);
		AssertFalse(pWindow->IsLayoutDirty());
		pWindow->Paint();
		AssertInt(15, pCounter->miLayouts);
		pWindow->InputCommand(CIC_Backspace, true);
		AssertTrue(pWindow->IsLayoutDirty());
		pWindow->Paint();
		AssertInt(16, pCounter->miLayouts);

		pLabel = NULL;
		pFont = NULL;
		pOther = NULL;
		pFill = NULL;
		pCounter = NULL;
		KillRedrawTestWindow(pWindow, &sData);
	}
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestWindowRedraw(void)
{
	CFileUtil	cFileUtil;
	char		szDirectory[] = "Output" _FS_ "WindowRedraw";

	BeginTests();
	ObjectsInit();

	//The reference windows write each frame they present here.
	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	TestWindowRedrawIdle();
	TestWindowRedrawFrameRate();
	TestWindowRedrawRequestTick();
	TestWindowRedrawInput();
	TestWindowRedrawContinuous();
	TestWindowRedrawPresentBeforePaint();
	TestWindowRedrawLayoutCache();

	cFileUtil.RemoveDir(szDirectory);

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
