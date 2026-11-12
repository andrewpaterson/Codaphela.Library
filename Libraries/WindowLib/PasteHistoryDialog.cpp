/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "FillContainer.h"
#include "PasteHistoryDialog.h"


//////////////////////////////////////////////////////////////////////////
//
// pOwner is the window it is first shown over, and whose factory makes
// its window; psLook isn't held.  The preview is in pPreviewFamily, or in
// pPreviewFont (a white font): an editor's font, so the text looks as it
// did where it was copied.
//
//////////////////////////////////////////////////////////////////////////
void CPasteHistoryDialog::Init(Ptr<CWindow> pOwner, SDialogLook* psLook, Ptr<CFontFamily> pPreviewFamily)
{
	Init(pOwner, psLook, pPreviewFamily, Ptr<CFont>());
}


void CPasteHistoryDialog::Init(Ptr<CWindow> pOwner, SDialogLook* psLook, Ptr<CFont> pPreviewFont)
{
	Init(pOwner, psLook, Ptr<CFontFamily>(), pPreviewFont);
}


void CPasteHistoryDialog::Init(Ptr<CWindow> pOwner, SDialogLook* psLook, Ptr<CFontFamily> pPreviewFamily, Ptr<CFont> pPreviewFont)
{
	PreInit();

	mauiRows.Init();
	mszChosen.Init();
	mbChosen = false;

	mpDialog = OMalloc<CDialog>(pOwner, psLook, "Choose Content to Paste");
	mpDialog->SetResizable(GetMetric(TM_GoToWidth) / 2, GetMetric(TM_GoToHeight) / 2);
	mpDialog->SetApprover(this, &CPasteHistoryDialog::Approve);
	mpDialog->SetBody(CreateBody(pPreviewFamily, pPreviewFont), mpList);

	mpList->GetActivationEvent()->Add(this, &CPasteHistoryDialog::RowActivated);
	mpList->GetSelectionEvent()->Add(this, &CPasteHistoryDialog::SelectionChanged);
	mpList->AddInputAction(CIC_Delete, this, &CPasteHistoryDialog::DeleteSelected);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPasteHistoryDialog::Class(void)
{
	M_Pointer(mpDialog);
	M_Pointer(mpSplit);
	M_Pointer(mpListPane);
	M_Pointer(mpList);
	M_Pointer(mpPreviewPane);
	M_Pointer(mpPreview);
	M_Pointer(mpHistory);
	U_Unknown(CArrayPasteHistoryRows, mauiRows);
	U_Unknown(CChars, mszChosen);
	U_Bool(mbChosen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPasteHistoryDialog::Free(void)
{
	if (mpDialog.IsNotNull())
	{
		mpDialog->ClearApprover();
	}
	if (mpList.IsNotNull())
	{
		mpList->GetActivationEvent()->Remove(this);
		mpList->GetSelectionEvent()->Remove(this);
	}
	mszChosen.Kill();
	mauiRows.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// It is for the session only.
//
//////////////////////////////////////////////////////////////////////////
bool CPasteHistoryDialog::Save(CObjectWriter* pcFile) { return false; }
bool CPasteHistoryDialog::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// The list above the preview, a sash between them.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CPasteHistoryDialog::CreateBody(Ptr<CFontFamily> pPreviewFamily, Ptr<CFont> pPreviewFont)
{
	Ptr<CWindow>	pWindow;
	SDialogLook*	psLook;

	pWindow = mpDialog->GetWindow();
	psLook = mpDialog->GetLook();

	mpList = OMalloc<CListContainer>(pWindow, psLook->pcTheme);
	mpList->SetAlignment(CSH_Stretch, CSV_Top);
	mpList->SetDoubleClickActivates(true);
	mpList->SetMultiSelect(true);
	mpListPane = OMalloc<CScrollPane>(pWindow, psLook->pcTheme);
	mpListPane->SetContent(mpList);
	mpListPane->SetAlignment(CSH_Stretch, CSV_Stretch);

	if (pPreviewFamily.IsNotNull())
	{
		mpPreview = OMalloc<CTextArea>(pWindow, psLook->pcTheme, pPreviewFamily, "");
	}
	else
	{
		mpPreview = OMalloc<CTextArea>(pWindow, psLook->pcTheme, pPreviewFont, "");
	}
	mpPreview->SetEditable(false);
	mpPreviewPane = OMalloc<CScrollPane>(pWindow, psLook->pcTheme);
	mpPreviewPane->SetContent(mpPreview);
	mpPreviewPane->SetAlignment(CSH_Stretch, CSV_Stretch);

	mpSplit = OMalloc<CSplitContainer>(pWindow, psLook->pcTheme, false);
	mpSplit->SetComponents(mpListPane, mpPreviewPane);
	mpSplit->SetAlignment(CSH_Stretch, CSV_Stretch);
	return mpSplit;
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the dialog over pOwner for pHistory's entries.  The text chosen
// is appended to pszDest; false if none was, or the history is empty.
//
//////////////////////////////////////////////////////////////////////////
bool CPasteHistoryDialog::ChooseContent(Ptr<CWindow> pOwner, Ptr<CClipboardHistory> pHistory, CChars* pszDest)
{
	bool	bChosen;

	if (pHistory.IsNull() || (pHistory->NumEntries() == 0))
	{
		return false;
	}
	mpDialog->SetOwner(pOwner);
	Start(pHistory);
	bChosen = Show();
	if (bChosen)
	{
		pszDest->Append(mszChosen);
	}
	mpHistory = NULL;
	return bChosen;
}


//////////////////////////////////////////////////////////////////////////
//
// The dialog as it opens: pHistory's entries listed, the newest selected
// and previewed, nothing chosen.
//
//////////////////////////////////////////////////////////////////////////
void CPasteHistoryDialog::Start(Ptr<CClipboardHistory> pHistory)
{
	mpHistory = pHistory;
	mbChosen = false;
	mszChosen.Clear();
	FillList(0);
}


//////////////////////////////////////////////////////////////////////////
//
// Shown until it is closed.  True if something was chosen.
//
//////////////////////////////////////////////////////////////////////////
bool CPasteHistoryDialog::Show(void)
{
	return mpDialog->Show(GetMetric(TM_GoToWidth), GetMetric(TM_GoToHeight)) && mbChosen;
}


bool CPasteHistoryDialog::IsChosen(void) { return mbChosen; }
char* CPasteHistoryDialog::GetChosen(void) { return mszChosen.Text(); }
Ptr<CListContainer> CPasteHistoryDialog::GetList(void) { return mpList; }
Ptr<CTextArea> CPasteHistoryDialog::GetPreview(void) { return mpPreview; }
Ptr<CDialog> CPasteHistoryDialog::GetDialog(void) { return mpDialog; }
size CPasteHistoryDialog::GetRowEntry(int32 iRow) { return *mauiRows.Get((size)iRow); }  //The history's entry row iRow shows.
void CPasteHistoryDialog::GetRowText(int32 iRow, CChars* pszDest) { AppendRowText(pszDest, mpHistory->GetText(GetRowEntry(iRow))); }  //What row iRow shows of it, appended.


//////////////////////////////////////////////////////////////////////////
//
// OK, enter or a double click: what is selected is chosen, the rows'
// texts in the list's order with a line feed between them.  Nothing
// selected, the dialog stays open.
//
//////////////////////////////////////////////////////////////////////////
bool CPasteHistoryDialog::Approve(void)
{
	if (mpList->NumSelected() == 0)
	{
		return false;
	}
	mszChosen.Clear();
	AppendSelected(&mszChosen);
	mbChosen = true;
	return true;
}


void CPasteHistoryDialog::RowActivated(void) { mpDialog->OKClicked(); }


//////////////////////////////////////////////////////////////////////////
//
// The preview shows what would be pasted.
//
//////////////////////////////////////////////////////////////////////////
void CPasteHistoryDialog::SelectionChanged(void)
{
	CChars	szText;

	szText.Init();
	AppendSelected(&szText);
	mpPreview->SetText(szText.Text());
	mpPreviewPane->SetScrollPosition(0, 0);
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Delete: the selected entries leave the history, and the row that comes
// to where the first of them was is selected.
//
//////////////////////////////////////////////////////////////////////////
void CPasteHistoryDialog::DeleteSelected(void)
{
	int32	i;
	int32	iFirst;

	iFirst = -1;
	for (i = mpList->NumRows() - 1; i >= 0; i--)
	{
		if (mpList->IsRowSelected(i))
		{
			mpHistory->Remove(GetRowEntry(i));
			iFirst = i;
		}
	}
	if (iFirst >= 0)
	{
		FillList(iFirst);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The history's entries, a row each, row iSelect selected (the last if
// there are fewer).
//
//////////////////////////////////////////////////////////////////////////
void CPasteHistoryDialog::FillList(int32 iSelect)
{
	size	i;

	mpList->RemoveAllComponents();
	mauiRows.ReInit();
	for (i = 0; i < mpHistory->NumEntries(); i++)
	{
		AddRow(i);
	}
	if (iSelect >= mpList->NumRows())
	{
		iSelect = mpList->NumRows() - 1;
	}
	mpList->Select(iSelect);
	mpListPane->SetScrollPosition(0, 0);
	mpList->ShowRow(iSelect);
	SelectionChanged();
}


//////////////////////////////////////////////////////////////////////////
//
// A row for the history's entry uiEntry: the start of its text on one
// line.
//
//////////////////////////////////////////////////////////////////////////
void CPasteHistoryDialog::AddRow(size uiEntry)
{
	Ptr<CWindow>	pWindow;
	Ptr<CLabel>		pText;
	CChars			szRow;
	int32			iRow;

	pWindow = mpDialog->GetWindow();
	iRow = mpList->AddRow(true);
	*mauiRows.Add() = uiEntry;

	szRow.Init();
	AppendRowText(&szRow, mpHistory->GetText(uiEntry));
	pText = OMalloc<CLabel>(pWindow, mpDialog->GetLook()->pcTheme->GetFont(), szRow.Text());
	pText->SetAlignment(CSH_Left, CSV_Center);
	pText->SetDesiredSize(-1, GetMetric(TM_GoToRowHeight));
	mpList->AddToRow(iRow, pText, 0, 1, 1);
	szRow.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What a row shows of szText: its first PASTE_HISTORY_ROW_CHARACTERS
// bytes on one line, the spaces, tabs and line breaks before it left out
// and each run of them inside it one space, then "..." if that isn't all
// of it.
//
//////////////////////////////////////////////////////////////////////////
void CPasteHistoryDialog::AppendRowText(CChars* pszDest, const char* szText)
{
	size	i;
	size	uiStart;
	bool	bSpace;
	char	c;

	uiStart = pszDest->Length();
	bSpace = false;
	for (i = 0; szText[i] != '\0'; i++)
	{
		c = szText[i];
		if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
		{
			bSpace = pszDest->Length() > uiStart;
			continue;
		}
		//Cut between characters: not in the middle of a UTF8 character's bytes.
		if ((pszDest->Length() - uiStart >= PASTE_HISTORY_ROW_CHARACTERS) && (((uint8)c & 0xC0) != 0x80))
		{
			pszDest->Append("...");
			return;
		}
		if (bSpace)
		{
			pszDest->Append(' ');
			bSpace = false;
		}
		pszDest->Append(c);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The selected rows' entries, in the list's order, a line feed between
// them: what is pasted.
//
//////////////////////////////////////////////////////////////////////////
void CPasteHistoryDialog::AppendSelected(CChars* pszDest)
{
	int32	i;
	bool	bFirst;

	bFirst = true;
	for (i = 0; i < mpList->NumRows(); i++)
	{
		if (mpList->IsRowSelected(i))
		{
			if (!bFirst)
			{
				pszDest->Append('\n');
			}
			pszDest->Append(mpHistory->GetText(GetRowEntry(i)));
			bFirst = false;
		}
	}
}


int32 CPasteHistoryDialog::GetMetric(EThemeMetric eMetric) { return mpDialog->GetLook()->pcTheme->GetMetric(eMetric); }

