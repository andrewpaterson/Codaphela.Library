#include "BaseLib/GlobalMemory.h"
#include "BaseLib/UndoStack.h"
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
//
//////////////////////////////////////////////////////////////////////////
void TestUndoStack(void)
{
	BeginTests();

	TestUndoStackStates();
	TestUndoStackMaximum();

	TestStatistics();
}

