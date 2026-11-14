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
#include "Label.h"
#include "RoundedBoxCanvasDraw.h"
#include "SettingsDialog.h"


#define SETTINGS_DIALOG_WIDTH		860
#define SETTINGS_DIALOG_HEIGHT		620
#define SETTINGS_DIALOG_LIST_WIDTH	160
#define SETTINGS_DIALOG_GAP			12


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSettingsPage::Init(void)
{
	PreInit();
	PostInit();
}


void CSettingsPage::Class(void) {}
void CSettingsPage::Free(void) {}
bool CSettingsPage::Save(CObjectWriter* pcFile) { return false; }
bool CSettingsPage::Load(CObjectReader* pcFile) { return false; }
const char* CSettingsPage::GetTitle(void) { return ""; }
Ptr<CComponent> CSettingsPage::GetBody(void) { return NULL; }
Ptr<CComponent> CSettingsPage::GetFirstEditor(void) { return NULL; }
void CSettingsPage::Keep(void) {}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSettingsDialog::Init(Ptr<CWindow> pOwner, SDialogLook* psLook)
{
	PreInit();
	maPages.Init();
	mpDialog = OMalloc<CDialog>(pOwner, psLook, "Settings");
	mpDialog->SetResizable(SETTINGS_DIALOG_WIDTH / 2, SETTINGS_DIALOG_HEIGHT / 2);
	CreateBody();
	mpPageList->GetSelectionEvent()->Add(this, &CSettingsDialog::PageSelected);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSettingsDialog::Class(void)
{
	M_Pointer(mpDialog);
	M_Pointer(mpPageList);
	M_Pointer(mpPageHolder);
	M_Embedded(maPages);
}


//////////////////////////////////////////////////////////////////////////
//
// An object's memory is freed without its destructor running so the
// dialog takes itself off the widgets still here.
//
//////////////////////////////////////////////////////////////////////////
void CSettingsDialog::Free(void)
{
	if (mpPageList.IsNotNull())
	{
		mpPageList->GetSelectionEvent()->Remove(this);
	}
}


bool CSettingsDialog::Save(CObjectWriter* pcFile) { return false; }
bool CSettingsDialog::Load(CObjectReader* pcFile) { return false; }
Ptr<CDialog> CSettingsDialog::GetDialog(void) { return mpDialog; }


//////////////////////////////////////////////////////////////////////////
//
// The pages' list on the left, what the selected one shows on the right.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CSettingsDialog::CreateBody(void)
{
	Ptr<CWindow>			pWindow;
	SDialogLook*			psLook;
	Ptr<CGridContainer>		pBody;
	Ptr<CFillContainer>		pGap;
	SInsets					sInsets;

	pWindow = mpDialog->GetWindow();
	psLook = mpDialog->GetLook();

	sInsets.Init(0, 0);
	mpPageList = OMalloc<CListContainer>(pWindow, psLook->pcTheme);
	mpPageList->SetHighlightDraws(OMalloc<CRoundedBoxCanvasDraw>(psLook->pcTheme->GetColour(TC_ListSelected), 4, &sInsets), OMalloc<CRoundedBoxCanvasDraw>(psLook->pcTheme->GetColour(TC_ListHover), 4, &sInsets));
	mpPageList->SetAlignment(CSH_Stretch, CSV_Top);
	mpPageList->SetDesiredSize(SETTINGS_DIALOG_LIST_WIDTH, -1);

	mpPageHolder = OMalloc<CGridContainer>(pWindow);
	mpPageHolder->SetAlignment(CSH_Stretch, CSV_Stretch);

	pBody = OMalloc<CGridContainer>(pWindow);
	pBody->AddComponent(mpPageList, 0, 0, 1, 1, 0, 1);
	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(SETTINGS_DIALOG_GAP, 1);
	pBody->AddComponent(pGap, 1, 0);
	pBody->AddComponent(mpPageHolder, 2, 0, 1, 1, 1, 1);
	return pBody;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSettingsDialog::AddPage(Ptr<CSettingsPage> pPage)
{
	Ptr<CLabel>	pLabel;

	maPages.Add(pPage);
	pLabel = OMalloc<CLabel>(mpDialog->GetWindow(), mpDialog->GetLook()->pcTheme->GetFont(), pPage->GetTitle());
	pLabel->SetAlignment(CSH_Left, CSV_Center);
	mpPageList->AddItem(pLabel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSettingsDialog::PageSelected(void)
{
	Ptr<CSettingsPage>	pPage;
	int32					iSelected;

	iSelected = mpPageList->GetSelected();
	if ((iSelected < 0) || ((size)iSelected >= maPages.NumElements()))
	{
		return;
	}
	pPage = maPages.Get((size)iSelected);
	mpPageHolder->RemoveAllComponents();
	mpPageHolder->AddComponent(pPage->GetBody(), 0, 0, 1, 1, 1, 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The first page is shown.  True, once every page has kept what it was
// given, if OK closed it.
//
//////////////////////////////////////////////////////////////////////////
bool CSettingsDialog::Show(void)
{
	bool	bResult;
	size	i;

	mpDialog->SetBody(mpPageHolder->GetParent(), maPages.Get(0)->GetFirstEditor());
	mpPageList->Select(0);
	bResult = mpDialog->Show(SETTINGS_DIALOG_WIDTH, SETTINGS_DIALOG_HEIGHT);
	if (bResult)
	{
		for (i = 0; i < maPages.NumElements(); i++)
		{
			maPages.Get(i)->Keep();
		}
	}
	return bResult;
}

