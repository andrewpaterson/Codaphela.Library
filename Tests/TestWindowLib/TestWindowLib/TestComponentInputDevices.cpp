#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "InputLib/Input.h"
#include "WindowLib/Window.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/ComponentInput.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "InputTestComponent.h"
#include "ComponentInputTestDevices.h"


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// The window is 64x32 with A on the left and B on the right, both can
// take focus.
//
//////////////////////////////////////////////////////////////////////////
void TestComponentInputKeyboard(void)
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
		CComponentInputTestDevices	cDevices;
		Ptr<CWindowInputSource>		pInputSource;
		CComponentInput				cComponentInput;

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

		cDevices.Init();
		pInputSource = OMalloc<CWindowInputSource>(Ptr<CInput>(cDevices.mpcInput));
		cComponentInput.Init(&pInputSource, &pTestWindow);
		cDevices.mpcComponentInput = &cComponentInput;
		AssertTrue(cComponentInput.AddKeyboard(cDevices.GetKeyboard()));
		AssertTrue(cComponentInput.AddMouse(cDevices.GetMouse()));
		AssertFalse(cComponentInput.AddKeyboard(cDevices.GetKeyboard()));

		//An application's keys (the HCP65 IDE's).
		AssertTrue(cComponentInput.BindKeys("Control+S", CIC_Command));
		AssertTrue(cComponentInput.BindKeys("Control+Shift+S", CIC_Command));
		AssertTrue(cComponentInput.BindKeys("Shift+F6", CIC_Command));
		AssertTrue(cComponentInput.BindKeys("Alt+F7", CIC_Command));
		AssertTrue(cComponentInput.BindKeys("Control+N", CIC_Command));
		AssertTrue(cComponentInput.BindKeys("Control+Shift+N", CIC_Command));
		AssertTrue(cComponentInput.BindKeys("Control+Alt+Shift+N", CIC_Command));
		AssertTrue(cComponentInput.BindKeys("Control+B", CIC_Command));
		AssertTrue(cComponentInput.BindKeys("Alt+O", CIC_Command));
		AssertTrue(cComponentInput.BindKeys("Alt+1", CIC_Command));
		AssertTrue(cComponentInput.BindKeys("Alt+Backspace", CIC_Command));

		//Nothing has focus so nothing hears the keyboard.
		cDevices.Tap("A");
		AssertLog("");

		pTestWindow->PointerMoved(40, 10);
		cDevices.PressButton("Button 1");
		cDevices.ReleaseButton("Button 1");
		AssertLog("B:Into;B:Gained;B:Activate(8,10);B:Execute(8,10);");

		cDevices.Tap("A");
		cDevices.Tap("1");
		cDevices.Tap("Space");
		cDevices.Tap("Tab");
		cDevices.Tap("Enter");
		cDevices.Tap("Cursor Up");
		AssertLog("B:Printable(a,Insert);B:Printable(1,Insert);B:Space(Insert);B:Tab(Insert);B:Enter;B:Up;");

		//Function keys type nothing.
		cDevices.Tap("F1");
		AssertLog("");

		cDevices.Press("Left Shift");
		AssertTrue(cComponentInput.IsShift());
		cDevices.Tap("A");
		cDevices.Tap("1");
		cDevices.Tap("Space");
		cDevices.Tap("Tab");
		cDevices.Tap("Enter");
		cDevices.Tap("Cursor Up");
		cDevices.Release("Left Shift");
		AssertFalse(cComponentInput.IsShift());
		AssertLog("B:Printable(A,Insert);B:Printable(!,Insert);B:Space(Insert);B:BackTab;B:EndEnter;B:SelectUp;");

		//Caps lock swaps the case of letters only.
		cDevices.Tap("Caps Lock");
		AssertTrue(cComponentInput.IsCapsLock());
		cDevices.Tap("A");
		cDevices.Tap("1");
		cDevices.Press("Left Shift");
		cDevices.Tap("A");
		cDevices.Release("Left Shift");
		cDevices.Tap("Caps Lock");
		AssertFalse(cComponentInput.IsCapsLock());
		AssertLog("B:Printable(A,Insert);B:Printable(1,Insert);B:Printable(a,Insert);");

		//Shift is held while either shift key is.
		cDevices.Press("Left Shift");
		cDevices.Press("Right Shift");
		cDevices.Release("Left Shift");
		cDevices.Tap("A");
		cDevices.Release("Right Shift");
		cDevices.Tap("A");
		AssertLog("B:Printable(A,Insert);B:Printable(a,Insert);");

		cDevices.Press("Left Control");
		cDevices.Tap("A");
		cDevices.Tap("C");
		cDevices.Tap("Cursor Left");
		cDevices.Tap("Cursor Up");
		cDevices.Tap("1");
		cDevices.Release("Left Control");
		AssertLog("B:SelectAll;B:Copy;B:WordLeft;B:LineUp;");

		//Control+S; with shift too, another.
		cDevices.Press("Left Control");
		cDevices.Tap("S");
		cDevices.Press("Left Shift");
		cDevices.Tap("S");
		cDevices.Release("Left Shift");
		cDevices.Release("Left Control");
		AssertLog("B:Command(Control+S);B:Command(Control+Shift+S);");

		//Shift and a function key.
		cDevices.Press("Left Shift");
		cDevices.Tap("F6");
		cDevices.Release("Left Shift");
		AssertLog("B:Command(Shift+F6);");

		//F7 goes to the next difference; with shift, the previous; with alt it is bound.
		cDevices.Tap("F7");
		cDevices.Press("Left Shift");
		cDevices.Tap("F7");
		cDevices.Release("Left Shift");
		cDevices.Press("Left Alt");
		cDevices.Tap("F7");
		cDevices.Release("Left Alt");
		AssertLog("B:NextDifference;B:PreviousDifference;B:Command(Alt+F7);");

		//Control+N, with shift and with alt and shift.  The modifiers can be pressed in any order, and either alt.
		cDevices.Press("Left Control");
		cDevices.Tap("N");
		cDevices.Press("Left Shift");
		cDevices.Tap("N");
		cDevices.Press("Left Alt");
		cDevices.Tap("N");
		cDevices.Release("Left Alt");
		cDevices.Release("Left Shift");
		cDevices.Release("Left Control");
		AssertLog("B:Command(Control+N);B:Command(Control+Shift+N);B:Command(Control+Alt+Shift+N);");
		cDevices.Press("Right Alt");
		cDevices.Press("Left Shift");
		cDevices.Press("Left Control");
		cDevices.Tap("N");
		cDevices.Release("Left Control");
		cDevices.Release("Left Shift");
		cDevices.Release("Right Alt");
		AssertLog("B:Command(Control+Alt+Shift+N);");

		//Control and alt with a letter.
		cDevices.Press("Left Control");
		cDevices.Tap("B");
		cDevices.Release("Left Control");
		cDevices.Press("Left Alt");
		cDevices.Tap("O");
		cDevices.Release("Left Alt");
		AssertLog("B:Command(Control+B);B:Command(Alt+O);");

		//Alt+1 doesn't type a 1.
		cDevices.Press("Left Alt");
		cDevices.Tap("1");
		cDevices.Release("Left Alt");
		AssertLog("B:Command(Alt+1);");

		//Alt+Backspace, with either alt.
		cDevices.Press("Left Alt");
		cDevices.Tap("Backspace");
		cDevices.Release("Left Alt");
		cDevices.Press("Right Alt");
		cDevices.Tap("Backspace");
		cDevices.Release("Right Alt");
		AssertLog("B:Command(Alt+Backspace);B:Command(Alt+Backspace);");

		//Alt alone types nothing; nothing else is mapped with it.
		cDevices.Press("Left Alt");
		cDevices.Tap("A");
		cDevices.Tap("Cursor Up");
		cDevices.Release("Left Alt");
		AssertLog("");

		//Insert alone toggles insert.  With control or shift it copies or pastes.
		AssertTrue(cComponentInput.IsInsert());
		cDevices.Tap("Insert");
		AssertFalse(cComponentInput.IsInsert());
		cDevices.Tap("A");
		cDevices.Tap("Tab");
		cDevices.Press("Left Control");
		cDevices.Tap("Insert");
		cDevices.Release("Left Control");
		cDevices.Press("Left Shift");
		cDevices.Tap("Insert");
		cDevices.Release("Left Shift");
		AssertFalse(cComponentInput.IsInsert());
		cDevices.Tap("Insert");
		AssertTrue(cComponentInput.IsInsert());
		AssertLog("B:Printable(a,Overwrite);B:Tab(Overwrite);B:Copy;B:Paste();");

		//With num lock off the numpad moves and shift doesn't select.
		AssertFalse(cComponentInput.IsNumLock());
		cDevices.Tap("Numpad 8");
		cDevices.Tap("Numpad 1");
		cDevices.Tap("Numpad 5");
		cDevices.Press("Left Shift");
		cDevices.Tap("Numpad 8");
		cDevices.Release("Left Shift");
		AssertLog("B:Up;B:End;B:Up;");

		//With num lock off numpad 0 is insert, which toggles insert.  With control held it is nothing.
		cDevices.Tap("Numpad 0");
		AssertFalse(cComponentInput.IsInsert());
		cDevices.Press("Left Control");
		cDevices.Tap("Numpad 0");
		cDevices.Release("Left Control");
		AssertFalse(cComponentInput.IsInsert());
		cDevices.Tap("Numpad 0");
		AssertTrue(cComponentInput.IsInsert());
		AssertLog("");

		cDevices.Tap("Num Lock");
		AssertTrue(cComponentInput.IsNumLock());
		cDevices.Tap("Numpad 8");
		cDevices.Tap("Numpad 1");
		cDevices.Press("Left Shift");
		cDevices.Tap("Numpad 1");
		cDevices.Release("Left Shift");
		cDevices.Tap("Num Lock");
		AssertLog("B:Printable(8,Insert);B:Printable(1,Insert);B:Printable(!,Insert);");

		//Keys released while the window didn't have focus are never seen.
		cDevices.Press("Left Shift");
		cComponentInput.ReleaseAll();
		AssertFalse(cComponentInput.IsShift());
		cDevices.Tap("A");
		cDevices.Release("Left Shift");
		AssertFalse(cComponentInput.IsShift());
		cDevices.Tap("A");
		AssertLog("B:Printable(a,Insert);B:Printable(a,Insert);");

		cComponentInput.Kill();
		cDevices.mpcComponentInput = NULL;
		pInputSource = NULL;
		cDevices.Kill();

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
//
//////////////////////////////////////////////////////////////////////////
void TestComponentInputMouse(void)
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
		CComponentInputTestDevices	cDevices;
		Ptr<CWindowInputSource>		pInputSource;
		CComponentInput				cComponentInput;

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

		cDevices.Init();
		pInputSource = OMalloc<CWindowInputSource>(Ptr<CInput>(cDevices.mpcInput));
		cComponentInput.Init(&pInputSource, &pTestWindow);
		cDevices.mpcComponentInput = &cComponentInput;
		AssertTrue(cComponentInput.AddKeyboard(cDevices.GetKeyboard()));
		AssertTrue(cComponentInput.AddMouse(cDevices.GetMouse()));

		pTestWindow->PointerMoved(40, 10);
		AssertLog("B:Into;");

		//The release is read with the modifiers held at the press so letting go of shift first still stops the drag.
		cDevices.Press("Left Shift");
		cDevices.PressButton("Button 1");
		AssertLog("B:Gained;B:StartDragSelect(8,10);");
		pTestWindow->PointerMoved(10, 10);
		AssertLog("B:Outof;A:Into;B:ContDragSelect(-22,10);");
		cDevices.Release("Left Shift");
		cDevices.ReleaseButton("Button 1");
		AssertLog("B:StopDragSelect(-22,10);");

		cDevices.PressButton("Button 1");
		AssertLog("B:Lost;A:Gained;A:Activate(10,10);");
		cDevices.ReleaseButton("Button 1");
		AssertLog("A:Execute(10,10);");

		cDevices.PressButton("Button 2");
		cDevices.ReleaseButton("Button 2");
		AssertLog("A:ContextMenu(10,10);");

		cDevices.PressButton("Button 3");
		pTestWindow->PointerMoved(12, 11);
		cDevices.ReleaseButton("Button 3");
		AssertLog("A:StartDragScroll(10,10);A:ContDragScroll(12,11);A:StopDragScroll(12,11);");

		//The wheel goes to the component under the pointer, sideways with shift.
		cDevices.PressButton("Wheel Forward");
		cDevices.ReleaseButton("Wheel Forward");
		cDevices.PressButton("Wheel Backward");
		cDevices.ReleaseButton("Wheel Backward");
		AssertLog("A:Wheel(0,-1);A:Wheel(0,1);");
		cDevices.Press("Left Shift");
		cDevices.PressButton("Wheel Forward");
		cDevices.ReleaseButton("Wheel Forward");
		cDevices.PressButton("Wheel Backward");
		cDevices.ReleaseButton("Wheel Backward");
		cDevices.Release("Left Shift");
		AssertLog("A:Wheel(-1,0);A:Wheel(1,0);");

		//A control click, and a control shift click, drag a selection as a shift click does.
		cDevices.Press("Left Control");
		cDevices.PressButton("Button 1");
		cDevices.ReleaseButton("Button 1");
		cDevices.Release("Left Control");
		AssertLog("A:StartDragSelect(12,11);A:StopDragSelect(12,11);");
		cDevices.Press("Left Control");
		cDevices.Press("Left Shift");
		cDevices.PressButton("Button 1");
		cDevices.ReleaseButton("Button 1");
		cDevices.Release("Left Shift");
		cDevices.Release("Left Control");
		AssertLog("A:StartDragSelect(12,11);A:StopDragSelect(12,11);");

		//Once killed the devices are disabled and nothing is heard.
		cComponentInput.Kill();
		cDevices.mpcComponentInput = NULL;
		cDevices.Tap("A");
		cDevices.PressButton("Button 1");
		cDevices.ReleaseButton("Button 1");
		AssertLog("");

		pInputSource = NULL;
		cDevices.Kill();

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
// The keys a command is given by, as a menu shows them: a key alone
// first, else the first chord, with IntelliJ's names.  A command no key
// of the keyboard gives (the test keyboard has no Z) and a mouse button's
// have none.
//
//////////////////////////////////////////////////////////////////////////
void TestComponentInputShortcutText(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		CPointer					pNull;
		CComponentInputTestDevices	cDevices;
		Ptr<CWindowInputSource>		pInputSource;
		CComponentInput				cComponentInput;
		CChars						szKeys;

		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTestWindow = OMalloc<CWindow>("Input Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		cDevices.Init();
		pInputSource = OMalloc<CWindowInputSource>(Ptr<CInput>(cDevices.mpcInput));
		cComponentInput.Init(&pInputSource, &pTestWindow);
		AssertTrue(cComponentInput.AddKeyboard(cDevices.GetKeyboard()));
		AssertTrue(cComponentInput.AddMouse(cDevices.GetMouse()));

		szKeys.Init();
		AssertTrue(cComponentInput.GetShortcutText(CIC_Left, &szKeys));
		AssertString("Left", szKeys.Text());
		szKeys.Clear();
		AssertTrue(cComponentInput.GetShortcutText(CIC_Enter, &szKeys));
		AssertString("Enter", szKeys.Text());
		szKeys.Clear();
		AssertTrue(cComponentInput.GetShortcutText(CIC_WordLeft, &szKeys));
		AssertString("Ctrl+Left", szKeys.Text());
		szKeys.Clear();
		AssertTrue(cComponentInput.GetShortcutText(CIC_SelectLeft, &szKeys));
		AssertString("Shift+Left", szKeys.Text());
		szKeys.Clear();
		AssertTrue(cComponentInput.GetShortcutText(CIC_PreviousDifference, &szKeys));
		AssertString("Shift+F7", szKeys.Text());

		//Appended to what is there.
		szKeys.Set("Keys ");
		AssertTrue(cComponentInput.GetShortcutText(CIC_SelectWordLeft, &szKeys));
		AssertString("Keys Ctrl+Shift+Left", szKeys.Text());

		//None.
		szKeys.Set("Keys");
		AssertFalse(cComponentInput.GetShortcutText(CIC_Undo, &szKeys));
		AssertFalse(cComponentInput.GetShortcutText(CIC_ContextMenu, &szKeys));
		AssertString("Keys", szKeys.Text());
		szKeys.Kill();

		cComponentInput.Kill();
		pInputSource = NULL;
		cDevices.Kill();
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
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
void TestComponentInputDevices(void)
{
	BeginTests();
	ObjectsInit();

	TestComponentInputKeyboard();
	TestComponentInputMouse();
	TestComponentInputShortcutText();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

