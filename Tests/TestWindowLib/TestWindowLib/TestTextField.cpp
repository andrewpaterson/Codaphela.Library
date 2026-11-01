#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/TextField.h"
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


//////////////////////////////////////////////////////////////////////////
//
// Logs "Changed:text;".
//
//////////////////////////////////////////////////////////////////////////
class CTestTextFieldListener : public CTextFieldListener
{
CONSTRUCTABLE(CTestTextFieldListener);
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void TextChanged(CUnknown* pcSource, void* pvContext) override
	{
		mpszLog->Append("Changed:");
		mpszLog->Append(((CTextField*)pcSource)->GetText());
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
		AssertTrue(pField->AddListener(&cListener));
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

		//Selecting to the start then copying puts the selection on the clipboard.
		pTestWindow->InputCommand(CIC_SelectHome, true);
		AssertTrue(pField->GetSelection(&uiStart, &uiEnd));
		AssertSize(0, uiStart);
		AssertSize(2, uiEnd);
		pTestWindow->InputCommand(CIC_Copy, true);
		szClipboard.Init();
		cNativeFactory.GetClipboardText(&szClipboard);
		AssertString("ab", szClipboard.Text());
		szClipboard.Kill();

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
		cNativeFactory.GetClipboardText(&szClipboard);
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

		pField->RemoveListener(&cListener);
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
//
//////////////////////////////////////////////////////////////////////////
void TestTextField(void)
{
	BeginTests();
	ObjectsInit();

	TestTextFieldEditing();
	TestTextFieldPointer();
	TestTextFieldFrames();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

