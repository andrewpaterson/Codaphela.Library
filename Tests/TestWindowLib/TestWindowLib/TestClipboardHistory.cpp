#include "WindowLib/Clipboard.h"
#include "WindowLib/ClipboardHistory.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/TextArea.h"
#include "TestWindow.h"


Ptr<CWindowTheme> CreateTestTheme(Ptr<CFont> pFont);
Ptr<CFont> ReadLabelFont(void);


//////////////////////////////////////////////////////////////////////////
//
// A history keeps its texts newest first: one added again moves to the
// front, the oldest goes past the limit, and empty or too big texts
// aren't kept.
//
//////////////////////////////////////////////////////////////////////////
void TestClipboardHistoryEntries(void)
{
	Ptr<CClipboardHistory>	pHistory;
	size					uiIndex;

	pHistory = OMalloc<CClipboardHistory>();
	AssertSize(0, pHistory->NumEntries());
	AssertSize(CLIPBOARD_HISTORY_LIMIT, pHistory->GetLimit());
	AssertFalse(pHistory->IsNewest("one"));

	AssertTrue(pHistory->Add("one"));
	AssertTrue(pHistory->Add("two"));
	AssertTrue(pHistory->Add("three"));
	AssertSize(3, pHistory->NumEntries());
	AssertString("three", pHistory->GetText(0));
	AssertString("two", pHistory->GetText(1));
	AssertString("one", pHistory->GetText(2));
	AssertTrue(pHistory->IsNewest("three"));
	AssertFalse(pHistory->IsNewest("two"));

	//Added again, it moves to the front.
	AssertTrue(pHistory->Add("one"));
	AssertSize(3, pHistory->NumEntries());
	AssertString("one", pHistory->GetText(0));
	AssertString("three", pHistory->GetText(1));
	AssertString("two", pHistory->GetText(2));
	AssertTrue(pHistory->Find("two", &uiIndex));
	AssertSize(2, uiIndex);
	AssertFalse(pHistory->Find("Two", &uiIndex));

	//Not kept: nothing, and more than the biggest entry.
	AssertFalse(pHistory->Add(""));
	AssertFalse(pHistory->Add(NULL));
	pHistory->SetMaxEntryBytes(4);
	AssertFalse(pHistory->Add("fives"));
	AssertTrue(pHistory->Add("four"));
	AssertSize(4, pHistory->NumEntries());
	AssertString("four", pHistory->GetText(0));

	pHistory->MoveToFront(3);
	AssertString("two", pHistory->GetText(0));
	AssertString("four", pHistory->GetText(1));
	pHistory->MoveToFront(0);
	AssertString("two", pHistory->GetText(0));

	pHistory->Remove(1);
	AssertSize(3, pHistory->NumEntries());
	AssertString("two", pHistory->GetText(0));
	AssertString("one", pHistory->GetText(1));
	AssertString("three", pHistory->GetText(2));

	//Past the limit the oldest goes; a smaller limit drops the oldest at once.
	pHistory->SetLimit(3);
	AssertTrue(pHistory->Add("new"));
	AssertSize(3, pHistory->NumEntries());
	AssertString("new", pHistory->GetText(0));
	AssertString("one", pHistory->GetText(2));
	pHistory->SetLimit(2);
	AssertSize(2, pHistory->NumEntries());
	AssertString("two", pHistory->GetText(1));
	pHistory->SetLimit(0);
	AssertSize(1, pHistory->GetLimit());
	AssertSize(1, pHistory->NumEntries());

	pHistory->Clear();
	AssertSize(0, pHistory->NumEntries());

	//An entry copied from a box selection is a block, and stays one as the entries move; added again as plain text it
	//is plain.
	pHistory->SetLimit(3);
	pHistory->SetMaxEntryBytes(CLIPBOARD_HISTORY_MAX_ENTRY_BYTES);
	AssertTrue(pHistory->Add("a\nb", true));
	AssertTrue(pHistory->Add("plain"));
	AssertTrue(pHistory->IsBlock(1));
	AssertFalse(pHistory->IsBlock(0));
	AssertFalse(pHistory->IsBlockText("a\nb"));
	pHistory->MoveToFront(1);
	AssertString("a\nb", pHistory->GetText(0));
	AssertTrue(pHistory->IsBlock(0));
	AssertTrue(pHistory->IsBlockText("a\nb"));
	AssertFalse(pHistory->IsBlock(1));
	AssertTrue(pHistory->Add("c\nd", true));
	AssertTrue(pHistory->Add("more"));
	AssertSize(3, pHistory->NumEntries());
	AssertString("a\nb", pHistory->GetText(2));
	AssertTrue(pHistory->IsBlock(2));
	AssertTrue(pHistory->IsBlock(1));
	pHistory->Remove(1);
	AssertTrue(pHistory->IsBlock(1));
	AssertFalse(pHistory->IsBlock(0));
	AssertTrue(pHistory->Add("a\nb"));
	AssertFalse(pHistory->IsBlock(0));
	AssertSize(2, pHistory->NumEntries());
	pHistory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What a window's clipboard is given goes into its history, a text area's
// copy and cut too.  Another window has a history of its own until it is
// shares the first's, as a dialog shares its owner's.  What another
// program copies gets into a history when its clipboard is looked at:
// when its window is activated and when it is pasted from.
//
//////////////////////////////////////////////////////////////////////////
void TestClipboardHistoryClipboard(void)
{
	STestWindow		cWindow;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CWindow>			pOtherWindow;
		Ptr<CFillContainer>		pFill;
		Ptr<CTextArea>			pTextArea;
		Ptr<CClipboardHistory>	pHistory;
		CPointer				pNull;
		CChars					sz;

		cWindow.Init("Clipboard Window", 300, 200);
		pTestWindow = cWindow.pWindow;
		pFill = OMalloc<CFillContainer>(pTestWindow);
		pTestWindow->SetContainer(pFill);
		pTextArea = OMalloc<CTextArea>(pTestWindow, CreateTestTheme(Ptr<CFont>()), ReadLabelFont(), "one two three");
		pFill->AddComponent(pTextArea);
		pTestWindow->Layout();

		pHistory = pTestWindow->GetClipboard()->GetHistory();
		AssertTrue(pHistory.IsNotNull());
		AssertSize(0, pHistory->NumEntries());

		AssertTrue(pTestWindow->GetClipboard()->SetText("set"));
		AssertSize(1, pHistory->NumEntries());
		AssertString("set", pHistory->GetText(0));

		//Copying nothing leaves it as it was.
		pTextArea->CopyToClipboard();
		AssertSize(1, pHistory->NumEntries());
		pTextArea->Select(0, 3);
		pTextArea->CopyToClipboard();
		pTextArea->Select(4, 7);
		pTextArea->CutToClipboard();
		AssertString("one  three", pTextArea->GetText());
		AssertSize(3, pHistory->NumEntries());
		AssertString("two", pHistory->GetText(0));
		AssertString("one", pHistory->GetText(1));
		AssertString("set", pHistory->GetText(2));

		//Another window: the same system clipboard, its own history.
		pOtherWindow = OMalloc<CWindow>("Other Window", &cWindow.cFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pOtherWindow);
		AssertTrue(pOtherWindow->GetClipboard()->GetHistory() != pHistory);
		pOtherWindow->GetClipboard()->SetText("other");
		AssertSize(3, pHistory->NumEntries());
		AssertSize(1, pOtherWindow->GetClipboard()->GetHistory()->NumEntries());

		//Sharing the first's, what it copies is in the first's.
		pOtherWindow->GetClipboard()->ShareHistory(pTestWindow->GetClipboard());
		AssertTrue(pOtherWindow->GetClipboard()->GetHistory() == pHistory);
		pOtherWindow->GetClipboard()->SetText("shared");
		AssertSize(4, pHistory->NumEntries());
		AssertString("shared", pHistory->GetText(0));

		//Its own again, it stands for another program: what it copies isn't in the first's history until the first
		//looks at the system's clipboard, as it does when it is activated.
		pOtherWindow->GetClipboard()->ShareHistory(Ptr<CClipboard>());
		pOtherWindow->GetClipboard()->SetText("elsewhere");
		AssertSize(4, pHistory->NumEntries());
		pTestWindow->Activate();
		AssertSize(5, pHistory->NumEntries());
		AssertString("elsewhere", pHistory->GetText(0));
		pTestWindow->Activate();
		AssertTrue(pTestWindow->GetClipboard()->Sync());
		AssertSize(5, pHistory->NumEntries());

		//And when it pastes it.  What is there that the history has already moves to the front.
		pOtherWindow->GetClipboard()->SetText("pasted");
		pTextArea->SetCaretPosition(0);
		pTextArea->PasteFromClipboard();
		AssertString("pastedone  three", pTextArea->GetText());
		AssertSize(6, pHistory->NumEntries());
		AssertString("pasted", pHistory->GetText(0));
		pOtherWindow->GetClipboard()->SetText("one");
		AssertTrue(pTestWindow->GetClipboard()->Sync());
		AssertSize(6, pHistory->NumEntries());
		AssertString("one", pHistory->GetText(0));
		AssertString("pasted", pHistory->GetText(1));

		//Detached, the first's text is its own: what it copies doesn't reach the other, nor the other's it, though the
		//history keeps on.  A clipboard that shares its history has its text.
		sz.Init();
		pOtherWindow->GetClipboard()->SetText("system");
		pTestWindow->GetClipboard()->SetDetached(true);
		AssertTrue(pTestWindow->GetClipboard()->IsDetached());
		AssertFalse(pTestWindow->GetClipboard()->GetText(&sz));
		pTestWindow->GetClipboard()->SetText("private");
		AssertString("private", pHistory->GetText(0));
		AssertTrue(pTestWindow->GetClipboard()->GetText(&sz));
		AssertString("private", sz.Text());
		sz.Clear();
		AssertTrue(pOtherWindow->GetClipboard()->GetText(&sz));
		AssertString("system", sz.Text());
		pTestWindow->Activate();
		AssertString("private", pHistory->GetText(0));
		pOtherWindow->GetClipboard()->ShareHistory(pTestWindow->GetClipboard());
		AssertTrue(pOtherWindow->GetClipboard()->IsDetached());
		sz.Clear();
		AssertTrue(pOtherWindow->GetClipboard()->GetText(&sz));
		AssertString("private", sz.Text());
		pOtherWindow->GetClipboard()->SetText("shared too");
		sz.Clear();
		AssertTrue(pTestWindow->GetClipboard()->GetText(&sz));
		AssertString("shared too", sz.Text());
		pTestWindow->GetClipboard()->SetDetached(false);
		sz.Clear();
		AssertTrue(pTestWindow->GetClipboard()->GetText(&sz));
		AssertString("system", sz.Text());
		sz.Kill();
		ORoot()->Remove(pOtherWindow);
		pOtherWindow = NULL;

		pHistory = NULL;
		pTextArea = NULL;
		pFill = NULL;
		pTestWindow = NULL;
	}

	cWindow.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestClipboardHistory(void)
{
	BeginTests();
	ObjectsInit();

	TestClipboardHistoryEntries();
	TestClipboardHistoryClipboard();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

