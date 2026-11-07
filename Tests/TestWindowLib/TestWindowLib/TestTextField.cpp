#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "StandardLib/ClassDefines.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/TextField.h"
#include "WindowLib/EditCommands.h"
#include "WindowLib/MenuContainer.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestLib/AssertFile.h"
#include "ScriptFramesTick.h"


void			ValidateLabelObjects(void);
Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);
SInt32Vec2		ComponentCentre(Ptr<CComponent> pComponent);
void			MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition);
void			Click(Ptr<CWindow> pWindow);


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//Edit commands that act on a field of the test's choosing while the keys haven't gone to one.
class CTestEditCommands : public CEditCommands
{
CONSTRUCTABLE(CTestEditCommands);
DESTRUCTABLE(CTestEditCommands);
public:
	Ptr<CComponent>		mpDefault;

	void Init(Ptr<CWindow> pWindow, Ptr<CCommandSet> pCommands)
	{
		PreInit();
		CEditCommands::Init(pWindow, pCommands);
		mpDefault = NULL;
		PostInit();
	}

	void Class(void) override
	{
		CEditCommands::Class();
		M_Pointer(mpDefault);
	}

	Ptr<CComponent> GetDefaultTarget(void) override { return mpDefault; }
};


//////////////////////////////////////////////////////////////////////////
//
// Logs "Changed:text;".
//
//////////////////////////////////////////////////////////////////////////
class CTestTextFieldListener
{
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void TextChanged(CTextField* pcField)
	{
		mpszLog->Append("Changed:");
		mpszLog->Append(pcField->GetText());
		mpszLog->Append(";");
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A text field at the top left of a dark window, its default width.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTextField> CreateTestTextField(Ptr<CWindow> pWindow, const char* szText)
{
	Ptr<CFillContainer>		pFill;
	Ptr<CTextField>			pField;
	SInsets					sInsets;
	int						aiInsets[4] = { 10, 10, 10, 10 };

	pFill = OMalloc<CFillContainer>(pWindow);
	sInsets.Init(aiInsets);
	pFill->SetInsets(&sInsets);
	pWindow->SetContainer(pFill);
	pField = OMalloc<CTextField>(pWindow, CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow)), szText);
	pField->SetAlignment(CSH_Left, CSV_Top);
	pFill->AddComponent(pField);
	return pField;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextFieldEditing(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTextField>			pField;
		Ptr<CFocus>				pFocus;
		Ptr<CWindow>			pOtherWindow;
		CTestTextFieldListener	cListener;
		CPointer				pNull;
		CChars					szLog;
		CChars					szClipboard;
		size					uiStart;
		size					uiEnd;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 300, 100, NULL);
		pTestWindow = OMalloc<CWindow>("Text Field Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		pField = CreateTestTextField(pTestWindow, "");
		pField->GetChangeEvent()->Add(&cListener, &CTestTextFieldListener::TextChanged);
		pTestWindow->Layout();

		//The pointer over it lightens its border, pressing it gives it focus and its focus ring.
		AssertInt(BS_Normal, pField->GetBorderState());
		MoveTo(pTestWindow, ComponentCentre(pField));
		AssertInt(BS_MouseOver, pField->GetBorderState());
		Click(pTestWindow);
		AssertTrue(pFocus->GetFocussedComponent() == pField);
		AssertInt(BS_Focussed, pField->GetBorderState());

		//Typing inserts at the caret.
		pTestWindow->InputPrintable('a', true);
		pTestWindow->InputPrintable('b', true);
		pTestWindow->InputCommand(CIC_Space, true);
		pTestWindow->InputPrintable('c', true);
		AssertString("ab c", pField->GetText());
		AssertSize(4, pField->GetCaretPosition());
		AssertLog("Changed:a;Changed:ab;Changed:ab ;Changed:ab c;");

		//Moving doesn't change the text.
		pTestWindow->InputCommand(CIC_Left, true);
		pTestWindow->InputCommand(CIC_Left, true);
		AssertSize(2, pField->GetCaretPosition());
		AssertLog("");

		//Nothing has been copied yet.
		szClipboard.Init();
		AssertFalse(pTestWindow->GetClipboard()->GetText(&szClipboard));
		AssertString("", szClipboard.Text());
		szClipboard.Kill();

		//Selecting to the start then copying puts the selection on the clipboard.
		pTestWindow->InputCommand(CIC_SelectHome, true);
		AssertTrue(pField->GetSelection(&uiStart, &uiEnd));
		AssertSize(0, uiStart);
		AssertSize(2, uiEnd);
		pTestWindow->InputCommand(CIC_Copy, true);
		szClipboard.Init();
		AssertTrue(pTestWindow->GetClipboard()->GetText(&szClipboard));
		AssertString("ab", szClipboard.Text());
		szClipboard.Kill();

		//Every window's clipboard is the same one, as a system's is.
		pOtherWindow = OMalloc<CWindow>("Other Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pOtherWindow);
		szClipboard.Init();
		AssertTrue(pOtherWindow->GetClipboard()->GetText(&szClipboard));
		AssertString("ab", szClipboard.Text());
		szClipboard.Kill();
		ORoot()->Remove(pOtherWindow);
		pOtherWindow = NULL;

		//Pasting at the end.
		pTestWindow->InputCommand(CIC_End, true);
		AssertFalse(pField->GetSelection(&uiStart, &uiEnd));
		pTestWindow->InputCommand(CIC_Paste, true);
		AssertString("ab cab", pField->GetText());
		AssertLog("Changed:ab cab;");

		//Typing over a selection replaces it.
		pTestWindow->InputCommand(CIC_SelectWordLeft, true);
		pTestWindow->InputPrintable('X', true);
		AssertString("ab X", pField->GetText());
		AssertLog("Changed:ab X;");

		//Backspace and delete.
		pTestWindow->InputCommand(CIC_Backspace, true);
		AssertString("ab ", pField->GetText());
		pTestWindow->InputCommand(CIC_Home, true);
		pTestWindow->InputCommand(CIC_Delete, true);
		AssertString("b ", pField->GetText());
		AssertLog("Changed:ab ;Changed:b ;");

		//Cutting everything empties it.
		pTestWindow->InputCommand(CIC_SelectAll, true);
		pTestWindow->InputCommand(CIC_Cut, true);
		AssertString("", pField->GetText());
		AssertLog("Changed:;");
		szClipboard.Init();
		AssertTrue(pTestWindow->GetClipboard()->GetText(&szClipboard));
		AssertString("b ", szClipboard.Text());
		szClipboard.Kill();

		//SetText puts the caret at the end, line feeds become spaces.
		pField->SetText("one\ntwo");
		AssertString("one two", pField->GetText());
		AssertSize(7, pField->GetCaretPosition());
		AssertLog("Changed:one two;");

		//Losing focus loses the ring, the pointer is still over it.
		pFocus->SetFocus(NULL);
		AssertInt(BS_MouseOver, pField->GetBorderState());
		MoveTo(pTestWindow, SInt32Vec2(290, 90));
		AssertInt(BS_Normal, pField->GetBorderState());

		ValidateLabelObjects();

		AssertTrue(pField->GetChangeEvent()->Remove(&cListener));
		pField = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Undo and redo as IntelliJ's: typing is one edit up to a space after a
// word or a pause, the caret and selection go back to how they were.
//
//////////////////////////////////////////////////////////////////////////
void TestTextFieldUndo(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTextField>			pField;
		CTestTextFieldListener	cListener;
		CPointer				pNull;
		CChars					szLog;
		size					uiStart;
		size					uiEnd;

		szLog.Init();
		cListener.Init(&szLog);
		cNativeFactory.Init(&gcMemoryAllocator, 300, 100, NULL);
		pTestWindow = OMalloc<CWindow>("Text Field Undo Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pField = CreateTestTextField(pTestWindow, "");
		pField->SetTypingPause(1000000);
		pTestWindow->Layout();
		MoveTo(pTestWindow, ComponentCentre(pField));
		Click(pTestWindow);

		//Nothing to undo yet.
		AssertFalse(pField->CanUndo());
		AssertFalse(pField->CanRedo());
		AssertFalse(pField->Undo());

		//"ab" is one edit, the space after it starts the next.
		pTestWindow->InputPrintable('a', true);
		pTestWindow->InputPrintable('b', true);
		pTestWindow->InputCommand(CIC_Space, true);
		pTestWindow->InputPrintable('c', true);
		AssertString("ab c", pField->GetText());
		AssertTrue(pField->CanUndo());
		pField->GetChangeEvent()->Add(&cListener, &CTestTextFieldListener::TextChanged);

		pTestWindow->InputCommand(CIC_Undo, true);
		AssertString("ab", pField->GetText());
		AssertSize(2, pField->GetCaretPosition());
		AssertLog("Changed:ab;");
		pTestWindow->InputCommand(CIC_Undo, true);
		AssertString("", pField->GetText());
		AssertSize(0, pField->GetCaretPosition());
		AssertFalse(pField->CanUndo());
		AssertTrue(pField->CanRedo());
		pTestWindow->InputCommand(CIC_Undo, true);
		AssertString("", pField->GetText());
		AssertLog("Changed:;");

		//Control Shift Z redoes.
		pTestWindow->InputCommand(CIC_Redo, true);
		AssertString("ab", pField->GetText());
		AssertSize(2, pField->GetCaretPosition());
		pTestWindow->InputCommand(CIC_Redo, true);
		AssertString("ab c", pField->GetText());
		AssertSize(4, pField->GetCaretPosition());
		AssertFalse(pField->CanRedo());
		AssertLog("Changed:ab;Changed:ab c;");

		//Deleting a selection and undoing it gives the selection back.
		pField->Select(1, 2);
		pTestWindow->InputCommand(CIC_Backspace, true);
		AssertString("a c", pField->GetText());
		AssertFalse(pField->GetSelection(&uiStart, &uiEnd));
		pTestWindow->InputCommand(CIC_Undo, true);
		AssertString("ab c", pField->GetText());
		AssertTrue(pField->GetSelection(&uiStart, &uiEnd));
		AssertSize(1, uiStart);
		AssertSize(2, uiEnd);
		AssertSize(2, pField->GetCaretPosition());
		pTestWindow->InputCommand(CIC_Redo, true);
		AssertString("a c", pField->GetText());
		AssertFalse(pField->GetSelection(&uiStart, &uiEnd));
		AssertSize(1, pField->GetCaretPosition());
		AssertLog("Changed:a c;Changed:ab c;Changed:a c;");

		//An edit after an undo forgets what could be redone.
		pTestWindow->InputCommand(CIC_Undo, true);
		pTestWindow->InputCommand(CIC_End, true);
		pTestWindow->InputPrintable('d', true);
		AssertString("ab cd", pField->GetText());
		AssertFalse(pField->CanRedo());
		AssertLog("Changed:ab c;Changed:ab cd;");

		//Setting the text is an edit: the old text comes back with undo.
		pField->SetText("hello");
		pTestWindow->InputCommand(CIC_Undo, true);
		AssertString("ab cd", pField->GetText());
		AssertSize(5, pField->GetCaretPosition());
		AssertLog("Changed:hello;Changed:ab cd;");

		//Typing after a pause is a new edit.
		pField->SetTypingPause(0);
		pTestWindow->InputPrintable('e', true);
		pTestWindow->InputPrintable('f', true);
		pTestWindow->InputCommand(CIC_Undo, true);
		AssertString("ab cde", pField->GetText());
		AssertLog("Changed:ab cde;Changed:ab cdef;Changed:ab cde;");

		//Clearing the history keeps the text.
		pField->ClearUndo();
		AssertString("ab cde", pField->GetText());
		AssertFalse(pField->CanUndo());
		AssertFalse(pField->CanRedo());

		AssertTrue(pField->GetChangeEvent()->Remove(&cListener));
		pField = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		szLog.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Pressing puts the caret in the nearest gap, dragging selects and the
// text scrolls to keep the caret in view.
//
//////////////////////////////////////////////////////////////////////////
void TestTextFieldPointer(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTextField>			pField;
		CPointer				pNull;
		CInt32Rect				cCaret;
		size					uiStart;
		size					uiEnd;
		int32					iTextLeft;
		int32					iY;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 100, NULL);
		pTestWindow = OMalloc<CWindow>("Text Field Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pField = CreateTestTextField(pTestWindow, "Hello World");
		pTestWindow->Layout();

		//The text starts after the border and the padding.
		iTextLeft = pField->GetTextLabel()->GetPosition().x;
		AssertInt(10 + 8 + 6, iTextLeft);
		iY = ComponentCentre(pField).y;

		//Just right of the gap after 'Hello' puts the caret there.
		MoveTo(pTestWindow, SInt32Vec2(iTextLeft + pField->GetCharacterX(5) + 1, iY));
		Click(pTestWindow);
		AssertSize(5, pField->GetCaretPosition());
		AssertFalse(pField->GetSelection(&uiStart, &uiEnd));

		//The caret is drawn there once the field is laid out.
		pTestWindow->Layout();
		AssertTrue(pField->GetMarksDraw()->IsCaretShown());
		pField->GetMarksDraw()->GetCaret(&cCaret);
		AssertInt(pField->GetCharacterX(5), cCaret.GetLeft());

		//Dragging from the start to the gap after 'World' selects all but the first gap's text.
		MoveTo(pTestWindow, SInt32Vec2(iTextLeft + 1, iY));
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		MoveTo(pTestWindow, SInt32Vec2(iTextLeft + pField->GetCharacterX(11) + 5, iY));
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		AssertTrue(pField->GetSelection(&uiStart, &uiEnd));
		AssertSize(0, uiStart);
		AssertSize(11, uiEnd);
		pTestWindow->Layout();
		AssertTrue(pField->GetMarksDraw()->IsSelectionShown());

		//Editing during a drag, deleting the selection, leaves the drag's start past the end of the text.  The selection
		//stays inside the text.
		pTestWindow->InputCommand(CIC_Backspace, true);
		AssertString("", pField->GetText());
		MoveTo(pTestWindow, SInt32Vec2(iTextLeft + 30, iY));
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		pField->SetText("Hello World");
		pField->Select(11, 6);
		pTestWindow->InputCommand(CIC_Backspace, true);
		AssertString("Hello ", pField->GetText());
		MoveTo(pTestWindow, SInt32Vec2(iTextLeft + 1, iY));
		AssertTrue(pField->GetSelection(&uiStart, &uiEnd));
		AssertSize(0, uiStart);
		AssertSize(6, uiEnd);
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);

		//Cutting without a selection cuts nothing, pasting an empty clipboard over a selection leaves it.
		pField->SetText("Hello");
		pTestWindow->InputCommand(CIC_Cut, true);
		AssertString("Hello", pField->GetText());
		pTestWindow->InputCommand(CIC_SelectAll, true);
		pTestWindow->InputCommand(CIC_Paste, true);
		AssertString("Hello", pField->GetText());
		AssertTrue(pField->GetSelection(&uiStart, &uiEnd));

		//Text wider than the field scrolls so the caret is in view.
		pField->SetText("The quick brown fox jumps over the lazy dog and keeps on running");
		pTestWindow->Layout();
		AssertTrue(pField->GetScroll() > 0);
		AssertInt(iTextLeft - pField->GetScroll(), pField->GetTextLabel()->GetPosition().x);
		pTestWindow->InputCommand(CIC_Home, true);
		pTestWindow->Layout();
		AssertInt(0, pField->GetScroll());

		ValidateLabelObjects();

		pField = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
}


struct STextFieldFrames
{
	Ptr<CTextField>	pField;
};


//////////////////////////////////////////////////////////////////////////
//
// Frame 0: not focussed.  1: focussed with the caret at the end.  2: some
// of the text selected.
//
//////////////////////////////////////////////////////////////////////////
bool TextFieldFrames(Ptr<CWindow> pWindow, int32 iFrame, void* pvContext)
{
	Ptr<CTextField>	pField;

	pField = ((STextFieldFrames*)pvContext)->pField;
	if (iFrame == 0)
	{
		MoveTo(pWindow, ComponentCentre(pField));
		Click(pWindow);
		pWindow->InputCommand(CIC_End, true);
		MoveTo(pWindow, SInt32Vec2(280, 80));
		return true;
	}
	else if (iFrame == 1)
	{
		pWindow->InputCommand(CIC_SelectWordLeft, true);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextFieldFrames(void)
{
	CWinRefWindowFactory	cNativeFactory;
	CFileUtil				cFileUtil;
	char					szDirectory[] = "Output" _FS_ "TextFieldFrames";

	cFileUtil.RemoveDir(szDirectory);
	cFileUtil.MakeDir(szDirectory);

	{
		Ptr<CWindow>				pTestWindow;
		Ptr<CScriptFramesTick>		pTick;
		STextFieldFrames			sFrames;
		CChars						szExpectedDirectory;

		cNativeFactory.Init(&gcMemoryAllocator, 240, 60, szDirectory);
		pTick = OMalloc<CScriptFramesTick>(TextFieldFrames, &sFrames);
		pTestWindow = OMalloc<CWindow>("Text Field Frames", &cNativeFactory, pTick, OMalloc<CClearCanvasDraw>(Set32BitColour((uint8)0x2B, 0x2D, 0x30)), CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		sFrames.pField = CreateTestTextField(pTestWindow, "Clock frequency");

		pTestWindow->Show();

		ValidateLabelObjects();

		sFrames.pField = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
		pTick = NULL;

		szExpectedDirectory.Init(szDirectory);
		szExpectedDirectory.Replace("Output", "Input");
		AssertDirectory(szExpectedDirectory.Text(), szDirectory);
		szExpectedDirectory.Kill();
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	AssertSize(2, gcObjects.NumMemoryIndexes());
	cNativeFactory.Kill();
	cFileUtil.RemoveDir(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// The Edit commands act on the field the keys last went to, even while a
// menu holds the keys, and can only run while they would do something.
//
//////////////////////////////////////////////////////////////////////////
void TestTextFieldEditCommands(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTextField>			pField;
		Ptr<CCommandSet>		pCommands;
		Ptr<CEditCommands>		pEdit;
		Ptr<CTestEditCommands>	pDefaulted;
		Ptr<CMenuContainer>		pMenu;
		CPointer				pNull;
		size					uiStart;
		size					uiEnd;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 100, NULL);
		pTestWindow = OMalloc<CWindow>("Text Field Edit Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pField = CreateTestTextField(pTestWindow, "");
		pField->SetTypingPause(1000000);
		pCommands = OMalloc<CCommandSet>();
		pEdit = OMalloc<CEditCommands>(pTestWindow, pCommands);
		pTestWindow->SetCommands(pCommands);
		AssertSize(6, pCommands->NumCommands());
		AssertInt(CIC_Copy, pCommands->Find("Edit.Copy")->GetStandardCommand());
		pTestWindow->Layout();

		//Nothing has had the keys yet.
		AssertTrue(pEdit->GetTarget().IsNull());
		AssertFalse(pCommands->Find("Edit.Copy")->IsEnabled());

		//Until something has had the keys, the commands act on the default target, if any.
		pDefaulted = OMalloc<CTestEditCommands>(pTestWindow, OMalloc<CCommandSet>());
		AssertTrue(pDefaulted->GetTarget().IsNull());
		pDefaulted->mpDefault = pField;
		AssertTrue(pDefaulted->GetTarget() == pField);
		AssertTrue(pDefaulted->CanCopy());
		MoveTo(pTestWindow, ComponentCentre(pField));
		Click(pTestWindow);
		AssertTrue(pEdit->GetTarget() == pField);
		AssertTrue(pCommands->Find("Edit.Copy")->IsEnabled());
		AssertTrue(pCommands->Find("Edit.Paste")->IsEnabled());
		AssertFalse(pCommands->Find("Edit.Undo")->IsEnabled());

		pTestWindow->InputPrintable('a', true);
		pTestWindow->InputPrintable('b', true);
		AssertTrue(pCommands->Find("Edit.Undo")->IsEnabled());

		//A menu taking the keys doesn't change what the commands act on, and running one gives the field the keys back.
		pMenu = OMalloc<CMenuContainer>(pTestWindow, CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow)));
		pMenu->AddItem("Item");
		pMenu->Open(SInt32Vec2(10, 10), NULL, true);
		AssertFalse(pField->IsFocussed());
		AssertTrue(pEdit->GetTarget() == pField);
		pMenu->Close();
		AssertTrue(pCommands->Run("Edit.SelectAll"));
		AssertTrue(pField->IsFocussed());
		AssertTrue(pField->GetSelection(&uiStart, &uiEnd));
		AssertSize(0, uiStart);
		AssertSize(2, uiEnd);
		AssertTrue(pCommands->Run("Edit.Undo"));
		AssertString("", pField->GetText());
		AssertFalse(pCommands->Find("Edit.Undo")->IsEnabled());
		AssertTrue(pCommands->Find("Edit.Redo")->IsEnabled());
		AssertTrue(pCommands->Run("Edit.Redo"));
		AssertString("ab", pField->GetText());

		pMenu = NULL;
		pDefaulted = NULL;
		pTestWindow->SetCommands(NULL);
		pEdit = NULL;
		pCommands = NULL;
		pField = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A field edits as a text area does (CTextEditing): home is smart, the
// word deletes take a selection, an edit that changes nothing isn't one,
// and line feeds don't go in.
//
//////////////////////////////////////////////////////////////////////////
void TestTextFieldSharedEditing(void)
{
	CWinRefWindowFactory	cNativeFactory;

	{
		Ptr<CWindow>	pTestWindow;
		Ptr<CTextField>	pField;
		CPointer		pNull;

		cNativeFactory.Init(&gcMemoryAllocator, 300, 100, NULL);
		pTestWindow = OMalloc<CWindow>("Text Field Shared Window", &cNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pField = CreateTestTextField(pTestWindow, "  ab cd");
		pField->SetTypingPause(1000000);
		pTestWindow->Layout();
		MoveTo(pTestWindow, ComponentCentre(pField));
		Click(pTestWindow);
		AssertTrue(pField->IsFocussed());

		//Home goes to the first character that isn't a space, then the start, and back.
		pTestWindow->InputCommand(CIC_End, true);
		pTestWindow->InputCommand(CIC_Home, true);
		AssertSize(2, pField->GetCaretPosition());
		pTestWindow->InputCommand(CIC_Home, true);
		AssertSize(0, pField->GetCaretPosition());
		pTestWindow->InputCommand(CIC_Home, true);
		AssertSize(2, pField->GetCaretPosition());

		//A word delete takes the selection.
		pField->Select(3, 5);
		pTestWindow->InputCommand(CIC_DeleteWordRemainingLeft, true);
		AssertString("  acd", pField->GetText());

		//Backspace at the start changes nothing, so undo undoes the delete.
		pField->SetCaretPosition(0);
		pTestWindow->InputCommand(CIC_Backspace, true);
		pTestWindow->InputCommand(CIC_Undo, true);
		AssertString("  ab cd", pField->GetText());

		//Enter goes on to the window, and line breaks set or pasted are spaces.
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertString("  ab cd", pField->GetText());
		pField->SetText("x\ny");
		AssertString("x y", pField->GetText());
		AssertSize(3, pField->GetCaretPosition());
		pField->Paste((char*)"\r\nz");
		AssertString("x y z", pField->GetText());

		pField = NULL;
		ORoot()->Remove(pTestWindow);
		pTestWindow = NULL;
	}

	AssertSize(0, gcObjects.NumFreeObjects());
	AssertTrue(ORoot()->RemoveAll());
	cNativeFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextField(void)
{
	BeginTests();
	ObjectsInit();

	TestTextFieldEditing();
	TestTextFieldUndo();
	TestTextFieldEditCommands();
	TestTextFieldSharedEditing();
	TestTextFieldPointer();
	TestTextFieldFrames();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

