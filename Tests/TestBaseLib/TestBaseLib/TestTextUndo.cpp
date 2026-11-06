#include "BaseLib/GlobalMemory.h"
#include "BaseLib/TextUndo.h"
#include "BaseLib/ArrayElementNotFound.h"
#include "TestLib/Assert.h"


#define TEST_TEXT_UNDO_NO_PAUSE		0x7FFFFFFF	//Typing never pauses long enough to start a new edit.


//////////////////////////////////////////////////////////////////////////
//
// Types c where the caret is and keeps the text after it.
//
//////////////////////////////////////////////////////////////////////////
static ETextUndoKeep TestTextUndoType(CTextUndo* pcUndo, CChars* pszText, char c)
{
	size	uiAt;

	uiAt = pszText->Length();
	pcUndo->BeginEdit(uiAt, MAX_SIZE);
	pcUndo->Typed(c, uiAt);
	pszText->Append(c);
	return pcUndo->Keep(pszText->Text(), pszText->Length(), MAX_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
// Typing goes on the state of the typing before it: not the first
// state, not after a pause, a new line, the space after a word, a
// character typed somewhere else or over a selection.
//
//////////////////////////////////////////////////////////////////////////
void TestTextUndoTyping(void)
{
	CTextUndo	cUndo;
	CChars		szText;

	cUndo.Init();
	cUndo.SetTypingPause(TEST_TEXT_UNDO_NO_PAUSE);
	szText.Init();

	//The history starts with the empty text, and the first typing doesn't go on it.
	cUndo.Clear(szText.Text(), 0, MAX_SIZE);
	AssertInt(TUK_Pushed, TestTextUndoType(&cUndo, &szText, 'a'));
	AssertInt(TUK_Merged, TestTextUndoType(&cUndo, &szText, 'b'));

	//The space after a word is a new edit, the spaces after it aren't, and the next word goes on them.
	AssertInt(TUK_Pushed, TestTextUndoType(&cUndo, &szText, ' '));
	AssertInt(TUK_Merged, TestTextUndoType(&cUndo, &szText, ' '));
	AssertInt(TUK_Merged, TestTextUndoType(&cUndo, &szText, 'c'));

	//A new line starts a new edit, and what is typed after it goes on that.
	AssertInt(TUK_Pushed, TestTextUndoType(&cUndo, &szText, '\n'));
	AssertInt(TUK_Merged, TestTextUndoType(&cUndo, &szText, 'd'));

	//Typed somewhere other than where the last typing left the caret.
	cUndo.BeginEdit(0, MAX_SIZE);
	cUndo.Typed('e', 0);
	szText.Set("e");
	AssertInt(TUK_Pushed, cUndo.Keep(szText.Text(), 1, MAX_SIZE));

	//Over a selection.
	cUndo.BeginEdit(1, 0);
	cUndo.Typed('f', MAX_SIZE);
	szText.Set("f");
	AssertInt(TUK_Pushed, cUndo.Keep(szText.Text(), 1, MAX_SIZE));

	//Any pause starts a new edit.
	szText.Set("x");
	cUndo.Clear(szText.Text(), 1, MAX_SIZE);
	cUndo.SetTypingPause(0);
	AssertInt(TUK_Pushed, TestTextUndoType(&cUndo, &szText, 'y'));
	AssertInt(TUK_Pushed, TestTextUndoType(&cUndo, &szText, 'z'));

	//An edit that isn't typing ends the typing before it.
	cUndo.SetTypingPause(TEST_TEXT_UNDO_NO_PAUSE);
	AssertInt(TUK_Merged, TestTextUndoType(&cUndo, &szText, 'w'));
	szText.Set("xyz");
	cUndo.BeginEdit(4, MAX_SIZE);
	AssertInt(TUK_Pushed, cUndo.Keep(szText.Text(), 3, MAX_SIZE));
	AssertInt(TUK_Pushed, TestTextUndoType(&cUndo, &szText, 'v'));

	szText.Kill();
	cUndo.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Undo puts the text, caret and selection back as they were before the
// edit, redo as they were after it.  Nothing is kept while it loads, and
// typing after it starts a new edit.
//
//////////////////////////////////////////////////////////////////////////
void TestTextUndoLoad(void)
{
	CTextUndo	cUndo;
	CChars		szText;
	size		uiIndex;
	size		uiCaret;
	size		uiAnchor;

	cUndo.Init();
	cUndo.SetTypingPause(TEST_TEXT_UNDO_NO_PAUSE);
	szText.Init("abc");

	//Nothing to undo or redo in a history with only its first state.
	AssertInt(TUK_Started, cUndo.Keep(szText.Text(), 3, MAX_SIZE));
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cUndo.FindUndo());
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cUndo.FindRedo());
	AssertFalse(cUndo.CanLoad(cUndo.FindUndo()));

	//"b" selected and deleted.
	cUndo.BeginEdit(2, 1);
	szText.Set("ac");
	AssertInt(TUK_Pushed, cUndo.Keep(szText.Text(), 1, MAX_SIZE));

	uiIndex = cUndo.FindUndo();
	AssertSize(0, uiIndex);
	AssertTrue(cUndo.CanLoad(uiIndex));
	AssertTrue(cUndo.IsUndo(uiIndex));
	AssertString("abc", cUndo.GetLoadText(uiIndex, &uiCaret, &uiAnchor));
	AssertSize(2, uiCaret);
	AssertSize(1, uiAnchor);

	//While it loads the edit the editor makes isn't kept.
	cUndo.Loading(uiIndex);
	cUndo.BeginEdit(0, MAX_SIZE);
	AssertFalse(cUndo.IsEditStarted());
	AssertInt(TUK_None, cUndo.Keep((char*)"abc", 2, 1));
	cUndo.Loaded(2);
	szText.Set("abc");
	AssertFalse(cUndo.CanLoad(cUndo.FindId(cUndo.GetCurrentId())));

	uiIndex = cUndo.FindRedo();
	AssertSize(1, uiIndex);
	AssertFalse(cUndo.IsUndo(uiIndex));
	AssertString("ac", cUndo.GetLoadText(uiIndex, &uiCaret, &uiAnchor));
	AssertSize(1, uiCaret);
	AssertSize(MAX_SIZE, uiAnchor);

	//Typed after an undo: a new edit that drops what could be redone, not typing on the state undone to.
	TestTextUndoType(&cUndo, &szText, 'd');
	TestTextUndoType(&cUndo, &szText, 'e');
	uiIndex = cUndo.FindUndo();
	cUndo.GetLoadText(uiIndex, &uiCaret, &uiAnchor);
	cUndo.Loading(uiIndex);
	cUndo.Loaded(uiCaret);
	szText.Set("abc");
	AssertInt(TUK_Pushed, TestTextUndoType(&cUndo, &szText, 'f'));

	//A caret or selection past the loaded text's end is brought inside it, and an empty selection is none.
	szText.Set("abcdef");
	cUndo.Clear(szText.Text(), 6, MAX_SIZE);
	cUndo.BeginEdit(6, 6);
	szText.Set("");
	cUndo.Keep(szText.Text(), 0, MAX_SIZE);
	cUndo.BeginEdit(0, MAX_SIZE);
	szText.Set("x");
	cUndo.Keep(szText.Text(), 1, MAX_SIZE);
	uiIndex = cUndo.FindUndo();
	AssertString("", cUndo.GetLoadText(uiIndex, &uiCaret, &uiAnchor));
	AssertSize(0, uiCaret);
	AssertSize(MAX_SIZE, uiAnchor);

	szText.Kill();
	cUndo.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Only what differs between two texts changes, on whole UTF-8 characters.
//
//////////////////////////////////////////////////////////////////////////
void TestTextUndoDifference(void)
{
	CTextUndo	cUndo;
	size		uiPrefix;
	size		uiSuffix;
	char		szText[] = "a\xC3\xA9z";
	char		szTarget[] = "a\xC3\xA8z";

	cUndo.Init();

	cUndo.FindDifference((char*)"hello world", 11, (char*)"hello there world", 17, &uiPrefix, &uiSuffix);
	AssertSize(6, uiPrefix);
	AssertSize(5, uiSuffix);

	cUndo.FindDifference((char*)"same", 4, (char*)"same", 4, &uiPrefix, &uiSuffix);
	AssertSize(4, uiPrefix);
	AssertSize(0, uiSuffix);

	//The two characters share their first byte; the difference starts there, not inside them.
	cUndo.FindDifference(szText, 4, szTarget, 4, &uiPrefix, &uiSuffix);
	AssertSize(1, uiPrefix);
	AssertSize(1, uiSuffix);

	cUndo.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTextUndo(void)
{
	BeginTests();

	TestTextUndoTyping();
	TestTextUndoLoad();
	TestTextUndoDifference();

	TestStatistics();
}

