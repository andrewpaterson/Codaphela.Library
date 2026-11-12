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

	//A press and release of button one at x, y, the canvas focussed so the press starts a drag.  The drag's end is
	//where the window's pointer is.
	bool	Click(int x, int y)
	{
		bool	bTaken;

		pWindow->PointerMoved(x, y);
		bTaken = Command(CIC_ActivateComponent, x, y);
		Release(x, y);
		return bTaken;
	}

	void	Release(int x, int y)
	{
		pWindow->PointerMoved(x, y);
		Command(CIC_ExecuteComponent, x, y);
	}

	void	AssertSelection(size uiExpectedStart, size uiExpectedEnd, size uiExpectedCaret)
	{
		size	uiStart;
		size	uiEnd;

		AssertTrue(cEditing.GetSelection(&uiStart, &uiEnd));
		AssertSize(uiExpectedStart, uiStart);
		AssertSize(uiExpectedEnd, uiEnd);
		AssertSize(uiExpectedCaret, Caret());
	}
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


//////////////////////////////////////////////////////////////////////////
//
// Presses within half a second and four pixels of the last count on from
// it, up to three; a fourth is a single click again.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingInputClickCount(void)
{
	SClickCount	sClicks;

	sClicks.Init();
	AssertInt(1, sClicks.Pressed(10, 10, 1000));
	AssertInt(2, sClicks.Pressed(10, 10, 1500));
	AssertInt(3, sClicks.Pressed(11, 9, 1600));
	AssertInt(1, sClicks.Pressed(11, 9, 1700));
	AssertInt(1, sClicks.Pressed(11, 9, 2201));
	AssertInt(2, sClicks.Pressed(15, 9, 2300));
	AssertInt(1, sClicks.Pressed(20, 9, 2350));
	AssertInt(1, sClicks.Pressed(20, 14, 2400));
	sClicks.Forget();
	AssertInt(1, sClicks.Pressed(20, 14, 2450));
}


//////////////////////////////////////////////////////////////////////////
//
// A double click selects the word under the pointer, a triple click its
// line with its line feed; a fourth click is a single one.  Dragging on
// selects by whole words or lines and keeps the first.  A shift press
// extends by characters.  Losing focus forgets the clicks.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingInputClicks(void)
{
	STestEditingInput	s;

	ObjectsInit();
	//int 0, abc_d 4, ; 9, LF 10, line 11, two 16, LF 19, end 20, end of text 23.  The view's x is a row's column.
	s.Init("int abc_d;\nline two\nend", true);
	s.pWindow->GetFocus()->SetFocus(s.pCanvas);

	AssertTrue(s.Click(5, 0));
	AssertSize(5, s.Caret());
	AssertFalse(s.cEditing.IsSelected());
	s.Click(5, 0);
	s.AssertSelection(4, 9, 9);
	s.Click(5, 0);
	s.AssertSelection(0, 11, 11);
	s.Click(5, 0);
	AssertSize(5, s.Caret());
	AssertFalse(s.cEditing.IsSelected());

	//Losing focus, or a press too far from the last, starts again.
	s.Click(5, 0);
	s.cInput.FocusLost();
	s.Click(5, 0);
	AssertFalse(s.cEditing.IsSelected());
	s.Click(1, 1);
	s.Click(6, 1);
	AssertSize(17, s.Caret());
	AssertFalse(s.cEditing.IsSelected());
	s.Click(6, 1);
	s.AssertSelection(16, 19, 19);

	//Punctuation is a word by itself; past a line's end there is no word.
	s.cInput.FocusLost();
	s.Click(9, 0);
	s.Click(9, 0);
	s.AssertSelection(9, 10, 10);
	s.cInput.FocusLost();
	s.Click(15, 0);
	s.Click(15, 0);
	AssertFalse(s.cEditing.IsSelected());
	AssertSize(10, s.Caret());

	//Dragging after a double click goes by words, keeping the first.
	s.cInput.FocusLost();
	s.Click(2, 1);
	s.pWindow->PointerMoved(2, 1);
	AssertTrue(s.Command(CIC_ActivateComponent, 2, 1));
	s.AssertSelection(11, 15, 15);
	s.Command(CIC_ContDragSelect, 6, 1);
	s.AssertSelection(11, 19, 19);
	s.Command(CIC_ContDragSelect, 4, 1);
	s.AssertSelection(11, 16, 16);
	s.Command(CIC_ContDragSelect, 1, 1);
	s.AssertSelection(11, 15, 15);
	s.Command(CIC_ContDragSelect, 5, 0);
	s.AssertSelection(4, 15, 4);
	s.Release(5, 0);
	s.AssertSelection(4, 15, 4);
	s.Command(CIC_ContDragSelect, 1, 2);
	s.AssertSelection(4, 15, 4);

	//And after a triple click by lines.
	s.cInput.FocusLost();
	s.Click(1, 1);
	s.Click(1, 1);
	s.pWindow->PointerMoved(1, 1);
	AssertTrue(s.Command(CIC_ActivateComponent, 1, 1));
	s.AssertSelection(11, 20, 20);
	s.Command(CIC_ContDragSelect, 0, 2);
	s.AssertSelection(11, 23, 23);
	s.Command(CIC_ContDragSelect, 0, 0);
	s.AssertSelection(0, 20, 0);
	s.Release(0, 0);
	s.AssertSelection(0, 20, 0);

	//A shift press after a double click extends from the word's start by characters, and the press after it is a
	//single click.
	s.cInput.FocusLost();
	s.Click(5, 0);
	s.Click(5, 0);
	s.AssertSelection(4, 9, 9);
	AssertTrue(s.Command(CIC_StartDragSelect, 1, 0));
	s.AssertSelection(1, 4, 1);
	s.Command(CIC_ContDragSelect, 2, 0);
	s.AssertSelection(2, 4, 2);
	s.Command(CIC_StopDragSelect, 2, 0);
	s.Click(2, 0);
	AssertSize(2, s.Caret());
	AssertFalse(s.cEditing.IsSelected());

	//Pointer actions turned off take no clicks.
	s.cInput.FocusLost();
	s.cInput.SetPointerEnabled(false);
	AssertFalse(s.Click(5, 0));
	AssertFalse(s.Click(5, 0));
	AssertFalse(s.cEditing.IsSelected());
	AssertSize(2, s.Caret());

	s.Kill();
	ObjectsFlush();
	ObjectsKill();
}


//////////////////////////////////////////////////////////////////////////
//
// A single line's triple click selects all of it.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingInputSingleLineClicks(void)
{
	STestEditingInput	s;

	ObjectsInit();
	s.Init("ab cd", false);
	s.pWindow->GetFocus()->SetFocus(s.pCanvas);

	s.Click(4, 0);
	s.Click(4, 0);
	s.AssertSelection(3, 5, 5);
	s.Click(4, 0);
	s.AssertSelection(0, 5, 5);

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
// A press in the selection that moves more than TEXT_DRAG_DISTANCE drags
// the selected text: the drop position follows the pointer and the
// release moves the text there as one undo step.  Released without
// dragging, the caret goes where it was pressed.  Dropped in the
// selection nothing changes, and Escape cancels the drag.  A text that
// isn't editable, or without text drags, selects by dragging as before.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingInputDragText(void)
{
	STestEditingInput	s;
	size				uiDrop;
	bool				bRowEnd;

	ObjectsInit();
	//one 0, two 4, three 8, LF 13, four 14, five 19.  The view's x is a row's column.
	s.Init("one two three\nfour five", true);
	s.cInput.SetTextDragEnabled(true);
	s.pWindow->GetFocus()->SetFocus(s.pCanvas);

	//Pressed on 'w', not dragged until the pointer is more than the distance away.
	s.cEditing.Select(4, 7);
	s.pWindow->PointerMoved(5, 0);
	AssertTrue(s.Command(CIC_ActivateComponent, 5, 0));
	s.AssertSelection(4, 7, 7);
	AssertFalse(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	s.pWindow->PointerMoved(5 + TEXT_DRAG_DISTANCE, 0);
	AssertFalse(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	s.AssertSelection(4, 7, 7);
	s.pWindow->PointerMoved(13, 0);
	AssertTrue(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	AssertSize(13, uiDrop);
	s.AssertSelection(4, 7, 7);
	AssertString("one two three\nfour five", s.Text());
	s.Release(13, 0);
	AssertFalse(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	AssertString("one  threetwo\nfour five", s.Text());
	s.AssertSelection(10, 13, 13);
	AssertTrue(s.Command(CIC_Undo));
	AssertString("one two three\nfour five", s.Text());
	s.AssertSelection(4, 7, 7);

	//To the next line, below it the last (rows are 1 high).
	s.cInput.FocusLost();
	s.pWindow->PointerMoved(5, 0);
	s.Command(CIC_ActivateComponent, 5, 0);
	s.pWindow->PointerMoved(4, 1 + TEXT_DRAG_DISTANCE);
	s.Release(4, 1 + TEXT_DRAG_DISTANCE);
	AssertString("one  three\nfourtwo five", s.Text());
	s.AssertSelection(15, 18, 18);
	AssertTrue(s.Command(CIC_Undo));
	s.AssertSelection(4, 7, 7);

	//Released where it was pressed: the caret goes there.
	s.cInput.FocusLost();
	s.Click(6, 0);
	AssertSize(6, s.Caret());
	AssertFalse(s.cEditing.IsSelected());

	//The character after the selection isn't in it: a press there is a plain press.
	s.cInput.FocusLost();
	s.cEditing.Select(4, 7);
	s.pWindow->PointerMoved(7, 0);
	s.Command(CIC_ActivateComponent, 7, 0);
	s.pWindow->PointerMoved(13, 0);
	AssertFalse(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	s.Release(13, 0);
	s.AssertSelection(7, 13, 13);

	//Losing the focus ends the drag.
	s.cEditing.Select(4, 7);
	s.cInput.FocusLost();
	s.pWindow->PointerMoved(5, 0);
	s.Command(CIC_ActivateComponent, 5, 0);
	s.pWindow->PointerMoved(13, 0);
	AssertTrue(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	s.cInput.FocusLost();
	AssertFalse(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	s.Release(13, 0);
	AssertString("one two three\nfour five", s.Text());
	s.AssertSelection(4, 7, 7);

	//Dragged away and back into the selection: nothing changes.
	s.cInput.FocusLost();
	s.cEditing.Select(4, 7);
	s.pWindow->PointerMoved(5, 0);
	s.Command(CIC_ActivateComponent, 5, 0);
	s.pWindow->PointerMoved(11, 0);
	AssertTrue(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	AssertSize(11, uiDrop);
	s.pWindow->PointerMoved(6, 0);
	AssertTrue(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	AssertSize(6, uiDrop);
	s.Release(6, 0);
	AssertString("one two three\nfour five", s.Text());
	s.AssertSelection(4, 7, 7);

	//Escape cancels the drag; the release after it does nothing.  Then Escape clears the selection.
	s.cInput.FocusLost();
	s.pWindow->PointerMoved(5, 0);
	s.Command(CIC_ActivateComponent, 5, 0);
	s.pWindow->PointerMoved(13, 0);
	AssertTrue(s.Command(CIC_ClearSelection));
	AssertFalse(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	s.Release(13, 0);
	AssertString("one two three\nfour five", s.Text());
	s.AssertSelection(4, 7, 7);
	AssertTrue(s.Command(CIC_ClearSelection));
	AssertFalse(s.cEditing.IsSelected());

	//A double click in the selection selects the word.
	s.cInput.FocusLost();
	s.cEditing.Select(0, 13);
	s.Click(9, 0);
	s.Click(9, 0);
	s.AssertSelection(8, 13, 13);

	//Not editable, or without text drags, the press selects from where it was.
	s.cInput.FocusLost();
	s.cEditing.SetEditable(false);
	s.cEditing.Select(4, 7);
	s.pWindow->PointerMoved(5, 0);
	s.Command(CIC_ActivateComponent, 5, 0);
	s.pWindow->PointerMoved(13, 0);
	AssertFalse(s.cInput.GetDropPosition(&uiDrop, &bRowEnd));
	s.Release(13, 0);
	s.AssertSelection(5, 13, 13);
	s.cInput.FocusLost();
	s.cEditing.SetEditable(true);
	s.cInput.SetTextDragEnabled(false);
	s.cEditing.Select(4, 7);
	s.pWindow->PointerMoved(5, 0);
	s.Command(CIC_ActivateComponent, 5, 0);
	s.pWindow->PointerMoved(13, 0);
	s.Release(13, 0);
	s.AssertSelection(5, 13, 13);
	AssertString("one two three\nfour five", s.Text());

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
	TestTextEditingInputClickCount();
	TestTextEditingInputClicks();
	TestTextEditingInputSingleLineClicks();
	TestTextEditingInputDragText();
	TestTextEditingInputKeepsLayout();

	TestStatistics();
}

