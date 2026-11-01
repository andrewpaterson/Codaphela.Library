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


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Programmatic devices stand in for a keyboard and a mouse.  Their
// sources have the names a real keyboard and mouse have.
//
//////////////////////////////////////////////////////////////////////////
class CComponentInputTestDevices
{
public:
	CInput					mcInput;
	CProgramDeviceDetail*	mpcKeyboard;
	CProgramDeviceDetail*	mpcMouse;
	CComponentInput*		mpcComponentInput;  //Updates the input through its source once it is set.

	void Init(void)
	{
		CProgramInput*	pcProgram;

		mpcComponentInput = NULL;
		mcInput.Init(NULL);

		//The details are in an array that moves as it grows so they are only got once both are added.
		pcProgram = mcInput.GetProgramInput();
		pcProgram->AddDetail("Programmatic: Keyboard", "Test Keyboard");
		pcProgram->AddDetail("Programmatic: Mouse", "Test Mouse");
		mpcKeyboard = pcProgram->macPDeviceDetails.Get(0);
		mpcMouse = pcProgram->macPDeviceDetails.Get(1);

		mpcKeyboard->AddSource("Left Shift");
		mpcKeyboard->AddSource("Right Shift");
		mpcKeyboard->AddSource("Left Control");
		mpcKeyboard->AddSource("Left Alt");
		mpcKeyboard->AddSource("Caps Lock");
		mpcKeyboard->AddSource("Num Lock");
		mpcKeyboard->AddSource("Insert");
		mpcKeyboard->AddSource("A");
		mpcKeyboard->AddSource("C");
		mpcKeyboard->AddSource("1");
		mpcKeyboard->AddSource("8");
		mpcKeyboard->AddSource("Space");
		mpcKeyboard->AddSource("Tab");
		mpcKeyboard->AddSource("Enter");
		mpcKeyboard->AddSource("End");
		mpcKeyboard->AddSource("Cursor Up");
		mpcKeyboard->AddSource("Cursor Left");
		mpcKeyboard->AddSource("Numpad 1");
		mpcKeyboard->AddSource("Numpad 5");
		mpcKeyboard->AddSource("Numpad 8");
		mpcKeyboard->AddSource("F1");

		mpcMouse->AddSource("Button 1");
		mpcMouse->AddSource("Button 2");
		mpcMouse->AddSource("Button 3");
		mpcMouse->AddSource("Wheel Forward");
		mpcMouse->AddSource("Wheel Backward");

		mcInput.AddProgramInput();
	}

	void Kill(void)
	{
		mcInput.Kill();
	}

	CInputDevice* GetKeyboard(void)
	{
		return mcInput.GetDevices()->GetDevice(mpcKeyboard->miUniqueID);
	}

	CInputDevice* GetMouse(void)
	{
		return mcInput.GetDevices()->GetDevice(mpcMouse->miUniqueID);
	}

	void Event(CProgramDeviceDetail* pcDetail, char* szSource, float fValue)
	{
		pcDetail->AddEvent(szSource, fValue);
		if (mpcComponentInput)
		{
			mpcComponentInput->Update();
		}
		else
		{
			mcInput.Update();
		}
	}

	void Press(char* szKey) { Event(mpcKeyboard, szKey, 1.0f); }
	void Release(char* szKey) { Event(mpcKeyboard, szKey, 0.0f); }
	void Tap(char* szKey) { Press(szKey); Release(szKey); }
	void PressButton(char* szButton) { Event(mpcMouse, szButton, 1.0f); }
	void ReleaseButton(char* szButton) { Event(mpcMouse, szButton, 0.0f); }
};


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
		pInputSource = OMalloc<CWindowInputSource>(&cDevices.mcInput);
		cComponentInput.Init(&pInputSource, &pTestWindow);
		cDevices.mpcComponentInput = &cComponentInput;
		AssertTrue(cComponentInput.AddKeyboard(cDevices.GetKeyboard()));
		AssertTrue(cComponentInput.AddMouse(cDevices.GetMouse()));
		AssertFalse(cComponentInput.AddKeyboard(cDevices.GetKeyboard()));

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

		//Nothing is mapped with alt.
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
		pInputSource = OMalloc<CWindowInputSource>(&cDevices.mcInput);
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
//
//////////////////////////////////////////////////////////////////////////
void TestComponentInputDevices(void)
{
	BeginTests();
	ObjectsInit();

	TestComponentInputKeyboard();
	TestComponentInputMouse();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

