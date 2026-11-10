#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TextDocument.h"
#include "BaseLib/ArrayElementNotFound.h"
#include "TestLib/Assert.h"


typedef CArrayTemplate<STextDocumentChange>	CArrayTestTextDocumentChange;


//Keeps the changes it is told.
class CTestTextDocumentListener : public CTextDocumentListener
{
public:
	CArrayTestTextDocumentChange	masChanges;

	void	Init(void) { masChanges.Init(); }
	void	Kill(void) { masChanges.Kill(); }
	void	DocumentChanged(STextDocumentChange* psChange) override { *masChanges.Add() = *psChange; }
};


//////////////////////////////////////////////////////////////////////////
//
// The line starts as "0,4,9".
//
//////////////////////////////////////////////////////////////////////////
static char* TestTextDocumentLines(CTextDocument* pcDocument, CChars* pszDest)
{
	size	i;

	pszDest->Clear();
	for (i = 0; i < pcDocument->NumLines(); i++)
	{
		if (i > 0)
		{
			pszDest->Append(',');
		}
		pszDest->Append(pcDocument->GetLineStart(i));
	}
	return pszDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertTextDocumentChange(ETextDocumentChange eChange, size uiStart, size uiEnd, size uiLine, size uiLines, bool bLineStart, STextDocumentChange* psChange)
{
	AssertInt(eChange, psChange->eChange);
	AssertSize(uiStart, psChange->uiStart);
	AssertSize(uiEnd, psChange->uiEnd);
	AssertSize(uiLine, psChange->uiLine);
	AssertSize(uiLines, psChange->uiLines);
	AssertBool(bLineStart, psChange->bLineStart);
}


//////////////////////////////////////////////////////////////////////////
//
// Lines are ended by line feeds: there is always one, and a line feed at
// the end starts an empty last line.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDocumentLines(void)
{
	CTextDocument	cDocument;
	CChars			sz;

	sz.Init();
	cDocument.Init("");
	AssertSize(1, cDocument.NumLines());
	AssertString("0", TestTextDocumentLines(&cDocument, &sz));
	AssertSize(0, cDocument.FindLine(0));
	cDocument.Kill();

	cDocument.Init(NULL);
	AssertString("", cDocument.Text());
	AssertSize(0, cDocument.Length());
	cDocument.Kill();

	cDocument.Init("abc\ndefg\n\nh\n");
	AssertString("0,4,9,10,12", TestTextDocumentLines(&cDocument, &sz));
	AssertSize(0, cDocument.FindLine(0));
	AssertSize(0, cDocument.FindLine(3));
	AssertSize(1, cDocument.FindLine(4));
	AssertSize(1, cDocument.FindLine(8));
	AssertSize(2, cDocument.FindLine(9));
	AssertSize(3, cDocument.FindLine(10));
	AssertSize(4, cDocument.FindLine(12));
	AssertSize(4, cDocument.FindLine(100));

	//A line past the last starts at the end.
	AssertSize(12, cDocument.GetLineStart(5));
	AssertSize(12, cDocument.GetLineStart(MAX_SIZE));

	//A line ends at its line feed, the last at the end, as does one past it.
	AssertSize(3, cDocument.GetLineEnd(0));
	AssertSize(8, cDocument.GetLineEnd(1));
	AssertSize(9, cDocument.GetLineEnd(2));
	AssertSize(11, cDocument.GetLineEnd(3));
	AssertSize(12, cDocument.GetLineEnd(4));
	AssertSize(12, cDocument.GetLineEnd(5));
	AssertSize(12, cDocument.GetLineEnd(MAX_SIZE));
	cDocument.Kill();

	cDocument.Init("ab");
	AssertSize(2, cDocument.GetLineEnd(0));
	cDocument.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Text put in moves the lines after it and its line feeds start new
// lines.  The listeners are told where it went, on which line, how many
// line feeds it had and whether it went at the start of its line.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDocumentInsert(void)
{
	CTextDocument				cDocument;
	CTestTextDocumentListener	cListener;
	CChars						sz;

	sz.Init();
	cListener.Init();
	cDocument.Init("ab\ncd\nef");
	cDocument.AddListener(&cListener);

	//In a line, without line feeds.
	cDocument.Insert(4, "XY");
	AssertString("ab\ncXYd\nef", cDocument.Text());
	AssertString("0,3,8", TestTextDocumentLines(&cDocument, &sz));
	AssertSize(1, cListener.masChanges.NumElements());
	AssertTextDocumentChange(TDC_Inserted, 4, 6, 1, 0, false, cListener.masChanges.Get(0));

	//At the start of a line, with line feeds.
	cDocument.Insert(3, "1\n2\n");
	AssertString("ab\n1\n2\ncXYd\nef", cDocument.Text());
	AssertString("0,3,5,7,12", TestTextDocumentLines(&cDocument, &sz));
	AssertSize(2, cListener.masChanges.NumElements());
	AssertTextDocumentChange(TDC_Inserted, 3, 7, 1, 2, true, cListener.masChanges.Get(1));

	//In the middle of a line, splitting it.
	cDocument.Insert(1, "\n");
	AssertString("a\nb\n1\n2\ncXYd\nef", cDocument.Text());
	AssertString("0,2,4,6,8,13", TestTextDocumentLines(&cDocument, &sz));
	AssertTextDocumentChange(TDC_Inserted, 1, 2, 0, 1, false, cListener.masChanges.Get(2));

	//At the end.
	cDocument.Insert(cDocument.Length(), "\n");
	AssertString("0,2,4,6,8,13,16", TestTextDocumentLines(&cDocument, &sz));
	AssertTextDocumentChange(TDC_Inserted, 15, 16, 5, 1, false, cListener.masChanges.Get(3));

	//Nothing put in is no change.
	cDocument.Insert(2, "");
	AssertSize(4, cListener.masChanges.NumElements());

	//Edits don't count as changes until they are ended.
	AssertSize(0, cDocument.GetChangeCount());

	cDocument.Kill();
	cListener.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Text taken out joins the lines its line feeds ended.  The listeners
// are told what went, from which line, how many line feeds and whether
// it was whole lines.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDocumentRemove(void)
{
	CTextDocument				cDocument;
	CTestTextDocumentListener	cListener;
	CChars						sz;

	sz.Init();
	cListener.Init();
	cDocument.Init("ab\ncd\nef\ngh\nij");
	cDocument.AddListener(&cListener);

	//In a line.
	cDocument.Remove(4, 5);
	AssertString("ab\nc\nef\ngh\nij", cDocument.Text());
	AssertString("0,3,5,8,11", TestTextDocumentLines(&cDocument, &sz));
	AssertTextDocumentChange(TDC_Removed, 4, 5, 1, 0, false, cListener.masChanges.Get(0));

	//Whole lines: "ef\ngh\n".
	cDocument.Remove(5, 11);
	AssertString("ab\nc\nij", cDocument.Text());
	AssertString("0,3,5", TestTextDocumentLines(&cDocument, &sz));
	AssertTextDocumentChange(TDC_Removed, 5, 11, 2, 2, true, cListener.masChanges.Get(1));
	AssertSize(4, cListener.masChanges.Get(1)->LastLine());

	//Across a line feed, joining two lines.
	cDocument.Remove(1, 4);
	AssertString("a\nij", cDocument.Text());
	AssertString("0,2", TestTextDocumentLines(&cDocument, &sz));
	AssertTextDocumentChange(TDC_Removed, 1, 4, 0, 1, false, cListener.masChanges.Get(2));

	//An empty range takes out nothing.
	cDocument.Remove(1, 1);
	cDocument.Remove(2, 1);
	AssertSize(3, cListener.masChanges.NumElements());

	//A listener taken off isn't told.
	cDocument.RemoveListener(&cListener);
	cDocument.Remove(0, 1);
	AssertString("\nij", cDocument.Text());
	AssertSize(3, cListener.masChanges.NumElements());

	cDocument.Kill();
	cListener.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Setting and appending text aren't edits: the listeners aren't told,
// but the lines follow the text.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDocumentSetAndAppend(void)
{
	CTextDocument				cDocument;
	CTestTextDocumentListener	cListener;
	CChars						sz;

	sz.Init();
	cListener.Init();
	cDocument.Init("ab\ncd");
	cDocument.AddListener(&cListener);

	cDocument.Append("e\nfg\n");
	AssertString("ab\ncde\nfg\n", cDocument.Text());
	AssertString("0,3,7,10", TestTextDocumentLines(&cDocument, &sz));
	cDocument.Append("h");
	cDocument.Append(NULL);
	AssertString("0,3,7,10", TestTextDocumentLines(&cDocument, &sz));
	AssertString("ab\ncde\nfg\nh", cDocument.Text());

	cDocument.SetText("x\ny");
	AssertString("0,2", TestTextDocumentLines(&cDocument, &sz));
	cDocument.SetText(NULL);
	AssertString("", cDocument.Text());
	AssertString("0", TestTextDocumentLines(&cDocument, &sz));

	AssertSize(0, cListener.masChanges.NumElements());
	AssertSize(0, cDocument.GetChangeCount());

	cDocument.Kill();
	cListener.Kill();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Ended edits count as changes and are kept in the history.  Loading a
// state changes only the part of the text that differs from it and gives
// back its caret and selection.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDocumentUndo(void)
{
	CTextDocument				cDocument;
	CTestTextDocumentListener	cListener;
	size						uiCaret;
	size						uiAnchor;

	cListener.Init();
	cDocument.Init("one two");
	cDocument.ClearUndo(0, MAX_SIZE);
	AssertFalse(cDocument.CanUndo());
	AssertFalse(cDocument.CanRedo());

	//An edit of two changes keeps the caret from before the first.
	cDocument.StartEdit(4, 7);
	cDocument.Remove(4, 7);
	cDocument.StartEdit(4, MAX_SIZE);
	cDocument.Insert(4, "three");
	AssertInt(TUK_Pushed, cDocument.EndEdit(9, MAX_SIZE));
	AssertSize(1, cDocument.GetChangeCount());
	AssertString("one three", cDocument.Text());
	AssertTrue(cDocument.CanUndo());
	AssertFalse(cDocument.CanRedo());

	//Undone: only what differs, after "one t", is changed: "hree" is taken out and "wo" put in.
	cDocument.AddListener(&cListener);
	AssertTrue(cDocument.Load(cDocument.GetUndo()->FindUndo(), &uiCaret, &uiAnchor));
	AssertString("one two", cDocument.Text());
	AssertSize(4, uiCaret);
	AssertSize(7, uiAnchor);
	AssertSize(2, cListener.masChanges.NumElements());
	AssertTextDocumentChange(TDC_Removed, 5, 9, 0, 0, false, cListener.masChanges.Get(0));
	AssertTextDocumentChange(TDC_Inserted, 5, 7, 0, 0, false, cListener.masChanges.Get(1));

	//The load ends as an edit that isn't kept.
	AssertInt(TUK_None, cDocument.EndEdit(uiCaret, uiAnchor));
	cDocument.GetUndo()->Loaded(uiCaret);
	AssertSize(2, cDocument.GetChangeCount());
	AssertFalse(cDocument.CanUndo());
	AssertTrue(cDocument.CanRedo());

	//Redone: as after the edit.
	AssertTrue(cDocument.Load(cDocument.GetUndo()->FindRedo(), &uiCaret, &uiAnchor));
	cDocument.EndEdit(uiCaret, uiAnchor);
	cDocument.GetUndo()->Loaded(uiCaret);
	AssertString("one three", cDocument.Text());
	AssertSize(9, uiCaret);
	AssertSize(MAX_SIZE, uiAnchor);

	//Nothing to redo.
	AssertFalse(cDocument.Load(cDocument.GetUndo()->FindRedo(), &uiCaret, &uiAnchor));

	cDocument.Kill();
	cListener.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A document that isn't undoable keeps no history, but its edits still
// count as changes.
//
//////////////////////////////////////////////////////////////////////////
void TestTextDocumentNotUndoable(void)
{
	CTextDocument	cDocument;
	size			uiCaret;
	size			uiAnchor;

	cDocument.Init("abc");
	cDocument.SetUndoable(false);
	cDocument.ClearUndo(0, MAX_SIZE);
	AssertFalse(cDocument.IsUndoable());

	cDocument.StartEdit(3, MAX_SIZE);
	cDocument.Insert(3, "d");
	AssertInt(TUK_None, cDocument.EndEdit(4, MAX_SIZE));
	AssertSize(1, cDocument.GetChangeCount());
	AssertFalse(cDocument.CanUndo());
	AssertFalse(cDocument.Load(0, &uiCaret, &uiAnchor));
	AssertString("abcd", cDocument.Text());

	//Made undoable, the history starts with the text as it is.
	cDocument.SetUndoable(true);
	cDocument.ClearUndo(4, MAX_SIZE);
	cDocument.StartEdit(4, MAX_SIZE);
	cDocument.Remove(0, 1);
	AssertInt(TUK_Pushed, cDocument.EndEdit(0, MAX_SIZE));
	AssertTrue(cDocument.Load(cDocument.GetUndo()->FindUndo(), &uiCaret, &uiAnchor));
	AssertString("abcd", cDocument.Text());

	cDocument.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextDocument(void)
{
	BeginTests();

	TestTextDocumentLines();
	TestTextDocumentInsert();
	TestTextDocumentRemove();
	TestTextDocumentSetAndAppend();
	TestTextDocumentUndo();
	TestTextDocumentNotUndoable();

	TestStatistics();
}

