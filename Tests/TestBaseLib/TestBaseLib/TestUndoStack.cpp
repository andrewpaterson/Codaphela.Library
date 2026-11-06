#include "BaseLib/GlobalMemory.h"
#include "BaseLib/UndoStack.h"
#include "BaseLib/UndoLog.h"
#include "BaseLib/ArrayElementNotFound.h"
#include "BaseLib/ArrayChars.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Every snapshot's state is the one pushed, whichever is current: the
// newest kept whole, the others made back from it through their
// differences.
//
//////////////////////////////////////////////////////////////////////////
static void AssertUndoStates(CUndoStack* pcStack, CArrayChars* paszStates)
{
	size	i;

	AssertSize(paszStates->NumElements(), pcStack->NumSnapshots());
	for (i = 0; i < paszStates->NumElements(); i++)
	{
		AssertString(paszStates->GetText(i), pcStack->Get(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void KeepUndoStates(CArrayChars* paszStates, size uiNumStates)
{
	while (paszStates->NumElements() > uiNumStates)
	{
		paszStates->RemoveTail();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pushed states come back whole, at the start, in the middle and at the
// end of a text, growing and shrinking; undo and redo find them; a push
// after undoing drops what could be redone; and the current state can be
// replaced (typing a word) with a new id.
//
//////////////////////////////////////////////////////////////////////////
void TestUndoStackStates(void)
{
	CUndoStack	cStack;
	CArrayChars	aszStates;
	size		uiId;
	size		auiMarks[UNDO_STACK_MARKS] = { 1, 2, 3, 4 };

	cStack.Init();
	aszStates.Init();
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cStack.FindUndo(false));
	AssertSize(0, cStack.GetCurrentId());

	aszStates.Add("Hello world");
	cStack.Push((char*)"Hello world", false);
	aszStates.Add("Hello, big world");
	cStack.Push((char*)"Hello, big world", false, auiMarks);
	aszStates.Add("Hello, big world!");
	cStack.Push((char*)"Hello, big world!", false);
	aszStates.Add("Oh, big world!");
	cStack.Push((char*)"Oh, big world!", false);
	aszStates.Add("");
	cStack.Push((char*)"", false);
	aszStates.Add("Again");
	cStack.Push((char*)"Again", false);
	AssertUndoStates(&cStack, &aszStates);
	AssertSize(5, cStack.GetCurrent());
	AssertSize(2, cStack.GetMarks(1)[1]);
	AssertSize(4, cStack.GetMarks(1)[3]);
	AssertSize(0, cStack.GetMarks(2)[0]);

	//Undo, redo.
	AssertSize(4, cStack.FindUndo(false));
	cStack.MoveTo(2);
	AssertString("Hello, big world!", cStack.Get(cStack.GetCurrent()));
	AssertSize(3, cStack.FindRedo(false));
	AssertSize(1, cStack.FindUndo(false));

	//A push after undoing drops what could be redone; the states before are still there.
	uiId = cStack.GetCurrentId();
	KeepUndoStates(&aszStates, 3);
	aszStates.Add("Hello, big wide world!");
	cStack.Push((char*)"Hello, big wide world!", false);
	AssertUndoStates(&cStack, &aszStates);
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cStack.FindRedo(false));
	AssertSize(2, cStack.FindId(uiId));

	//Replaced, the current state has a new id and the one before it still comes back.
	uiId = cStack.GetCurrentId();
	KeepUndoStates(&aszStates, 3);
	aszStates.Add("Hello, big wide wonderful world!");
	cStack.ReplaceCurrent((char*)"Hello, big wide wonderful world!");
	AssertUndoStates(&cStack, &aszStates);
	AssertFalse(uiId == cStack.GetCurrentId());
	AssertSize(ARRAY_ELEMENT_NOT_FOUND, cStack.FindId(uiId));

	//Replaced after undoing: the later ones go.
	cStack.MoveTo(1);
	KeepUndoStates(&aszStates, 1);
	aszStates.Add("Hello, small world");
	cStack.ReplaceCurrent((char*)"Hello, small world", auiMarks);
	AssertUndoStates(&cStack, &aszStates);
	AssertSize(1, cStack.GetCurrent());
	AssertSize(3, cStack.GetMarks(1)[2]);

	cStack.Clear();
	AssertSize(0, cStack.NumSnapshots());
	aszStates.Kill();
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The oldest snapshots go past the maximum and the rest still come back
// whole.  Undo skips changes that were only to the selection.
//
//////////////////////////////////////////////////////////////////////////
void TestUndoStackMaximum(void)
{
	CUndoStack	cStack;
	CArrayChars	aszStates;
	CChars		sz;
	size		i;

	cStack.Init(5);
	aszStates.Init();
	sz.Init();
	for (i = 0; i < 9; i++)
	{
		sz.Append((int)i);
		cStack.Push(sz.Text(), false);
		aszStates.Add(sz.Text());
	}
	for (i = 0; i < 4; i++)
	{
		aszStates.Remove((size)0);
	}
	AssertUndoStates(&cStack, &aszStates);
	AssertSize(4, cStack.GetCurrent());
	sz.Kill();

	cStack.Push((char*)"a", false);
	cStack.Push((char*)"a", true);
	cStack.Push((char*)"a", true);
	AssertSize(1, cStack.FindUndo(false));
	AssertSize(3, cStack.FindUndo(true));

	aszStates.Kill();
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The edits of two documents, a and b, undone and redone across both in
// the order they were made, and a group of edits to both undone as one.
//
//////////////////////////////////////////////////////////////////////////
void TestUndoLogAcrossDocuments(void)
{
	CUndoLog	cLog;
	size		uiGroup;

	cLog.Init();
	AssertSize(MAX_SIZE, cLog.FindUndoGroup());
	AssertSize(MAX_SIZE, cLog.FindRedoGroup());

	//a 0->1, b 0->1, a 1->2 merged into a 1->3, then a and b reloaded together: a 3->4, b 1->2.
	cLog.Kept("a", 0, 1);
	cLog.Kept("b", 0, 1);
	cLog.Kept("a", 1, 2);
	cLog.Merged("a", 2, 3);
	cLog.BeginGroup();
	cLog.Kept("a", 3, 4);
	cLog.BeginGroup();
	cLog.Kept("b", 1, 2);
	cLog.EndGroup();
	cLog.EndGroup();
	AssertSize(5, cLog.NumSteps());
	AssertSize(3, cLog.Get(2)->uiIdAfter);
	AssertSize(cLog.Get(3)->uiGroup, cLog.Get(4)->uiGroup);
	AssertTrue(cLog.Get(2)->uiGroup != cLog.Get(3)->uiGroup);

	//The newest group is the reload: both its steps.
	uiGroup = cLog.FindUndoGroup();
	AssertSize(cLog.Get(4)->uiGroup, uiGroup);
	cLog.MarkUndone(4);
	cLog.MarkUndone(3);
	AssertTrue(cLog.IsUndone(3));
	AssertSize(uiGroup, cLog.FindRedoGroup());
	AssertSize(cLog.Get(2)->uiGroup, cLog.FindUndoGroup());

	//b's own undo of its first edit: the log is told, then redo across both finds it first.
	AssertSize(cLog.Get(1)->uiGroup, cLog.Undone("b", 1, 0));
	AssertTrue(cLog.IsUndone(1));
	AssertSize(cLog.Get(1)->uiGroup, cLog.FindRedoGroup());
	AssertSize(MAX_SIZE, cLog.Undone("b", 1, 0));
	AssertSize(cLog.Get(1)->uiGroup, cLog.Redone("b", 0, 1));
	AssertFalse(cLog.IsUndone(1));
	AssertSize(uiGroup, cLog.FindRedoGroup());

	//A new edit of a drops a's undone step; b's undone step of the group is still there.
	cLog.Kept("a", 3, 5);
	AssertSize(5, cLog.NumSteps());
	AssertString("b", cLog.Get(3)->szDocument.Text());
	AssertTrue(cLog.IsUndone(3));
	AssertString("a", cLog.Get(4)->szDocument.Text());
	AssertSize(5, cLog.Get(4)->uiIdAfter);
	AssertFalse(cLog.IsUndone(4));

	//Closing b forgets its steps.
	cLog.RemoveDocument("b");
	AssertSize(3, cLog.NumSteps());
	AssertSize(MAX_SIZE, cLog.FindRedoGroup());
	AssertSize(cLog.Get(2)->uiGroup, cLog.FindUndoGroup());

	cLog.Clear();
	AssertSize(0, cLog.NumSteps());
	cLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Past the maximum the oldest steps are dropped.
//
//////////////////////////////////////////////////////////////////////////
void TestUndoLogMaximum(void)
{
	CUndoLog	cLog;
	size		i;

	cLog.Init(3);
	for (i = 0; i < 5; i++)
	{
		cLog.Kept("a", i, i + 1);
	}
	AssertSize(3, cLog.NumSteps());
	AssertSize(2, cLog.Get(0)->uiIdBefore);
	AssertSize(5, cLog.Get(2)->uiIdAfter);
	cLog.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestUndoStack(void)
{
	BeginTests();

	TestUndoStackStates();
	TestUndoStackMaximum();
	TestUndoLogAcrossDocuments();
	TestUndoLogMaximum();

	TestStatistics();
}

