#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ClassDefines.h"
#include "InputLib/Input.h"
#include "WindowLib/Window.h"
#include "WindowLib/Component.h"
#include "WindowLib/GridContainer.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/ComponentInput.h"
#include "WindowLib/CommandSet.h"
#include "WindowLib/EditCommands.h"
#include "WindowLib/KeyCaptureField.h"
#include "WindowLib/JewelTheme.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"
#include "ScriptFramesTick.h"


Ptr<CFont>	GenerateMenuFont(Ptr<CWindow> pWindow);


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()
#define TEST_KEYMAP_DIRECTORY	"Output" _FS_ "Keymap"
#define TEST_KEYMAP_FILE		"Output" _FS_ "Keymap" _FS_ "Keymap.xml"


struct SKeymapTestCommand
{
	EComponentInputCommand	eCommand;
	const char*				szName;
};


static SKeymapTestCommand	gasKeymapTestCommands[] =
{
	{ CIC_Up, "Up" }, { CIC_Enter, "Enter" }, { CIC_Space, "Space" }, { CIC_Tab, "Tab" }, { CIC_Copy, "Copy" }, { CIC_Cut, "Cut" }, { CIC_Paste, "Paste" },
	{ CIC_Undo, "Undo" }, { CIC_Redo, "Redo" }, { CIC_SelectAll, "SelectAll" }, { CIC_ExtendSelection, "ExtendSelection" }, { CIC_Duplicate, "Duplicate" },
	{ CIC_ClearSelection, "Escape" }, { CIC_NextDifference, "NextDifference" }, { CIC_CloseDocument, "CloseDocument" }, { CIC_JumpToSource, "JumpToSource" },
};


//Logs the commands and characters it is given, and the context the last command's keys were bound in.  Its keys are
//read in the Editor context, or the context it's given, and it takes typing or doesn't.
class CKeymapTestComponent : public CComponent
{
CONSTRUCTABLE(CKeymapTestComponent);
DESTRUCTABLE(CKeymapTestComponent);
public:
	CChars*		mpszLog;
	bool		mbEditor;
	char*		mszContext;
	bool		mbTakesTyping;
	char*		mszLastContext;

public:
	void Init(Ptr<CWindow> pWindow, CChars* pszLog, bool bEditor)
	{
		size	i;

		PreInit();
		CComponent::Init(pWindow);
		mpszLog = pszLog;
		mbEditor = bEditor;
		mszContext = NULL;
		mbTakesTyping = false;
		mszLastContext = NULL;
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
		U_Pointer(mszContext);
		U_Bool(mbTakesTyping);
		U_Pointer(mszLastContext);
	}

	const char* GetKeyContext(void) override { return mbEditor ? COMPONENT_INPUT_EDITOR_CONTEXT : mszContext; }
	bool TakesTyping(void) override { return mbTakesTyping; }

	void CommandAction(SComponentInputEvent* psEvent)
	{
		size	i;

		mszLastContext = (char*)psEvent->szContext;
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


//Counts the commands run, by the command's name.
class CKeymapTestCommands
{
public:
	CChars*		mpszLog;

	void Run(CCommand* pcCommand)
	{
		mpszLog->Append(pcCommand->GetName());
		mpszLog->Append(';');
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Contexts: one is added inside the global keys or another, not inside
// one that isn't there; keys are bound in one that is, and in one that
// isn't a typing context a key that types isn't.  A command's event says
// the context its keys were bound in, and a standard key's none.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapContexts(void)
{
	CChars				szLog;
	CKeymapTestWindow	cTest;
	SLogConfig			sLogConfig;

	szLog.Init();
	cTest.Init(&szLog);

	AssertTrue(cTest.cInput.HasContext(COMPONENT_INPUT_GLOBAL_CONTEXT));
	AssertTrue(cTest.cInput.HasContext(NULL));
	AssertFalse(cTest.cInput.HasContext(COMPONENT_INPUT_EDITOR_CONTEXT));
	AssertTrue(cTest.cInput.AddContext("Outer", NULL, false));
	AssertTrue(cTest.cInput.AddContext("Inner", "Outer", true));
	AssertTrue(cTest.cInput.HasContext("Inner"));

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(cTest.cInput.AddContext("Lost", "Nowhere", false));
	AssertFalse(cTest.cInput.BindKeys("F5", CIC_Copy, "Nowhere"));
	AssertFalse(cTest.cInput.BindKeys("A", CIC_Copy, "Outer"));
	AssertFalse(cTest.cInput.BindKeys("Shift+A", CIC_Copy, "Outer"));
	AssertFalse(cTest.cInput.BindEditorKeys("A", CIC_Copy));
	gcLogger.SetConfig(&sLogConfig);
	AssertFalse(cTest.cInput.HasContext("Lost"));
	AssertFalse(cTest.cInput.IsBound("A", "Outer"));

	AssertTrue(cTest.cInput.BindKeys("A", CIC_Copy, "Inner"));
	AssertTrue(cTest.cInput.BindKeys("Shift+A", CIC_Paste, "Inner"));
	AssertTrue(cTest.cInput.BindKeys("F5", CIC_Copy, "Outer"));
	AssertTrue(cTest.cInput.IsBound("A", "Inner"));
	AssertTrue(cTest.cInput.IsBound("Shift+A", "Inner"));
	AssertTrue(cTest.cInput.IsBound("F5", "Outer"));
	AssertFalse(cTest.cInput.IsBound("F5", "Inner"));
	AssertFalse(cTest.cInput.IsBound("F5"));

	//Keys bound in a context mean nothing elsewhere: the field still types A and a, and F5 does nothing.
	cTest.FocusField();
	cTest.cDevices.Tap("A");
	cTest.cDevices.Shift("A");
	cTest.cDevices.Tap("F5");
	AssertLog("a;A;");

	//The editor's keys say which context they're from; a standard key's say none.
	AssertTrue(cTest.cInput.BindEditorKeys("F6", CIC_Up));
	cTest.FocusEditor();
	cTest.cDevices.Tap("F6");
	AssertLog("Up;");
	AssertString(COMPONENT_INPUT_EDITOR_CONTEXT, cTest.pEditor->mszLastContext);
	cTest.FocusField();
	cTest.cDevices.Control("C");
	AssertLog("Copy;");
	AssertNull(cTest.pField->mszLastContext);

	AssertTrue(cTest.cInput.UnbindKeys("A", "Inner"));
	AssertFalse(cTest.cInput.UnbindKeys("A", "Inner"));
	AssertFalse(cTest.cInput.IsBound("A", "Inner"));

	cTest.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A component's keys are read in its context, falling back to the
// contexts around it, the innermost binding winning.  In a typing context
// a bound key that types (with shift too) gives its command and an
// unbound one types.  Over a component in a typing context its keys are
// read, for it, wherever the focus is, unless the focussed component
// takes typing.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapContextFocus(void)
{
	CChars				szLog;
	CKeymapTestWindow	cTest;

	szLog.Init();
	cTest.Init(&szLog);
	AssertTrue(cTest.cInput.AddContext("Outer", NULL, false));
	AssertTrue(cTest.cInput.AddContext("Inner", "Outer", true));
	cTest.pField->mszContext = "Inner";
	cTest.FocusField();

	//Outer's key, from Inner, then Inner's own on the same key.
	AssertTrue(cTest.cInput.BindKeys("F5", CIC_Copy, "Outer"));
	cTest.cDevices.Tap("F5");
	AssertLog("Copy;");
	AssertString("Outer", cTest.pField->mszLastContext);
	AssertTrue(cTest.cInput.BindKeys("F5", CIC_Paste, "Inner"));
	cTest.cDevices.Tap("F5");
	AssertLog("Paste;");
	AssertString("Inner", cTest.pField->mszLastContext);

	//Bare keys in a typing context; one that isn't bound types.
	AssertTrue(cTest.cInput.BindKeys("A", CIC_Cut, "Inner"));
	AssertTrue(cTest.cInput.BindKeys("Shift+A", CIC_Duplicate, "Inner"));
	cTest.cDevices.Tap("A");
	cTest.cDevices.Shift("A");
	cTest.cDevices.Tap("C");
	AssertLog("Cut;Duplicate;c;");

	//Over the field, with the editor focussed: the field's keys, for the field.
	cTest.FocusEditor();
	cTest.pField->mszLastContext = NULL;
	cTest.pWindow->PointerMoved(48, 16);
	cTest.cDevices.Tap("A");
	AssertLog("Cut;");
	AssertString("Inner", cTest.pField->mszLastContext);
	AssertNull(cTest.pEditor->mszLastContext);
	cTest.cDevices.Tap("C");
	AssertLog("c;");

	//Unless the editor takes typing; or the pointer is over the editor itself.
	cTest.pEditor->mbTakesTyping = true;
	cTest.cDevices.Tap("A");
	AssertLog("a;");
	cTest.pEditor->mbTakesTyping = false;
	cTest.pWindow->PointerMoved(16, 16);
	cTest.cDevices.Tap("A");
	AssertLog("a;");

	cTest.Kill();
	szLog.Kill();
}


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
// A command with several keys, changed and bound again: both of a pair
// swapped, one moved off a standard chord that then has its own back, and
// the first being those a menu shows.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapCommands(void)
{
	CChars					szLog;
	CChars					szKeys;
	CKeymapTestWindow		cTest;
	CKeymapTestCommands		cRun;
	Ptr<CCommandSet>		pCommands;
	Ptr<CCommand>			pFirst;
	Ptr<CCommand>			pSecond;
	CArrayChars				aszKeys;

	szLog.Init();
	cTest.Init(&szLog);
	cRun.mpszLog = &szLog;
	pCommands = OMalloc<CCommandSet>();
	cTest.pWindow->SetCommands(pCommands);
	pFirst = pCommands->Add("First", "First", "Control+Q", Ptr<CImageCel>(), &cRun, &CKeymapTestCommands::Run);
	pSecond = pCommands->Add("Second", "Second", "F5", Ptr<CImageCel>(), &cRun, &CKeymapTestCommands::Run);
	pFirst->AddDefaultKeys("Alt+Z");
	AssertTrue(pCommands->BindKeys(&cTest.cInput));
	cTest.FocusField();

	AssertSize(2, pFirst->NumKeys());
	AssertSize(2, pFirst->NumDefaultKeys());
	AssertString("Control+Q", pFirst->GetKeys());
	AssertString("Alt+Z", pFirst->GetKeys(1));
	AssertTrue(pFirst->HasKeys("Control+Q"));
	AssertFalse(pFirst->HasKeys("Control+W"));
	AssertFalse(pFirst->HasKeys(""));
	AssertTrue(pFirst->HasDefaultKeys());
	AssertString("First", pCommands->FindKeys("Alt+Z")->GetName());

	//Any of its keys run it.
	cTest.cDevices.Control("Q");
	cTest.cDevices.Press("Left Alt");
	cTest.cDevices.Tap("Z");
	cTest.cDevices.Release("Left Alt");
	cTest.cDevices.Tap("F5");
	AssertLog("First;First;Second;");

	//Changed and bound again.
	pFirst->ClearKeys();
	AssertTrue(pFirst->AddKeys("Control+W"));
	AssertFalse(pFirst->AddKeys("Control+W"));
	AssertFalse(pFirst->AddKeys(""));
	AssertFalse(pFirst->HasDefaultKeys());
	AssertTrue(pCommands->BindKeys(&cTest.cInput));
	cTest.cDevices.Control("Q");
	AssertLog("");
	cTest.cDevices.Control("W");
	AssertLog("First;");

	//The first of them is what's shown.
	szKeys.Init();
	AssertTrue(pFirst->AppendKeysText(&szKeys, &cTest.cInput));
	AssertString("Ctrl+W", szKeys.Text());
	szKeys.Kill();

	//Swapped in one go, and off the standard chord, which has its own back.
	aszKeys.Init();
	aszKeys.Add("F5");
	pFirst->SetKeys(&aszKeys);
	aszKeys.ReInit();
	aszKeys.Add("Control+Q");
	pSecond->SetKeys(&aszKeys);
	aszKeys.Kill();
	AssertTrue(pCommands->BindKeys(&cTest.cInput));
	cTest.cDevices.Tap("F5");
	cTest.cDevices.Control("Q");
	cTest.cDevices.Control("W");
	AssertLog("First;Second;ExtendSelection;");

	//No keys at all.
	pSecond->ClearKeys();
	AssertTrue(pCommands->BindKeys(&cTest.cInput));
	AssertFalse(cTest.cInput.IsBound("Control+Q"));
	cTest.cDevices.Control("Q");
	AssertLog("");
	AssertString("", pSecond->GetKeys());

	//Back to what they were made with.
	pCommands->ResetKeys();
	AssertTrue(pCommands->BindKeys(&cTest.cInput));
	AssertTrue(pFirst->HasDefaultKeys());
	cTest.cDevices.Control("Q");
	cTest.cDevices.Tap("F5");
	AssertLog("First;Second;");

	cTest.pWindow->SetCommands(NULL);
	pCommands = NULL;
	pFirst = NULL;
	pSecond = NULL;
	cTest.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The keys of a window's standard action can be changed for good: the
// keys it had stop giving it, a default put back gives it again.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapWindowCommands(void)
{
	CChars				szLog;
	CKeymapTestWindow	cTest;
	CKeymapTestCommands	cRun;
	Ptr<CCommandSet>	pCommands;
	Ptr<CCommand>		pClose;
	Ptr<CCommand>		pDifference;

	szLog.Init();
	cTest.Init(&szLog);
	cRun.mpszLog = &szLog;
	pCommands = OMalloc<CCommandSet>();
	cTest.pWindow->SetCommands(pCommands);
	pClose = pCommands->Add("Close", "Close", "Control+F4", Ptr<CImageCel>(), &cRun, &CKeymapTestCommands::Run);
	pClose->SetStandardCommand(CIC_CloseDocument);
	pClose->SetKeyScope(CKS_Window);
	pDifference = pCommands->Add("Difference", "Difference", "F7", Ptr<CImageCel>(), &cRun, &CKeymapTestCommands::Run);
	pDifference->SetStandardCommand(CIC_NextDifference);
	pDifference->SetKeyScope(CKS_Window);
	AssertTrue(pCommands->BindKeys(&cTest.cInput));

	//They give the standard command, they don't run the command.
	cTest.FocusField();
	cTest.cDevices.Control("F4");
	cTest.cDevices.Tap("F7");
	AssertLog("CloseDocument;NextDifference;");

	//Moved, to everywhere.
	pClose->ClearKeys();
	AssertTrue(pClose->AddKeys("Alt+Q"));
	pDifference->ClearKeys();
	AssertTrue(pDifference->AddKeys("F6"));
	AssertTrue(pCommands->BindKeys(&cTest.cInput));
	cTest.cDevices.Control("F4");
	cTest.cDevices.Tap("F7");
	AssertLog("");
	cTest.cDevices.Press("Left Alt");
	cTest.cDevices.Tap("Q");
	cTest.cDevices.Release("Left Alt");
	cTest.cDevices.Tap("F6");
	AssertLog("CloseDocument;NextDifference;");
	cTest.FocusEditor();
	cTest.cDevices.Tap("F6");
	AssertLog("NextDifference;");

	//Back.
	pCommands->ResetKeys();
	AssertTrue(pCommands->BindKeys(&cTest.cInput));
	cTest.cDevices.Control("F4");
	cTest.cDevices.Tap("F7");
	AssertLog("CloseDocument;NextDifference;");
	cTest.cDevices.Tap("F6");
	AssertLog("");

	cTest.pWindow->SetCommands(NULL);
	pCommands = NULL;
	pClose = NULL;
	pDifference = NULL;
	cTest.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The standard actions as commands: every key does as it did until it is
// changed, the Edit commands show theirs, and changing one changes it in
// the editor alone.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapEditCommands(void)
{
	CChars				szLog;
	CChars				szKeys;
	CKeymapTestWindow	cTest;
	Ptr<CCommandSet>	pCommands;
	Ptr<CEditCommands>	pEdit;
	Ptr<CCommand>		pCopy;

	szLog.Init();
	cTest.Init(&szLog);
	pCommands = OMalloc<CCommandSet>();
	cTest.pWindow->SetCommands(pCommands);
	pEdit = OMalloc<CEditCommands>(cTest.pWindow, pCommands);
	pEdit->AddKeymapCommands(pCommands);
	AssertTrue(pCommands->BindKeys(&cTest.cInput));

	pCopy = pCommands->Find("Edit.Copy");
	AssertSize(2, pCopy->NumKeys());
	AssertInt(CKS_Editor, pCopy->GetKeyScope());
	AssertInt(CIC_Copy, pCopy->GetStandardCommand());
	AssertInt(CIC_Up, pCommands->Find("Editor.Up")->GetStandardCommand());
	AssertInt(CKS_Window, pCommands->Find("Window.CloseDocument")->GetKeyScope());
	AssertInt(CKS_Window, pCommands->Find("Window.JumpToSource")->GetKeyScope());
	AssertInt(CIC_JumpToSource, pCommands->Find("Window.JumpToSource")->GetStandardCommand());
	AssertSize(2, pCommands->Find("Editor.Enter")->NumKeys());
	AssertTrue(cTest.cInput.IsEditorBound("Control+C"));
	AssertTrue(cTest.cInput.IsEditorBound("Control+Insert"));
	AssertTrue(cTest.cInput.IsEditorBound("Cursor Up"));
	AssertFalse(cTest.cInput.IsBound("Control+C"));

	szKeys.Init();
	AssertTrue(pCopy->AppendKeysText(&szKeys, &cTest.cInput));
	AssertString("Ctrl+C", szKeys.Text());
	szKeys.Kill();

	//Everything as it was, in and out of an editor.
	cTest.FocusEditor();
	cTest.cDevices.Control("C");
	cTest.cDevices.Control("Insert");
	cTest.cDevices.Control("X");
	cTest.cDevices.Shift("Delete");
	cTest.cDevices.Control("Z");
	cTest.cDevices.ControlShift("Z");
	cTest.cDevices.Control("A");
	cTest.cDevices.Control("W");
	cTest.cDevices.Control("D");
	cTest.cDevices.Tap("Cursor Up");
	cTest.cDevices.Tap("Enter");
	cTest.cDevices.Tap("Tab");
	cTest.cDevices.Tap("Escape");
	cTest.cDevices.Tap("Space");
	cTest.cDevices.Tap("F7");
	cTest.cDevices.Control("F4");
	cTest.cDevices.Tap("F4");
	cTest.cDevices.Tap("A");
	AssertLog("Copy;Copy;Cut;Cut;Undo;Redo;SelectAll;ExtendSelection;Duplicate;Up;Enter;Tab;Escape;Space;NextDifference;CloseDocument;JumpToSource;a;");
	AssertTrue(cTest.cInput.IsInsert());
	cTest.cDevices.Tap("Insert");
	AssertFalse(cTest.cInput.IsInsert());
	cTest.cDevices.Tap("Insert");

	cTest.FocusField();
	cTest.cDevices.Control("C");
	cTest.cDevices.Tap("Cursor Up");
	cTest.cDevices.Tap("Enter");
	cTest.cDevices.Tap("F7");
	AssertLog("Copy;Up;Enter;NextDifference;");

	//Copy moves in the editor and nowhere else; a menu shows the new keys.
	pCopy->ClearKeys();
	AssertTrue(pCopy->AddKeys("Control+Q"));
	AssertTrue(pCommands->BindKeys(&cTest.cInput));
	szKeys.Init();
	AssertTrue(pCopy->AppendKeysText(&szKeys, &cTest.cInput));
	AssertString("Ctrl+Q", szKeys.Text());
	szKeys.Kill();
	cTest.cDevices.Control("C");
	AssertLog("Copy;");
	cTest.cDevices.Control("Q");
	AssertLog("");
	cTest.FocusEditor();
	cTest.cDevices.Control("C");
	AssertLog("");
	cTest.cDevices.Control("Q");
	AssertLog("Copy;");

	//With no keys a menu shows none, rather than the standard ones.
	pCopy->ClearKeys();
	AssertTrue(pCommands->BindKeys(&cTest.cInput));
	szKeys.Init();
	AssertFalse(pCopy->AppendKeysText(&szKeys, &cTest.cInput));
	AssertString("", szKeys.Text());
	szKeys.Kill();

	//Run from a menu or a script, a made command gives the target its standard command.
	AssertTrue(pCommands->Run("Editor.Duplicate"));
	AssertLog("Duplicate;");

	cTest.pWindow->SetCommands(NULL);
	pCommands = NULL;
	pEdit = NULL;
	pCopy = NULL;
	cTest.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Without AddKeymapCommands the Edit commands are as they always were:
// the standard keys, shown but not bound.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapEditCommandsStandard(void)
{
	CChars				szLog;
	CChars				szKeys;
	CKeymapTestWindow	cTest;
	Ptr<CCommandSet>	pCommands;
	Ptr<CEditCommands>	pEdit;

	szLog.Init();
	cTest.Init(&szLog);
	pCommands = OMalloc<CCommandSet>();
	cTest.pWindow->SetCommands(pCommands);
	pEdit = OMalloc<CEditCommands>(cTest.pWindow, pCommands);
	AssertTrue(pCommands->BindKeys(&cTest.cInput));

	AssertSize(0, pCommands->Find("Edit.Copy")->NumKeys());
	AssertFalse(cTest.cInput.IsBound("Control+C"));
	szKeys.Init();
	AssertTrue(pCommands->Find("Edit.Copy")->AppendKeysText(&szKeys, &cTest.cInput));
	AssertString("Ctrl+C", szKeys.Text());
	szKeys.Kill();

	cTest.FocusEditor();
	cTest.cDevices.Control("C");
	AssertLog("Copy;");

	cTest.pWindow->SetCommands(NULL);
	pCommands = NULL;
	pEdit = NULL;
	cTest.Kill();
	szLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A user's keys go to a file and come back: only those that differ, and
// a command with none is kept as having none.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapFile(void)
{
	CChars				szLog;
	CKeymapTestCommands	cRun;
	Ptr<CCommandSet>	pCommands;
	Ptr<CCommand>		pFirst;
	Ptr<CCommand>		pSecond;
	Ptr<CCommand>		pThird;
	CFileUtil			cFileUtil;
	CTextFile			cFile;
	CChars				szText;

	szLog.Init();
	cRun.mpszLog = &szLog;
	cFileUtil.RemoveDir(TEST_KEYMAP_DIRECTORY);

	pCommands = OMalloc<CCommandSet>();
	ORoot()->Add(pCommands);
	pFirst = pCommands->Add("Debug.First", "First", "F5", Ptr<CImageCel>(), &cRun, &CKeymapTestCommands::Run);
	pSecond = pCommands->Add("Debug.Second", "Second", "Control+Pause/Break", Ptr<CImageCel>(), &cRun, &CKeymapTestCommands::Run);
	pThird = pCommands->Add("Debug.Third", "Third", "Control+Q", Ptr<CImageCel>(), &cRun, &CKeymapTestCommands::Run);

	//It isn't there yet, and everything keeps what it was made with.
	AssertFalse(pCommands->ReadKeys(TEST_KEYMAP_FILE));
	AssertTrue(pFirst->HasDefaultKeys());

	//Nothing changed.
	AssertTrue(pCommands->WriteKeys(TEST_KEYMAP_FILE));
	cFile.Init();
	AssertTrue(cFile.Read(TEST_KEYMAP_FILE));
	AssertTrue(strstr(cFile.Text(), "<Keymap") != NULL);
	AssertTrue(strstr(cFile.Text(), "Command") == NULL);
	cFile.Kill();
	AssertTrue(pCommands->ReadKeys(TEST_KEYMAP_FILE));
	AssertTrue(pFirst->HasDefaultKeys());

	//Several keys, a name with a slash, and none.
	pFirst->ClearKeys();
	AssertTrue(pFirst->AddKeys("Control+Shift+Pause/Break"));
	AssertTrue(pFirst->AddKeys("F6"));
	pSecond->ClearKeys();
	AssertTrue(pCommands->WriteKeys(TEST_KEYMAP_FILE));

	cFile.Init();
	AssertTrue(cFile.Read(TEST_KEYMAP_FILE));
	AssertTrue(strstr(cFile.Text(), "Debug.First") != NULL);
	AssertTrue(strstr(cFile.Text(), "Debug.Second") != NULL);
	AssertTrue(strstr(cFile.Text(), "Debug.Third") == NULL);
	cFile.Kill();

	pCommands->ResetKeys();
	AssertTrue(pFirst->HasDefaultKeys());
	AssertTrue(pCommands->ReadKeys(TEST_KEYMAP_FILE));
	AssertSize(2, pFirst->NumKeys());
	AssertString("Control+Shift+Pause/Break", pFirst->GetKeys(0));
	AssertString("F6", pFirst->GetKeys(1));
	AssertSize(0, pSecond->NumKeys());
	AssertTrue(pThird->HasDefaultKeys());

	//Reading replaces what's there.
	pThird->ClearKeys();
	AssertTrue(pCommands->ReadKeys(TEST_KEYMAP_FILE));
	AssertTrue(pThird->HasDefaultKeys());

	//Going back to the defaults leaves nothing to keep.
	pCommands->ResetKeys();
	AssertTrue(pCommands->WriteKeys(TEST_KEYMAP_FILE));
	cFile.Init();
	AssertTrue(cFile.Read(TEST_KEYMAP_FILE));
	AssertTrue(strstr(cFile.Text(), "Debug.") == NULL);
	cFile.Kill();

	//What a user gave a command is taken from one that kept it; the first of two given it has it.  Commands that aren't
	//there and keys that can't be read are ignored.
	szText.Init("<Keymap>\n");
	szText.Append("  <Command Name=\"Nothing.Here\"><Keys>F1</Keys></Command>\n");
	szText.Append("  <Command Name=\"Debug.First\"><Keys>Control+Q</Keys><Keys>Control+F, H</Keys><Keys>F8</Keys></Command>\n");
	szText.Append("  <Command Name=\"Debug.Second\"><Keys>F8</Keys><Keys>F9</Keys></Command>\n");
	szText.Append("</Keymap>\n");
	cFile.Init(szText.Text());
	AssertTrue(cFile.Write(TEST_KEYMAP_FILE));
	cFile.Kill();
	szText.Kill();
	AssertTrue(pCommands->ReadKeys(TEST_KEYMAP_FILE));
	AssertSize(2, pFirst->NumKeys());
	AssertString("Control+Q", pFirst->GetKeys(0));
	AssertString("F8", pFirst->GetKeys(1));
	AssertSize(1, pSecond->NumKeys());
	AssertString("F9", pSecond->GetKeys(0));
	AssertSize(0, pThird->NumKeys());

	//A file that isn't a keymap: everything keeps what it was made with.
	cFile.Init("<NotAKeymap><Command Name=\"Debug.First\"><Keys>F1</Keys></Command></NotAKeymap>");
	AssertTrue(cFile.Write(TEST_KEYMAP_FILE));
	cFile.Kill();
	AssertFalse(pCommands->ReadKeys(TEST_KEYMAP_FILE));
	AssertTrue(pFirst->HasDefaultKeys());
	AssertTrue(pThird->HasDefaultKeys());

	ORoot()->Remove(pCommands);
	pCommands = NULL;
	pFirst = NULL;
	pSecond = NULL;
	pThird = NULL;
	szLog.Kill();
	cFileUtil.RemoveDir(TEST_KEYMAP_DIRECTORY);
}


//////////////////////////////////////////////////////////////////////////
//
// The field has every key and chord before it can do anything else, and
// waits through modifiers; escape and enter go on.
//
//////////////////////////////////////////////////////////////////////////
void TestKeymapCapture(void)
{
	CChars						szLog;
	CKeymapTestWindow			cTest;
	Ptr<CKeyCaptureField>		pField;
	Ptr<CCommandSet>			pCommands;
	CKeymapTestCommands			cRun;

	szLog.Init();
	cTest.Init(&szLog);
	cRun.mpszLog = &szLog;
	pCommands = OMalloc<CCommandSet>();
	cTest.pWindow->SetCommands(pCommands);
	pCommands->Add("Bound", "Bound", "F5", Ptr<CImageCel>(), &cRun, &CKeymapTestCommands::Run);
	pCommands->Add("BoundChord", "Bound Chord", "Control+D", Ptr<CImageCel>(), &cRun, &CKeymapTestCommands::Run);
	AssertTrue(pCommands->BindKeys(&cTest.cInput));
	pField = OMalloc<CKeyCaptureField>(cTest.pWindow, CJewelTheme::CreateDarkTheme(cTest.pWindow, GenerateMenuFont(cTest.pWindow)));
	AssertTrue(cTest.pGrid->AddComponent(pField, 0, 1, 2, 1, 1, 1));
	cTest.pWindow->Layout();
	cTest.pWindow->GetFocus()->SetFocus(pField);
	AssertString("", pField->GetKeys());

	//One key, not run, and shown as a menu shows it.
	cTest.cDevices.Tap("F5");
	AssertString("F5", pField->GetKeys());
	AssertString("F5", pField->GetText());
	AssertLog("");

	//A chord the keyboard knows, and one it doesn't.
	cTest.cDevices.Control("C");
	AssertString("Control+C", pField->GetKeys());
	AssertString("Ctrl+C", pField->GetText());
	cTest.cDevices.Control("D");
	AssertString("Control+D", pField->GetKeys());
	AssertLog("");
	cTest.cDevices.Control("Q");
	AssertString("Control+Q", pField->GetKeys());
	cTest.cDevices.Press("Right Control");
	cTest.cDevices.Press("Left Alt");
	cTest.cDevices.Press("Left Shift");
	cTest.cDevices.Tap("Q");
	cTest.cDevices.Release("Left Shift");
	cTest.cDevices.Release("Left Alt");
	cTest.cDevices.Release("Right Control");
	AssertString("Control+Alt+Shift+Q", pField->GetKeys());
	cTest.cDevices.Shift("Q");
	AssertString("Shift+Q", pField->GetKeys());

	//What would type or move in a field is only keys.
	cTest.cDevices.Tap("Q");
	AssertString("Q", pField->GetKeys());
	cTest.cDevices.Tap("Tab");
	AssertString("Tab", pField->GetKeys());
	cTest.cDevices.Tap("Cursor Up");
	AssertString("Cursor Up", pField->GetKeys());
	AssertString("Up", pField->GetText());

	//Modifiers alone change nothing.
	cTest.cDevices.Tap("Left Control");
	cTest.cDevices.Tap("Left Shift");
	cTest.cDevices.Tap("Right Alt");
	AssertString("Cursor Up", pField->GetKeys());

	//Plain escape and enter aren't captured; with a modifier they are.
	cTest.cDevices.Tap("Escape");
	cTest.cDevices.Tap("Enter");
	AssertString("Cursor Up", pField->GetKeys());
	cTest.cDevices.Control("Enter");
	AssertString("Control+Enter", pField->GetKeys());

	//Cleared.
	pField->SetKeys("");
	AssertString("", pField->GetKeys());
	AssertString("", pField->GetText());

	cTest.pWindow->SetCommands(NULL);
	pCommands = NULL;
	pField = NULL;
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
	TestKeymapContexts();
	TestKeymapContextFocus();
	TestKeymapEditorKeys();
	TestKeymapInsert();
	TestKeymapDialog();
	TestKeymapCommands();
	TestKeymapWindowCommands();
	TestKeymapEditCommands();
	TestKeymapEditCommandsStandard();
	TestKeymapFile();
	TestKeymapCapture();
	TestKeymapKeyQueries();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
