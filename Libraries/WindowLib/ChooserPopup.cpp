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
#include <stdlib.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "FillContainer.h"
#include "GridContainer.h"
#include "RoundedBoxCanvasDraw.h"
#include "ChooserPopup.h"


#define CHOOSER_POPUP_ROWS		300		//The most rows listed: more than can be looked through without filtering.
#define CHOOSER_POPUP_INSET		6		//Between the popup's edge and what it holds.


//////////////////////////////////////////////////////////////////////////
//
// pMatchFont draws the characters the filter's pattern matched; the rest
// of a name is in the look's font and where it is in its dim font.  The
// look isn't held.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::Init(Ptr<CWindow> pWindow, SDialogLook* psLook, Ptr<CFont> pMatchFont)
{
	PreInit();

	mpWindow = pWindow;
	mpsLook = psLook;
	mpMatchFont = pMatchFont;
	maIcons.Init();
	mszTitle.Init();
	mcItems.Init();
	maiMatched.Init();
	maiRows.Init();
	mcMatcher.Init("");
	mbSortMatches = false;
	mbShorterFirst = true;
	mbOpensAbove = false;
	mbDriven = false;
	mszFilter.Init();
	mszNote.Init();
	msOpenPosition.Init(0, 0);
	miChosen = -1;
	mcChosen.Init();
	mcSelected.Init();
	mcButtonClicked.Init();
	mcClosed.Init();

	mpList = OMalloc<CListContainer>(pWindow, psLook->pcTheme);
	mpScrollPane = OMalloc<CScrollPane>(pWindow, psLook->pcTheme);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::Class(void)
{
	M_Pointer(mpWindow);
	U_Pointer(mpsLook);
	M_Pointer(mpMatchFont);
	M_Pointer(mpPanel);
	M_Pointer(mpTitle);
	M_Pointer(mpNote);
	M_Pointer(mpField);
	M_Pointer(mpScrollPane);
	M_Pointer(mpList);
	M_Pointer(mpPreview);
	M_Pointer(mpButton);
	M_Pointer(mpPopup);
	M_Pointer(mpPreviousFocus);
	M_Embedded(maIcons);
	U_String(mszTitle);
	U_Unknown(CGoToItems, mcItems);
	U_Unknown(CArrayInt, maiMatched);
	U_Unknown(CArrayInt, maiRows);
	U_Unknown(CNameMatcher, mcMatcher);
	U_Bool(mbSortMatches);
	U_Bool(mbShorterFirst);
	U_Bool(mbOpensAbove);
	U_Bool(mbDriven);
	U_String(mszFilter);
	U_String(mszNote);
	U_Data(SInt32Vec2, msOpenPosition);
	U_Int32(miChosen);
	U_Unknown(CComponentEvent<CChooserPopup>, mcChosen);
	U_Unknown(CComponentEvent<CChooserPopup>, mcSelected);
	U_Unknown(CComponentEvent<CChooserPopup>, mcButtonClicked);
	U_Unknown(CComponentEvent<CChooserPopup>, mcClosed);
}


//////////////////////////////////////////////////////////////////////////
//
// An object's memory is freed without its destructor running, so the
// chooser takes itself off the widgets still here.  Its popup is closed.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::Free(void)
{
	if (mpPopup.IsNotNull())
	{
		mpPopup->GetClosedEvent()->Remove(this);
		if (mpWindow.IsNotNull() && mpPanel.IsNotNull())
		{
			mpWindow->ClosePopup(mpPanel);
		}
	}
	if (mpField.IsNotNull())
	{
		mpField->GetChangeEvent()->Remove(this);
	}
	if (mpList.IsNotNull())
	{
		mpList->GetSelectionEvent()->Remove(this);
		mpList->GetActivationEvent()->Remove(this);
	}
	if (mpButton.IsNotNull())
	{
		mpButton->GetClickEvent()->Remove(this);
	}
	mcClosed.Kill();
	mcButtonClicked.Kill();
	mcSelected.Kill();
	mcChosen.Kill();
	mcMatcher.Kill();
	maiRows.Kill();
	maiMatched.Kill();
	mcItems.Kill();
	mszNote.Kill();
	mszFilter.Kill();
	mszTitle.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChooserPopup::Save(CObjectWriter* pcFile) { return false; }
bool CChooserPopup::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// A line over the rows saying what they are: "Usages of Init  14 usages".
// Empty for none.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::SetTitle(const char* szTitle)
{
	mszTitle.Set(szTitle);
	if (mpTitle.IsNotNull())
	{
		mpTitle->SetText(szTitle);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A field over the rows: what is typed in it filters them.  Before the
// chooser is first opened.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::AddFilter(void)
{
	if (mpField.IsNotNull() || mpPanel.IsNotNull())
	{
		return;
	}
	mpField = OMalloc<CTextField>(mpWindow, mpsLook->pcTheme, "");
	mpField->SetColumns(10);
	mpField->SetAlignment(CSH_Stretch, CSV_Center);
	mpField->GetChangeEvent()->Add(this, &CChooserPopup::Filter);

	//The keys that move through the rows, and choose one, while typing.
	mpField->AddInputAction(CIC_Up, this, &CChooserPopup::Up);
	mpField->AddInputAction(CIC_Down, this, &CChooserPopup::Down);
	mpField->AddInputAction(CIC_PageUp, this, &CChooserPopup::PageUp);
	mpField->AddInputAction(CIC_PageDown, this, &CChooserPopup::PageDown);
	mpField->AddInputAction(CIC_Enter, this, &CChooserPopup::Enter);
}


//////////////////////////////////////////////////////////////////////////
//
// A preview of the selected item's code right of the rows, which whoever
// hears the selected event fills (GetPreview).  Before the chooser is
// first opened.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::AddPreview(Ptr<CFont> pTitleFont, Ptr<CFontFamily> pCodeFamily)
{
	if (mpPreview.IsNotNull() || mpPanel.IsNotNull())
	{
		return;
	}
	mpPreview = OMalloc<CCodePreview>(mpWindow, mpsLook->pcTheme, pTitleFont, pCodeFamily);
	mpPreview->SetAlignment(CSH_Stretch, CSV_Stretch);
}


//////////////////////////////////////////////////////////////////////////
//
// A button under the rows, such as "Open in Find Window": it closes the
// chooser and calls the button click event.  Before the chooser is first
// opened.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::AddButton(const char* szCaption)
{
	if (mpButton.IsNotNull() || mpPanel.IsNotNull())
	{
		return;
	}
	mpButton = OMalloc<CButton>(mpWindow, mpsLook->pcTheme, szCaption);
	mpButton->SetAlignment(CSH_Left, CSV_Center);
	mpButton->GetClickEvent()->Add(this, &CChooserPopup::ButtonClicked);
}


//////////////////////////////////////////////////////////////////////////
//
// Filtered rows are the best match first, as a Go to dialog's; or stay
// in the order their items were added (the default), their headings
// kept over them.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::SetSortMatches(bool bSortMatches)
{
	mbSortMatches = bSortMatches;
}


//////////////////////////////////////////////////////////////////////////
//
// Sorted matches as good as each other are the shorter name first (the
// default: the matcher's score counts a name's length against it), or
// stay in the order they were added.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::SetShorterFirst(bool bShorterFirst)
{
	mbShorterFirst = bShorterFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// What an item's match is sorted by: its score, without what its name's
// length took off it unless shorter names go first.
//
//////////////////////////////////////////////////////////////////////////
int32 CChooserPopup::GetSortScore(size uiItem)
{
	SGoToItem*	psItem;

	psItem = mcItems.Get(uiItem);
	return mbShorterFirst ? psItem->sMatch.iScore : psItem->sMatch.iScore + ((maiMatched.GetValue(uiItem) > 0) ? maiMatched.GetValue(uiItem) : (int32)psItem->szName.Length());
}


//////////////////////////////////////////////////////////////////////////
//
// Open's position is where the chooser's bottom left goes, not its top
// left: for one opened from something at the bottom of the window.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::SetOpensAbove(bool bOpensAbove)
{
	mbOpensAbove = bOpensAbove;
}


//////////////////////////////////////////////////////////////////////////
//
// The keys stay with whatever has them: it has no title and no field,
// and is filtered, moved through and chosen from by what opened it.
// Before the chooser is first opened.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::SetDriven(bool bDriven)
{
	if (mpPanel.IsNull())
	{
		mbDriven = bDriven;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A driven chooser's dim line under its rows; empty for none.  It can
// change while the chooser is open, which is then as tall as it needs.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::SetNote(const char* szNote)
{
	if (mszNote.Equals(szNote))
	{
		return;
	}
	mszNote.Set(szNote);
	if (mpNote.IsNotNull())
	{
		mpNote->SetText(szNote);
	}
	if (mpPopup.IsNotNull())
	{
		Resize();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An icon items can have, its index returned.
//
//////////////////////////////////////////////////////////////////////////
int32 CChooserPopup::AddIcon(Ptr<CImageCel> pIcon)
{
	maIcons.Add(pIcon);
	return (int32)maIcons.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// An item: its name, where it is (NULL for nowhere), its icon (-1 for
// none) and what it is to whoever added it, which mustn't be
// CHOOSER_HEADING.  Items are added while the chooser is closed, or to an
// open one that is then told (ItemsChanged).  The filter matches the first
// uiMatched characters of its name, all of it for 0: a function's row
// shows its parameters, "SetSpeed(int iSpeed)", which aren't its name.
//
//////////////////////////////////////////////////////////////////////////
SGoToItem* CChooserPopup::AddItem(const char* szName, const char* szLocation, int32 iIcon, size uiData, size uiMatched)
{
	maiMatched.Add((int)uiMatched);
	return mcItems.Add(szName, szLocation, iIcon, uiData, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// A heading over the items added after it: dim, and it can't be chosen.
// It is listed while any item under it is.
//
//////////////////////////////////////////////////////////////////////////
SGoToItem* CChooserPopup::AddHeading(const char* szName)
{
	maiMatched.Add(0);
	return mcItems.Add(szName, NULL, -1, CHOOSER_HEADING, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::RemoveAllItems(void)
{
	mcItems.Clear();
	maiMatched.ReInit();
	maiRows.ReInit();
	miChosen = -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Items were added while the chooser is open: its rows are made again
// for them, filtered as they were, the selected item still selected and
// in view, and it is as tall as its rows now need.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::ItemsChanged(void)
{
	SInt32Vec2	sScroll;
	int32		iSelected;
	int32		iRow;

	if (mpPopup.IsNull())
	{
		return;
	}
	iRow = mpList->GetSelected();
	iSelected = ((iRow >= 0) && ((size)iRow < maiRows.NumElements())) ? maiRows.GetValue((size)iRow) : -1;
	sScroll = mpScrollPane->GetScrollPosition();
	Filter();
	Resize();
	iRow = (iSelected >= 0) ? FindItemRow((size)iSelected) : -1;
	if (iRow >= 0)
	{
		mpList->Select(iRow);
		mpWindow->Layout();
		mpScrollPane->SetScrollPosition(sScroll.x, sScroll.y);
		mpList->ShowRow(iRow);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The title, the field, the rows with the preview beside them, the
// button and the note, each only if it was asked for.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::CreatePanel(void)
{
	Ptr<CFillContainer>		pGap;
	Ptr<CGridContainer>		pRows;
	SInsets					sInsets;
	int32					iGap;
	int						iRow;

	iGap = mpsLook->pcTheme->GetMetric(TM_GoToGap);
	mpPanel = OMalloc<CGridContainer>(mpWindow);
	iRow = 0;

	if (!mbDriven)
	{
		mpTitle = OMalloc<CLabel>(mpWindow, mpsLook->pcDimFont, mszTitle.Text());
		mpTitle->SetAlignment(CSH_Left, CSV_Center);
		mpPanel->AddComponent(mpTitle, 0, iRow, 1, 1, 1, 0);
		iRow++;
		pGap = OMalloc<CFillContainer>(mpWindow);
		pGap->SetDesiredSize(iGap, iGap / 2);
		mpPanel->AddComponent(pGap, 0, iRow);
		iRow++;
	}

	if (mpField.IsNotNull())
	{
		mpPanel->AddComponent(mpField, 0, iRow, 1, 1, 1, 0);
		iRow++;
		pGap = OMalloc<CFillContainer>(mpWindow);
		pGap->SetDesiredSize(iGap, iGap / 2);
		mpPanel->AddComponent(pGap, 0, iRow);
		iRow++;
	}

	sInsets.Init(4, 0);
	mpList->SetHighlightDraws(OMalloc<CRoundedBoxCanvasDraw>(mpsLook->pcTheme->GetColour(TC_ListSelected), 4, &sInsets), OMalloc<CRoundedBoxCanvasDraw>(mpsLook->pcTheme->GetColour(TC_ListHover), 4, &sInsets));
	mpList->SetAlignment(CSH_Stretch, CSV_Top);
	mpList->SetWrapAround(true);
	mpList->GetSelectionEvent()->Add(this, &CChooserPopup::RowSelected);
	mpList->GetActivationEvent()->Add(this, &CChooserPopup::RowActivated);
	if (mpField.IsNull() && !mbDriven)
	{
		//With no field to filter them, typing finds a row.
		mpList->SetTypeToFind(true);
	}
	mpScrollPane->SetContent(mpList);
	mpScrollPane->SetAlignment(CSH_Stretch, CSV_Stretch);

	pRows = OMalloc<CGridContainer>(mpWindow);
	pRows->SetAlignment(CSH_Stretch, CSV_Stretch);
	pRows->AddComponent(mpScrollPane, 0, 0, 1, 1, 0, 1);
	if (mpPreview.IsNotNull())
	{
		pGap = OMalloc<CFillContainer>(mpWindow);
		pGap->SetDesiredSize(iGap, 1);
		pRows->AddComponent(pGap, 1, 0);
		mpPreview->SetDesiredSize(mpsLook->pcTheme->GetMetric(TM_ChooserPreviewWidth), -1);
		pRows->AddComponent(mpPreview, 2, 0, 1, 1, 1, 1);
	}
	mpPanel->AddComponent(pRows, 0, iRow, 1, 1, 1, 1);
	iRow++;

	if (mpButton.IsNotNull())
	{
		pGap = OMalloc<CFillContainer>(mpWindow);
		pGap->SetDesiredSize(iGap, iGap / 2);
		mpPanel->AddComponent(pGap, 0, iRow);
		iRow++;
		mpPanel->AddComponent(mpButton, 0, iRow);
		iRow++;
	}

	if (mbDriven)
	{
		mpNote = OMalloc<CLabel>(mpWindow, mpsLook->pcDimFont, mszNote.Text());
		mpNote->SetAlignment(CSH_Left, CSV_Center);
		mpPanel->AddComponent(mpNote, 0, iRow, 1, 1, 1, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the chooser with its top left at sPosition, in the window's
// client space (it is moved back in if that would put it past an edge),
// every item listed and item uiSelectedItem selected, or the first that
// can be chosen if it can't.  pOwner is the component it is opened for: a
// press on it closes the chooser, as one anywhere else outside it does.
// The field, or without one the list, gets the keys; what had them gets
// them back when it closes.  A driven chooser leaves them where they
// are, and is filtered by nothing until it is told.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::Open(SInt32Vec2 sPosition, Ptr<CComponent> pOwner, size uiSelectedItem)
{
	SInsets		sInsets;
	int32		iRow;

	Close();
	if (mpPanel.IsNull())
	{
		CreatePanel();
	}
	miChosen = -1;

	if (mpField.IsNotNull())
	{
		mpField->GetChangeEvent()->Remove(this);
		mpField->SetText("");
		mpField->ClearUndo();
		mpField->GetChangeEvent()->Add(this, &CChooserPopup::Filter);
	}
	mszFilter.Set("");
	Filter();

	msOpenPosition = sPosition;
	mpPreviousFocus = mpWindow->GetFocus()->GetFocussedComponent();
	mpPopup = mpWindow->OpenPopup(mpPanel, sPosition, pOwner);
	sInsets.Init(CHOOSER_POPUP_INSET, CHOOSER_POPUP_INSET);
	mpPopup->SetLook(mpsLook->pcTheme->GetColour(TC_MenuBackground), mpsLook->pcTheme->GetColour(TC_MenuBorder), &sInsets);
	mpPopup->SetOwnerPressKeepsOpen(false);
	mpPopup->GetClosedEvent()->Add(this, &CChooserPopup::PopupClosed);
	Resize();
	if (!mbDriven)
	{
		if (mpField.IsNotNull())
		{
			mpWindow->GetFocus()->SetFocus(mpField);
		}
		else
		{
			mpWindow->GetFocus()->SetFocus(mpList);
		}
	}

	iRow = FindItemRow(uiSelectedItem);
	if ((iRow < 0) || !mpList->IsRowEnabled(iRow))
	{
		iRow = FindFirstRow();
	}
	mpList->Select(iRow);
	if (iRow >= 0)
	{
		mpWindow->Layout();
		mpList->ShowRow(iRow);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The open chooser is the size its parts need now, where it was opened:
// as tall as its rows, to the theme's most (with a preview, always that
// many, so the code has room).
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::Resize(void)
{
	SInt32Vec2	sPosition;
	int32		iRows;
	int32		iRowHeight;
	int32		iMaximum;
	int32		iGap;
	int32		iWidth;
	int32		iHeight;

	iRowHeight = mpsLook->pcTheme->GetMetric(TM_GoToRowHeight);
	iMaximum = mpsLook->pcTheme->GetMetric(TM_ChooserRows);
	iRows = (int32)maiRows.NumElements();
	if ((iRows > iMaximum) || mpPreview.IsNotNull())
	{
		iRows = iMaximum;
	}
	if (iRows < 1)
	{
		iRows = 1;
	}
	mpScrollPane->SetDesiredSize(mpsLook->pcTheme->GetMetric(TM_ChooserWidth), iRows * iRowHeight);

	//A grid is the size it is given, not the size its parts need: the panel is given what they add up to.
	iGap = mpsLook->pcTheme->GetMetric(TM_GoToGap);
	iWidth = mpsLook->pcTheme->GetMetric(TM_ChooserWidth);
	iHeight = iRows * iRowHeight;
	if (mpTitle.IsNotNull())
	{
		iHeight += mpTitle->GetDesiredSize().y + iGap / 2;
	}
	if (mpNote.IsNotNull() && !mszNote.Empty())
	{
		iHeight += mpNote->GetDesiredSize().y;
	}
	if (mpField.IsNotNull())
	{
		iHeight += mpField->GetDesiredSize().y + iGap / 2;
	}
	if (mpPreview.IsNotNull())
	{
		iWidth += iGap + mpsLook->pcTheme->GetMetric(TM_ChooserPreviewWidth);
	}
	if (mpButton.IsNotNull())
	{
		iHeight += iGap / 2 + mpButton->GetDesiredSize().y;
	}
	mpPanel->SetDesiredSize(iWidth, iHeight);
	sPosition = msOpenPosition;
	if (mbOpensAbove)
	{
		sPosition.y = (sPosition.y > iHeight) ? sPosition.y - iHeight : 0;
	}
	mpPopup->SetOpenPosition(sPosition);
	mpWindow->InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Closes it if it is open.  Nothing is chosen.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::Close(void)
{
	if (mpPopup.IsNotNull())
	{
		mpWindow->ClosePopup(mpPanel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The popup has gone, however it went: the keys go back to what had
// them, if they were still in the chooser.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::PopupClosed(void)
{
	Ptr<CComponent>	pFocussed;
	Ptr<CComponent>	pPrevious;

	mpPopup = NULL;
	pPrevious = mpPreviousFocus;
	mpPreviousFocus = NULL;
	pFocussed = mpWindow->GetFocus()->GetFocussedComponent();
	if (pPrevious.IsNotNull() && (pFocussed.IsNull() || pFocussed->IsWithin(mpPanel)))
	{
		mpWindow->GetFocus()->SetFocus(pPrevious);
	}
	mcClosed.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The rows for what the field has in it: every item with none or an
// empty one; else the items whose names its pattern matches, with their
// matches, and the headings that have one under them.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::Filter(void)
{
	SGoToItem*	psItem;
	SNameMatch	sMatch;
	size		i;
	size		j;
	int			iHeading;
	bool		bAll;
	int			iBest;
	int			iItem;

	mcMatcher.Kill();
	mcMatcher.Init(GetPattern());
	bAll = GetPattern()[0] == '\0';

	maiRows.ReInit();
	iHeading = -1;
	for (i = 0; i < mcItems.NumItems(); i++)
	{
		psItem = mcItems.Get(i);
		if (psItem->uiData == CHOOSER_HEADING)
		{
			memset(&psItem->sMatch, 0, sizeof(SNameMatch));
			iHeading = (int)i;
			if (bAll)
			{
				maiRows.Add((int)i);
				iHeading = -1;
			}
		}
		else if (bAll)
		{
			memset(&psItem->sMatch, 0, sizeof(SNameMatch));
			maiRows.Add((int)i);
		}
		else if (mcMatcher.Match(psItem->szName.Text(), (maiMatched.GetValue(i) > 0) ? (size)maiMatched.GetValue(i) : psItem->szName.Length(), mcMatcher.GetPattern(), strlen(mcMatcher.GetPattern()), &sMatch))
		{
			memcpy(&psItem->sMatch, &sMatch, sizeof(SNameMatch));
			if ((iHeading >= 0) && !mbSortMatches)
			{
				maiRows.Add(iHeading);
				iHeading = -1;
			}
			maiRows.Add((int)i);
		}
	}

	if (mbSortMatches && !bAll)
	{
		//The best match first, those as good in the order they were added: few rows, so a selection sort does.
		for (i = 0; i + 1 < maiRows.NumElements(); i++)
		{
			iBest = (int)i;
			for (j = i + 1; j < maiRows.NumElements(); j++)
			{
				if (GetSortScore((size)maiRows.GetValue(j)) > GetSortScore((size)maiRows.GetValue((size)iBest)))
				{
					iBest = (int)j;
				}
			}
			if ((size)iBest != i)
			{
				iItem = maiRows.GetValue((size)iBest);
				maiRows.RemoveAt((size)iBest);
				maiRows.InsertAt(iItem, i);
			}
		}
	}
	if (maiRows.NumElements() > CHOOSER_POPUP_ROWS)
	{
		maiRows.RemoveRange(CHOOSER_POPUP_ROWS, maiRows.NumElements());
	}
	FillList();
}


//////////////////////////////////////////////////////////////////////////
//
// The list's rows are made again, the first that can be chosen selected.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::FillList(void)
{
	Ptr<CImageCel>	pCel;
	SGoToItem*		psItem;
	size			i;

	mpList->GetSelectionEvent()->Remove(this);
	mpList->RemoveAllComponents();
	for (i = 0; i < maiRows.NumElements(); i++)
	{
		psItem = mcItems.Get((size)maiRows.GetValue(i));
		pCel = NULL;
		if ((psItem->iIcon >= 0) && ((size)psItem->iIcon < maIcons.NumElements()))
		{
			pCel = maIcons.Get((size)psItem->iIcon);
		}
		AddNameRow(mpList, mpWindow, mpsLook, mpMatchFont, pCel, psItem, psItem->uiData != CHOOSER_HEADING);
	}
	mpList->Select(FindFirstRow());
	mpScrollPane->SetScrollPosition(0, 0);
	if (mpPanel.IsNotNull())
	{
		mpList->GetSelectionEvent()->Add(this, &CChooserPopup::RowSelected);
	}
	if (mpPopup.IsNotNull())
	{
		mcSelected.Call(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The row showing item uiItem, -1 if none does.
//
//////////////////////////////////////////////////////////////////////////
int32 CChooserPopup::FindItemRow(size uiItem)
{
	size	i;

	for (i = 0; i < maiRows.NumElements(); i++)
	{
		if ((size)maiRows.GetValue(i) == uiItem)
		{
			return (int32)i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The first row that isn't a heading, -1 if there is none.
//
//////////////////////////////////////////////////////////////////////////
int32 CChooserPopup::FindFirstRow(void)
{
	size	i;

	for (i = 0; i < maiRows.NumElements(); i++)
	{
		if (mcItems.Get((size)maiRows.GetValue(i))->uiData != CHOOSER_HEADING)
		{
			return (int32)i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The field's text, as typed: the rows are filtered by it.  A driven
// chooser has no field: it is filtered by the text, and as tall as the
// rows left need.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::SetFilterText(const char* szText)
{
	if (mpField.IsNotNull())
	{
		mpField->SetText(szText);
	}
	else if (mbDriven && !mszFilter.Equals(szText))
	{
		mszFilter.Set(szText);
		if (mpPanel.IsNotNull())
		{
			Filter();
			if (mpPopup.IsNotNull())
			{
				Resize();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What the rows are filtered by: the field's text, or a driven chooser's.
//
//////////////////////////////////////////////////////////////////////////
const char* CChooserPopup::GetPattern(void)
{
	if (mpField.IsNotNull())
	{
		return mpField->GetText();
	}
	return mszFilter.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// The item row uiRow shows, NULL if there is no such row.
//
//////////////////////////////////////////////////////////////////////////
SGoToItem* CChooserPopup::GetRowItem(size uiRow)
{
	if (uiRow >= maiRows.NumElements())
	{
		return NULL;
	}
	return mcItems.Get((size)maiRows.GetValue(uiRow));
}


//////////////////////////////////////////////////////////////////////////
//
// The selected row's item, NULL if none is selected.
//
//////////////////////////////////////////////////////////////////////////
SGoToItem* CChooserPopup::GetSelectedItem(void)
{
	int32	iRow;

	iRow = mpList->GetSelected();
	return (iRow < 0) ? NULL : GetRowItem((size)iRow);
}


//////////////////////////////////////////////////////////////////////////
//
// The item chosen, NULL if none was since it was last opened.
//
//////////////////////////////////////////////////////////////////////////
SGoToItem* CChooserPopup::GetChosen(void)
{
	if ((miChosen < 0) || ((size)miChosen >= mcItems.NumItems()))
	{
		return NULL;
	}
	return mcItems.Get((size)miChosen);
}


//////////////////////////////////////////////////////////////////////////
//
// Enter, or a click on a row: the selected item is chosen, the chooser
// closed and the chosen event called.  False, and it stays open, if no
// row that can be chosen is selected.
//
//////////////////////////////////////////////////////////////////////////
bool CChooserPopup::ChooseSelected(void)
{
	SGoToItem*	psItem;
	int32		iRow;

	iRow = mpList->GetSelected();
	psItem = (iRow < 0) ? NULL : GetRowItem((size)iRow);
	if ((psItem == NULL) || (psItem->uiData == CHOOSER_HEADING))
	{
		return false;
	}
	miChosen = maiRows.GetValue((size)iRow);
	Close();
	mcChosen.Call(this);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::RowSelected(void)
{
	mcSelected.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::RowActivated(void)
{
	ChooseSelected();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::ButtonClicked(void)
{
	Close();
	mcButtonClicked.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The keys typed into the field move the rows' selection, as a Go to
// dialog's do, and enter chooses.  A driven chooser's are called by what
// has the keys.
//
//////////////////////////////////////////////////////////////////////////
void CChooserPopup::Up(void) { mpList->Up(); }
void CChooserPopup::Down(void) { mpList->Down(); }
void CChooserPopup::PageUp(void) { mpList->PageUp(); }
void CChooserPopup::PageDown(void) { mpList->PageDown(); }
void CChooserPopup::Enter(void) { ChooseSelected(); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChooserPopup::IsOpen(void) const { return mpPopup.IsNotNull(); }
size CChooserPopup::NumItems(void) const { return mcItems.NumItems(); }
SGoToItem* CChooserPopup::GetItem(size uiIndex) { return mcItems.Get(uiIndex); }
size CChooserPopup::NumRows(void) const { return maiRows.NumElements(); }
bool CChooserPopup::SelectRow(int32 iRow) { return mpList->Select(iRow); }
int32 CChooserPopup::GetSelectedRow(void) { return mpList->GetSelected(); }
const char* CChooserPopup::GetFilterText(void) { return GetPattern(); }
bool CChooserPopup::IsDriven(void) const { return mbDriven; }
const char* CChooserPopup::GetNote(void) const { return mszNote.Text(); }
const char* CChooserPopup::GetTitle(void) const { return mszTitle.Text(); }
Ptr<CComponent> CChooserPopup::GetPanel(void) const { return mpPanel; }
Ptr<CTextField> CChooserPopup::GetField(void) const { return mpField; }
Ptr<CListContainer> CChooserPopup::GetList(void) const { return mpList; }
Ptr<CCodePreview> CChooserPopup::GetPreview(void) const { return mpPreview; }
Ptr<CButton> CChooserPopup::GetButton(void) const { return mpButton; }
Ptr<CPopup> CChooserPopup::GetPopup(void) const { return mpPopup; }
CComponentEvent<CChooserPopup>* CChooserPopup::GetChosenEvent(void) { return &mcChosen; }
CComponentEvent<CChooserPopup>* CChooserPopup::GetSelectedEvent(void) { return &mcSelected; }
CComponentEvent<CChooserPopup>* CChooserPopup::GetButtonClickEvent(void) { return &mcButtonClicked; }
CComponentEvent<CChooserPopup>* CChooserPopup::GetClosedEvent(void) { return &mcClosed; }

