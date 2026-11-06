#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ClassDefines.h"
#include "InputLib/Input.h"
#include "WindowLib/Window.h"
#include "WindowLib/Component.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/ComponentInput.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//Logs which of its actions are called.  A component has more than one base class so its member function pointers are
//the larger kind.
class CActionTestComponent : public CComponent
{
CONSTRUCTABLE(CActionTestComponent);
DESTRUCTABLE(CActionTestComponent);
protected:
	CChars*		mpszLog;

public:
	void Init(Ptr<CWindow> pWindow, CChars* pszLog)
	{
		PreInit();
		CComponent::Init(pWindow);
		mpszLog = pszLog;
		SetCanGetFocus(true);
		PostInit();
	}

	void Class(void) override
	{
		CComponent::Class();
		U_Pointer(mpszLog);
	}

	void Free(void) override
	{
		CComponent::Free();
	}

	void AnyKeyDown(SComponentInputEvent* psEvent)	{ mpszLog->Append("AnyDown("); mpszLog->Append(psEvent->szName); mpszLog->Append(");"); }
	void ADown(SComponentInputEvent* psEvent)		{ mpszLog->Append("A;"); }
	void AUp(SComponentInputEvent* psEvent)			{ mpszLog->Append("AUp;"); }
	void Chord(SComponentInputEvent* psEvent)		{ mpszLog->Append("Chord("); mpszLog->Append(psEvent->szName); mpszLog->Append(");"); }
	void Copy(SComponentInputEvent* psEvent)		{ mpszLog->Append("Copy;"); }
	void AnyCommand(SComponentInputEvent* psEvent)	{ mpszLog->Append("Command;"); }
	void Printable(SComponentInputEvent* psEvent)	{ mpszLog->Append("Printable("); mpszLog->Append(psEvent->cPrintable); mpszLog->Append(");"); }
	void Movement(SComponentInputEvent* psEvent)	{ mpszLog->Append("Move("); mpszLog->Append(psEvent->iRight); mpszLog->Append(","); mpszLog->Append(psEvent->iDown); mpszLog->Append(");"); }
	void Wheel(SComponentInputEvent* psEvent)		{ mpszLog->Append("Wheel("); mpszLog->Append(psEvent->iDown); mpszLog->Append(");"); }
	void Undo(SComponentInputEvent* psEvent)		{ mpszLog->Append("Undo;"); }
	void StepUndo(SComponentInputEvent* psEvent)	{ mpszLog->Append("StepUndo;"); }
	void Redo(SComponentInputEvent* psEvent)		{ mpszLog->Append("Redo;"); }
	void StepRedo(SComponentInputEvent* psEvent)	{ mpszLog->Append("StepRedo;"); }
	void HomeEnter(SComponentInputEvent* psEvent)	{ mpszLog->Append("HomeEnter;"); }
	void FlipH(SComponentInputEvent* psEvent)		{ mpszLog->Append("FlipH;"); }
	void FlipV(SComponentInputEvent* psEvent)		{ mpszLog->Append("FlipV;"); }
};


//A callback's object needn't be the component, nor a CObject.
class CActionTestCounter
{
public:
	int		miCalls;

	void Count(SComponentInputEvent* psEvent) { miCalls++; }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentInputEvent* InitActionTestEvent(SComponentInputEvent* psEvent, EComponentInputEventType eType, char* szName)
{
	memset(psEvent, 0, sizeof(SComponentInputEvent));
	psEvent->eType = eType;
	psEvent->szName = szName;
	psEvent->eCommand = CIC_Ignored;
	return psEvent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestInputActionMatching(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CActionTestComponent>	pComponent;
		Ptr<CInputAction>			pAnyKeyDown;
		CChars						szLog;
		SComponentInputEvent		sEvent;
		CActionTestCounter			cCounter;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Action Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pComponent = OMalloc<CActionTestComponent>(pTestWindow, &szLog);
		cCounter.miCalls = 0;

		AssertSize(0, pComponent->NumInputActions());
		pAnyKeyDown = pComponent->AddInputAction(IAF_AnyKeyDown, NULL, &pComponent, &CActionTestComponent::AnyKeyDown);
		pComponent->AddInputAction(IAF_KeyDown, "A", &pComponent, &CActionTestComponent::ADown);
		pComponent->AddInputAction(IAF_KeyUp, "A", &pComponent, &CActionTestComponent::AUp);
		pComponent->AddInputAction(IAF_Chord, "Copy Chord", &pComponent, &CActionTestComponent::Chord);
		pComponent->AddInputAction(CIC_Copy, &pComponent, &CActionTestComponent::Copy);
		pComponent->AddInputAction(IAF_AnyCommand, NULL, &pComponent, &CActionTestComponent::AnyCommand);
		pComponent->AddInputAction(IAF_Printable, NULL, &pComponent, &CActionTestComponent::Printable);
		pComponent->AddInputAction(IAF_Movement, NULL, &pComponent, &CActionTestComponent::Movement);
		pComponent->AddInputAction(IAF_Wheel, NULL, &pComponent, &CActionTestComponent::Wheel);
		pComponent->AddInputAction(IAF_KeyDown, "B", &cCounter, &CActionTestCounter::Count);
		AssertSize(10, pComponent->NumInputActions());

		//Every action that matches is called, in the order they were added.
		AssertTrue(pComponent->CallInputActions(InitActionTestEvent(&sEvent, CIET_KeyDown, "A")));
		AssertLog("AnyDown(A);A;");
		AssertTrue(pComponent->CallInputActions(InitActionTestEvent(&sEvent, CIET_KeyDown, "B")));
		AssertLog("AnyDown(B);");
		AssertInt(1, cCounter.miCalls);

		AssertFalse(pComponent->CallInputActions(InitActionTestEvent(&sEvent, CIET_KeyUp, "B")));
		AssertLog("");
		AssertTrue(pComponent->CallInputActions(InitActionTestEvent(&sEvent, CIET_KeyUp, "A")));
		AssertLog("AUp;");

		AssertTrue(pComponent->CallInputActions(InitActionTestEvent(&sEvent, CIET_Chord, "Copy Chord")));
		AssertLog("Chord(Copy Chord);");
		AssertFalse(pComponent->CallInputActions(InitActionTestEvent(&sEvent, CIET_Chord, "Paste Chord")));
		AssertLog("");

		InitActionTestEvent(&sEvent, CIET_Command, NULL)->eCommand = CIC_Copy;
		AssertTrue(pComponent->CallInputActions(&sEvent));
		AssertLog("Copy;Command;");
		InitActionTestEvent(&sEvent, CIET_Command, NULL)->eCommand = CIC_Paste;
		AssertTrue(pComponent->CallInputActions(&sEvent));
		AssertLog("Command;");

		InitActionTestEvent(&sEvent, CIET_Printable, NULL)->cPrintable = 'x';
		AssertTrue(pComponent->CallInputActions(&sEvent));
		AssertLog("Printable(x);");

		InitActionTestEvent(&sEvent, CIET_Movement, NULL);
		sEvent.iRight = 3;
		sEvent.iDown = -2;
		AssertTrue(pComponent->CallInputActions(&sEvent));
		AssertLog("Move(3,-2);");
		InitActionTestEvent(&sEvent, CIET_Wheel, NULL)->iDown = 1;
		AssertTrue(pComponent->CallInputActions(&sEvent));
		AssertLog("Wheel(1);");

		//Removed, B down is only the counter's.
		AssertTrue(pComponent->RemoveInputAction(pAnyKeyDown));
		AssertSize(9, pComponent->NumInputActions());
		AssertTrue(pComponent->CallInputActions(InitActionTestEvent(&sEvent, CIET_KeyDown, "B")));
		AssertLog("");
		AssertInt(2, cCounter.miCalls);
		AssertFalse(pComponent->CallInputActions(InitActionTestEvent(&sEvent, CIET_KeyDown, "C")));

		pAnyKeyDown = NULL;
		pComponent = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//A command's function can take the event, nothing, or the pointer.  Its own functions and the ones it inherits from
//CActionTestComponent can be given.
class CActionShapeTestComponent : public CActionTestComponent
{
CONSTRUCTABLE(CActionShapeTestComponent);
DESTRUCTABLE(CActionShapeTestComponent);
public:
	void Left(void)				{ mpszLog->Append("Left;"); }
	void Press(int x, int y)	{ mpszLog->Append("Press("); mpszLog->Append(x); mpszLog->Append(","); mpszLog->Append(y); mpszLog->Append(");"); }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SComponentInputEvent* InitActionTestCommand(SComponentInputEvent* psEvent, EComponentInputCommand eCommand)
{
	InitActionTestEvent(psEvent, CIET_Command, NULL);
	psEvent->eCommand = eCommand;
	return psEvent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestInputActionShapes(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>						pTestWindow;
		Ptr<CTickTestRefWindow>				pTick;
		SDataTestRefWindow					cData;
		CPointer							pNull;
		Ptr<CActionShapeTestComponent>		pComponent;
		CChars								szLog;
		SComponentInputEvent				sEvent;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Shapes Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pComponent = OMalloc<CActionShapeTestComponent>(pTestWindow, &szLog);

		pComponent->AddInputAction(CIC_Left, &pComponent, &CActionShapeTestComponent::Left);
		pComponent->AddInputAction(CIC_ActivateComponent, &pComponent, &CActionShapeTestComponent::Press);
		pComponent->AddInputAction(CIC_Copy, &pComponent, &CActionTestComponent::Copy);
		AssertSize(3, pComponent->NumInputActions());

		AssertTrue(pComponent->CallInputActions(InitActionTestCommand(&sEvent, CIC_Left)));
		AssertLog("Left;");

		InitActionTestCommand(&sEvent, CIC_ActivateComponent);
		sEvent.x = 5;
		sEvent.y = 7;
		AssertTrue(pComponent->CallInputActions(&sEvent));
		AssertLog("Press(5,7);");

		AssertTrue(pComponent->CallInputActions(InitActionTestCommand(&sEvent, CIC_Copy)));
		AssertLog("Copy;");

		pComponent = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A component's actions for a command can be turned off.  The command
// then goes to the parent, as if they weren't there.
//
//////////////////////////////////////////////////////////////////////////
void TestInputActionEnabled(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>					pTestWindow;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow				cData;
		CPointer						pNull;
		Ptr<CGridContainer>				pGrid;
		Ptr<CActionShapeTestComponent>	pChild;
		Ptr<CInputAction>				pLeft;
		CChars							szLog;
		CActionTestCounter				cParentLefts;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Enabled Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		pChild = OMalloc<CActionShapeTestComponent>(pTestWindow, &szLog);
		AssertTrue(pGrid->AddComponent(pChild, 0, 0, 1, 1, 1, 1));
		pTestWindow->Layout();
		cParentLefts.miCalls = 0;

		pLeft = pChild->AddInputAction(CIC_Left, &pChild, &CActionShapeTestComponent::Left);
		pChild->AddInputAction(CIC_Copy, &pChild, &CActionTestComponent::Copy);
		pGrid->AddInputAction(CIC_Left, &cParentLefts, &CActionTestCounter::Count);
		pTestWindow->GetFocus()->SetFocus(pChild);

		pTestWindow->InputCommand(CIC_Left, true);
		AssertLog("Left;");
		AssertInt(0, cParentLefts.miCalls);

		//Only left's actions are turned off.
		pChild->SetInputActionsEnabled(CIC_Left, false);
		AssertFalse(pLeft->IsEnabled());
		pTestWindow->InputCommand(CIC_Left, true);
		AssertLog("");
		AssertInt(1, cParentLefts.miCalls);
		pTestWindow->InputCommand(CIC_Copy, true);
		AssertLog("Copy;");

		pChild->SetInputActionsEnabled(CIC_Left, true);
		AssertTrue(pLeft->IsEnabled());
		pTestWindow->InputCommand(CIC_Left, true);
		AssertLog("Left;");
		AssertInt(1, cParentLefts.miCalls);

		pLeft = NULL;
		pChild = NULL;
		pGrid = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A programmatic keyboard and mouse with the sources the dispatch test
// uses, each input updated through the component input.
//
//////////////////////////////////////////////////////////////////////////
class CActionTestDevices
{
public:
	CInput*					mpcInput;  //Held by the root from Init to Kill.
	CProgramDeviceDetail*	mpcKeyboard;
	CProgramDeviceDetail*	mpcMouse;
	CComponentInput*		mpcComponentInput;

	void Init(bool bRightModifiers = true)
	{
		CProgramInput*	pcProgram;
		Ptr<CInput>		pInput;

		mpcComponentInput = NULL;
		pInput = OMalloc<CInput>((CNativeInput*)NULL);
		ORoot()->Add(pInput);
		mpcInput = &pInput;
		pcProgram = mpcInput->GetProgramInput();
		pcProgram->AddDetail("Programmatic: Keyboard", "Test Keyboard");
		pcProgram->AddDetail("Programmatic: Mouse", "Test Mouse");
		mpcKeyboard = pcProgram->macPDeviceDetails.Get(0);
		mpcMouse = pcProgram->macPDeviceDetails.Get(1);
		mpcKeyboard->AddSource("Left Shift");
		mpcKeyboard->AddSource("Left Control");
		if (bRightModifiers)
		{
			mpcKeyboard->AddSource("Right Shift");
			mpcKeyboard->AddSource("Right Control");
		}
		mpcKeyboard->AddSource("A");
		mpcKeyboard->AddSource("C");
		mpcKeyboard->AddSource("Y");
		mpcKeyboard->AddSource("Z");
		mpcKeyboard->AddSource("Enter");
		mpcKeyboard->AddSource("F");
		mpcKeyboard->AddSource("H");
		mpcKeyboard->AddSource("V");
		mpcKeyboard->AddSource("Tab");
		mpcMouse->AddSource("Button 1");
		mpcMouse->AddSource("Wheel Forward");
		mpcInput->AddProgramInput();
	}

	void Kill(void)
	{
		ORoot()->Remove(mpcInput);
		mpcInput = NULL;
	}
	CInputDevice* GetKeyboard(void) { return mpcInput->GetDevices()->GetDevice(mpcKeyboard->miUniqueID); }
	CInputDevice* GetMouse(void) { return mpcInput->GetDevices()->GetDevice(mpcMouse->miUniqueID); }

	void Event(CProgramDeviceDetail* pcDetail, char* szSource, float fValue)
	{
		pcDetail->AddEvent(szSource, fValue);
		mpcComponentInput->Update();
	}

	void Press(char* szKey) { Event(mpcKeyboard, szKey, 1.0f); }
	void Release(char* szKey) { Event(mpcKeyboard, szKey, 0.0f); }
	void Tap(char* szKey) { Press(szKey); Release(szKey); }
	void Click(char* szButton) { Event(mpcMouse, szButton, 1.0f); Event(mpcMouse, szButton, 0.0f); }
};


//////////////////////////////////////////////////////////////////////////
//
// The window is 64x32 with A on the left and B on the right.  Their grid
// is their parent.
//
//////////////////////////////////////////////////////////////////////////
void TestInputActionDispatch(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CGridContainer>			pGrid;
		Ptr<CActionTestComponent>	pA;
		Ptr<CActionTestComponent>	pB;
		CChars						szLog;
		CActionTestCounter			cTabs;
		CActionTestDevices			cDevices;
		Ptr<CWindowInputSource>		pInputSource;
		CComponentInput				cComponentInput;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Dispatch Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		pA = OMalloc<CActionTestComponent>(pTestWindow, &szLog);
		pB = OMalloc<CActionTestComponent>(pTestWindow, &szLog);
		AssertTrue(pGrid->AddComponent(pA, 0, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pB, 1, 0, 1, 1, 1, 1));
		pTestWindow->Layout();
		cTabs.miCalls = 0;

		cDevices.Init();
		pInputSource = OMalloc<CWindowInputSource>(Ptr<CInput>(cDevices.mpcInput));
		cComponentInput.Init(&pInputSource, &pTestWindow);
		cDevices.mpcComponentInput = &cComponentInput;
		AssertTrue(cComponentInput.AddKeyboard(cDevices.GetKeyboard()));
		AssertTrue(cComponentInput.AddMouse(cDevices.GetMouse()));

		pB->AddInputAction(IAF_KeyDown, "A", &pB, &CActionTestComponent::ADown);
		pB->AddInputAction(IAF_Printable, NULL, &pB, &CActionTestComponent::Printable);
		pB->AddInputAction(CIC_Copy, &pB, &CActionTestComponent::Copy);
		pA->AddInputAction(IAF_Wheel, NULL, &pA, &CActionTestComponent::Wheel);
		pGrid->AddInputAction(CIC_Tab, &cTabs, &CActionTestCounter::Count);

		//A click focusses the component under the pointer first.
		pTestWindow->PointerMoved(40, 10);
		cDevices.Click("Button 1");
		AssertTrue(pB->IsFocussed());
		AssertLog("");

		//A's key event is taken so it isn't also typed.  C's isn't so it is.
		cDevices.Tap("A");
		AssertLog("A;");
		cDevices.Tap("C");
		AssertLog("Printable(c);");
		cDevices.Press("Left Shift");
		cDevices.Tap("C");
		cDevices.Release("Left Shift");
		AssertLog("Printable(C);");

		//Control+C is a standard chord whose command is copy.
		cDevices.Press("Left Control");
		cDevices.Tap("C");
		cDevices.Release("Left Control");
		AssertLog("Copy;");

		//B has no action for tab so it goes up to the grid.
		cDevices.Tap("Tab");
		AssertInt(1, cTabs.miCalls);
		AssertLog("");

		//The wheel is for the component under the pointer, not the focussed one.
		pTestWindow->PointerMoved(10, 10);
		cDevices.Click("Wheel Forward");
		AssertLog("Wheel(-1);");
		AssertTrue(pB->IsFocussed());

		cComponentInput.Kill();
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
// The undo and redo chords, with and without shift, in the orders people
// press them, and the flip chords.  Shift can be pressed or let go while
// control is held.  The keys a chord uses up are never seen as keys.  A
// keyboard without right modifiers gets only the left ones' chords, so
// control+Z isn't also control+shift+Z.
//
//////////////////////////////////////////////////////////////////////////
void TestInputActionUndoChords(bool bRightModifiers)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CTickTestRefWindow>		pTick;
		SDataTestRefWindow			cData;
		CPointer					pNull;
		Ptr<CGridContainer>			pGrid;
		Ptr<CActionTestComponent>	pA;
		CChars						szLog;
		CActionTestDevices			cDevices;
		Ptr<CWindowInputSource>		pInputSource;
		CComponentInput				cComponentInput;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Undo Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		pA = OMalloc<CActionTestComponent>(pTestWindow, &szLog);
		AssertTrue(pGrid->AddComponent(pA, 0, 0, 1, 1, 1, 1));
		pTestWindow->Layout();

		cDevices.Init(bRightModifiers);
		pInputSource = OMalloc<CWindowInputSource>(Ptr<CInput>(cDevices.mpcInput));
		cComponentInput.Init(&pInputSource, &pTestWindow);
		cDevices.mpcComponentInput = &cComponentInput;
		AssertTrue(cComponentInput.AddKeyboard(cDevices.GetKeyboard()));
		AssertTrue(cComponentInput.AddMouse(cDevices.GetMouse()));

		pA->AddInputAction(CIC_Undo, &pA, &CActionTestComponent::Undo);
		pA->AddInputAction(CIC_StepUndo, &pA, &CActionTestComponent::StepUndo);
		pA->AddInputAction(CIC_Redo, &pA, &CActionTestComponent::Redo);
		pA->AddInputAction(CIC_StepRedo, &pA, &CActionTestComponent::StepRedo);
		pA->AddInputAction(CIC_HomeEnter, &pA, &CActionTestComponent::HomeEnter);
		pA->AddInputAction(CIC_FlipHorizontally, &pA, &CActionTestComponent::FlipH);
		pA->AddInputAction(CIC_FlipVertically, &pA, &CActionTestComponent::FlipV);
		pA->AddInputAction(IAF_AnyKeyDown, NULL, &pA, &CActionTestComponent::AnyKeyDown);

		pTestWindow->PointerMoved(10, 10);
		cDevices.Click("Button 1");
		AssertTrue(pA->IsFocussed());
		szLog.Clear();

		cDevices.Press("Left Control"); cDevices.Tap("Z"); cDevices.Release("Left Control");
		AssertLog("Undo;");

		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Tap("Z"); cDevices.Release("Left Shift"); cDevices.Release("Left Control");
		AssertLog("StepUndo;");

		cDevices.Press("Left Shift"); cDevices.Press("Left Control"); cDevices.Tap("Z"); cDevices.Release("Left Control"); cDevices.Release("Left Shift");
		AssertLog("StepUndo;");

		cDevices.Press("Left Control"); cDevices.Tap("Z"); cDevices.Tap("Z"); cDevices.Tap("Z"); cDevices.Release("Left Control");
		AssertLog("Undo;Undo;Undo;");

		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Tap("Z"); cDevices.Tap("Z"); cDevices.Tap("Z"); cDevices.Release("Left Shift"); cDevices.Release("Left Control");
		AssertLog("StepUndo;StepUndo;StepUndo;");

		cDevices.Press("Left Control"); cDevices.Tap("Z"); cDevices.Press("Left Shift"); cDevices.Tap("Z"); cDevices.Release("Left Shift"); cDevices.Tap("Z"); cDevices.Release("Left Control");
		AssertLog("Undo;StepUndo;Undo;");

		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Tap("Z"); cDevices.Release("Left Shift"); cDevices.Tap("Z"); cDevices.Release("Left Control");
		AssertLog("StepUndo;Undo;");

		if (bRightModifiers)
		{
			cDevices.Press("Right Control"); cDevices.Press("Right Shift"); cDevices.Tap("Z"); cDevices.Release("Right Shift"); cDevices.Release("Right Control");
			AssertLog("StepUndo;");
		}

		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Press("Z"); cDevices.Release("Left Shift"); cDevices.Release("Z"); cDevices.Release("Left Control");
		AssertLog("StepUndo;");

		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Tap("Y"); cDevices.Release("Left Shift"); cDevices.Tap("Y"); cDevices.Release("Left Control");
		AssertLog("StepRedo;Redo;");

		cDevices.Tap("Z");
		AssertLog("AnyDown(Z);");

		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Tap("Enter"); cDevices.Release("Left Shift"); cDevices.Release("Left Control");
		AssertLog("HomeEnter;");

		//Control+Enter isn't a chord.
		cDevices.Press("Left Control"); cDevices.Tap("Enter"); cDevices.Release("Left Control");
		AssertLog("AnyDown(Left Control);AnyDown(Enter);");

		//Control+Shift+F then H or V, with control and shift held or let go of first.
		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Tap("F"); cDevices.Tap("H"); cDevices.Release("Left Shift"); cDevices.Release("Left Control");
		AssertLog("FlipH;");
		cDevices.Press("Left Shift"); cDevices.Press("Left Control"); cDevices.Tap("F"); cDevices.Release("Left Control"); cDevices.Release("Left Shift"); cDevices.Tap("V");
		AssertLog("FlipV;");
		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Tap("F"); cDevices.Release("Left Shift"); cDevices.Tap("H"); cDevices.Release("Left Control");
		AssertLog("FlipH;");
		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Tap("F"); cDevices.Release("Left Control"); cDevices.Tap("V"); cDevices.Release("Left Shift");
		AssertLog("FlipV;");
		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Press("F"); cDevices.Release("Left Control"); cDevices.Release("F"); cDevices.Release("Left Shift"); cDevices.Tap("H");
		AssertLog("FlipH;");

		//Held, they carry on to the next.
		cDevices.Press("Left Control"); cDevices.Press("Left Shift"); cDevices.Tap("F"); cDevices.Tap("H"); cDevices.Tap("F"); cDevices.Tap("V"); cDevices.Release("Left Shift"); cDevices.Release("Left Control");
		AssertLog("FlipH;FlipV;");

		//H and V alone are keys.
		cDevices.Tap("H");
		cDevices.Tap("V");
		AssertLog("AnyDown(H);AnyDown(V);");

		cComponentInput.Kill();
		pInputSource = NULL;
		cDevices.Kill();

		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
		pGrid = NULL;
		pA = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
// A command nothing focussed takes goes to the window's content, with
// nothing focussed too, and from a tool window to the window it floats
// over, so an application's commands (Control+N, Control+S) work
// wherever the keys are.
//
//////////////////////////////////////////////////////////////////////////
void TestInputActionUnhandledCommands(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>					pTestWindow;
		Ptr<CWindow>					pTool;
		Ptr<CTickTestRefWindow>			pTick;
		SDataTestRefWindow				cData;
		CPointer						pNull;
		Ptr<CGridContainer>				pGrid;
		Ptr<CGridContainer>				pToolGrid;
		Ptr<CActionTestComponent>		pChild;
		Ptr<CActionTestComponent>		pToolChild;
		CChars							szLog;
		CActionTestCounter				cSaves;
		CInt32Rect						cRect;

		szLog.Init();
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pTestWindow = OMalloc<CWindow>("Main Window", &cNativeFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pGrid = OMalloc<CGridContainer>(pTestWindow);
		pTestWindow->SetContainer(pGrid);
		pChild = OMalloc<CActionTestComponent>(pTestWindow, &szLog);
		AssertTrue(pGrid->AddComponent(pChild, 0, 0, 1, 1, 1, 1));
		pTestWindow->Layout();
		cSaves.miCalls = 0;
		pGrid->AddInputAction(CIC_Save, &cSaves, &CActionTestCounter::Count);
		pChild->AddInputAction(CIC_Copy, &pChild, &CActionTestComponent::Copy);

		//Nothing focussed: the content takes it.
		AssertTrue(pTestWindow->GetFocus()->GetFocussedComponent().IsNull());
		pTestWindow->InputCommand(CIC_Save, true);
		AssertInt(1, cSaves.miCalls);
		pTestWindow->InputCommand(CIC_Copy, true);
		AssertLog("");

		//Focussed, it goes up to the content as before, once.
		pTestWindow->GetFocus()->SetFocus(pChild);
		pTestWindow->InputCommand(CIC_Save, true);
		AssertInt(2, cSaves.miCalls);
		pTestWindow->InputCommand(CIC_Copy, true);
		AssertLog("Copy;");

		//A tool window floating over it passes on what it doesn't take, focussed or not.
		pTool = OMalloc<CWindow>("Tool Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTool);
		pToolGrid = OMalloc<CGridContainer>(pTool);
		pTool->SetContainer(pToolGrid);
		pToolChild = OMalloc<CActionTestComponent>(pTool, &szLog);
		AssertTrue(pToolGrid->AddComponent(pToolChild, 0, 0, 1, 1, 1, 1));
		cRect.Init(0, 0, 31, 15);
		AssertTrue(pTool->ShowOwned(pTestWindow, &cRect));
		pTool->Layout();
		pTool->InputCommand(CIC_Save, true);
		AssertInt(3, cSaves.miCalls);
		pTool->GetFocus()->SetFocus(pToolChild);
		pTool->InputCommand(CIC_Save, true);
		AssertInt(4, cSaves.miCalls);
		pTool->InputCommand(CIC_Copy, true);
		AssertLog("");

		pToolChild = NULL;
		pToolGrid = NULL;
		ORoot()->Remove(pTool);
		pTool = NULL;
		pChild = NULL;
		pGrid = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;
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
void TestInputAction(void)
{
	BeginTests();
	ObjectsInit();

	TestInputActionMatching();
	TestInputActionShapes();
	TestInputActionEnabled();
	TestInputActionUnhandledCommands();
	TestInputActionDispatch();
	TestInputActionUndoChords(true);
	TestInputActionUndoChords(false);

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
