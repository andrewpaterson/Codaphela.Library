#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ClassDefines.h"
#include "InputLib/Input.h"
#include "WindowLib/Window.h"
#include "WindowLib/Component.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/ComponentInput.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "ScriptFramesTick.h"


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


struct SKeymapTestCommand
{
	EComponentInputCommand	eCommand;
	const char*				szName;
};


static SKeymapTestCommand	gasKeymapTestCommands[] =
{
	{ CIC_Up, "Up" }, { CIC_Enter, "Enter" }, { CIC_Space, "Space" }, { CIC_Tab, "Tab" }, { CIC_Copy, "Copy" }, { CIC_Cut, "Cut" }, { CIC_Paste, "Paste" },
	{ CIC_Undo, "Undo" }, { CIC_Redo, "Redo" }, { CIC_SelectAll, "SelectAll" }, { CIC_SelectWholeWord, "SelectWord" }, { CIC_Duplicate, "Duplicate" },
	{ CIC_ClearSelection, "Escape" }, { CIC_NextDifference, "NextDifference" }, { CIC_CloseDocument, "CloseDocument" },
};


//Logs the commands and characters it is given.  It's an editor to the keys, or isn't.
class CKeymapTestComponent : public CComponent
{
CONSTRUCTABLE(CKeymapTestComponent);
DESTRUCTABLE(CKeymapTestComponent);
protected:
	CChars*		mpszLog;
	bool		mbEditor;

public:
	void Init(Ptr<CWindow> pWindow, CChars* pszLog, bool bEditor)
	{
		size	i;

		PreInit();
		CComponent::Init(pWindow);
		mpszLog = pszLog;
		mbEditor = bEditor;
		SetCanGetFocus(true);
		for (i = 0; i < sizeof(gasKeymapTestCommands) / sizeof(gasKeymapTestCommands[0]); i++)
		{
			AddInputAction(gasKeymapTestCommands[i].eCommand, this, &CKeymapTestComponent::CommandAction);
		}
		AddInputAction(IAF_Printable, NULL, this, &CKeymapTestComponent::PrintableAction);
		PostInit();
	}

	void Class(void) override
	{
		CComponent::Class();
		U_Pointer(mpszLog);
		U_Bool(mbEditor);
	}

	bool UsesEditorKeys(void) override { return mbEditor; }

	void CommandAction(SComponentInputEvent* psEvent)
	{
		size	i;

		for (i = 0; i < sizeof(gasKeymapTestCommands) / sizeof(gasKeymapTestCommands[0]); i++)
		{
			if (gasKeymapTestCommands[i].eCommand == psEvent->eCommand)
			{
				mpszLog->Append(gasKeymapTestCommands[i].szName);
				mpszLog->Append(';');
			}
		}
	}

	void PrintableAction(SComponentInputEvent* psEvent)
	{
		mpszLog->Append(psEvent->cPrintable);
		mpszLog->Append(';');
	}
};


//A keyboard with the keys these tests use.
class CKeymapTestDevices
{
public:
	CInput*					mpcInput;  //Held by the root from Init to Kill.
	CProgramDeviceDetail*	mpcKeyboard;
	CComponentInput*		mpcComponentInput;

	void Init(void)
	{
		CProgramInput*	pcProgram;
		Ptr<CInput>		pInput;

		mpcComponentInput = NULL;
		pInput = OMalloc<CInput>((CNativeInput*)NULL);
		ORoot()->Add(pInput);
		mpcInput = &pInput;
		pcProgram = mpcInput->GetProgramInput();
		pcProgram->AddDetail("Programmatic: Keyboard", "Test Keyboard");
		mpcKeyboard = pcProgram->macPDeviceDetails.Get(0);
		mpcKeyboard->AddSource("Left Shift");
		mpcKeyboard->AddSource("Right Shift");
		mpcKeyboard->AddSource("Left Control");
		mpcKeyboard->AddSource("Right Control");
		mpcKeyboard->AddSource("Left Alt");
		mpcKeyboard->AddSource("Right Alt");
		mpcKeyboard->AddSource("A");
		mpcKeyboard->AddSource("C");
		mpcKeyboard->AddSource("D");
		mpcKeyboard->AddSource("Q");
		mpcKeyboard->AddSource("W");
		mpcKeyboard->AddSource("X");
		mpcKeyboard->AddSource("Z");
		mpcKeyboard->AddSource("Cursor Up");
		mpcKeyboard->AddSource("Enter");
		mpcKeyboard->AddSource("Space");
		mpcKeyboard->AddSource("Tab");
		mpcKeyboard->AddSource("Escape");
		mpcKeyboard->AddSource("Insert");
		mpcKeyboard->AddSource("Delete");
		mpcKeyboard->AddSource("Num Lock");
		mpcKeyboard->AddSource("Numpad 8");
		mpcKeyboard->AddSource("Numpad 0");
		mpcKeyboard->AddSource("F1");
		mpcKeyboard->AddSource("F4");
		mpcKeyboard->AddSource("F5");
		mpcKeyboard->AddSource("F6");
		mpcKeyboard->AddSource("F7");
		mpcInput->AddProgramInput();
	}

	void Kill(void)
	{
		ORoot()->Remove(mpcInput);
		mpcInput = NULL;
	}

	CInputDevice* GetKeyboard(void) { return mpcInput->GetDevices()->GetDevice(mpcKeyboard->miUniqueID); }
	void Press(char* szKey) { mpcKeyboard->AddEvent(szKey, 1.0f); mpcComponentInput->Update(); }
	void Release(char* szKey) { mpcKeyboard->AddEvent(szKey, 0.0f); mpcComponentInput->Update(); }
	void Tap(char* szKey) { Press(szKey); Release(szKey); }
	void Control(char* szKey) { Press("Left Control"); Tap(szKey); Release("Left Control"); }
	void Shift(char* szKey) { Press("Left Shift"); Tap(szKey); Release("Left Shift"); }
	void ControlShift(char* szKey) { Press("Left Control"); Press("Left Shift"); Tap(szKey); Release("Left Shift"); Release("Left Control"); }
};


//A window with an editor on the left and a component that isn't one on the right, and a keyboard.
class CKeymapTestWindow
{
public:
	CWinRefWindowFactory		cNativeFactory;
	Ptr<CWindow>				pWindow;
	Ptr<CTickTestRefWindow>		pTick;
	SDataTestRefWindow			cData;
	Ptr<CGridContainer>			pGrid;
	Ptr<CKeymapTestComponent>	pEditor;
	Ptr<CKeymapTestComponent>	pField;
	CKeymapTestDevices			cDevices;
	Ptr<CWindowInputSource>		pInputSource;
	CComponentInput				cInput;

	void Init(CChars* pszLog)
	{
		cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		Init(pszLog, pTick);
	}

	//The factory is initialised already, and the window ticks with pWindowTick.
	void Init(CChars* pszLog, Ptr<CWindowTick> pWindowTick)
	{
		CPointer	pNull;

		pWindow = OMalloc<CWindow>("Keymap Window", &cNativeFactory, pWindowTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pGrid = OMalloc<CGridContainer>(pWindow);
		pWindow->SetContainer(pGrid);
		pEditor = OMalloc<CKeymapTestComponent>(pWindow, pszLog, true);
		pField = OMalloc<CKeymapTestComponent>(pWindow, pszLog, false);
		AssertTrue(pGrid->AddComponent(pEditor, 0, 0, 1, 1, 1, 1));
		AssertTrue(pGrid->AddComponent(pField, 1, 0, 1, 1, 1, 1));
		pWindow->Layout();

		cDevices.Init();
		pInputSource = OMalloc<CWindowInputSource>(Ptr<CInput>(cDevices.mpcInput));
		cInput.Init(&pInputSource, &pWindow);
		cDevices.mpcComponentInput = &cInput;
		AssertTrue(cInput.AddKeyboard(cDevices.GetKeyboard()));
		pWindow->SetComponentInput(&cInput);
	}

	void Kill(void)
	{
		pWindow->SetComponentInput(NULL);
		cInput.Kill();
		pInputSource = NULL;
		cDevices.Kill();
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
		pGrid = NULL;
		pEditor = NULL;
		pField = NULL;
		AssertSize(0, gcObjects.NumFreeObjects());
		AssertTrue(ORoot()->RemoveAll());
		AssertSize(2, gcObjects.NumMemoryIndexes());
		cNativeFactory.Kill();
	}

	void FocusEditor(void) { pWindow->GetFocus()->SetFocus(pEditor); }
	void FocusField(void) { pWindow->GetFocus()->SetFocus(pField); }
};


//////////////////////////////////////////////////////////////////////////
//
// Keys bound then unbound give what they gave before: a standard chord, a
// key with a command, a key that types, and nothing for a function key or
// a chord only the binding made.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapUnbindKeys(void)
{
	CChars				szLog;
	CChars				szShortcut;
	CKeymapTestWindow	cTest;

	szLog.Init();
	cTest.Init(&szLog);
	cTest.FocusField();

	cTest.cDevices.ControlShift("Z");
	AssertLog("Redo;");
	AssertTrue(cTest.cInput.BindKeys("Control+Shift+Z", CIC_Copy));
	cTest.cDevices.ControlShift("Z");
	AssertLog("Copy;");

	//Bound again it still remembers the first.  The modifiers can be named in any order.
	AssertTrue(cTest.cInput.BindKeys("Shift+Control+Z", CIC_Paste));
	cTest.cDevices.ControlShift("Z");
	AssertLog("Paste;");
	AssertTrue(cTest.cInput.UnbindKeys("Shift+Control+Z"));
	AssertFalse(cTest.cInput.IsBound("Control+Shift+Z"));
	cTest.cDevices.ControlShift("Z");
	AssertLog("Redo;");
	AssertFalse(cTest.cInput.UnbindKeys("Control+Shift+Z"));

	//The menus go back to the standard keys.
	szShortcut.Init();
	AssertTrue(cTest.cInput.GetShortcutText(CIC_Redo, &szShortcut));
	AssertString("Ctrl+Shift+Z", szShortcut.Text());
	szShortcut.Kill();

	//A function key does nothing again.
	AssertTrue(cTest.cInput.BindKeys("F5", CIC_Copy));
	cTest.cDevices.Tap("F5");
	AssertLog("Copy;");
	AssertTrue(cTest.cInput.UnbindKeys("F5"));
	cTest.cDevices.Tap("F5");
	AssertLog("");

	//A key with a command has it again.
	AssertTrue(cTest.cInput.BindKeys("Enter", CIC_Copy));
	cTest.cDevices.Tap("Enter");
	AssertLog("Copy;");
	AssertTrue(cTest.cInput.UnbindKeys("Enter"));
	cTest.cDevices.Tap("Enter");
	AssertLog("Enter;");

	//A key that types types again.
	AssertTrue(cTest.cInput.BindKeys("A", CIC_Copy));
	cTest.cDevices.Tap("A");
	AssertLog("Copy;");
	AssertTrue(cTest.cInput.UnbindKeys("A"));
	cTest.cDevices.Tap("A");
	AssertLog("a;");

	//A chord that wasn't there before, made while the keyboard is in use, and gone again.
	cTest.cDevices.Control("Q");
	AssertLog("");
	AssertTrue(cTest.cInput.BindKeys("Control+Q", CIC_Copy));
	cTest.cDevices.Control("Q");
	AssertLog("Copy;");
	cTest.cDevices.Press("Right Control");
	cTest.cDevices.Tap("Q");
	cTest.cDevices.Release("Right Control");
	AssertLog("Copy;");
	AssertTrue(cTest.cInput.UnbindKeys("Control+Q"));
	cTest.cDevices.Control("Q");
	AssertLog("");

	cTest.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A component that uses editor keys has them; one that doesn't, the
// standard ones.  Until any are bound an editor has the standard ones too.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapEditorKeys(void)
{
	CChars				szLog;
	CKeymapTestWindow	cTest;

	szLog.Init();
	cTest.Init(&szLog);

	//Standard keys until there are editor keys.
	cTest.FocusEditor();
	cTest.cDevices.Control("C");
	cTest.cDevices.Tap("Cursor Up");
	AssertLog("Copy;Up;");

	AssertTrue(cTest.cInput.BindEditorKeys("Control+Q", CIC_Copy));
	AssertTrue(cTest.cInput.BindEditorKeys("F6", CIC_Up));
	AssertTrue(cTest.cInput.BindEditorKeys("Enter", CIC_Enter));
	AssertTrue(cTest.cInput.BindEditorKeys("Insert", CIC_ToggleInsert));
	AssertTrue(cTest.cInput.IsEditorBound("Control+Q"));
	AssertTrue(cTest.cInput.IsEditorBound("F6"));
	AssertFalse(cTest.cInput.IsEditorBound("Control+C"));
	AssertFalse(cTest.cInput.IsEditorBound("F13"));

	//In the editor the bound keys work and those moved off do nothing.
	cTest.cDevices.Control("Q");
	cTest.cDevices.Tap("F6");
	cTest.cDevices.Tap("Enter");
	AssertLog("Copy;Up;Enter;");
	cTest.cDevices.Control("C");
	cTest.cDevices.Tap("Cursor Up");
	AssertLog("");

	//The other component has the standard keys, and the editor's mean nothing to it.
	cTest.FocusField();
	cTest.cDevices.Control("C");
	cTest.cDevices.Tap("Cursor Up");
	cTest.cDevices.Tap("Enter");
	AssertLog("Copy;Up;Enter;");
	cTest.cDevices.Control("Q");
	cTest.cDevices.Tap("F6");
	AssertLog("");

	//Keys that aren't an editor's are the same everywhere.
	cTest.FocusEditor();
	cTest.cDevices.Tap("F7");
	AssertLog("NextDifference;");
	AssertTrue(cTest.cInput.BindKeys("Control+D", CIC_NextDifference));
	cTest.cDevices.Control("D");
	AssertLog("NextDifference;");
	AssertTrue(cTest.cInput.UnbindKeys("Control+D"));

	//Typing and space aren't an editor's keys to move.
	cTest.cDevices.Tap("A");
	cTest.cDevices.Tap("Space");
	cTest.cDevices.Shift("Space");
	AssertLog("a;Space;Space;");

	//With num lock off a numpad key is the key it stands for.
	cTest.cDevices.Tap("Numpad 8");
	AssertLog("");
	AssertTrue(cTest.cInput.BindEditorKeys("Cursor Up", CIC_Up));
	cTest.cDevices.Tap("Numpad 8");
	AssertLog("Up;");
	AssertTrue(cTest.cInput.UnbindEditorKeys("Cursor Up"));
	AssertFalse(cTest.cInput.UnbindEditorKeys("Cursor Up"));

	cTest.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Insert is one of an editor's keys, and what does it can be moved; the
// insert key still does it elsewhere, and copying and pasting with it are
// chords that don't.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapInsert(void)
{
	CChars				szLog;
	CKeymapTestWindow	cTest;

	szLog.Init();
	cTest.Init(&szLog);
	cTest.FocusEditor();
	AssertTrue(cTest.cInput.BindEditorKeys("Insert", CIC_ToggleInsert));
	AssertTrue(cTest.cInput.BindEditorKeys("Control+Insert", CIC_Copy));

	AssertTrue(cTest.cInput.IsInsert());
	cTest.cDevices.Tap("Insert");
	AssertFalse(cTest.cInput.IsInsert());
	cTest.cDevices.Control("Insert");
	AssertFalse(cTest.cInput.IsInsert());
	AssertLog("Copy;");
	cTest.cDevices.Tap("Numpad 0");
	AssertTrue(cTest.cInput.IsInsert());

	//Moved to a key and to a chord.
	AssertTrue(cTest.cInput.UnbindEditorKeys("Insert"));
	AssertTrue(cTest.cInput.BindEditorKeys("F1", CIC_ToggleInsert));
	AssertTrue(cTest.cInput.BindEditorKeys("Control+Q", CIC_ToggleInsert));
	cTest.cDevices.Tap("Insert");
	AssertTrue(cTest.cInput.IsInsert());
	cTest.cDevices.Tap("F1");
	AssertFalse(cTest.cInput.IsInsert());
	cTest.cDevices.Control("Q");
	AssertTrue(cTest.cInput.IsInsert());
	AssertLog("");

	//The insert key given something else.
	AssertTrue(cTest.cInput.BindEditorKeys("Insert", CIC_Copy));
	cTest.cDevices.Tap("Insert");
	AssertTrue(cTest.cInput.IsInsert());
	AssertLog("Copy;");

	//Elsewhere it does what it always did.
	cTest.FocusField();
	cTest.cDevices.Tap("Insert");
	AssertFalse(cTest.cInput.IsInsert());
	cTest.cDevices.Tap("Insert");
	AssertTrue(cTest.cInput.IsInsert());
	AssertLog("");

	cTest.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A dialog has the standard keys whatever is focussed in it.
//
//////////////////////////////////////////////////////////////////////////
class CKeymapTestDialog
{
public:
	Ptr<CWindow>				pDialog;
	Ptr<CKeymapTestComponent>	pEditor;
	CKeymapTestDevices*			pcDevices;
	CChars*						pszLog;
	CChars						szKeptLog;
	bool						bModal;
};


bool KeymapDialogFrames(Ptr<CWindow> pWindow, int32 iFrame, void* pvContext)
{
	CKeymapTestDialog*	pcDialog;

	pcDialog = (CKeymapTestDialog*)pvContext;
	pcDialog->bModal = pWindow->IsModal();
	pWindow->GetFocus()->SetFocus(pcDialog->pEditor);
	pcDialog->pcDevices->Control("C");
	pcDialog->pcDevices->Tap("Cursor Up");
	pcDialog->pcDevices->Control("Q");
	return false;
}


bool KeymapDialogOwnerFrames(Ptr<CWindow> pWindow, int32 iFrame, void* pvContext)
{
	CKeymapTestDialog*	pcDialog;

	pcDialog = (CKeymapTestDialog*)pvContext;
	if (iFrame == 0)
	{
		pcDialog->pDialog->ShowModal(pWindow);
		return true;
	}
	return false;
}


void TestKeymapDialog(void)
{
	CChars						szLog;
	CKeymapTestWindow			cTest;
	CKeymapTestDialog			cDialog;
	Ptr<CScriptFramesTick>		pOwnerTick;
	Ptr<CScriptFramesTick>		pDialogTick;
	Ptr<CGridContainer>			pGrid;

	szLog.Init();
	cTest.cNativeFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
	pOwnerTick = OMalloc<CScriptFramesTick>(&KeymapDialogOwnerFrames, &cDialog);
	cTest.Init(&szLog, pOwnerTick);
	AssertTrue(cTest.cInput.BindEditorKeys("Control+Q", CIC_Copy));
	AssertTrue(cTest.cInput.BindEditorKeys("Enter", CIC_Enter));
	AssertFalse(cTest.pWindow->IsModal());

	cDialog.pcDevices = &cTest.cDevices;
	cDialog.bModal = false;
	pDialogTick = OMalloc<CScriptFramesTick>(&KeymapDialogFrames, &cDialog);
	cDialog.pDialog = OMalloc<CWindow>("Keymap Dialog", &cTest.cNativeFactory, pDialogTick, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x2B, 0x2D, 0x30)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
	ORoot()->Add(cDialog.pDialog);
	pGrid = OMalloc<CGridContainer>(cDialog.pDialog);
	cDialog.pDialog->SetContainer(pGrid);
	cDialog.pEditor = OMalloc<CKeymapTestComponent>(cDialog.pDialog, &szLog, true);
	AssertTrue(pGrid->AddComponent(cDialog.pEditor, 0, 0, 1, 1, 1, 1));
	cDialog.pDialog->Layout();

	AssertTrue(cTest.pWindow->Show());

	//Control+C and up as ever; the editor's Control+Q means nothing in a dialog.
	AssertTrue(cDialog.bModal);
	AssertFalse(cDialog.pDialog->IsModal());
	AssertLog("Copy;Up;");

	ORoot()->Remove(cDialog.pDialog);
	cDialog.pDialog = NULL;
	cDialog.pEditor = NULL;
	pDialogTick = NULL;
	pOwnerTick = NULL;
	pGrid = NULL;
	cTest.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What the dialog asks about keys before it lets them be bound.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapKeyQueries(void)
{
	CChars				szLog;
	CKeymapTestWindow	cTest;

	szLog.Init();
	cTest.Init(&szLog);

	//Keys that type, with shift too; with control or alt they don't, and space does though it's a command.
	AssertTrue(cTest.cInput.IsTypingKeys("A"));
	AssertTrue(cTest.cInput.IsTypingKeys("Shift+A"));
	AssertTrue(cTest.cInput.IsTypingKeys("Space"));
	AssertTrue(cTest.cInput.IsTypingKeys("Shift+Space"));
	AssertFalse(cTest.cInput.IsTypingKeys("Control+A"));
	AssertFalse(cTest.cInput.IsTypingKeys("Alt+A"));
	AssertFalse(cTest.cInput.IsTypingKeys("F5"));
	AssertFalse(cTest.cInput.IsTypingKeys("Enter"));
	AssertFalse(cTest.cInput.IsTypingKeys(""));

	//What keys give when nothing has bound them, even once something has.
	AssertInt(CIC_Copy, cTest.cInput.GetStandardCommand("Control+C"));
	AssertInt(CIC_Enter, cTest.cInput.GetStandardCommand("Enter"));
	AssertInt(CIC_Ignored, cTest.cInput.GetStandardCommand("F5"));
	AssertInt(CIC_Ignored, cTest.cInput.GetStandardCommand("Control+Q"));
	AssertInt(CIC_Ignored, cTest.cInput.GetStandardCommand(""));
	AssertTrue(cTest.cInput.BindKeys("Control+C", CIC_Command));
	AssertInt(CIC_Copy, cTest.cInput.GetStandardCommand("Control+C"));
	AssertTrue(cTest.cInput.UnbindKeys("Control+C"));

	//Dialogs and text fields need these; only an editor needs these.
	AssertTrue(CComponentInput::IsWidgetCommand(CIC_Enter));
	AssertTrue(CComponentInput::IsWidgetCommand(CIC_Copy));
	AssertTrue(CComponentInput::IsWidgetCommand(CIC_Up));
	AssertFalse(CComponentInput::IsWidgetCommand(CIC_Duplicate));
	AssertFalse(CComponentInput::IsWidgetCommand(CIC_DeleteLine));
	AssertFalse(CComponentInput::IsWidgetCommand(CIC_LineUp));
	AssertFalse(CComponentInput::IsWidgetCommand(CIC_CloseDocument));
	AssertFalse(CComponentInput::IsWidgetCommand(CIC_Ignored));

	//An editor has its own keys for these and not for typing or the window's.
	AssertTrue(CComponentInput::IsEditorCommand(CIC_Up));
	AssertTrue(CComponentInput::IsEditorCommand(CIC_Redo));
	AssertTrue(CComponentInput::IsEditorCommand(CIC_ToggleInsert));
	AssertFalse(CComponentInput::IsEditorCommand(CIC_Space));
	AssertFalse(CComponentInput::IsEditorCommand(CIC_NextDifference));
	AssertFalse(CComponentInput::IsEditorCommand(CIC_Command));
	AssertFalse(CComponentInput::IsEditorCommand(CIC_Ignored));

	cTest.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestKeymap(void)
{
	BeginTests();
	ObjectsInit();

	TestKeymapUnbindKeys();
	TestKeymapEditorKeys();
	TestKeymapInsert();
	TestKeymapDialog();
	TestKeymapKeyQueries();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
