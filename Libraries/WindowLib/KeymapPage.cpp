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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/StringHelper.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "FillContainer.h"
#include "GridContainer.h"
#include "RoundedBoxCanvasDraw.h"
#include "KeymapPage.h"


#define KEYMAP_CAPTURE_WIDTH	460
#define KEYMAP_GAP				6
#define KEYMAP_COLUMN_GAP		24


//////////////////////////////////////////////////////////////////////////
//
// Whether szName is in the group, so "File.Save" is in "File".
//
//////////////////////////////////////////////////////////////////////////
static bool IsCommandInGroup(const char* szName, const char* szPrefix)
{
	size	uiLength;

	uiLength = strlen(szPrefix);
	return (strncmp(szName, szPrefix, uiLength) == 0) && (szName[uiLength] == '.');
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szFilter is in szText, whatever the case.
//
//////////////////////////////////////////////////////////////////////////
static bool ContainsIgnoringCase(const char* szText, const char* szFilter)
{
	size	uiFilter;
	size	i;
	size	j;

	uiFilter = strlen(szFilter);
	for (i = 0; szText[i] != '\0'; i++)
	{
		for (j = 0; (j < uiFilter) && (szText[i + j] != '\0') && (tolower((unsigned char)szText[i + j]) == tolower((unsigned char)szFilter[j])); j++)
		{
		}
		if (j == uiFilter)
		{
			return true;
		}
	}
	return uiFilter == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyCaptureDialog::Init(Ptr<CWindow> pOwner, SDialogLook* psLook, CKeyCaptureDescriber* pcDescriber, CCommand* pcCommand, const char* szContext)
{
	size	i;

	PreInit();
	mpcDescriber = pcDescriber;
	mpcCommand = pcCommand;
	mpDialog = OMalloc<CDialog>(pOwner, psLook, "Add Shortcut");
	mpField = OMalloc<CKeyCaptureField>(mpDialog->GetWindow(), psLook->pcTheme);
	mpField->SetAlignment(CSH_Stretch, CSV_Center);
	mpMessage = OMalloc<CLabel>(mpDialog->GetWindow(), psLook->pcTheme->GetFont(), "Press the keys.");
	mpMessage->SetAlignment(CSH_Left, CSV_Center);
	mpDialog->AddEditor("Shortcut", mpField);
	if (pcCommand->NumContexts() > 1)
	{
		mpWhere = OMalloc<CComboBox>(mpDialog->GetWindow(), psLook->pcTheme);
		for (i = 0; i < pcCommand->NumContexts(); i++)
		{
			mpWhere->AddItem(GetContextCaption(pcCommand->GetContext(i)));
			if (StringCompare(pcCommand->GetContext(i), szContext) == 0)
			{
				mpWhere->Select((int32)i);
			}
		}
		if (mpWhere->GetSelected() < 0)
		{
			mpWhere->Select(0);
		}
		mpWhere->GetSelectionEvent()->Add(this, &CKeyCaptureDialog::KeysCaptured);
		mpDialog->AddEditor("Where", mpWhere);
	}
	mpDialog->AddEditor("", mpMessage);
	mpDialog->SetApprover(this, &CKeyCaptureDialog::Approve);
	mpField->GetCapturedEvent()->Add(this, &CKeyCaptureDialog::KeysCaptured);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyCaptureDialog::Class(void)
{
	M_Pointer(mpDialog);
	M_Pointer(mpField);
	M_Pointer(mpWhere);
	M_Pointer(mpMessage);
	U_Pointer(mpcDescriber);
	U_Pointer(mpcCommand);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyCaptureDialog::Free(void)
{
	if (mpDialog.IsNotNull())
	{
		mpDialog->ClearApprover();
	}
	if (mpField.IsNotNull())
	{
		mpField->GetCapturedEvent()->Remove(this);
	}
	if (mpWhere.IsNotNull())
	{
		mpWhere->GetSelectionEvent()->Remove(this);
	}
}


bool CKeyCaptureDialog::Save(CObjectWriter* pcFile) { return false; }
bool CKeyCaptureDialog::Load(CObjectReader* pcFile) { return false; }
bool CKeyCaptureDialog::Show(void) { return mpDialog->Show(KEYMAP_CAPTURE_WIDTH); }
char* CKeyCaptureDialog::GetKeys(void) { return mpField->GetKeys(); }
void CKeyCaptureDialog::SetKeys(const char* szKeys) { mpField->SetKeys(szKeys); }
Ptr<CDialog> CKeyCaptureDialog::GetDialog(void) { return mpDialog; }


//////////////////////////////////////////////////////////////////////////
//
// The context the keys are for: Where's, or the command's only one.
//
//////////////////////////////////////////////////////////////////////////
const char* CKeyCaptureDialog::GetContext(void)
{
	if (mpWhere.IsNotNull() && (mpWhere->GetSelected() >= 0))
	{
		return mpcCommand->GetContext((size)mpWhere->GetSelected());
	}
	return mpcCommand->GetFirstContext();
}


//////////////////////////////////////////////////////////////////////////
//
// How a context is named to a user: Global keys work "Everywhere".
//
//////////////////////////////////////////////////////////////////////////
const char* GetContextCaption(const char* szContext)
{
	return (StrEmpty(szContext) || (StringCompare(szContext, COMPONENT_INPUT_GLOBAL_CONTEXT) == 0)) ? "Everywhere" : szContext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeyCaptureDialog::KeysCaptured(void)
{
	CChars	szMessage;

	szMessage.Init();
	mpcDescriber->DescribeCapturedKeys(GetContext(), mpField->GetKeys(), &szMessage);
	mpMessage->SetText(szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// OK only closes it for keys the command can have.
//
//////////////////////////////////////////////////////////////////////////
bool CKeyCaptureDialog::Approve(void)
{
	CChars	szMessage;
	bool	bResult;

	szMessage.Init();
	bResult = mpcDescriber->DescribeCapturedKeys(GetContext(), mpField->GetKeys(), &szMessage);
	szMessage.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// pDialog is what the page is on.  The commands and input aren't held.
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::Init(Ptr<CDialog> pDialog, Ptr<CCommandSet> pCommands, CComponentInput* pcInput, SKeymapGroup* asGroups, size uiNumGroups, CKeymapPageWatcher* pcWatcher)
{
	PreInit();
	CSettingsPage::Init();
	mpDialog = pDialog;
	mpCommands = pCommands;
	mpcInput = pcInput;
	mpcWatcher = pcWatcher;
	masGroups = asGroups;
	muiNumGroups = uiNumGroups;
	mszScriptedKeys.Init();
	mbUpdating = false;
	masRows.Init();
	maiListRows.Init();
	muiCaptureRow = 0;
	AddRows();
	mcGiver.Init(this, pcInput);
	mcGiver.SetDialog(pDialog->GetWindow(), pDialog->GetLook(), pcWatcher);
	mpBody = CreateBody();

	mpFilter->GetChangeEvent()->Add(this, &CKeymapPage::FilterChanged);
	mpList->GetActivationEvent()->Add(this, &CKeymapPage::RowActivated);
	mpAdd->GetClickEvent()->Add(this, &CKeymapPage::AddClicked);
	mpRemove->GetClickEvent()->Add(this, &CKeymapPage::RemoveClicked);
	mpReset->GetClickEvent()->Add(this, &CKeymapPage::ResetClicked);
	mpResetAll->GetClickEvent()->Add(this, &CKeymapPage::ResetAllClicked);

	ShowList();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::Class(void)
{
	CSettingsPage::Class();
	M_Pointer(mpDialog);
	M_Pointer(mpCommands);
	U_Pointer(mpcInput);
	U_Pointer(mpcWatcher);
	U_Pointer(masGroups);
	U_Size(muiNumGroups);
	U_String(mszScriptedKeys);
	M_Pointer(mpBody);
	M_Pointer(mpFilter);
	M_Pointer(mpList);
	M_Pointer(mpScrollPane);
	M_Pointer(mpAdd);
	M_Pointer(mpRemove);
	M_Pointer(mpReset);
	M_Pointer(mpResetAll);
	M_Pointer(mpStatus);
	M_Pointer(mpCapture);
	U_Unknown(CArrayKeymapRows, masRows);
	U_Unknown(CArrayInt, maiListRows);
	U_Unknown(CKeyGiver, mcGiver);
	U_Size(muiCaptureRow);
	U_Bool(mbUpdating);
}


//////////////////////////////////////////////////////////////////////////
//
// An object's memory is freed without its destructor running so the page
// takes itself off the widgets still here.
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::Free(void)
{
	size	i;

	if (mpDialog.IsNotNull())
	{
		mpDialog->GetWindow()->CancelPosted(this);
	}
	if (mpFilter.IsNotNull())
	{
		mpFilter->GetChangeEvent()->Remove(this);
	}
	if (mpList.IsNotNull())
	{
		mpList->GetActivationEvent()->Remove(this);
	}
	if (mpAdd.IsNotNull())
	{
		mpAdd->GetClickEvent()->Remove(this);
		mpRemove->GetClickEvent()->Remove(this);
		mpReset->GetClickEvent()->Remove(this);
		mpResetAll->GetClickEvent()->Remove(this);
	}
	mcGiver.Kill();
	for (i = 0; i < masRows.NumElements(); i++)
	{
		masRows.Get(i)->cKeys.Kill();
	}
	masRows.Kill();
	maiListRows.Kill();
	mszScriptedKeys.Kill();
	CSettingsPage::Free();
}


bool CKeymapPage::Save(CObjectWriter* pcFile) { return false; }
bool CKeymapPage::Load(CObjectReader* pcFile) { return false; }
const char* CKeymapPage::GetTitle(void) { return "Keymap"; }
Ptr<CComponent> CKeymapPage::GetBody(void) { return mpBody; }
Ptr<CComponent> CKeymapPage::GetFirstEditor(void) { return mpFilter; }


//////////////////////////////////////////////////////////////////////////
//
// Copies of the commands' keys, a group's together.  A command in no group
// goes after them all.
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::AddRows(void)
{
	SKeymapRow*	psRow;
	Ptr<CCommand>	pCommand;
	size			uiNumGroups;
	size			uiGroup;
	size			i;
	size			j;
	bool			bInGroup;

	uiNumGroups = muiNumGroups;
	for (uiGroup = 0; uiGroup <= uiNumGroups; uiGroup++)
	{
		for (i = 0; i < mpCommands->NumCommands(); i++)
		{
			pCommand = mpCommands->GetCommand(i);
			if (uiGroup < uiNumGroups)
			{
				bInGroup = IsCommandInGroup(pCommand->GetName(), masGroups[uiGroup].szPrefix);
			}
			else
			{
				bInGroup = true;
				for (j = 0; j < uiNumGroups; j++)
				{
					bInGroup = bInGroup && !IsCommandInGroup(pCommand->GetName(), masGroups[j].szPrefix);
				}
			}
			if (bInGroup)
			{
				psRow = masRows.Add();
				psRow->pcCommand = &pCommand;
				psRow->cKeys.Init();
				psRow->cKeys.Copy(pCommand->GetKeyList());
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The filter over the list, its buttons under it and what's been said
// under them.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CKeymapPage::CreateBody(void)
{
	Ptr<CWindow>			pWindow;
	SDialogLook*			psLook;
	Ptr<CGridContainer>		pBody;
	Ptr<CGridContainer>		pButtons;
	Ptr<CFillContainer>		pGap;
	SInsets					sInsets;

	pWindow = mpDialog->GetWindow();
	psLook = mpDialog->GetLook();

	mpFilter = OMalloc<CTextField>(pWindow, psLook->pcTheme, "");
	mpFilter->SetAlignment(CSH_Stretch, CSV_Center);

	sInsets.Init(0, 0);
	mpList = OMalloc<CListContainer>(pWindow, psLook->pcTheme);
	mpList->SetHighlightDraws(OMalloc<CRoundedBoxCanvasDraw>(psLook->pcTheme->GetColour(TC_ListSelected), 4, &sInsets), OMalloc<CRoundedBoxCanvasDraw>(psLook->pcTheme->GetColour(TC_ListHover), 4, &sInsets));
	mpList->SetAlignment(CSH_Stretch, CSV_Top);
	mpList->SetDoubleClickActivates(true);
	mpScrollPane = OMalloc<CScrollPane>(pWindow, psLook->pcTheme);
	mpScrollPane->SetContent(mpList);
	mpScrollPane->SetAlignment(CSH_Stretch, CSV_Stretch);

	pButtons = OMalloc<CGridContainer>(pWindow);
	pButtons->SetAlignment(CSH_Stretch, CSV_Top);
	mpAdd = CreateButton(pButtons, 0, "Add Shortcut...");
	mpRemove = CreateButton(pButtons, 2, "Remove");
	mpReset = CreateButton(pButtons, 4, "Reset");
	mpResetAll = CreateButton(pButtons, 6, "Reset All");
	pButtons->AddComponent(OMalloc<CFillContainer>(pWindow), 7, 0, 1, 1, 1, 0);  //Keeps the buttons on the left.

	mpStatus = OMalloc<CLabel>(pWindow, psLook->pcTheme->GetFont(), "");
	mpStatus->SetAlignment(CSH_Left, CSV_Center);

	pBody = OMalloc<CGridContainer>(pWindow);
	pBody->SetAlignment(CSH_Stretch, CSV_Stretch);
	pBody->AddComponent(mpFilter, 0, 0, 1, 1, 1, 0);
	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(1, KEYMAP_GAP);
	pBody->AddComponent(pGap, 0, 1);
	pBody->AddComponent(mpScrollPane, 0, 2, 1, 1, 1, 1);
	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(1, KEYMAP_GAP);
	pBody->AddComponent(pGap, 0, 3);
	pBody->AddComponent(pButtons, 0, 4, 1, 1, 1, 0);
	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(1, KEYMAP_GAP);
	pBody->AddComponent(pGap, 0, 5);
	pBody->AddComponent(mpStatus, 0, 6, 1, 1, 1, 0);
	return pBody;
}


//////////////////////////////////////////////////////////////////////////
//
// A button, with a gap in the column after it.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButton> CKeymapPage::CreateButton(Ptr<CGridContainer> pButtons, int32 iColumn, const char* szCaption)
{
	Ptr<CButton>		pButton;
	Ptr<CFillContainer>	pGap;

	pButton = OMalloc<CButton>(mpDialog->GetWindow(), mpDialog->GetLook()->pcTheme, szCaption);
	pButton->SetAlignment(CSH_Left, CSV_Center);
	pButtons->AddComponent(pButton, iColumn, 0);
	pGap = OMalloc<CFillContainer>(mpDialog->GetWindow());
	pGap->SetDesiredSize(KEYMAP_GAP, 1);
	pButtons->AddComponent(pGap, iColumn + 1, 0);
	return pButton;
}


//////////////////////////////////////////////////////////////////////////
//
// The group a command is shown under, "Other" if it's in none.
//
//////////////////////////////////////////////////////////////////////////
const char* CKeymapPage::GetGroupCaption(CCommand* pcCommand)
{
	size	i;

	for (i = 0; i < muiNumGroups; i++)
	{
		if (IsCommandInGroup(pcCommand->GetName(), masGroups[i].szPrefix))
		{
			return masGroups[i].szCaption;
		}
	}
	return "Other";
}


//////////////////////////////////////////////////////////////////////////
//
// The row's keys (see AppendKeyListText).
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::ShowKeys(size uiRow, CChars* pszDest)
{
	AppendKeyListText(pszDest, &masRows.Get(uiRow)->cKeys);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the filter lets the row be shown: its caption, its name, its
// group or its keys have what's typed in them.
//
//////////////////////////////////////////////////////////////////////////
bool CKeymapPage::IsShown(size uiRow)
{
	SKeymapRow*	psRow;
	CChars			szKeys;
	char*			szFilter;
	bool			bResult;

	psRow = masRows.Get(uiRow);
	szFilter = mpFilter->GetText();
	szKeys.Init();
	ShowKeys(uiRow, &szKeys);
	bResult = ContainsIgnoringCase(psRow->pcCommand->GetCaption(), szFilter) ||
			  ContainsIgnoringCase(psRow->pcCommand->GetName(), szFilter) ||
			  ContainsIgnoringCase(GetGroupCaption(psRow->pcCommand), szFilter) ||
			  ContainsIgnoringCase(szKeys.Text(), szFilter);
	szKeys.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The rows the filter lets through under their groups' headings, with the
// row selected kept selected if it's still there.
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::ShowList(void)
{
	Ptr<CWindow>	pWindow;
	SDialogLook*	psLook;
	SKeymapRow*	psRow;
	Ptr<CLabel>		pLabel;
	CChars			szCaption;
	CChars			szKeys;
	const char*		szGroup;
	int32			iSelected;
	int32			iListRow;
	size			i;

	iSelected = GetSelectedRow();
	pWindow = mpDialog->GetWindow();
	psLook = mpDialog->GetLook();
	mbUpdating = true;
	mpList->RemoveAllComponents();
	maiListRows.ReInit();
	szGroup = NULL;
	for (i = 0; i < masRows.NumElements(); i++)
	{
		if (!IsShown(i))
		{
			continue;
		}
		psRow = masRows.Get(i);
		if ((szGroup == NULL) || (StringCompare(szGroup, GetGroupCaption(psRow->pcCommand)) != 0))
		{
			szGroup = GetGroupCaption(psRow->pcCommand);
			pLabel = OMalloc<CLabel>(pWindow, psLook->pcDimFont, szGroup);
			pLabel->SetAlignment(CSH_Left, CSV_Center);
			iListRow = mpList->AddRow(false);
			mpList->AddToRow(iListRow, pLabel, 0, 2, 1);
			maiListRows.Add(-1);
		}

		szCaption.Init(psRow->pcCommand->GetCaption());
		szKeys.Init();
		ShowKeys(i, &szKeys);
		pLabel = OMalloc<CLabel>(pWindow, psLook->pcTheme->GetFont(), szCaption.Text());
		pLabel->SetAlignment(CSH_Left, CSV_Center);
		iListRow = mpList->AddRow(true);
		mpList->AddToRow(iListRow, pLabel, 0, 1, 1);
		pLabel = OMalloc<CLabel>(pWindow, psLook->pcTheme->GetFont(), szKeys.Text());
		pLabel->SetAlignment(CSH_Left, CSV_Center);
		pLabel->SetDesiredSize(pLabel->GetRequiredSize().x + KEYMAP_COLUMN_GAP, -1);
		mpList->AddToRow(iListRow, pLabel, 1);
		maiListRows.Add((int)i);
		szKeys.Kill();
		szCaption.Kill();
	}
	if (iSelected >= 0)
	{
		SelectRow((size)iSelected);
	}
	mbUpdating = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The row of masRows selected in the list, -1 if none is.
//
//////////////////////////////////////////////////////////////////////////
int32 CKeymapPage::GetSelectedRow(void)
{
	int32	iListRow;

	iListRow = mpList->GetSelected();
	if ((iListRow < 0) || (iListRow >= (int32)maiListRows.NumElements()))
	{
		return -1;
	}
	return maiListRows.GetValue((size)iListRow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::SelectRow(size uiRow)
{
	size	i;

	for (i = 0; i < maiListRows.NumElements(); i++)
	{
		if (maiListRows.GetValue(i) == (int)uiRow)
		{
			mpList->Select((int32)i);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The rows as the key giver gives to them: each row's keys, captioned
// with its group ("Debug: Start Debugging").
//
//////////////////////////////////////////////////////////////////////////
size CKeymapPage::NumKeyLists(void) { return masRows.NumElements(); }
CCommand* CKeymapPage::GetKeyListCommand(size uiIndex) { return masRows.Get(uiIndex)->pcCommand; }
CCommandKeyList* CKeymapPage::GetKeyList(size uiIndex) { return &masRows.Get(uiIndex)->cKeys; }


void CKeymapPage::AppendKeyListCaption(size uiIndex, CChars* pszDest)
{
	pszDest->Append(GetGroupCaption(masRows.Get(uiIndex)->pcCommand));
	pszDest->Append(": ");
	pszDest->Append(masRows.Get(uiIndex)->pcCommand->GetCaption());
}


//////////////////////////////////////////////////////////////////////////
//
// Add Shortcut's keys for the row it's for.
//
//////////////////////////////////////////////////////////////////////////
bool CKeymapPage::DescribeCapturedKeys(const char* szContext, const char* szKeys, CChars* pszMessage)
{
	return mcGiver.Describe(muiCaptureRow, szContext, szKeys, pszMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// szKeys go to the row's command in szContext (NULL for its first), from
// whichever had them there, asking first (see CKeyGiver).  False, with
// why under the list, if it can't have them or moving them was cancelled.
//
//////////////////////////////////////////////////////////////////////////
bool CKeymapPage::AddKeys(size uiRow, const char* szKeys, const char* szContext)
{
	CChars	szMessage;
	bool	bResult;

	szMessage.Init();
	bResult = mcGiver.Give(uiRow, StrEmpty(szContext) ? masRows.Get(uiRow)->pcCommand->GetFirstContext() : szContext, szKeys, &szMessage);
	mpStatus->SetText(szMessage.Text());
	szMessage.Kill();
	if (bResult)
	{
		ShowList();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::RemoveKeys(size uiRow)
{
	masRows.Get(uiRow)->cKeys.Clear();
	mpStatus->SetText("");
	ShowList();
}


//////////////////////////////////////////////////////////////////////////
//
// The command has the keys it was made with, taking them from any other
// that has them now in the same context.
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::ResetKeys(size uiRow)
{
	CCommand*	pcCommand;
	size		i;
	size		j;

	pcCommand = masRows.Get(uiRow)->pcCommand;
	for (i = 0; i < pcCommand->NumDefaultKeys(); i++)
	{
		for (j = 0; j < masRows.NumElements(); j++)
		{
			masRows.Get(j)->cKeys.Remove(pcCommand->GetDefaultKeysContext(i), pcCommand->GetDefaultKeys(i));
		}
	}
	masRows.Get(uiRow)->cKeys.Clear();
	for (i = 0; i < pcCommand->NumDefaultKeys(); i++)
	{
		masRows.Get(uiRow)->cKeys.Add(pcCommand->GetDefaultKeysContext(i), pcCommand->GetDefaultKeys(i));
	}
	mpStatus->SetText("");
	ShowList();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::ResetAllKeys(void)
{
	SKeymapRow*	psRow;
	size		i;
	size		j;

	for (i = 0; i < masRows.NumElements(); i++)
	{
		psRow = masRows.Get(i);
		psRow->cKeys.Clear();
		for (j = 0; j < psRow->pcCommand->NumDefaultKeys(); j++)
		{
			psRow->cKeys.Add(psRow->pcCommand->GetDefaultKeysContext(j), psRow->pcCommand->GetDefaultKeys(j));
		}
	}
	mpStatus->SetText("");
	ShowList();
}


//////////////////////////////////////////////////////////////////////////
//
// OK: each command has the keys its row has.
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::Keep(void)
{
	size	i;

	for (i = 0; i < masRows.NumElements(); i++)
	{
		masRows.Get(i)->pcCommand->SetKeys(&masRows.Get(i)->cKeys);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Add Shortcut for the selected row, from the dialog's tick: a dialog
// can't be shown from an input action.
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::ShowCapture(void)
{
	int32	iRow;
	CChars	szKeys;
	bool	bOK;

	iRow = GetSelectedRow();
	if (iRow < 0)
	{
		mpStatus->SetText("Select a command first.");
		return;
	}
	muiCaptureRow = (size)iRow;
	mpCapture = OMalloc<CKeyCaptureDialog>(mpDialog->GetWindow(), mpDialog->GetLook(), this, masRows.Get(muiCaptureRow)->pcCommand, masRows.Get(muiCaptureRow)->pcCommand->GetFirstContext());
	if (!mszScriptedKeys.Empty())
	{
		mpCapture->SetKeys(mszScriptedKeys.Text());
	}
	if (mpcWatcher != NULL)
	{
		mpcWatcher->WatchDialog(mpCapture->GetDialog());
	}
	bOK = mpCapture->Show();
	if (mpcWatcher != NULL)
	{
		mpcWatcher->StopWatchingDialog();
	}
	if (bOK)
	{
		szKeys.Init(mpCapture->GetKeys());
		AddKeys((size)iRow, szKeys.Text(), mpCapture->GetContext());
		szKeys.Kill();
	}
	mpCapture = NULL;
	if (mpcWatcher != NULL)
	{
		mpcWatcher->WatchDialog(mpDialog);  //Again, as watching Add Shortcut replaced it.
	}
}


void CKeymapPage::FilterChanged(void) { ShowList(); }
void CKeymapPage::RowActivated(void) { mpDialog->GetWindow()->Post(this, &CKeymapPage::ShowCapture); }
void CKeymapPage::AddClicked(CButton* pcButton) { mpDialog->GetWindow()->Post(this, &CKeymapPage::ShowCapture); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::RemoveClicked(CButton* pcButton)
{
	if (GetSelectedRow() >= 0)
	{
		RemoveKeys((size)GetSelectedRow());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::ResetClicked(CButton* pcButton)
{
	if (GetSelectedRow() >= 0)
	{
		ResetKeys((size)GetSelectedRow());
	}
}


void CKeymapPage::ResetAllClicked(CButton* pcButton) { ResetAllKeys(); }


//////////////////////////////////////////////////////////////////////////
//
// A scripted edit's keys for the row: "<context>:<keys>" where the part
// before the colon is a context, else keys in the command's first.
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::AddEditedKeys(size uiRow, const char* szEdit)
{
	const char*	szColon;
	CChars		szContext;

	szColon = strchr(szEdit, ':');
	szContext.Init();
	if ((szColon != NULL) && (szColon != szEdit))
	{
		szContext.Append(szEdit, szColon - szEdit);
	}
	if (!szContext.Empty() && mpcInput->HasContext(szContext.Text()))
	{
		AddKeys(uiRow, &szColon[1], szContext.Text());
	}
	else
	{
		AddKeys(uiRow, szEdit);
	}
	szContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// For checking, what a user would do on the page, separated by semicolons:
// select:<command's name>, add:<keys> or add:<context>:<keys>,
// capture:<keys>, which goes through
// Add Shortcut with them pressed, remove, reset, resetall, filter:<text>,
// and show, which prints the rows the list has.
//
//////////////////////////////////////////////////////////////////////////
void CKeymapPage::ApplyEdits(const char* szEdits)
{
	CArrayChars	aszEdits;
	CChars		szKeys;
	size		i;
	size		j;
	char*		sz;
	int32		iRow;

	aszEdits.Init();
	aszEdits.Split(szEdits, ';');
	mcGiver.SetAnswer(KGA_Give);
	for (i = 0; i < aszEdits.NumElements(); i++)
	{
		sz = aszEdits.GetText(i);
		iRow = GetSelectedRow();
		if (strncmp(sz, "select:", 7) == 0)
		{
			for (j = 0; j < masRows.NumElements(); j++)
			{
				if (StringCompare(masRows.Get(j)->pcCommand->GetName(), &sz[7]) == 0)
				{
					SelectRow(j);
				}
			}
		}
		else if ((strncmp(sz, "add:", 4) == 0) && (iRow >= 0))
		{
			AddEditedKeys((size)iRow, &sz[4]);
			printf("Keymap: %s\n", mpStatus->GetText());
		}
		else if (strncmp(sz, "capture:", 8) == 0)
		{
			mszScriptedKeys.Set(&sz[8]);
			if (mpcWatcher != NULL)
			{
				mpcWatcher->ShowCaptureWhenDrawn(this);
			}
		}
		else if ((strcmp(sz, "remove") == 0) && (iRow >= 0))
		{
			RemoveKeys((size)iRow);
		}
		else if ((strcmp(sz, "reset") == 0) && (iRow >= 0))
		{
			ResetKeys((size)iRow);
		}
		else if (strcmp(sz, "resetall") == 0)
		{
			ResetAllKeys();
		}
		else if (strncmp(sz, "filter:", 7) == 0)
		{
			mpFilter->SetText(&sz[7]);
			ShowList();
		}
		else if (strcmp(sz, "show") == 0)
		{
			for (j = 0; j < maiListRows.NumElements(); j++)
			{
				if (maiListRows.GetValue(j) >= 0)
				{
					szKeys.Init();
					ShowKeys((size)maiListRows.GetValue(j), &szKeys);
					printf("Keymap row: %s [%s]\n", masRows.Get((size)maiListRows.GetValue(j))->pcCommand->GetName(), szKeys.Text());
					szKeys.Kill();
				}
			}
		}
		fflush(stdout);
	}
	mcGiver.SetAnswer(KGA_Ask);
	aszEdits.Kill();
}

