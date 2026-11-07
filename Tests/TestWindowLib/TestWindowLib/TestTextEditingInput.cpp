#include "BaseLib/GlobalMemory.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/Canvas.h"
#include "WindowLib/TextEditingInput.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/Assert.h"
#include "BoxFillCanvasDraw.h"
#include "TickTestRefWindow.h"
#include "DataTestRefWindow.h"


//A view with a row for each line of the text, a y for each row and a column of x for each character.
class CTestEditingInputView : public CTextEditingView
{
public:
	CTextDocument*	mpcDocument;

	bool	HasRows(void) override { return true; }
	size	NumRows(void) override { return mpcDocument->NumLines(); }
	size	FindRow(size uiPosition, bool bRowEnd) override { return mpcDocument->FindLine(uiPosition); }
	size	GetRowStart(size uiRow) override { return mpcDocument->GetLineStart(uiRow); }
	size	FindRowAt(int32 y) override { return ((y >= 0) && ((size)y < NumRows())) ? (size)y : ((y < 0) ? 0 : NumRows() - 1); }
	int32	GetPositionX(size uiPosition, bool bRowEnd) override { return (int32)(uiPosition - mpcDocument->GetNavigation()->FindLineStart(uiPosition)); }
	size	ScrollPage(size uiRow, bool bDown) override { return uiRow; }
	void	MarksChanged(void) override {}
	void	ShowCaret(void) override {}
	void	TextEdited(void) override {}
	void	UndoChanged(STextUndoChange* psChange) override {}

	size	GetRowEnd(size uiRow, bool* pbRowEnd) override
	{
		*pbRowEnd = false;
		return mpcDocument->GetNavigation()->FindLineEnd(mpcDocument->GetLineStart(uiRow));
	}

	size	FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd) override
	{
		*pbRowEnd = false;
		return mpcDocument->GetNavigation()->FindColumn(mpcDocument->GetLineStart(uiRow), (size)x);
	}
};


//A text edited through the input actions of a canvas in a window.
struct STestEditingInput
{
	CWinRefWindowFactory	cFactory;
	SDataTestRefWindow		cData;
	Ptr<CWindow>			pWindow;
	Ptr<CTickTestRefWindow>	pTick;
	Ptr<CCanvas>			pCanvas;
	CTextDocument			cDocument;
	CTestEditingInputView	cView;
	CTextEditing			cEditing;
	CTextEditingInput		cInput;

	void	Init(const char* szText, bool bMultiLine)
	{
		CPointer	pNull;

		cFactory.Init(&gcMemoryAllocator, 64, 32, NULL);
		pTick = OMalloc<CTickTestRefWindow>(&cData, 1);
		pWindow = OMalloc<CWindow>("Editing Input", &cFactory, pTick, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pWindow);
		pCanvas = OMalloc<CCanvas>(pWindow, OMalloc<CBoxFillCanvasDraw>(Set32BitColour(0.0f, 0.0f, 0.0f)));
		pWindow->SetContainer(pCanvas);

		cDocument.Init(szText);
		cView.mpcDocument = &cDocument;
		cEditing.Init(&cDocument, &cView);
		cEditing.ClearUndo();
		cInput.Init(&pCanvas, &cEditing, bMultiLine);
	}

	void	Kill(void)
	{
		cInput.Kill();
		cEditing.Kill();
		cDocument.Kill();
		pCanvas = NULL;
		ORoot()->Remove(pWindow);
		pWindow = NULL;
		pTick = NULL;
		cFactory.Kill();
	}

	bool	Command(EComponentInputCommand eCommand, int x = 0, int y = 0)
	{
		SComponentInputEvent	sEvent;

		InitEvent(&sEvent, CIET_Command);
		sEvent.eCommand = eCommand;
		sEvent.x = x;
		sEvent.y = y;
		return pCanvas->CallInputActions(&sEvent);
	}

	bool	Type(char c)
	{
		SComponentInputEvent	sEvent;

		InitEvent(&sEvent, CIET_Printable);
		sEvent.cPrintable = c;
		return pCanvas->CallInputActions(&sEvent);
	}

	void	InitEvent(SComponentInputEvent* psEvent, EComponentInputEventType eType)
	{
		memset(psEvent, 0, sizeof(SComponentInputEvent));
		psEvent->eType = eType;
		psEvent->eCommand = CIC_Ignored;
		psEvent->bInsert = true;
	}

	char*	Text(void) { return cDocument.Text(); }
	size	Caret(void) { return cEditing.GetCaretPosition(); }
};


//////////////////////////////////////////////////////////////////////////
//
// A single line text takes typing, moving, selecting, deleting, the
// clipboard and undo, but not up and down, tabs, line feeds or the line
// commands: they go on to its parent.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingInputSingleLine(void)
{
	STestEditingInput	s;

	ObjectsInit();
	s.Init("ab", false);

	AssertTrue(s.Type('x'));
	AssertString("xab", s.Text());
	AssertTrue(s.Command(CIC_End));
	AssertSize(3, s.Caret());
	AssertTrue(s.Command(CIC_Space));
	AssertTrue(s.Command(CIC_Backspace));
	AssertTrue(s.Command(CIC_WordLeft));
	AssertSize(0, s.Caret());

	AssertFalse(s.Command(CIC_Enter));
	AssertFalse(s.Command(CIC_Tab));
	AssertFalse(s.Command(CIC_Up));
	AssertFalse(s.Command(CIC_PageDown));
	AssertFalse(s.Command(CIC_Duplicate));
	AssertFalse(s.Command(CIC_DeleteLine));
	AssertString("xab", s.Text());

	//Copied all, pasted at the end.
	AssertTrue(s.Command(CIC_SelectAll));
	AssertTrue(s.Command(CIC_Copy));
	AssertTrue(s.Command(CIC_DocumentEnd));
	AssertTrue(s.Command(CIC_Paste));
	AssertString("xabxab", s.Text());
	AssertTrue(s.Command(CIC_Undo));
	AssertString("xab", s.Text());
	AssertTrue(s.Command(CIC_Redo));
	AssertString("xabxab", s.Text());

	//Cut the selection to the clipboard.
	AssertTrue(s.Command(CIC_SelectWordLeft));
	AssertTrue(s.Command(CIC_Cut));
	AssertString("", s.Text());
	AssertTrue(s.Command(CIC_Paste));
	AssertString("xabxab", s.Text());

	s.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A multi line text takes up and down, tabs, line feeds and the line
// commands too.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingInputMultiLine(void)
{
	STestEditingInput	s;

	ObjectsInit();
	s.Init("ab\ncd", true);

	AssertTrue(s.Command(CIC_Down));
	AssertSize(3, s.Caret());
	AssertTrue(s.Command(CIC_Tab));
	AssertString("ab\n    cd", s.Text());
	AssertTrue(s.Command(CIC_BackTab));
	AssertString("ab\ncd", s.Text());
	AssertTrue(s.Command(CIC_Enter));
	AssertString("ab\n\ncd", s.Text());
	AssertTrue(s.Command(CIC_Left));
	AssertTrue(s.Command(CIC_DeleteLine));
	AssertString("ab\ncd", s.Text());
	AssertTrue(s.Command(CIC_Duplicate));
	AssertString("ab\ncd\ncd", s.Text());
	AssertTrue(s.Command(CIC_Up));
	AssertTrue(s.Command(CIC_SelectDown));
	AssertTrue(s.cEditing.IsSelected());

	s.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A press puts the caret nearest the pointer; a shift press and drag
// selects to it.  A right press in the selection keeps it, elsewhere it
// moves the caret; either way it goes on to the parent for its context
// menu.  With the pointer off its presses go on to the parent.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingInputPointer(void)
{
	STestEditingInput	s;
	size				uiStart;
	size				uiEnd;

	ObjectsInit();
	s.Init("abcd\nefgh", true);

	AssertTrue(s.Command(CIC_ActivateComponent, 2, 1));
	AssertSize(7, s.Caret());
	AssertTrue(s.Command(CIC_ExecuteComponent, 2, 1));

	AssertTrue(s.Command(CIC_StartDragSelect, 1, 0));
	AssertTrue(s.Command(CIC_ContDragSelect, 3, 0));
	AssertTrue(s.Command(CIC_StopDragSelect, 3, 0));
	AssertTrue(s.cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(3, uiStart);
	AssertSize(7, uiEnd);

	//Dragging has stopped: moving doesn't select.
	AssertTrue(s.Command(CIC_ContDragSelect, 0, 0));
	AssertSize(3, s.Caret());

	AssertFalse(s.Command(CIC_ContextMenu, 1, 1));
	AssertTrue(s.cEditing.IsSelected());
	AssertFalse(s.Command(CIC_ContextMenu, 0, 0));
	AssertFalse(s.cEditing.IsSelected());
	AssertSize(0, s.Caret());

	s.cInput.SetPointerEnabled(false);
	AssertFalse(s.Command(CIC_ActivateComponent, 2, 1));
	AssertFalse(s.Command(CIC_StartDragSelect, 2, 1));
	AssertSize(0, s.Caret());
	s.cInput.SetPointerEnabled(true);
	AssertTrue(s.Command(CIC_ActivateComponent, 2, 1));
	AssertSize(7, s.Caret());

	s.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//Counts the calls of an action that doesn't keep the layout.
struct STestEditingCounter
{
	int	iCalls;

	void	Count(void) { iCalls++; }
};


//////////////////////////////////////////////////////////////////////////
//
// The editing actions keep the layout: a command one takes only paints the
// window.  A command nothing takes lays it out, as does one also taken by
// an action that doesn't keep the layout.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingInputKeepsLayout(void)
{
	STestEditingInput	s;
	STestEditingCounter	sCounter;
	size				uiFirst;

	ObjectsInit();
	s.Init("ab\ncd", true);
	s.pCanvas->SetCanGetFocus(true);
	s.pWindow->GetFocus()->SetFocus(s.pCanvas);
	s.pWindow->Layout();
	AssertFalse(s.pWindow->IsLayoutDirty());

	s.pWindow->InputCommand(CIC_End, true);
	AssertSize(2, s.Caret());
	AssertFalse(s.pWindow->IsLayoutDirty());
	s.pWindow->InputCommand(CIC_Down, true);
	AssertSize(5, s.Caret());
	AssertFalse(s.pWindow->IsLayoutDirty());

	//The context menu's press goes on to the parent, which has no action for it.
	s.pWindow->InputCommand(CIC_ContextMenu, true);
	AssertTrue(s.pWindow->IsLayoutDirty());
	s.pWindow->Layout();

	sCounter.iCalls = 0;
	uiFirst = s.pCanvas->NumInputActions();
	s.pCanvas->AddInputAction(CIC_Home, &sCounter, &STestEditingCounter::Count);
	AssertSize(uiFirst + 1, s.pCanvas->NumInputActions());
	s.pWindow->InputCommand(CIC_Home, true);
	AssertInt(1, sCounter.iCalls);
	AssertSize(3, s.Caret());
	AssertTrue(s.pWindow->IsLayoutDirty());

	s.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingInput(void)
{
	BeginTests();

	TestTextEditingInputSingleLine();
	TestTextEditingInputMultiLine();
	TestTextEditingInputPointer();
	TestTextEditingInputKeepsLayout();

	TestStatistics();
}

