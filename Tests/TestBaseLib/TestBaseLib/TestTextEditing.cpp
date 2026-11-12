#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TextEditing.h"
#include "BaseLib/ArrayInt.h"
#include "TestLib/Assert.h"


#define TEST_TEXT_EDITING_NO_PAUSE		0x7FFFFFFF	//Typing never pauses long enough to start a new edit.
#define TEST_TEXT_EDITING_PAGE_ROWS		3


//A view with a row for each line of the text, a y for each row, a column of x for each character (tabs to tab stops),
//and a page of three rows.  It counts what it is told.
class CTestTextEditingView : public CTextEditingView
{
public:
	CTextDocument*	mpcDocument;
	int				miMarksChanged;
	int				miCaretShown;
	int				miTextEdited;
	CArrayInt		maiUndoChanges;
	size			muiLastIdAfter;
	int32			miColumnWidth;  //Each character's width in x.

	void	Init(CTextDocument* pcDocument)
	{
		mpcDocument = pcDocument;
		miColumnWidth = 1;
		miMarksChanged = 0;
		miCaretShown = 0;
		miTextEdited = 0;
		maiUndoChanges.Init();
		muiLastIdAfter = 0;
	}

	void	Kill(void) { maiUndoChanges.Kill(); }

	bool	HasRows(void) override { return true; }
	size	NumRows(void) override { return mpcDocument->NumLines(); }
	size	FindRow(size uiPosition, bool bRowEnd) override { return mpcDocument->FindLine(uiPosition); }
	size	GetRowStart(size uiRow) override { return mpcDocument->GetLineStart(uiRow); }

	size	GetRowEnd(size uiRow, bool* pbRowEnd) override
	{
		*pbRowEnd = false;
		return mpcDocument->GetNavigation()->FindLineEnd(mpcDocument->GetLineStart(uiRow));
	}

	size	FindRowAt(int32 y) override
	{
		if (y < 0)
		{
			return 0;
		}
		return ((size)y < NumRows()) ? (size)y : NumRows() - 1;
	}

	size	FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd) override
	{
		*pbRowEnd = false;
		if (x < 0)
		{
			x = 0;
		}
		return mpcDocument->GetNavigation()->FindColumn(mpcDocument->GetLineStart(uiRow), (size)((x + miColumnWidth / 2) / miColumnWidth));
	}

	int32	GetPositionX(size uiPosition, bool bRowEnd) override
	{
		return (int32)mpcDocument->GetNavigation()->CountColumns(mpcDocument->GetNavigation()->FindLineStart(uiPosition), uiPosition) * miColumnWidth;
	}

	size	ScrollPage(size uiRow, bool bDown) override
	{
		if (bDown)
		{
			return (uiRow + TEST_TEXT_EDITING_PAGE_ROWS < NumRows()) ? uiRow + TEST_TEXT_EDITING_PAGE_ROWS : NumRows() - 1;
		}
		return (uiRow > TEST_TEXT_EDITING_PAGE_ROWS) ? uiRow - TEST_TEXT_EDITING_PAGE_ROWS : 0;
	}

	void	MarksChanged(void) override { miMarksChanged++; }
	void	ShowCaret(void) override { miCaretShown++; }
	void	TextEdited(void) override { miTextEdited++; }

	void	UndoChanged(STextUndoChange* psChange) override
	{
		maiUndoChanges.Add(psChange->eChange);
		muiLastIdAfter = psChange->uiIdAfter;
	}
};


//A document, its editing and the editing's view, set up together.
struct STestTextEditing
{
	CTextDocument			cDocument;
	CTestTextEditingView	cView;
	CTextEditing			cEditing;

	void	Init(const char* szText)
	{
		cDocument.Init(szText);
		cDocument.GetUndo()->SetTypingPause(TEST_TEXT_EDITING_NO_PAUSE);
		cView.Init(&cDocument);
		cEditing.Init(&cDocument, &cView);
		cEditing.ClearUndo();
	}

	void	Kill(void)
	{
		cEditing.Kill();
		cView.Kill();
		cDocument.Kill();
	}

	char*	Text(void) { return cDocument.Text(); }
	size	Caret(void) { return cEditing.GetCaretPosition(); }
};


//////////////////////////////////////////////////////////////////////////
//
// Typing inserts, or overwrites up to the end of a line; backspace and
// delete take out the characters around the caret.  The view is told of
// each edit and each move.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingTyping(void)
{
	STestTextEditing	s;

	s.Init("");
	s.cEditing.Printable('H', true);
	s.cEditing.Printable('i', true);
	AssertString("Hi", s.Text());
	AssertSize(2, s.Caret());
	AssertInt(2, s.cView.miTextEdited);

	s.cEditing.Left();
	s.cEditing.Printable('x', true);
	AssertString("Hxi", s.Text());
	AssertSize(2, s.Caret());

	s.cEditing.Printable('y', false);
	AssertString("Hxy", s.Text());

	//Overwriting at the end of a line inserts.
	s.cEditing.Enter();
	s.cEditing.Printable('z', true);
	s.cEditing.Left();
	s.cEditing.Left();
	s.cEditing.Printable('w', false);
	AssertString("Hxyw\nz", s.Text());

	s.cEditing.Backspace();
	AssertString("Hxy\nz", s.Text());
	AssertSize(3, s.Caret());
	s.cEditing.Delete();
	AssertString("Hxyz", s.Text());
	s.cEditing.DocumentHome();
	s.cEditing.Delete();
	s.cEditing.Backspace();
	AssertString("xyz", s.Text());
	AssertSize(0, s.Caret());

	s.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Left, right, words, smart home, end, the ends of the text.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingMoving(void)
{
	STestTextEditing	s;
	int					iMarks;
	size				uiPosition;
	bool				bRowEnd;

	s.Init("one two\n    three");

	s.cEditing.WordRight();
	AssertSize(3, s.Caret());
	s.cEditing.WordRight();
	AssertSize(4, s.Caret());
	s.cEditing.End();
	AssertSize(7, s.Caret());
	s.cEditing.WordLeft();
	AssertSize(4, s.Caret());

	//Home goes to the first character that isn't a space, then the start of the line, and back.
	s.cEditing.DocumentEnd();
	AssertSize(17, s.Caret());
	s.cEditing.Home();
	AssertSize(12, s.Caret());
	s.cEditing.Home();
	AssertSize(8, s.Caret());
	s.cEditing.Home();
	AssertSize(12, s.Caret());

	iMarks = s.cView.miMarksChanged;
	s.cEditing.Right();
	AssertInt(iMarks + 1, s.cView.miMarksChanged);
	AssertSize(13, s.Caret());

	s.cEditing.SetCaretPosition(100);
	AssertSize(17, s.Caret());
	AssertInt(0, s.cView.miTextEdited);

	//Where a pointer puts the caret: the row y is in or nearest, the column nearest x.
	AssertTrue(s.cEditing.FindPositionAt(2, 0, &uiPosition, &bRowEnd));
	AssertSize(2, uiPosition);
	AssertFalse(bRowEnd);
	s.cEditing.FindPositionAt(100, 1, &uiPosition, &bRowEnd);
	AssertSize(17, uiPosition);
	s.cEditing.FindPositionAt(5, -3, &uiPosition, &bRowEnd);
	AssertSize(5, uiPosition);
	s.cEditing.FindPositionAt(1, 9, &uiPosition, &bRowEnd);
	AssertSize(9, uiPosition);

	s.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Up and down keep the x they started at; up from the first row goes to
// the start, down from the last to the end.  Pages are the view's.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingUpDown(void)
{
	STestTextEditing	s;

	s.Init("abcdef\nab\nabcdef\n1\n2\n3\n4");
	s.cEditing.SetCaretPosition(5);
	s.cEditing.Down();
	AssertSize(9, s.Caret());
	s.cEditing.Down();
	AssertSize(15, s.Caret());
	s.cEditing.Up();
	s.cEditing.Up();
	AssertSize(5, s.Caret());
	s.cEditing.Up();
	AssertSize(0, s.Caret());

	//A move that isn't up or down starts the x again.
	s.cEditing.SetCaretPosition(1);
	s.cEditing.Down();
	AssertSize(8, s.Caret());

	s.cEditing.PageDown();
	AssertSize(20, s.Caret());
	s.cEditing.PageDown();
	AssertSize(24, s.Caret());
	s.cEditing.Down();
	AssertSize(24, s.Caret());

	//A page up or down from the first or last row doesn't move.
	s.cEditing.PageDown();
	AssertSize(24, s.Caret());
	s.cEditing.SelectPageUp();
	AssertSize(18, s.Caret());
	AssertTrue(s.cEditing.IsSelected());

	s.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is from where it started to the caret.  Copy, cut and
// paste; typing replaces the selection.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingSelection(void)
{
	STestTextEditing	s;
	size				uiStart;
	size				uiEnd;
	CChars				sz;

	sz.Init();
	s.Init("hello big world");

	AssertFalse(s.cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(0, uiStart);
	AssertSize(0, uiEnd);

	s.cEditing.SetCaretPosition(9);
	s.cEditing.SelectWordLeft();
	AssertTrue(s.cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(6, uiStart);
	AssertSize(9, uiEnd);
	s.cEditing.Copy(&sz);
	AssertString("big", sz.Text());

	s.cEditing.Printable('X', true);
	AssertString("hello X world", s.Text());
	AssertFalse(s.cEditing.IsSelected());

	s.cEditing.SetCaretPosition(9);
	s.cEditing.ExtendSelection();
	s.cEditing.GetSelection(&uiStart, &uiEnd);
	AssertSize(8, uiStart);
	AssertSize(13, uiEnd);
	sz.Clear();
	s.cEditing.Cut(&sz);
	AssertString("world", sz.Text());
	AssertString("hello X ", s.Text());

	s.cEditing.Paste((char*)"a\r\nb\rc");
	AssertString("hello X a\nb\nc", s.Text());
	AssertSize(13, s.Caret());

	s.cEditing.SelectAll();
	s.cEditing.GetSelection(&uiStart, &uiEnd);
	AssertSize(0, uiStart);
	AssertSize(13, uiEnd);
	s.cEditing.ClearSelection();
	AssertFalse(s.cEditing.IsSelected());

	//Selecting backwards keeps where it started.
	s.cEditing.SetCaretPosition(5);
	s.cEditing.SelectLeft();
	s.cEditing.SelectLeft();
	s.cEditing.GetSelection(&uiStart, &uiEnd);
	AssertSize(3, uiStart);
	AssertSize(5, uiEnd);
	AssertSize(3, s.Caret());

	s.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The character under the pointer, as a double click finds it, is the one
// x is over, not the gap nearest x: x on a character's right half is on
// it, not on the next.  Left of a row is its first character; past its
// end there is none.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingCharacterAt(void)
{
	STestTextEditing	s;
	size				uiPosition;

	s.Init("ab\n\ncd");
	s.cView.miColumnWidth = 10;

	AssertTrue(s.cEditing.FindCharacterAt(2, 0, &uiPosition));
	AssertSize(0, uiPosition);
	AssertTrue(s.cEditing.FindCharacterAt(8, 0, &uiPosition));
	AssertSize(0, uiPosition);
	AssertTrue(s.cEditing.FindCharacterAt(10, 0, &uiPosition));
	AssertSize(1, uiPosition);
	AssertTrue(s.cEditing.FindCharacterAt(17, 0, &uiPosition));
	AssertSize(1, uiPosition);
	AssertTrue(s.cEditing.FindCharacterAt(-5, 0, &uiPosition));
	AssertSize(0, uiPosition);

	AssertFalse(s.cEditing.FindCharacterAt(20, 0, &uiPosition));
	AssertSize(2, uiPosition);
	AssertFalse(s.cEditing.FindCharacterAt(55, 0, &uiPosition));
	AssertSize(2, uiPosition);
	AssertFalse(s.cEditing.FindCharacterAt(3, 1, &uiPosition));
	AssertSize(3, uiPosition);

	AssertTrue(s.cEditing.FindCharacterAt(16, 2, &uiPosition));
	AssertSize(5, uiPosition);
	AssertTrue(s.cEditing.FindCharacterAt(16, 9, &uiPosition));
	AssertSize(5, uiPosition);
	AssertFalse(s.cEditing.FindCharacterAt(26, 2, &uiPosition));
	AssertSize(6, uiPosition);

	s.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Tab puts in spaces to the next tab stop; over more than one line it
// indents the lines, and back tab takes a tab's worth of spaces off them.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingTabs(void)
{
	STestTextEditing	s;
	size				uiStart;
	size				uiEnd;

	s.Init("a\nb\nc");
	s.cEditing.SetCaretPosition(1);
	s.cEditing.Tab(true);
	AssertString("a   \nb\nc", s.Text());

	s.cEditing.SetTabWidth(2);
	s.cEditing.Select(0, 7);
	s.cEditing.Tab(true);
	AssertString("  a   \n  b\nc", s.Text());
	s.cEditing.GetSelection(&uiStart, &uiEnd);
	AssertSize(0, uiStart);
	AssertSize(11, uiEnd);

	s.cEditing.BackTab();
	AssertString("a   \nb\nc", s.Text());

	//Back tab without a selection, on the caret's line; a tab is taken off whole.
	s.cEditing.SetCaretPosition(0);
	s.cEditing.BackTab();
	AssertString("a   \nb\nc", s.Text());
	s.cEditing.SetCaretPosition(7);
	s.cEditing.Printable('\t', true);
	s.cEditing.DocumentEnd();
	s.cEditing.Printable('\t', true);
	AssertString("a   \nb\n\tc\t", s.Text());
	s.cEditing.BackTab();
	AssertString("a   \nb\nc\t", s.Text());

	s.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Duplicate, delete line, cut line and the enters that start lines.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingLines(void)
{
	STestTextEditing	s;
	CChars				sz;

	sz.Init();
	s.Init("one\ntwo\nthree");

	s.cEditing.SetCaretPosition(5);
	s.cEditing.Duplicate();
	AssertString("one\ntwo\ntwo\nthree", s.Text());
	AssertSize(9, s.Caret());

	s.cEditing.Select(0, 2);
	s.cEditing.Duplicate();
	AssertString("onone\ntwo\ntwo\nthree", s.Text());

	s.cEditing.SetCaretPosition(8);
	s.cEditing.DeleteLine();
	AssertString("onone\ntwo\nthree", s.Text());
	AssertSize(8, s.Caret());

	//The last line takes the line feed before it.
	s.cEditing.DocumentEnd();
	s.cEditing.DeleteLine();
	AssertString("onone\ntwo", s.Text());

	s.cEditing.SetCaretPosition(1);
	s.cEditing.CutLine(&sz);
	AssertString("onone\n", sz.Text());
	AssertString("two", s.Text());
	AssertSize(1, s.Caret());

	s.cEditing.EndEnter();
	AssertString("two\n", s.Text());
	AssertSize(4, s.Caret());
	s.cEditing.SetCaretPosition(2);
	s.cEditing.HomeEnter();
	AssertString("\ntwo\n", s.Text());
	AssertSize(0, s.Caret());

	s.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Edits are kept in the document's history and the view told what was
// kept, undone or redone.  Undo puts the caret and selection back.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingUndo(void)
{
	STestTextEditing	s;
	size				auiStarts[2];
	size				auiEnds[2];
	size				uiId;

	s.Init("x y");

	s.cEditing.DocumentEnd();
	s.cEditing.Printable('a', true);
	s.cEditing.Printable('b', true);
	AssertString("x yab", s.Text());
	AssertSize(2, s.cView.maiUndoChanges.NumElements());
	AssertInt(TUC_Kept, s.cView.maiUndoChanges.GetValue(0));
	AssertInt(TUC_Merged, s.cView.maiUndoChanges.GetValue(1));
	uiId = s.cView.muiLastIdAfter;

	AssertTrue(s.cEditing.Undo());
	AssertString("x y", s.Text());
	AssertSize(3, s.Caret());
	AssertInt(TUC_Undone, s.cView.maiUndoChanges.GetValue(2));
	AssertTrue(s.cEditing.Redo());
	AssertString("x yab", s.Text());
	AssertSize(5, s.Caret());
	AssertInt(TUC_Redone, s.cView.maiUndoChanges.GetValue(3));

	//Replacing ranges is one edit.
	auiStarts[0] = 0;
	auiEnds[0] = 1;
	auiStarts[1] = 2;
	auiEnds[1] = 5;
	AssertTrue(s.cEditing.ReplaceRanges(auiStarts, auiEnds, 2, "zz"));
	AssertString("zz zz", s.Text());
	AssertFalse(s.cEditing.ReplaceRanges(auiEnds, auiStarts, 2, "q"));

	AssertTrue(s.cEditing.UndoTo(uiId));
	AssertString("x yab", s.Text());
	AssertTrue(s.cEditing.Undo());
	AssertFalse(s.cEditing.Undo());
	AssertString("x y", s.Text());

	s.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Moving or copying a range to another place is one edit: the text there
// is selected after, and one undo puts the text and the selection back.
// Into the range itself, or at its ends, nothing is moved.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingMoveText(void)
{
	STestTextEditing	s;
	size				uiStart;
	size				uiEnd;

	//one 0, two 4, three 8, end 13.
	s.Init("one two three");

	//Right, to the end.
	s.cEditing.Select(4, 7);
	AssertTrue(s.cEditing.MoveText(4, 7, 13, false));
	AssertString("one  threetwo", s.Text());
	AssertTrue(s.cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(10, uiStart);
	AssertSize(13, uiEnd);
	AssertSize(13, s.Caret());
	AssertInt(1, s.cView.miTextEdited);
	AssertTrue(s.cEditing.Undo());
	AssertString("one two three", s.Text());
	AssertTrue(s.cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(4, uiStart);
	AssertSize(7, uiEnd);
	AssertSize(7, s.Caret());
	AssertTrue(s.cEditing.Redo());
	AssertString("one  threetwo", s.Text());
	AssertTrue(s.cEditing.Undo());

	//Right, just past the range's end.
	AssertTrue(s.cEditing.MoveText(4, 7, 8, false));
	AssertString("one  twothree", s.Text());
	AssertTrue(s.cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(5, uiStart);
	AssertSize(8, uiEnd);
	AssertTrue(s.cEditing.Undo());

	//Left, to the start.
	AssertTrue(s.cEditing.MoveText(8, 13, 0, false));
	AssertString("threeone two ", s.Text());
	AssertTrue(s.cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(0, uiStart);
	AssertSize(5, uiEnd);
	AssertSize(5, s.Caret());
	AssertTrue(s.cEditing.Undo());
	AssertString("one two three", s.Text());

	//Copied, the range stays.
	AssertTrue(s.cEditing.MoveText(4, 7, 0, true));
	AssertString("twoone two three", s.Text());
	AssertTrue(s.cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(0, uiStart);
	AssertSize(3, uiEnd);
	AssertTrue(s.cEditing.Undo());
	AssertString("one two three", s.Text());
	AssertTrue(s.cEditing.MoveText(4, 7, 13, true));
	AssertString("one two threetwo", s.Text());
	AssertTrue(s.cEditing.Undo());

	//Into itself, empty, or past the end: nothing.
	s.cView.miTextEdited = 0;
	s.cEditing.Select(4, 7);
	AssertFalse(s.cEditing.MoveText(4, 7, 4, false));
	AssertFalse(s.cEditing.MoveText(4, 7, 5, false));
	AssertFalse(s.cEditing.MoveText(4, 7, 7, true));
	AssertFalse(s.cEditing.MoveText(4, 4, 0, false));
	AssertFalse(s.cEditing.MoveText(4, 14, 0, false));
	AssertFalse(s.cEditing.MoveText(4, 7, 14, false));
	AssertString("one two three", s.Text());
	AssertInt(0, s.cView.miTextEdited);
	AssertTrue(s.cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(4, uiStart);
	AssertSize(7, uiEnd);

	//Not editable.
	s.cEditing.SetEditable(false);
	AssertFalse(s.cEditing.MoveText(4, 7, 0, false));
	AssertString("one two three", s.Text());

	s.Kill();
}


//The syntax a parse might add: a call's range.
class CTestSelectionRanges : public CTextSelectionRanges
{
public:
	void	AddSelectionRanges(size uiStart, size uiEnd, CArrayTextRange* pasDest) override
	{
		STextRange*	psRange;

		psRange = pasDest->Add();
		psRange->uiStart = 31;
		psRange->uiEnd = 38;
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Extends the selection and asserts what it became.
//
//////////////////////////////////////////////////////////////////////////
void AssertTextEditingExtends(STestTextEditing* ps, size uiExpectedStart, size uiExpectedEnd)
{
	size	uiStart;
	size	uiEnd;

	ps->cEditing.ExtendSelection();
	AssertTrue(ps->cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(uiExpectedStart, uiStart);
	AssertSize(uiExpectedEnd, uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Shrinks the selection and asserts what it became.
//
//////////////////////////////////////////////////////////////////////////
void AssertTextEditingShrinks(STestTextEditing* ps, size uiExpectedStart, size uiExpectedEnd)
{
	size	uiStart;
	size	uiEnd;

	ps->cEditing.ShrinkSelection();
	AssertTrue(ps->cEditing.GetSelection(&uiStart, &uiEnd));
	AssertSize(uiExpectedStart, uiStart);
	AssertSize(uiExpectedEnd, uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Extend Selection grows to the smallest range around the selection that
// is bigger: the word, brackets' inside then the brackets, the line
// without its indent, the line, quotes, comments, whole lines, the whole
// text, and ranges added.  Shrink goes back through them, until the
// selection changes another way or the text is edited.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingExtendSelection(void)
{
	STestTextEditing		s;
	CTestSelectionRanges	cRanges;
	size					uiEnd;

	//f 4, ( 5, "b c" 13..18, ) 18, // note 20..27, LF 27, { 28, LF 29, tab 30, g 31, ( 32, x 33, [ 34, i 35, ] 36, ) 37,
	//; 38, LF 39, } 40, LF 41, /* one 42..48, LF 48, two */ 49..55, end 56..59.
	s.Init("int f(int a, \"b c\") // note\n{\n\tg(x[i]);\n}\n/* one\ntwo */ end");

	s.cEditing.SetCaretPosition(35);
	AssertTextEditingExtends(&s, 35, 36);
	AssertTextEditingExtends(&s, 34, 37);
	AssertTextEditingExtends(&s, 33, 37);
	AssertTextEditingExtends(&s, 32, 38);
	AssertTextEditingExtends(&s, 31, 39);
	AssertTextEditingExtends(&s, 30, 40);
	AssertTextEditingExtends(&s, 29, 40);
	AssertTextEditingExtends(&s, 28, 40);
	AssertTextEditingExtends(&s, 28, 41);
	AssertTextEditingExtends(&s, 28, 42);
	AssertTextEditingExtends(&s, 0, 59);
	AssertTextEditingExtends(&s, 0, 59);

	AssertTextEditingShrinks(&s, 28, 42);
	AssertTextEditingShrinks(&s, 28, 41);
	AssertTextEditingShrinks(&s, 28, 40);
	AssertTextEditingShrinks(&s, 29, 40);
	AssertTextEditingShrinks(&s, 30, 40);
	AssertTextEditingShrinks(&s, 31, 39);
	AssertTextEditingShrinks(&s, 32, 38);
	AssertTextEditingShrinks(&s, 33, 37);
	AssertTextEditingShrinks(&s, 34, 37);
	AssertTextEditingShrinks(&s, 35, 36);
	s.cEditing.ShrinkSelection();
	AssertFalse(s.cEditing.IsSelected());
	AssertSize(35, s.Caret());
	s.cEditing.ShrinkSelection();
	AssertFalse(s.cEditing.IsSelected());

	//Quotes, then the brackets, the line without its indent and the line with its line feed.
	s.cEditing.SetCaretPosition(14);
	AssertTextEditingExtends(&s, 14, 15);
	AssertTextEditingExtends(&s, 14, 17);
	AssertTextEditingExtends(&s, 13, 18);
	AssertTextEditingExtends(&s, 6, 18);
	AssertTextEditingExtends(&s, 5, 19);
	AssertTextEditingExtends(&s, 0, 27);
	AssertTextEditingExtends(&s, 0, 28);

	//A comment is its own range.
	s.cEditing.SetCaretPosition(24);
	AssertTextEditingExtends(&s, 23, 27);
	AssertTextEditingExtends(&s, 20, 27);
	AssertTextEditingExtends(&s, 0, 27);
	s.cEditing.SetCaretPosition(46);
	AssertTextEditingExtends(&s, 45, 48);
	AssertTextEditingExtends(&s, 42, 48);
	AssertTextEditingExtends(&s, 42, 49);
	AssertTextEditingExtends(&s, 42, 55);
	AssertTextEditingExtends(&s, 42, 59);

	//Ranges added are grown to too.
	s.cEditing.SetSelectionRanges(&cRanges);
	s.cEditing.Select(32, 38);
	AssertTextEditingExtends(&s, 31, 38);
	AssertTextEditingExtends(&s, 31, 39);
	s.cEditing.SetSelectionRanges(NULL);

	//Changed another way, or the text edited, the shrinks are forgotten.
	s.cEditing.SetCaretPosition(35);
	AssertTextEditingExtends(&s, 35, 36);
	AssertTextEditingExtends(&s, 34, 37);
	s.cEditing.SelectRight();
	AssertTextEditingShrinks(&s, 34, 38);
	s.cEditing.SetCaretPosition(35);
	AssertTextEditingExtends(&s, 35, 36);
	AssertTextEditingExtends(&s, 34, 37);
	s.cEditing.SelectRight();
	AssertTextEditingExtends(&s, 32, 38);
	AssertTextEditingShrinks(&s, 34, 38);
	AssertTextEditingShrinks(&s, 34, 38);
	s.cEditing.SetCaretPosition(35);
	AssertTextEditingExtends(&s, 35, 36);
	AssertTextEditingExtends(&s, 34, 37);
	uiEnd = 59;
	AssertTrue(s.cEditing.ReplaceRanges(&uiEnd, &uiEnd, 1, "!"));
	AssertTextEditingShrinks(&s, 34, 37);

	s.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A text that isn't editable can be moved in, selected and copied from,
// and cut copies what it would cut, but nothing changes it.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingNotEditable(void)
{
	STestTextEditing	s;
	CChars				sz;

	sz.Init();
	s.Init("read\nonly");
	s.cEditing.SetEditable(false);
	AssertFalse(s.cEditing.IsEditable());

	s.cEditing.Printable('x', true);
	s.cEditing.Enter();
	s.cEditing.Delete();
	s.cEditing.Paste((char*)"y");
	s.cEditing.DeleteLine();
	AssertString("read\nonly", s.Text());
	AssertInt(0, s.cView.miTextEdited);

	s.cEditing.SetCaretPosition(6);
	s.cEditing.Cut(&sz);
	AssertString("only", sz.Text());
	s.cEditing.SelectAll();
	sz.Clear();
	s.cEditing.Cut(&sz);
	AssertString("read\nonly", sz.Text());
	AssertString("read\nonly", s.Text());
	AssertFalse(s.cDocument.CanUndo());

	s.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The caret and selection follow edits made to the document by others.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingFollowsDocument(void)
{
	STestTextEditing	s;
	size				uiStart;
	size				uiEnd;

	s.Init("0123456789");
	s.cEditing.Select(4, 6);
	s.cDocument.Insert(0, "ab");
	s.cEditing.GetSelection(&uiStart, &uiEnd);
	AssertSize(6, uiStart);
	AssertSize(8, uiEnd);

	s.cDocument.Remove(7, 10);
	s.cEditing.GetSelection(&uiStart, &uiEnd);
	AssertSize(6, uiStart);
	AssertSize(7, uiEnd);
	AssertSize(7, s.Caret());

	s.cDocument.Remove(0, 9);
	AssertFalse(s.cEditing.IsSelected());
	AssertSize(0, s.Caret());

	s.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An edit that changes nothing isn't kept, counted or told as an edit.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingNothingChanged(void)
{
	STestTextEditing	s;

	s.Init("ab");
	s.cEditing.Backspace();
	s.cEditing.DocumentEnd();
	s.cEditing.Delete();
	s.cEditing.DeleteWordRemainingRight();
	s.cEditing.Paste((char*)"");
	AssertInt(0, s.cView.miTextEdited);
	AssertSize(0, s.cView.maiUndoChanges.NumElements());
	AssertSize(0, s.cDocument.GetChangeCount());
	AssertFalse(s.cDocument.CanUndo());

	//The edit after it keeps the caret from its own start.
	s.cEditing.Backspace();
	AssertString("a", s.Text());
	AssertInt(1, s.cView.miTextEdited);
	AssertTrue(s.cEditing.Undo());
	AssertString("ab", s.Text());
	AssertSize(2, s.Caret());

	s.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A single line text gets no line feeds: enters do nothing, line breaks
// pasted become spaces, and cut and duplicate need a selection.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingSingleLine(void)
{
	STestTextEditing	s;
	CChars				sz;

	sz.Init();
	s.Init("one");
	s.cEditing.SetSingleLine(true);
	AssertTrue(s.cEditing.IsSingleLine());

	s.cEditing.DocumentEnd();
	s.cEditing.Enter();
	s.cEditing.HomeEnter();
	s.cEditing.EndEnter();
	s.cEditing.Duplicate();
	s.cEditing.Cut(&sz);
	AssertString("one", s.Text());
	AssertString("", sz.Text());
	AssertInt(0, s.cView.miTextEdited);

	s.cEditing.Paste((char*)" a\r\nb\rc\nd");
	AssertString("one a b c d", s.Text());
	s.cEditing.ReplaceText("x\ny");
	AssertString("x y", s.Text());

	s.cEditing.SelectAll();
	s.cEditing.Duplicate();
	AssertString("x yx y", s.Text());

	s.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Replacing all of the text is one edit with the caret at the end; the
// same text again is no edit.
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditingReplaceText(void)
{
	STestTextEditing	s;

	s.Init("old\ntext");
	s.cEditing.SetCaretPosition(2);
	s.cEditing.ReplaceText("new\r\ntext");
	AssertString("new\ntext", s.Text());
	AssertSize(8, s.Caret());
	AssertInt(1, s.cView.miTextEdited);

	s.cEditing.ReplaceText("new\ntext");
	AssertInt(1, s.cView.miTextEdited);
	AssertSize(8, s.Caret());

	AssertTrue(s.cEditing.Undo());
	AssertString("old\ntext", s.Text());
	AssertSize(2, s.Caret());
	AssertFalse(s.cEditing.Undo());

	s.cEditing.SetEditable(false);
	s.cEditing.ReplaceText("x");
	AssertString("old\ntext", s.Text());

	s.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextEditing(void)
{
	BeginTests();

	TestTextEditingTyping();
	TestTextEditingMoving();
	TestTextEditingUpDown();
	TestTextEditingSelection();
	TestTextEditingCharacterAt();
	TestTextEditingTabs();
	TestTextEditingLines();
	TestTextEditingUndo();
	TestTextEditingMoveText();
	TestTextEditingExtendSelection();
	TestTextEditingNotEditable();
	TestTextEditingFollowsDocument();
	TestTextEditingNothingChanged();
	TestTextEditingSingleLine();
	TestTextEditingReplaceText();

	TestStatistics();
}

