#include "WindowLib/JewelTheme.h"
#include "WindowLib/JewelColours.h"
#include "WindowLib/ClearCanvasDraw.h"
#include "WindowLib/Clipboard.h"
#include "WindowLib/PasteHistoryDialog.h"
#include "TestWindow.h"


Ptr<CFont> GenerateMenuFont(Ptr<CWindow> pWindow);


//////////////////////////////////////////////////////////////////////////
//
// What the list's rows show, as "row text|row text".
//
//////////////////////////////////////////////////////////////////////////
static char* PrintPasteHistoryRows(Ptr<CPasteHistoryDialog> pDialog, CChars* psz)
{
	int32	i;

	psz->Clear();
	for (i = 0; i < pDialog->GetList()->NumRows(); i++)
	{
		psz->Append(i > 0 ? "|" : "");
		pDialog->GetRowText(i, psz);
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// Choose Content to Paste lists the history newest first, each entry's
// start on one line, and previews what is selected.  Enter chooses it,
// several rows together in the list's order; Delete takes entries out of
// the history.  Typing filters the list, escape clearing it first, and
// 1 to 9 choose a row when nothing is typed.
//
//////////////////////////////////////////////////////////////////////////
void TestPasteHistoryDialogChoose(void)
{
	STestWindow		cWindow;

	{
		Ptr<CWindow>				pTestWindow;
		SDialogLook					sLook;
		Ptr<CWindowTheme>			pTheme;
		Ptr<CPasteHistoryDialog>	pDialog;
		Ptr<CWindow>				pDialogWindow;
		Ptr<CClipboardHistory>		pHistory;
		CChars						sz;
		CChars						szLong;
		size						i;

		sz.Init();
		cWindow.Init("Paste History Owner", 300, 200, OMalloc<CClearCanvasDraw>(JEWEL_EDITOR));
		pTestWindow = cWindow.pWindow;
		pTheme = CJewelTheme::CreateDarkTheme(pTestWindow, GenerateMenuFont(pTestWindow));
		sLook.pcTheme = &pTheme;
		sLook.pcDimFont = &pTheme->GetFont();
		pDialog = OMalloc<CPasteHistoryDialog>(pTestWindow, &sLook, pTheme->GetFont());
		ORoot()->Add(pDialog);
		pDialogWindow = pDialog->GetDialog()->GetWindow();

		//A long entry, one of several lines, and two plain ones; "second" is the newest.
		szLong.Init();
		for (i = 0; i < 30; i++)
		{
			szLong.Append("abcde");
		}
		pHistory = pTestWindow->GetClipboard()->GetHistory();
		pHistory->Add(szLong.Text());
		pHistory->Add("  int x;\n\tint y;\n");
		pHistory->Add("first");
		pHistory->Add("second");

		//Nothing to choose from: not shown.
		AssertFalse(pDialog->ChooseContent(pTestWindow, Ptr<CClipboardHistory>(), &sz));

		pDialog->Start(pHistory);
		AssertInt(4, pDialog->GetList()->NumRows());
		PrintPasteHistoryRows(pDialog, &sz);
		AssertTrue(sz.StartsWith("second|first|int x; int y;|abcdeabcde"));
		AssertTrue(sz.EndsWith("abcde..."));
		AssertSize(strlen("second|first|int x; int y;|") + PASTE_HISTORY_ROW_CHARACTERS + 3, sz.Length());
		AssertInt(0, pDialog->GetList()->GetSelected());
		AssertString("second", pDialog->GetPreview()->GetText());
		AssertFalse(pDialog->IsChosen());

		//The list has the keys: down previews the next, whole.
		pDialogWindow->GetFocus()->SetFocus(pDialog->GetList());
		pDialogWindow->InputCommand(CIC_Down, true);
		pDialogWindow->InputCommand(CIC_Down, true);
		AssertInt(2, pDialog->GetList()->GetSelected());
		AssertString("  int x;\n\tint y;\n", pDialog->GetPreview()->GetText());

		//Enter chooses it.
		pDialogWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pDialog->IsChosen());
		AssertString("  int x;\n\tint y;\n", pDialog->GetChosen());

		//Several rows: what would be pasted is previewed, their texts in the list's order with a line feed between.
		pDialog->Start(pHistory);
		AssertFalse(pDialog->IsChosen());
		pDialogWindow->InputCommand(CIC_Down, true);
		pDialogWindow->InputCommand(CIC_SelectUp, true);
		AssertInt(2, pDialog->GetList()->NumSelected());
		AssertInt(0, pDialog->GetList()->GetSelected());
		AssertString("second\nfirst", pDialog->GetPreview()->GetText());
		AssertTrue(pDialog->Approve());
		AssertString("second\nfirst", pDialog->GetChosen());

		//What is typed filters the list, case not mattering, and shows over it; the first row left is selected.
		pDialog->Start(pHistory);
		pDialogWindow->SetWindowSize(600, 400);
		pDialogWindow->Layout();
		AssertString("", pDialog->GetFilter());
		AssertInt(0, pDialog->GetFilterLabel()->GetActualSize().y);
		pDialogWindow->InputPrintable('i', true);
		pDialogWindow->InputPrintable('N', true);
		AssertString("iN", pDialog->GetFilter());
		AssertString("Search for: iN", pDialog->GetFilterLabel()->GetText());
		pDialogWindow->Layout();
		AssertTrue(pDialog->GetFilterLabel()->GetActualSize().y > 0);
		AssertString("int x; int y;", PrintPasteHistoryRows(pDialog, &sz));
		AssertSize(2, pDialog->GetRowEntry(0));
		AssertInt(0, pDialog->GetList()->GetSelected());
		AssertString("  int x;\n\tint y;\n", pDialog->GetPreview()->GetText());

		//Backspace takes a character off; a digit typed after something is part of the filter.
		pDialogWindow->InputCommand(CIC_Backspace, true);
		AssertString("i", pDialog->GetFilter());
		AssertString("first|int x; int y;", PrintPasteHistoryRows(pDialog, &sz));
		pDialogWindow->InputPrintable('1', true);
		AssertString("i1", pDialog->GetFilter());
		AssertInt(0, pDialog->GetList()->NumRows());
		AssertFalse(pDialog->Approve());
		AssertFalse(pDialog->IsChosen());
		pDialogWindow->InputCommand(CIC_Backspace, true);

		//Enter chooses from what is listed.
		pDialogWindow->InputCommand(CIC_Down, true);
		pDialogWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pDialog->IsChosen());
		AssertString("  int x;\n\tint y;\n", pDialog->GetChosen());

		//Escape clears what was typed, and only with nothing typed cancels the dialog.
		pDialog->Start(pHistory);
		pDialogWindow->InputPrintable('f', true);
		AssertInt(1, pDialog->GetList()->NumRows());
		pDialog->GetDialog()->Close("");
		pDialogWindow->InputCommand(CIC_ClearSelection, true);
		AssertString("", pDialog->GetFilter());
		AssertInt(4, pDialog->GetList()->NumRows());
		AssertString("", pDialog->GetDialog()->GetClicked());
		pDialogWindow->Layout();
		AssertInt(0, pDialog->GetFilterLabel()->GetActualSize().y);
		pDialogWindow->InputCommand(CIC_ClearSelection, true);
		AssertString("Cancel", pDialog->GetDialog()->GetClicked());
		AssertFalse(pDialog->IsChosen());

		//With nothing typed, 1 to 9 chooses that row at once; a digit past the rows does nothing.
		pDialog->Start(pHistory);
		pDialogWindow->InputPrintable('9', true);
		AssertFalse(pDialog->IsChosen());
		AssertString("", pDialog->GetFilter());
		pDialogWindow->InputPrintable('2', true);
		AssertTrue(pDialog->IsChosen());
		AssertString("first", pDialog->GetChosen());

		//Delete takes the selected entries out of the history; the row that comes up to their place is selected.
		pDialog->Start(pHistory);
		pDialogWindow->InputCommand(CIC_Down, true);
		pDialogWindow->InputCommand(CIC_SelectDown, true);
		pDialogWindow->InputCommand(CIC_Delete, true);
		AssertSize(2, pHistory->NumEntries());
		AssertString("second", pHistory->GetText(0));
		AssertString(szLong.Text(), pHistory->GetText(1));
		AssertInt(2, pDialog->GetList()->NumRows());
		AssertInt(1, pDialog->GetList()->GetSelected());
		AssertString(szLong.Text(), pDialog->GetPreview()->GetText());
		pDialogWindow->InputCommand(CIC_SelectAll, true);
		pDialogWindow->InputCommand(CIC_Delete, true);
		AssertSize(0, pHistory->NumEntries());
		AssertInt(0, pDialog->GetList()->NumRows());
		AssertString("", pDialog->GetPreview()->GetText());

		//Nothing selected, nothing chosen: the dialog would stay open.
		AssertFalse(pDialog->Approve());
		AssertFalse(pDialog->IsChosen());

		szLong.Kill();
		sz.Kill();
		pHistory = NULL;
		ORoot()->Remove(pDialog);
		pDialog = NULL;
		pDialogWindow = NULL;
		sLook.Kill();
		pTheme = NULL;
		pTestWindow = NULL;
	}

	cWindow.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPasteHistoryDialog(void)
{
	BeginTests();
	ObjectsInit();

	TestPasteHistoryDialogChoose();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

