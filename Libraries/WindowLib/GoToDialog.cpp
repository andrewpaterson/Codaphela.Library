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
#include "BaseLib/StringHelper.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "FillContainer.h"
#include "GridContainer.h"
#include "RoundedBoxCanvasDraw.h"
#include "GoToDialog.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToItems::Init(void)
{
	masItems.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToItems::Kill(void)
{
	Clear();
	masItems.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToItems::Clear(void)
{
	size		i;
	SGoToItem*	psItem;

	for (i = 0; i < masItems.NumElements(); i++)
	{
		psItem = masItems.Get(i);
		psItem->szName.Kill();
		psItem->szLocation.Kill();
	}
	masItems.RemoveRange(0, masItems.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
// psMatch is the matcher's match of the name.
//
//////////////////////////////////////////////////////////////////////////
SGoToItem* CGoToItems::Add(const char* szName, const char* szLocation, int32 iIcon, size uiData, SNameMatch* psMatch)
{
	SGoToItem*	psItem;

	psItem = masItems.Add();
	psItem->szName.Init(szName);
	psItem->szLocation.Init(szLocation == NULL ? "" : szLocation);
	psItem->iIcon = iIcon;
	psItem->uiData = uiData;
	memcpy(&psItem->sMatch, psMatch, sizeof(SNameMatch));
	return psItem;
}


//////////////////////////////////////////////////////////////////////////
//
// The best score first, then by name, then by where it is.
//
//////////////////////////////////////////////////////////////////////////
static int __cdecl CompareGoToItems(const void* pv1, const void* pv2)
{
	SGoToItem*	ps1;
	SGoToItem*	ps2;
	int			iResult;

	ps1 = (SGoToItem*)pv1;
	ps2 = (SGoToItem*)pv2;
	if (ps1->sMatch.iScore != ps2->sMatch.iScore)
	{
		return (ps1->sMatch.iScore > ps2->sMatch.iScore) ? -1 : 1;
	}
	iResult = StringInsensitiveCompare(ps1->szName.Text(), ps2->szName.Text());
	if (iResult != 0)
	{
		return iResult;
	}
	return StringCompare(ps1->szLocation.Text(), ps2->szLocation.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToItems::Sort(void)
{
	if (masItems.NumElements() > 1)
	{
		qsort(masItems.GetData(), masItems.NumElements(), sizeof(SGoToItem), CompareGoToItems);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGoToItems::NumItems(void) { return masItems.NumElements(); }
SGoToItem* CGoToItems::Get(size uiIndex) { return masItems.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// pMatchFont draws the characters the pattern matched; the rest of a
// name is in the look's font and where it is in its dim font.  The
// searcher isn't held.
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle, CGoToSearcher* pcSearcher, Ptr<CFont> pMatchFont)
{
	Ptr<CComponent>	pBody;

	PreInit();

	mpcSearcher = pcSearcher;
	mpMatchFont = pMatchFont;
	maIcons.Init();
	mcItems.Init();
	mcMatcher.Init("");
	miChosen = -1;
	miLine = -1;

	mpDialog = OMalloc<CDialog>(pOwner, psLook, szTitle);
	mpDialog->RemoveButtons();
	mpDialog->SetResizable(GetMetric(TM_GoToWidth) / 2, GetMetric(TM_GoToHeight) / 2);
	mpDialog->SetApprover(this, &CGoToDialog::Approve);
	//The body is made first: it makes the component that has the keys when the dialog opens.
	pBody = CreateBody();
	mpDialog->SetBody(pBody, mpField);
	mpCommands = OMalloc<CCommandSet>();
	mpDialog->GetWindow()->SetCommands(mpCommands);

	mpTabs->GetSelectionEvent()->Add(this, &CGoToDialog::TabSelected);
	mpField->GetChangeEvent()->Add(this, &CGoToDialog::Search);
	mpList->GetActivationEvent()->Add(this, &CGoToDialog::ResultActivated);

	//The keys that move through the results and tabs while typing.
	mpField->AddInputAction(CIC_Up, this, &CGoToDialog::Up);
	mpField->AddInputAction(CIC_Down, this, &CGoToDialog::Down);
	mpField->AddInputAction(CIC_PageUp, this, &CGoToDialog::PageUp);
	mpField->AddInputAction(CIC_PageDown, this, &CGoToDialog::PageDown);
	mpField->AddInputAction(CIC_Tab, this, &CGoToDialog::NextTab);
	mpField->AddInputAction(CIC_BackTab, this, &CGoToDialog::PreviousTab);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::Class(void)
{
	M_Pointer(mpDialog);
	M_Pointer(mpTabs);
	M_Pointer(mpField);
	M_Pointer(mpScrollPane);
	M_Pointer(mpList);
	M_Pointer(mpMatchFont);
	M_Pointer(mpCommands);
	M_Embedded(maIcons);
	U_Pointer(mpcSearcher);
	U_Unknown(CGoToItems, mcItems);
	U_Unknown(CNameMatcher, mcMatcher);
	U_Int32(miChosen);
	U_Int32(miLine);
}


//////////////////////////////////////////////////////////////////////////
//
// An object's memory is freed without its destructor running so the
// dialog takes itself off the widgets still here.
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::Free(void)
{
	if (mpDialog.IsNotNull())
	{
		mpDialog->ClearApprover();
		if (mpDialog->GetWindow().IsNotNull())
		{
			mpDialog->GetWindow()->SetCommands(NULL);  //Their function is the dialog's.
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
	if (mpList.IsNotNull())
	{
		mpList->GetActivationEvent()->Remove(this);
	}
	mcMatcher.Kill();
	mcItems.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGoToDialog::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CGoToDialog::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The tabs along the top (each with an empty page: they all share the
// field and list under them), the field, and the results.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CGoToDialog::CreateBody(void)
{
	Ptr<CWindow>			pWindow;
	SDialogLook*			psLook;
	Ptr<CGridContainer>		pBody;
	Ptr<CFillContainer>		pGap;
	SInsets					sInsets;

	pWindow = mpDialog->GetWindow();
	psLook = mpDialog->GetLook();
	pBody = OMalloc<CGridContainer>(pWindow);

	mpTabs = OMalloc<CTabbedContainer>(pWindow, psLook->pcTheme);
	mpTabs->SetAlignment(CSH_Stretch, CSV_Top);
	pBody->AddComponent(mpTabs, 0, 0, 1, 1, 1, 0);

	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(GetMetric(TM_GoToGap), GetMetric(TM_GoToGap));
	pBody->AddComponent(pGap, 0, 1);

	mpField = OMalloc<CTextField>(pWindow, psLook->pcTheme, "");
	mpField->SetColumns(10);
	mpField->SetAlignment(CSH_Stretch, CSV_Center);
	pBody->AddComponent(mpField, 0, 2, 1, 1, 1, 0);

	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(GetMetric(TM_GoToGap), GetMetric(TM_GoToGap));
	pBody->AddComponent(pGap, 0, 3);

	sInsets.Init(4, 0);
	mpList = OMalloc<CListContainer>(pWindow, psLook->pcTheme);
	mpList->SetHighlightDraws(OMalloc<CRoundedBoxCanvasDraw>(psLook->pcTheme->GetColour(TC_ListSelected), 4, &sInsets), OMalloc<CRoundedBoxCanvasDraw>(psLook->pcTheme->GetColour(TC_ListHover), 4, &sInsets));
	mpList->SetAlignment(CSH_Stretch, CSV_Top);
	mpList->SetDoubleClickActivates(true);
	mpList->SetWrapAround(true);
	mpScrollPane = OMalloc<CScrollPane>(pWindow, psLook->pcTheme);
	mpScrollPane->SetContent(mpList);
	mpScrollPane->SetAlignment(CSH_Stretch, CSV_Stretch);
	pBody->AddComponent(mpScrollPane, 0, 4, 1, 1, 1, 1);
	return pBody;
}


//////////////////////////////////////////////////////////////////////////
//
// A tab for the searcher's uiTab, the index returned.  Tabs are added
// before the dialog is started.  pOpener is the application's command
// that opens the dialog on this tab, NULL for none: while the dialog is
// open its keys (which the application has bound; a modal dialog doesn't
// run its owner's commands) choose the tab instead.
//
//////////////////////////////////////////////////////////////////////////
size CGoToDialog::AddTab(const char* szName, Ptr<CCommand> pOpener)
{
	Ptr<CFillContainer>	pPage;
	Ptr<CCommand>		pCommand;
	CChars				szIndex;
	size				uiTab;
	size				i;

	pPage = OMalloc<CFillContainer>(mpDialog->GetWindow());
	pPage->SetDesiredSize(1, 0);
	mpTabs->AddTab(szName, pPage);
	uiTab = mpTabs->NumTabs() - 1;

	if (pOpener.IsNotNull() && (pOpener->NumKeys() > 0))
	{
		szIndex.Init();
		szIndex.Append((uint32)uiTab);
		pCommand = mpCommands->Add(szIndex.Text(), szName, NULL, Ptr<CImageCel>(), this, &CGoToDialog::TabCommand);
		szIndex.Kill();
		for (i = 0; i < pOpener->NumKeys(); i++)
		{
			pCommand->AddKeys(pOpener->GetKeys(i));
		}
	}
	return uiTab;
}


//////////////////////////////////////////////////////////////////////////
//
// An icon items can have, its index returned.
//
//////////////////////////////////////////////////////////////////////////
int32 CGoToDialog::AddIcon(Ptr<CImageCel> pIcon)
{
	maIcons.Add(pIcon);
	return (int32)maIcons.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The tab chosen and the text in the field, selected so typing replaces
// it, as IntelliJ remembers what was last typed; the results are found.
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::Start(size uiTab, const char* szText)
{
	miChosen = -1;
	mpTabs->GetSelectionEvent()->Remove(this);
	mpTabs->Select(uiTab);
	mpTabs->GetSelectionEvent()->Add(this, &CGoToDialog::TabSelected);
	mpField->GetChangeEvent()->Remove(this);
	mpField->SetText(szText);
	mpField->ClearUndo();
	mpField->GetChangeEvent()->Add(this, &CGoToDialog::Search);
	mpField->SelectAll();
	Search();
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the dialog modally (from a tick, see CWindow::ShowModal).  True
// if a result was chosen (GetChosen).
//
//////////////////////////////////////////////////////////////////////////
bool CGoToDialog::Show(void)
{
	return mpDialog->Show(GetMetric(TM_GoToWidth), GetMetric(TM_GoToHeight)) && (miChosen >= 0);
}


//////////////////////////////////////////////////////////////////////////
//
// As typed.
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::SetText(const char* szText)
{
	mpField->SetText(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::SelectTab(size uiTab)
{
	mpTabs->Select(uiTab);
}


//////////////////////////////////////////////////////////////////////////
//
// A ":<line>" at the end of the pattern is the line to go to, taken off.
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::SplitLine(CChars* pszPattern)
{
	char*	szText;
	size	uiLength;
	size	uiColon;

	miLine = -1;
	szText = pszPattern->Text();
	uiLength = pszPattern->Length();
	uiColon = uiLength;
	while ((uiColon > 0) && (szText[uiColon - 1] >= '0') && (szText[uiColon - 1] <= '9'))
	{
		uiColon--;
	}
	if ((uiColon > 0) && (uiColon < uiLength) && (szText[uiColon - 1] == ':'))
	{
		miLine = atoi(&szText[uiColon]);
		pszPattern->SetLength(uiColon - 1);
	}
	else if ((uiColon == uiLength) && (uiLength > 0) && (szText[uiLength - 1] == ':'))
	{
		//The line is still being typed.
		pszPattern->SetLength(uiLength - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the results for the field's text on the tab chosen, best first,
// the best selected.
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::Search(void)
{
	CChars	szPattern;

	szPattern.Init(mpField->GetText());
	SplitLine(&szPattern);
	mcMatcher.Kill();
	mcMatcher.Init(szPattern.Text());
	szPattern.Kill();

	mcItems.Clear();
	if (mpcSearcher)
	{
		mpcSearcher->GoToSearch(GetTab(), &mcMatcher, &mcItems);
	}
	mcItems.Sort();
	FillList();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::FillList(void)
{
	size	i;
	size	uiRows;

	mpList->RemoveAllComponents();
	uiRows = mcItems.NumItems();
	if (uiRows > GO_TO_DIALOG_ROWS)
	{
		uiRows = GO_TO_DIALOG_ROWS;
	}
	for (i = 0; i < uiRows; i++)
	{
		AddRow(mcItems.Get(i));
	}
	mpList->Select(uiRows > 0 ? 0 : -1);
	mpScrollPane->SetScrollPosition(0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
// One of the theme's sizes.
//
//////////////////////////////////////////////////////////////////////////
int32 CGoToDialog::GetMetric(EThemeMetric eMetric)
{
	return mpDialog->GetLook()->pcTheme->GetMetric(eMetric);
}


//////////////////////////////////////////////////////////////////////////
//
// The icon, the name and where it is.
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::AddRow(SGoToItem* psItem)
{
	Ptr<CWindow>			pWindow;
	Ptr<CIconComponent>		pIcon;
	Ptr<CLabel>				pLocation;
	Ptr<CImageCel>			pCel;
	int32					iRow;

	pWindow = mpDialog->GetWindow();
	iRow = mpList->AddRow(true);
	if ((psItem->iIcon >= 0) && ((size)psItem->iIcon < maIcons.NumElements()))
	{
		pCel = maIcons.Get((size)psItem->iIcon);
	}
	pIcon = OMalloc<CIconComponent>(pWindow, pCel, GetMetric(TM_GoToIconWidth), GetMetric(TM_GoToRowHeight));
	pIcon->SetCelAlignment(CSH_Center, CSV_Center);
	mpList->AddToRow(iRow, pIcon, 0);
	mpList->AddToRow(iRow, CreateName(psItem), 1);

	pLocation = OMalloc<CLabel>(pWindow, mpDialog->GetLook()->pcDimFont, psItem->szLocation.Text());
	pLocation->SetAlignment(CSH_Left, CSV_Center);
	mpList->AddToRow(iRow, pLocation, 2, 1, 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The name as a label for each run of it the pattern matched, in the
// match font, and for each run between, in the look's font.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CGoToDialog::CreateName(SGoToItem* psItem)
{
	Ptr<CWindow>			pWindow;
	Ptr<CGridContainer>		pName;
	Ptr<CLabel>				pLabel;
	Ptr<CFont>				pFont;
	CChars					szPiece;
	size					uiPosition;
	size					uiEnd;
	size					uiRange;
	size					uiLength;
	bool					bMatched;
	int						iColumn;
	int32					iWidth;
	int32					iHeight;

	pWindow = mpDialog->GetWindow();
	pFont = mpDialog->GetLook()->pcTheme->GetFont();
	pName = OMalloc<CGridContainer>(pWindow);
	pName->SetAlignment(CSH_Left, CSV_Center);

	uiLength = psItem->szName.Length();
	uiPosition = 0;
	uiRange = 0;
	iColumn = 0;
	iWidth = 0;
	iHeight = 0;
	while (uiPosition < uiLength)
	{
		if ((uiRange < psItem->sMatch.uiNumRanges) && (psItem->sMatch.asRanges[uiRange].uiStart == uiPosition))
		{
			uiEnd = uiPosition + psItem->sMatch.asRanges[uiRange].uiLength;
			uiRange++;
			bMatched = true;
		}
		else
		{
			uiEnd = (uiRange < psItem->sMatch.uiNumRanges) ? psItem->sMatch.asRanges[uiRange].uiStart : uiLength;
			bMatched = false;
		}
		if (uiEnd > uiLength)
		{
			uiEnd = uiLength;
		}
		szPiece.Init();
		szPiece.AppendSubString(psItem->szName.Text(), uiPosition, uiEnd);
		pLabel = OMalloc<CLabel>(pWindow, (bMatched && mpMatchFont.IsNotNull()) ? mpMatchFont : pFont, szPiece.Text());
		szPiece.Kill();
		pLabel->SetAlignment(CSH_Left, CSV_Center);
		pName->AddComponent(pLabel, iColumn, 0);
		iWidth += pLabel->GetDesiredSize().x;
		if (pLabel->GetDesiredSize().y > iHeight)
		{
			iHeight = pLabel->GetDesiredSize().y;
		}
		iColumn++;
		uiPosition = uiEnd;
	}

	//A grid in a grid is the size it's given, not the size its parts need.  A gap keeps where it is apart.
	pName->SetDesiredSize(iWidth + GetMetric(TM_GoToGap), iHeight);
	return pName;
}


//////////////////////////////////////////////////////////////////////////
//
// Enter chooses the selected result; with none it does nothing.
//
//////////////////////////////////////////////////////////////////////////
bool CGoToDialog::Approve(void)
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
void CGoToDialog::ResultActivated(void)
{
	mpDialog->OKClicked();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::TabSelected(void)
{
	Search();
	mpDialog->GetWindow()->GetFocus()->SetFocus(mpField);
}


//////////////////////////////////////////////////////////////////////////
//
// The keys of a tab's opener: the tab is chosen, which searches for the
// text typed (TabSelected).
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::TabCommand(CCommand* pcCommand)
{
	mpTabs->Select((size)atoi(pcCommand->GetName()));
}


//////////////////////////////////////////////////////////////////////////
//
// The keys typed into the search field move the results' selection: the
// list keeps it in view, and up from the first result goes to the last
// and down from the last to the first, as IntelliJ's.  A page is what
// the results pane shows.
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::Up(void) { mpList->Up(); }
void CGoToDialog::Down(void) { mpList->Down(); }
void CGoToDialog::PageUp(void) { mpList->PageUp(); }
void CGoToDialog::PageDown(void) { mpList->PageDown(); }


//////////////////////////////////////////////////////////////////////////
//
// Tab goes round the tabs.
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::NextTab(void)
{
	if (mpTabs->NumTabs() > 0)
	{
		mpTabs->Select((GetTab() + 1) % mpTabs->NumTabs());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CGoToDialog::PreviousTab(void)
{
	if (mpTabs->NumTabs() > 0)
	{
		mpTabs->Select((GetTab() + mpTabs->NumTabs() - 1) % mpTabs->NumTabs());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGoToDialog::GetTab(void)
{
	size	uiTab;

	uiTab = mpTabs->IndexOf(mpTabs->GetSelected());
	return (uiTab == ARRAY_ELEMENT_NOT_FOUND) ? 0 : uiTab;
}


//////////////////////////////////////////////////////////////////////////
//
// The result chosen, NULL if none was.
//
//////////////////////////////////////////////////////////////////////////
SGoToItem* CGoToDialog::GetChosen(void)
{
	if ((miChosen < 0) || ((size)miChosen >= mcItems.NumItems()))
	{
		return NULL;
	}
	return mcItems.Get((size)miChosen);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CGoToDialog::NumResults(void) { return mcItems.NumItems(); }
SGoToItem* CGoToDialog::GetResult(size uiIndex) { return mcItems.Get(uiIndex); }
int32 CGoToDialog::GetSelected(void) { return mpList->GetSelected(); }
int32 CGoToDialog::GetLine(void) { return miLine; }
char* CGoToDialog::GetPattern(void) { return mpField->GetText(); }
Ptr<CTextField> CGoToDialog::GetField(void) { return mpField; }
Ptr<CListContainer> CGoToDialog::GetList(void) { return mpList; }
Ptr<CDialog> CGoToDialog::GetDialog(void) { return mpDialog; }

