#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "InputTestComponent.h"


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// The window is 96x32 with a grid of three equal columns:
//
//   A [0..32)  can focus, entirely covered by its child 'a' which can't.
//   B [32..64) can focus.
//   C [64..96) can't focus, it is the background.
//
// Pointer positions are given in window client space and components are
// given positions relative to their own top left.
//
//////////////////////////////////////////////////////////////////////////
void TestComponentInputFocus(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CGridContainer>			pGrid;
		Ptr<CInputTestComponent>	pA;
		Ptr<CInputTestComponent>	pa;
		Ptr<CInputTestComponent>	pB;
		Ptr<CInputTestComponent>	pC;
		Ptr<CFocus>					pFocus;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		CChars						szLog;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 96, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Input Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();

		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);

		pA = OMalloc<CInputTestComponent>(pTestWindow, "A", &szLog, true);
		pa = OMalloc<CInputTestComponent>(pTestWindow, "a", &szLog, false);
		pB = OMalloc<CInputTestComponent>(pTestWindow, "B", &szLog, true);
		pC = OMalloc<CInputTestComponent>(pTestWindow, "C", &szLog, false);
		pA->AddComponent(pa);
		AssertTrue(pGrid->AddComponent(pA, 0, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pB, 1, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pC, 2, 0, 1, 1, 1, 1));

		pTestWindow->Layout();
		AssertLog("");
		AssertTrue(pFocus->GetMouseOverComponent().IsNull());
		AssertTrue(pFocus->GetFocussedComponent().IsNull());

		//The deepest component under the pointer is the one moved into, focusable or not.
		pTestWindow->PointerMoved(5, 5);
		AssertLog("a:Into;");
		AssertTrue(pFocus->GetMouseOverComponent() == pa);
		pTestWindow->PointerMoved(31, 5);
		AssertLog("");

		//The right edge of A belongs to B.
		pTestWindow->PointerMoved(32, 5);
		AssertLog("a:Outof;B:Into;");

		//Nothing has focus so commands go nowhere.
		pTestWindow->InputCommand(CIC_Up, true);
		pTestWindow->InputPrintable('x', true);
		AssertLog("");

		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertLog("B:Gained;B:Activate(0,5);");
		AssertTrue(pFocus->GetFocussedComponent() == pB);
		AssertTrue(pB->IsFocussed());

		pTestWindow->InputCommand(CIC_Up, true);
		pTestWindow->InputPrintable('x', true);
		pTestWindow->InputPrintable('y', false);
		pTestWindow->InputCommand(CIC_Space, false);
		pTestWindow->InputCommand(CIC_Paste, true);
		AssertLog("B:Up;B:Printable(x,Insert);B:Printable(y,Overwrite);B:Space(Overwrite);B:Paste();");

		//Focus stays with B when the pointer leaves it.
		pTestWindow->PointerMoved(10, 20);
		AssertLog("B:Outof;a:Into;");
		pTestWindow->InputCommand(CIC_Up, true);
		AssertLog("B:Up;");

		//'a' can't take focus so clicking it gives focus to A.
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertLog("B:Lost;A:Gained;A:Activate(10,20);");
		AssertTrue(pFocus->GetFocussedComponent() == pA);
		AssertFalse(pB->IsFocussed());

		//The bottom edge of the window is outside it.
		pTestWindow->PointerMoved(10, 32);
		AssertLog("a:Outof;");
		AssertTrue(pFocus->GetMouseOverComponent().IsNull());

		//Clicking the background clears focus.
		pTestWindow->PointerMoved(70, 10);
		AssertLog("C:Into;");
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertLog("A:Lost;");
		AssertTrue(pFocus->GetFocussedComponent().IsNull());
		pTestWindow->InputCommand(CIC_Up, true);
		AssertLog("");

		//Hover goes to the component under the pointer, focusable or not.
		pTestWindow->InputCommand(CIC_Hover, true);
		AssertLog("C:Hover(6,10);");

		//A context menu focusses like a click.
		pTestWindow->PointerMoved(40, 10);
		AssertLog("C:Outof;B:Into;");
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertLog("B:Gained;B:ContextMenu(8,10);");

		gcObjects.ValidateObjectsConsistency();

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pGrid = NULL;
		pA = NULL;
		pa = NULL;
		pB = NULL;
		pC = NULL;
		pFocus = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A drag goes to the component it started on until it stops, wherever
// the pointer goes.
//
//////////////////////////////////////////////////////////////////////////
void TestComponentInputDrag(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CGridContainer>			pGrid;
		Ptr<CInputTestComponent>	pA;
		Ptr<CInputTestComponent>	pB;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		CChars						szLog;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Input Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);

		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		pA = OMalloc<CInputTestComponent>(pTestWindow, "A", &szLog, true);
		pB = OMalloc<CInputTestComponent>(pTestWindow, "B", &szLog, true);
		AssertTrue(pGrid->AddComponent(pA, 0, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pB, 1, 0, 1, 1, 1, 1));
		pTestWindow->Layout();

		pTestWindow->PointerMoved(40, 10);
		AssertLog("B:Into;");
		pTestWindow->InputCommand(CIC_StartDragSelect, true);
		AssertLog("B:Gained;B:StartDragSelect(8,10);");
		pTestWindow->PointerMoved(45, 12);
		AssertLog("B:ContDragSelect(13,12);");
		pTestWindow->PointerMoved(5, 5);
		AssertLog("B:Outof;A:Into;B:ContDragSelect(-27,5);");

		//Stopping a scroll doesn't stop a select.
		pTestWindow->InputCommand(CIC_StopDragScroll, true);
		AssertLog("");
		pTestWindow->InputCommand(CIC_StopDragSelect, true);
		AssertLog("B:StopDragSelect(-27,5);");
		pTestWindow->PointerMoved(6, 5);
		AssertLog("");

		//The button release goes to the focussed component, not the one under the pointer.
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertLog("B:Execute(-26,5);");

		//Starting a drag focusses the component under the pointer.  Starting another stops the first.
		pTestWindow->InputCommand(CIC_StartDragScroll, true);
		AssertLog("B:Lost;A:Gained;A:StartDragScroll(6,5);");
		pTestWindow->PointerMoved(7, 6);
		AssertLog("A:ContDragScroll(7,6);");
		pTestWindow->InputCommand(CIC_StartDragSelect, true);
		AssertLog("A:StopDragScroll(7,6);A:StartDragSelect(7,6);");
		pTestWindow->InputCommand(CIC_StopDragSelect, true);
		AssertLog("A:StopDragSelect(7,6);");

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pGrid = NULL;
		pA = NULL;
		pB = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Deactivating the window takes focus and hover away.  Activating it
// gives focus back to the component that had it.
//
//////////////////////////////////////////////////////////////////////////
void TestComponentInputActivation(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CGridContainer>			pGrid;
		Ptr<CInputTestComponent>	pA;
		Ptr<CInputTestComponent>	pB;
		Ptr<CFocus>					pFocus;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		CChars						szLog;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Input Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();

		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		pA = OMalloc<CInputTestComponent>(pTestWindow, "A", &szLog, true);
		pB = OMalloc<CInputTestComponent>(pTestWindow, "B", &szLog, true);
		AssertTrue(pGrid->AddComponent(pA, 0, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pB, 1, 0, 1, 1, 1, 1));
		pTestWindow->Layout();

		pTestWindow->PointerMoved(40, 10);
		pTestWindow->InputCommand(CIC_StartDragSelect, true);
		AssertLog("B:Into;B:Gained;B:StartDragSelect(8,10);");

		//Deactivating stops the drag.
		pTestWindow->Deactivate();
		AssertLog("B:StopDragSelect(8,10);B:Outof;B:Lost;");
		AssertFalse(pFocus->IsActive());
		AssertTrue(pFocus->GetFocussedComponent() == pB);

		//An inactive window sends no commands, but the pointer still hovers.
		pTestWindow->InputCommand(CIC_Up, true);
		pTestWindow->InputPrintable('x', true);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertLog("");
		pTestWindow->PointerMoved(10, 10);
		AssertLog("A:Into;");

		pTestWindow->Activate();
		AssertLog("B:Gained;");
		pTestWindow->InputCommand(CIC_Up, true);
		AssertLog("B:Up;");

		//A press outside the window, on its title bar say, isn't for any component.
		pTestWindow->PointerLeft();
		AssertLog("A:Outof;");
		AssertFalse(pFocus->IsPointerIn());
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pTestWindow->InputCommand(CIC_StartDragSelect, true);
		AssertLog("");
		AssertTrue(pFocus->GetFocussedComponent() == pB);

		//While the mouse is captured the pointer can be outside the window.
		pTestWindow->PointerMoved(10, 32);
		pTestWindow->InputCommand(CIC_ContextMenu, true);
		AssertLog("");
		pTestWindow->InputCommand(CIC_Up, true);
		AssertLog("B:Up;");

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pGrid = NULL;
		pA = NULL;
		pB = NULL;
		pFocus = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Components removed from the window are forgotten.  A layout hit tests
// the pointer again because components may have moved under it.
//
//////////////////////////////////////////////////////////////////////////
void TestComponentInputRemoveAndLayout(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CGridContainer>			pGrid;
		Ptr<CInputTestComponent>	pA;
		Ptr<CInputTestComponent>	pa;
		Ptr<CInputTestComponent>	pB;
		Ptr<CInputTestComponent>	pC;
		Ptr<CFocus>					pFocus;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		CChars						szLog;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 96, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Input Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();

		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		pA = OMalloc<CInputTestComponent>(pTestWindow, "A", &szLog, true);
		pa = OMalloc<CInputTestComponent>(pTestWindow, "a", &szLog, false);
		pB = OMalloc<CInputTestComponent>(pTestWindow, "B", &szLog, true);
		pC = OMalloc<CInputTestComponent>(pTestWindow, "C", &szLog, false);
		pA->AddComponent(pa);
		AssertTrue(pGrid->AddComponent(pA, 0, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pB, 1, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pC, 2, 0, 1, 1, 1, 1));
		pTestWindow->Layout();

		pTestWindow->PointerMoved(40, 10);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertLog("B:Into;B:Gained;B:Activate(8,10);");

		//Removing the focussed component under the pointer.
		AssertTrue(pGrid->RemoveComponent(pB));
		AssertLog("B:Outof;B:Lost;");
		AssertTrue(pFocus->GetMouseOverComponent().IsNull());
		AssertTrue(pFocus->GetFocussedComponent().IsNull());
		AssertTrue(pB->GetParent().IsNull());
		pTestWindow->InputCommand(CIC_Up, true);
		AssertLog("");

		//A and C now share the window so A is under the pointer.
		pTestWindow->Layout();
		AssertLog("a:Into;");

		//Removing a parent forgets its children.
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		AssertLog("A:Gained;A:Activate(40,10);");
		pA->RemoveAllComponents();
		AssertLog("a:Outof;");
		AssertTrue(pFocus->GetFocussedComponent() == pA);
		pTestWindow->Layout();
		AssertLog("A:Into;");

		AssertTrue(pGrid->RemoveAllComponents());
		AssertLog("A:Outof;A:Lost;");
		pTestWindow->Layout();
		AssertLog("");

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pGrid = NULL;
		pA = NULL;
		pa = NULL;
		pB = NULL;
		pC = NULL;
		pFocus = NULL;
		szLog.Kill();
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
void TestComponentInput(void)
{
	BeginTests();
	ObjectsInit();

	TestComponentInputFocus();
	TestComponentInputDrag();
	TestComponentInputActivation();
	TestComponentInputRemoveAndLayout();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

