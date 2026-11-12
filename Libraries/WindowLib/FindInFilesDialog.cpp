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
#include "BaseLib/StringHelper.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "FillContainer.h"
#include "GridContainer.h"
#include "RoundedBoxCanvasDraw.h"
#include "FindInFilesDialog.h"


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesItems::Init(void)
{
	masItems.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesItems::Kill(void)
{
	Clear();
	masItems.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesItems::Clear(void)
{
	size				i;
	SFindInFilesItem*	psItem;

	for (i = 0; i < masItems.NumElements(); i++)
	{
		psItem = masItems.Get(i);
		psItem->szText.Kill();
		psItem->szLocation.Kill();
	}
	masItems.RemoveRange(0, masItems.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
// The text found is uiMatchLength of szText from uiMatchStart, kept
// within it.
//
//////////////////////////////////////////////////////////////////////////
SFindInFilesItem* CFindInFilesItems::Add(const char* szText, size uiMatchStart, size uiMatchLength, const char* szLocation, size uiFile, size uiData)
{
	SFindInFilesItem*	psItem;
	size				uiLength;

	psItem = masItems.Add();
	psItem->szText.Init(szText);
	psItem->szLocation.Init(szLocation == NULL ? "" : szLocation);
	uiLength = psItem->szText.Length();
	psItem->uiMatchStart = (uiMatchStart < uiLength) ? uiMatchStart : uiLength;
	psItem->uiMatchLength = (uiMatchLength < uiLength - psItem->uiMatchStart) ? uiMatchLength : uiLength - psItem->uiMatchStart;
	psItem->uiFile = uiFile;
	psItem->uiData = uiData;
	return psItem;
}


//////////////////////////////////////////////////////////////////////////
//
// How many files the results are in: a file's results come together.
//
//////////////////////////////////////////////////////////////////////////
size CFindInFilesItems::NumFiles(void)
{
	size	i;
	size	uiFiles;

	uiFiles = 0;
	for (i = 0; i < masItems.NumElements(); i++)
	{
		if ((i == 0) || (masItems.Get(i)->uiFile != masItems.Get(i - 1)->uiFile))
		{
			uiFiles++;
		}
	}
	return uiFiles;
}


size CFindInFilesItems::NumItems(void) { return masItems.NumElements(); }
SFindInFilesItem* CFindInFilesItems::Get(size uiIndex) { return masItems.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesSearcher::FindPreviewShown(SFindInFilesItem* psItem, Ptr<CTextArea> pPreview, bool bNewText)
{
}


//////////////////////////////////////////////////////////////////////////
//
// pMatchFont draws the text found in a result's line; the rest of the
// line is in the look's font and where it is in its dim font.  The
// preview is in pCodeFamily's font.  The searcher isn't held.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle, CFindInFilesSearcher* pcSearcher, Ptr<CFont> pMatchFont, Ptr<CFontFamily> pCodeFamily)
{
	Ptr<CComponent>	pBody;

	PreInit();

	mpcSearcher = pcSearcher;
	mpMatchFont = pMatchFont;
	mpCodeFamily = pCodeFamily;
	mcItems.Init();
	mbSearching = false;
	mbMore = false;
	miChosen = -1;
	meResult = FIFR_Cancelled;
	maScopeContents.Init();
	maScopePages.Init();

	mpDialog = OMalloc<CDialog>(pOwner, psLook, szTitle);
	mpDialog->RemoveButtons();
	mpDialog->SetResizable(GetMetric(TM_FindWidth) / 2, GetMetric(TM_FindHeight) / 2);
	mpDialog->SetApprover(this, &CFindInFilesDialog::Approve);
	//The body is made first: it makes the component that has the keys when the dialog opens.
	pBody = CreateBody();
	mpDialog->SetBody(pBody, mpField);

	mpTabs->GetSelectionEvent()->Add(this, &CFindInFilesDialog::ScopeSelected);
	mpField->GetChangeEvent()->Add(this, &CFindInFilesDialog::Search);
	mpMatchCase->GetChangeEvent()->Add(this, &CFindInFilesDialog::Search);
	mpWholeWords->GetChangeEvent()->Add(this, &CFindInFilesDialog::Search);
	mpList->GetActivationEvent()->Add(this, &CFindInFilesDialog::ResultActivated);
	mpList->GetSelectionEvent()->Add(this, &CFindInFilesDialog::ResultSelected);
	mpFindWindow->GetClickEvent()->Add(this, &CFindInFilesDialog::OpenInFindWindow);

	//The keys that move through the results while typing.
	mpField->AddInputAction(CIC_Up, this, &CFindInFilesDialog::Up);
	mpField->AddInputAction(CIC_Down, this, &CFindInFilesDialog::Down);
	mpField->AddInputAction(CIC_PageUp, this, &CFindInFilesDialog::PageUp);
	mpField->AddInputAction(CIC_PageDown, this, &CFindInFilesDialog::PageDown);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::Class(void)
{
	M_Pointer(mpDialog);
	M_Pointer(mpTabs);
	M_Pointer(mpScopeRow);
	M_Embedded(maScopeContents);
	M_Embedded(maScopePages);
	M_Pointer(mpField);
	M_Pointer(mpMatchCase);
	M_Pointer(mpWholeWords);
	M_Pointer(mpDirectory);
	M_Pointer(mpScrollPane);
	M_Pointer(mpList);
	M_Pointer(mpPreview);
	M_Pointer(mpStatus);
	M_Pointer(mpFindWindow);
	M_Pointer(mpMatchFont);
	M_Pointer(mpCodeFamily);
	U_Pointer(mpcSearcher);
	U_Unknown(CFindInFilesItems, mcItems);
	U_Bool(mbSearching);
	U_Bool(mbMore);
	U_Int32(miChosen);
	U_Enum(meResult);
}


//////////////////////////////////////////////////////////////////////////
//
// An object's memory is freed without its destructor running so the
// dialog takes itself off the widgets still here, and its step isn't
// called.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::Free(void)
{
	if (mpDialog.IsNotNull())
	{
		mpDialog->ClearApprover();
		if (mpDialog->GetWindow().IsNotNull())
		{
			mpDialog->GetWindow()->CancelPosted(this);
		}
	}
	if (mpTabs.IsNotNull())
	{
		mpTabs->GetSelectionEvent()->Remove(this);
	}
	if (mpField.IsNotNull())
	{
		mpField->GetChangeEvent()->Remove(this);
	}
	if (mpMatchCase.IsNotNull())
	{
		mpMatchCase->GetChangeEvent()->Remove(this);
	}
	if (mpWholeWords.IsNotNull())
	{
		mpWholeWords->GetChangeEvent()->Remove(this);
	}
	if (mpDirectory.IsNotNull())
	{
		mpDirectory->GetChangeEvent()->Remove(this);
	}
	if (mpList.IsNotNull())
	{
		mpList->GetActivationEvent()->Remove(this);
		mpList->GetSelectionEvent()->Remove(this);
	}
	if (mpFindWindow.IsNotNull())
	{
		mpFindWindow->GetClickEvent()->Remove(this);
	}
	mcItems.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CFindInFilesDialog::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CFindInFilesDialog::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CFindInFilesDialog::CreateGap(void)
{
	Ptr<CFillContainer>	pGap;

	pGap = OMalloc<CFillContainer>(mpDialog->GetWindow());
	pGap->SetDesiredSize(GetMetric(TM_GoToGap), GetMetric(TM_GoToGap));
	return pGap;
}


//////////////////////////////////////////////////////////////////////////
//
// The scopes' tabs along the top (each with its page: what it searches,
// or the directory to type), the text to find with its options, the
// results, the selected result's file, and along the bottom how many were
// found and Open in Find Window.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CFindInFilesDialog::CreateBody(void)
{
	Ptr<CWindow>			pWindow;
	SDialogLook*			psLook;
	Ptr<CWindowTheme>		pTheme;
	Ptr<CGridContainer>		pBody;
	Ptr<CGridContainer>		pFind;
	Ptr<CGridContainer>		pBottom;
	SInsets					sInsets;

	pWindow = mpDialog->GetWindow();
	psLook = mpDialog->GetLook();
	pTheme = psLook->pcTheme;
	pBody = OMalloc<CGridContainer>(pWindow);

	mpTabs = OMalloc<CTabbedContainer>(pWindow, pTheme);
	mpTabs->SetAlignment(CSH_Stretch, CSV_Top);
	pBody->AddComponent(mpTabs, 0, 0, 1, 1, 1, 0);
	mpScopeRow = OMalloc<CGridContainer>(pWindow);
	mpScopeRow->SetAlignment(CSH_Stretch, CSV_Center);
	mpScopeRow->SetInsets(GetMetric(TM_GoToGap), 0, GetMetric(TM_GoToGap), 0);
	pBody->AddComponent(mpScopeRow, 0, 1, 1, 1, 1, 0);

	//The text and, beside it, how it must match.
	pFind = OMalloc<CGridContainer>(pWindow);
	pFind->SetAlignment(CSH_Stretch, CSV_Center);
	mpField = OMalloc<CTextField>(pWindow, pTheme, "");
	mpField->SetColumns(10);
	mpField->SetAlignment(CSH_Stretch, CSV_Center);
	pFind->AddComponent(mpField, 0, 0, 1, 1, 1, 0);
	pFind->AddComponent(CreateGap(), 1, 0);
	mpMatchCase = OMalloc<CCheckBox>(pWindow, pTheme, "Match case", false);
	mpMatchCase->SetAlignment(CSH_Left, CSV_Center);
	pFind->AddComponent(mpMatchCase, 2, 0);
	pFind->AddComponent(CreateGap(), 3, 0);
	mpWholeWords = OMalloc<CCheckBox>(pWindow, pTheme, "Words", false);
	mpWholeWords->SetAlignment(CSH_Left, CSV_Center);
	pFind->AddComponent(mpWholeWords, 4, 0);
	pBody->AddComponent(pFind, 0, 2, 1, 1, 1, 0);
	pBody->AddComponent(CreateGap(), 0, 3);

	//The results, never scrolled sideways: a row is as wide as the view and shares it between its parts.
	sInsets.Init(4, 0);
	mpList = OMalloc<CListContainer>(pWindow, pTheme);
	mpList->SetHighlightDraws(OMalloc<CRoundedBoxCanvasDraw>(pTheme->GetColour(TC_ListSelected), 4, &sInsets), OMalloc<CRoundedBoxCanvasDraw>(pTheme->GetColour(TC_ListHover), 4, &sInsets));
	mpList->SetAlignment(CSH_Stretch, CSV_Top);
	mpList->SetDoubleClickActivates(true);
	mpList->SetWrapAround(true);
	mpScrollPane = OMalloc<CScrollPane>(pWindow, pTheme);
	mpScrollPane->SetPolicy(SBO_Horizontal, SPP_Never);
	mpScrollPane->SetContent(mpList);
	mpScrollPane->SetAlignment(CSH_Stretch, CSV_Stretch);
	pBody->AddComponent(mpScrollPane, 0, 4, 1, 1, 1, 1);
	pBody->AddComponent(CreateGap(), 0, 5);

	//The selected result's file, and its code.
	mpPreview = OMalloc<CCodePreview>(pWindow, pTheme, psLook->pcDimFont, mpCodeFamily);
	mpPreview->SetAlignment(CSH_Stretch, CSV_Stretch);
	pBody->AddComponent(mpPreview, 0, 6, 1, 1, 1, 1);
	pBody->AddComponent(CreateGap(), 0, 7);

	//How many were found, and Open in Find Window.
	pBottom = OMalloc<CGridContainer>(pWindow);
	pBottom->SetAlignment(CSH_Stretch, CSV_Center);
	mpStatus = OMalloc<CLabel>(pWindow, psLook->pcDimFont, " ");
	mpStatus->SetAlignment(CSH_Left, CSV_Center);
	pBottom->AddComponent(mpStatus, 0, 0, 1, 1, 1, 0);
	mpFindWindow = OMalloc<CButton>(pWindow, pTheme, "Open in Find Window");
	mpFindWindow->SetAlignment(CSH_Right, CSV_Center);
	pBottom->AddComponent(mpFindWindow, 1, 0);
	pBody->AddComponent(pBottom, 0, 8, 1, 1, 1, 0);
	return pBody;
}


//////////////////////////////////////////////////////////////////////////
//
// A tab for a scope, with pContent shown under the tabs while it is
// chosen.
//
//////////////////////////////////////////////////////////////////////////
size CFindInFilesDialog::AddScopePage(const char* szName, Ptr<CComponent> pContent)
{
	Ptr<CFillContainer>	pPage;

	//The tabs' own pages are empty: the scopes share what is under them, and the chosen one's content is shown there.
	pPage = OMalloc<CFillContainer>(mpDialog->GetWindow());
	pPage->SetDesiredSize(1, 0);
	maScopeContents.Add(pContent);
	maScopePages.Add(pPage);
	mpTabs->AddTab(szName, pPage);
	if (maScopeContents.NumElements() == 1)
	{
		ShowScopeContent();
	}
	return maScopePages.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Under the tabs is the chosen scope's content.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::ShowScopeContent(void)
{
	size	uiScope;

	uiScope = GetScope();
	mpScopeRow->RemoveAllComponents();
	if (uiScope < maScopeContents.NumElements())
	{
		mpScopeRow->AddComponent(maScopeContents.Get(uiScope), 0, 0, 1, 1, 1, 0);
	}
	mpScopeRow->InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// A tab for the searcher's scope, the index returned, with what it
// searches on its page.  Scopes are added before the dialog is started.
//
//////////////////////////////////////////////////////////////////////////
size CFindInFilesDialog::AddScope(const char* szName, const char* szDescription)
{
	Ptr<CLabel>	pLabel;

	pLabel = OMalloc<CLabel>(mpDialog->GetWindow(), mpDialog->GetLook()->pcDimFont, szDescription);
	pLabel->SetAlignment(CSH_Left, CSV_Center);
	return AddScopePage(szName, pLabel);
}


//////////////////////////////////////////////////////////////////////////
//
// A tab for a scope that searches the directory typed on its page (given
// to the searcher with the query).  A dialog has one such scope.
//
//////////////////////////////////////////////////////////////////////////
size CFindInFilesDialog::AddDirectoryScope(const char* szName, const char* szDirectory)
{
	if (mpDirectory.IsNotNull())
	{
		mpDirectory->GetChangeEvent()->Remove(this);
	}
	mpDirectory = OMalloc<CTextField>(mpDialog->GetWindow(), mpDialog->GetLook()->pcTheme, szDirectory);
	mpDirectory->SetColumns(10);
	mpDirectory->SetAlignment(CSH_Stretch, CSV_Center);
	mpDirectory->GetChangeEvent()->Add(this, &CFindInFilesDialog::Search);
	return AddScopePage(szName, mpDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::SetOptions(bool bMatchCase, bool bWholeWords)
{
	mpMatchCase->GetChangeEvent()->Remove(this);
	mpWholeWords->GetChangeEvent()->Remove(this);
	mpMatchCase->SetTicked(bMatchCase);
	mpWholeWords->SetTicked(bWholeWords);
	mpMatchCase->GetChangeEvent()->Add(this, &CFindInFilesDialog::Search);
	mpWholeWords->GetChangeEvent()->Add(this, &CFindInFilesDialog::Search);
}


//////////////////////////////////////////////////////////////////////////
//
// The scope chosen and the text in the field, selected so typing replaces
// it, as IntelliJ remembers what was last typed; the search starts.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::Start(size uiScope, const char* szText)
{
	miChosen = -1;
	meResult = FIFR_Cancelled;
	mpTabs->GetSelectionEvent()->Remove(this);
	SelectScope(uiScope);
	mpTabs->GetSelectionEvent()->Add(this, &CFindInFilesDialog::ScopeSelected);
	ShowScopeContent();
	mpField->GetChangeEvent()->Remove(this);
	mpField->SetText(szText);
	mpField->ClearUndo();
	mpField->GetChangeEvent()->Add(this, &CFindInFilesDialog::Search);
	mpField->SelectAll();
	Search();
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the dialog modally (from a tick, see CWindow::ShowModal).  How it
// was closed.
//
//////////////////////////////////////////////////////////////////////////
EFindInFilesResult CFindInFilesDialog::Show(void)
{
	bool	bOK;

	bOK = mpDialog->Show(GetMetric(TM_FindWidth), GetMetric(TM_FindHeight));
	mpDialog->GetWindow()->CancelPosted(this);
	if (meResult == FIFR_FindWindow)
	{
		return FIFR_FindWindow;
	}
	meResult = (bOK && (miChosen >= 0)) ? FIFR_Chosen : FIFR_Cancelled;
	return meResult;
}


//////////////////////////////////////////////////////////////////////////
//
// As typed.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::SetText(const char* szText)
{
	mpField->SetText(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::SetDirectory(const char* szDirectory)
{
	if (mpDirectory.IsNotNull())
	{
		mpDirectory->SetText(szDirectory);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::SelectScope(size uiScope)
{
	size	uiTab;

	if (uiScope < maScopePages.NumElements())
	{
		uiTab = mpTabs->IndexOfPage(maScopePages.Get(uiScope));
		if (uiTab != ARRAY_ELEMENT_NOT_FOUND)
		{
			mpTabs->Select(uiTab);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A new search for the field's text in the scope chosen: the results go,
// and the first of the new ones are found.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::Search(void)
{
	SFindInFilesQuery	sQuery;

	mcItems.Clear();
	mpList->RemoveAllComponents();
	mpList->Select(-1);
	mpScrollPane->SetScrollPosition(0, 0);
	ClearPreview();
	miChosen = -1;
	mbMore = false;

	sQuery.uiScope = GetScope();
	sQuery.szText = mpField->GetText();
	sQuery.szDirectory = GetDirectory();
	sQuery.bMatchCase = mpMatchCase->IsTicked();
	sQuery.bWholeWords = mpWholeWords->IsTicked();
	mbSearching = (mpcSearcher != NULL) && !StrEmpty(sQuery.szText);
	if (mbSearching)
	{
		mpcSearcher->FindStart(&sQuery);
		Step();
	}
	else
	{
		ShowStatus();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The searcher searches a little more, and what it found is listed, the
// first result selected.  While it has more to search, and the list isn't
// full, the next step is at the next tick.  False once the search is over.
//
//////////////////////////////////////////////////////////////////////////
bool CFindInFilesDialog::Step(void)
{
	size	uiListed;
	size	i;

	if (!mbSearching)
	{
		return false;
	}
	uiListed = (size)mpList->NumRows();
	mbSearching = mpcSearcher->FindStep(&mcItems);
	for (i = uiListed; (i < mcItems.NumItems()) && (i < FIND_IN_FILES_DIALOG_ROWS); i++)
	{
		AddRow(mcItems.Get(i));
	}
	if (mcItems.NumItems() >= FIND_IN_FILES_DIALOG_ROWS)
	{
		mbMore = mbSearching || (mcItems.NumItems() > FIND_IN_FILES_DIALOG_ROWS);
		mbSearching = false;
	}
	if ((uiListed == 0) && (mpList->NumRows() > 0))
	{
		mpList->Select(0);
	}
	ShowStatus();
	if (mbSearching)
	{
		mpDialog->GetWindow()->Post(this, &CFindInFilesDialog::PostedStep);
	}
	return mbSearching;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::PostedStep(void)
{
	Step();
}


//////////////////////////////////////////////////////////////////////////
//
// Searches to the end, or until the list is full, as a test wants.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::Finish(void)
{
	while (Step())
	{
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many were found, as IntelliJ's "100+ matches in 12+ files".
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::ShowStatus(void)
{
	CChars	szStatus;
	size	uiItems;
	size	uiFiles;

	szStatus.Init();
	uiItems = mcItems.NumItems();
	if (uiItems > FIND_IN_FILES_DIALOG_ROWS)
	{
		uiItems = FIND_IN_FILES_DIALOG_ROWS;
	}
	if (uiItems > 0)
	{
		uiFiles = mcItems.NumFiles();
		szStatus.Append((uint32)uiItems);
		szStatus.Append(mbMore ? "+" : "");
		szStatus.Append((uiItems == 1) && !mbMore ? " match in " : " matches in ");
		szStatus.Append((uint32)uiFiles);
		szStatus.Append(mbMore ? "+" : "");
		szStatus.Append((uiFiles == 1) && !mbMore ? " file" : " files");
		if (mbSearching)
		{
			szStatus.Append(", searching...");
		}
	}
	else if (mbSearching)
	{
		szStatus.Append("Searching...");
	}
	else if (!StrEmpty(mpField->GetText()))
	{
		szStatus.Append("Nothing found");
	}
	if (!szStatus.Equals(mpStatus->GetText()))
	{
		mpStatus->SetText(szStatus.Text());
	}
	szStatus.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// One of the theme's sizes.
//
//////////////////////////////////////////////////////////////////////////
int32 CFindInFilesDialog::GetMetric(EThemeMetric eMetric)
{
	return mpDialog->GetLook()->pcTheme->GetMetric(eMetric);
}


//////////////////////////////////////////////////////////////////////////
//
// A result's row: its line on the left and where it is, dimmer, on the
// right, each clipped to its part of the row.  The row is a grid of its
// own: the line, a gap, a space that takes what is left over, and where
// it is.  With too little room the space has nothing to give, so the grid
// shrinks the parts in proportion to their widths: they are sized by
// what this row holds, not by the other rows.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::AddRow(SFindInFilesItem* psItem)
{
	Ptr<CWindow>			pWindow;
	Ptr<CGridContainer>		pRow;
	Ptr<CFillContainer>		pLine;
	Ptr<CFillContainer>		pGap;
	Ptr<CFillContainer>		pSpace;
	Ptr<CFillContainer>		pWhere;
	Ptr<CLabel>				pLocation;
	int32					iRow;
	int32					iLineWidth;
	int32					iGap;

	pWindow = mpDialog->GetWindow();
	iGap = GetMetric(TM_GoToGap);

	pLine = OMalloc<CFillContainer>(pWindow);
	pLine->SetAlignment(CSH_Stretch, CSV_Stretch);
	pLine->AddComponent(CreateLine(psItem, &iLineWidth));

	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(iGap * 2, 0);
	pSpace = OMalloc<CFillContainer>(pWindow);
	pSpace->SetDesiredSize(0, 0);
	pSpace->SetAlignment(CSH_Stretch, CSV_Stretch);

	pLocation = OMalloc<CLabel>(pWindow, mpDialog->GetLook()->pcDimFont, psItem->szLocation.Text());
	pLocation->SetAlignment(CSH_Right, CSV_Center);
	pWhere = OMalloc<CFillContainer>(pWindow);
	pWhere->SetAlignment(CSH_Stretch, CSV_Stretch);
	pWhere->AddComponent(pLocation);

	pRow = OMalloc<CGridContainer>(pWindow);
	pRow->SetAlignment(CSH_Stretch, CSV_Center);
	pRow->SetInsets(0, iGap * 2, 0, iGap);
	pRow->AddComponent(pLine, 0, 0);
	pRow->AddComponent(pGap, 1, 0);
	pRow->AddComponent(pSpace, 2, 0, 1, 1, 1, 0);
	pRow->AddComponent(pWhere, 3, 0);
	pRow->SetDesiredSize(iGap + iLineWidth + iGap * 2 + pLocation->GetDesiredSize().x + iGap * 2, GetMetric(TM_GoToRowHeight));

	iRow = mpList->AddRow(true);
	mpList->AddToRow(iRow, pRow, 0, 1, 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The line as a label for the text found, in the match font, and for
// what is before and after it, in the look's font.  Its width.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CFindInFilesDialog::CreateLine(SFindInFilesItem* psItem, int32* piWidth)
{
	Ptr<CWindow>			pWindow;
	Ptr<CGridContainer>		pLine;
	Ptr<CLabel>				pLabel;
	Ptr<CFont>				pFont;
	CChars					szPiece;
	size					auiEnds[3];
	size					uiStart;
	size					i;
	int						iColumn;
	int32					iHeight;

	pWindow = mpDialog->GetWindow();
	pFont = mpDialog->GetLook()->pcTheme->GetFont();
	pLine = OMalloc<CGridContainer>(pWindow);
	pLine->SetAlignment(CSH_Left, CSV_Center);

	auiEnds[0] = psItem->uiMatchStart;
	auiEnds[1] = psItem->uiMatchStart + psItem->uiMatchLength;
	auiEnds[2] = psItem->szText.Length();
	uiStart = 0;
	iColumn = 0;
	iHeight = 0;
	*piWidth = 0;
	for (i = 0; i < 3; i++)
	{
		if (auiEnds[i] > uiStart)
		{
			szPiece.Init();
			szPiece.AppendSubString(psItem->szText.Text(), uiStart, auiEnds[i]);
			pLabel = OMalloc<CLabel>(pWindow, ((i == 1) && mpMatchFont.IsNotNull()) ? mpMatchFont : pFont, szPiece.Text());
			szPiece.Kill();
			pLabel->SetAlignment(CSH_Left, CSV_Center);
			pLine->AddComponent(pLabel, iColumn, 0);
			*piWidth += pLabel->GetDesiredSize().x;
			if (pLabel->GetDesiredSize().y > iHeight)
			{
				iHeight = pLabel->GetDesiredSize().y;
			}
			iColumn++;
			uiStart = auiEnds[i];
		}
	}

	//A grid in a container is the size it's given, not the size its parts need.
	pLine->SetDesiredSize(*piWidth, iHeight);
	return pLine;
}


//////////////////////////////////////////////////////////////////////////
//
// The selected result's file under the results, at the line it is on,
// the text found coloured.  A file is only read when it isn't the one
// shown.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::ShowPreview(void)
{
	SFindInFilesItem*	psItem;
	CChars				szTitle;
	CChars				szText;
	size				uiStart;
	size				uiLength;
	int32				iRow;
	bool				bNewText;

	iRow = mpList->GetSelected();
	if ((iRow < 0) || ((size)iRow >= mcItems.NumItems()) || (mpcSearcher == NULL))
	{
		ClearPreview();
		return;
	}
	psItem = mcItems.Get((size)iRow);
	szTitle.Init();
	uiStart = 0;
	uiLength = 0;
	if (!mpcSearcher->FindPreviewPlace(psItem, &szTitle, &uiStart, &uiLength))
	{
		szTitle.Kill();
		ClearPreview();
		return;
	}

	bNewText = !mpPreview->IsShowing(szTitle.Text());
	if (bNewText)
	{
		szText.Init();
		mpcSearcher->FindPreviewText(psItem, &szText);
		mpPreview->SetCode(szTitle.Text(), szText.Text());
		szText.Kill();
	}
	szTitle.Kill();
	mpPreview->ShowPlace(uiStart, uiLength);

	mpcSearcher->FindPreviewShown(psItem, mpPreview->GetTextArea(), bNewText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::ClearPreview(void)
{
	mpPreview->Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// Enter chooses the selected result; with none it does nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CFindInFilesDialog::Approve(void)
{
	int32	iRow;

	iRow = mpList->GetSelected();
	if ((iRow < 0) || ((size)iRow >= mcItems.NumItems()))
	{
		return false;
	}
	miChosen = iRow;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A double click on a result is OK.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::ResultActivated(void)
{
	mpDialog->OKClicked();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::ResultSelected(void)
{
	ShowPreview();
}


//////////////////////////////////////////////////////////////////////////
//
// Another scope searches for the text typed.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::ScopeSelected(void)
{
	ShowScopeContent();
	Search();
	mpDialog->GetWindow()->GetFocus()->SetFocus(mpField);
}


//////////////////////////////////////////////////////////////////////////
//
// The dialog closes for its owner to list everything the query finds.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::OpenInFindWindow(void)
{
	if (StrEmpty(mpField->GetText()))
	{
		return;
	}
	meResult = FIFR_FindWindow;
	mpDialog->Close("Open in Find Window");
}


//////////////////////////////////////////////////////////////////////////
//
// The keys typed into the field move the results' selection: the list
// keeps it in view, and up from the first result goes to the last and
// down from the last to the first, as IntelliJ's.
//
//////////////////////////////////////////////////////////////////////////
void CFindInFilesDialog::Up(void) { mpList->Up(); }
void CFindInFilesDialog::Down(void) { mpList->Down(); }
void CFindInFilesDialog::PageUp(void) { mpList->PageUp(); }
void CFindInFilesDialog::PageDown(void) { mpList->PageDown(); }


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
size CFindInFilesDialog::GetScope(void)
{
	Ptr<CComponent>	pPage;
	size			uiTab;
	size			i;

	uiTab = mpTabs->IndexOf(mpTabs->GetSelected());
	if (uiTab == ARRAY_ELEMENT_NOT_FOUND)
	{
		return 0;
	}
	pPage = mpTabs->GetPage(uiTab);
	for (i = 0; i < maScopePages.NumElements(); i++)
	{
		if (maScopePages.Get(i) == pPage)
		{
			return i;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The result chosen, NULL if none was.
//
//////////////////////////////////////////////////////////////////////////
SFindInFilesItem* CFindInFilesDialog::GetChosen(void)
{
	if ((miChosen < 0) || ((size)miChosen >= mcItems.NumItems()))
	{
		return NULL;
	}
	return mcItems.Get((size)miChosen);
}


EFindInFilesResult CFindInFilesDialog::GetOutcome(void) { return meResult; }  //How it was last closed, as Show returned.
char* CFindInFilesDialog::GetText(void) { return mpField->GetText(); }
char* CFindInFilesDialog::GetDirectory(void) { return mpDirectory.IsNotNull() ? mpDirectory->GetText() : (char*)""; }
bool CFindInFilesDialog::IsMatchCase(void) { return mpMatchCase->IsTicked(); }
bool CFindInFilesDialog::IsWholeWords(void) { return mpWholeWords->IsTicked(); }
bool CFindInFilesDialog::IsSearching(void) { return mbSearching; }
bool CFindInFilesDialog::HasMore(void) { return mbMore; }
size CFindInFilesDialog::NumResults(void) { return mcItems.NumItems(); }
SFindInFilesItem* CFindInFilesDialog::GetResult(size uiIndex) { return mcItems.Get(uiIndex); }
int32 CFindInFilesDialog::GetSelected(void) { return mpList->GetSelected(); }
char* CFindInFilesDialog::GetPreviewTitle(void) { return mpPreview->GetTitle(); }
char* CFindInFilesDialog::GetStatus(void) { return mpStatus->GetText(); }
Ptr<CTextField> CFindInFilesDialog::GetField(void) { return mpField; }
Ptr<CTextField> CFindInFilesDialog::GetDirectoryField(void) { return mpDirectory; }
Ptr<CCheckBox> CFindInFilesDialog::GetMatchCaseBox(void) { return mpMatchCase; }
Ptr<CCheckBox> CFindInFilesDialog::GetWholeWordsBox(void) { return mpWholeWords; }
Ptr<CListContainer> CFindInFilesDialog::GetList(void) { return mpList; }
Ptr<CCodeEditor> CFindInFilesDialog::GetPreview(void) { return mpPreview->GetEditor(); }
Ptr<CCodePreview> CFindInFilesDialog::GetCodePreview(void) { return mpPreview; }
Ptr<CButton> CFindInFilesDialog::GetFindWindowButton(void) { return mpFindWindow; }
Ptr<CDialog> CFindInFilesDialog::GetDialog(void) { return mpDialog; }

