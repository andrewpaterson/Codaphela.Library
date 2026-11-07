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
#include "Label.h"
#include "MessageDialog.h"




//////////////////////////////////////////////////////////////////////////
//
// A dialog titled szTitle showing szMessage, a line for each of its line
// feeds, with OK and Cancel.
//
//////////////////////////////////////////////////////////////////////////
void CMessageDialog::Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle, const char* szMessage)
{
	CArrayChars	aszLines;
	size		i;

	PreInit();

	mpDialog = OMalloc<CDialog>(pOwner, psLook, szTitle);
	mpBody = OMalloc<CGridContainer>(mpDialog->GetWindow());
	mpItems = NULL;
	mpItemsPane = NULL;
	miRows = 0;
	muiItems = 0;

	aszLines.Init();
	aszLines.Split(szMessage, '\n');
	for (i = 0; i < aszLines.NumElements(); i++)
	{
		AddLabel(mpBody, miRows, aszLines.GetText(i));
		miRows++;
	}
	aszLines.Kill();
	mpDialog->SetBody(mpBody, NULL);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CMessageDialog::Class(void)
{
	M_Pointer(mpDialog);
	M_Pointer(mpBody);
	M_Pointer(mpItems);
	M_Pointer(mpItemsPane);
	U_Int32(miRows);
	U_Size(muiItems);
}


//////////////////////////////////////////////////////////////////////////
//
// The dialog and its labels are freed with the pointers to them.
//
//////////////////////////////////////////////////////////////////////////
void CMessageDialog::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// A message dialog isn't saved.
//
//////////////////////////////////////////////////////////////////////////
bool CMessageDialog::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CMessageDialog::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A line of the list under the message, indented.  A list longer than
// MESSAGE_DIALOG_MAX_ITEMS scrolls.
//
//////////////////////////////////////////////////////////////////////////
void CMessageDialog::AddItem(const char* szItem)
{
	Ptr<CLabel>	pLabel;
	int32		iRowHeight;
	size		uiShown;

	if (mpItems.IsNull())
	{
		mpItems = OMalloc<CGridContainer>(mpDialog->GetWindow());
		mpItems->SetInsets(mpDialog->GetLook()->pcTheme->GetMetric(TM_MessageItemsGap), 0, 0, mpDialog->GetLook()->pcTheme->GetMetric(TM_MessageItemIndent));
		mpItemsPane = OMalloc<CScrollPane>(mpDialog->GetWindow(), mpDialog->GetLook()->pcTheme);
		mpItemsPane->SetContent(mpItems);
		mpItemsPane->SetAlignment(CSH_Stretch, CSV_Top);
		mpBody->AddComponent(mpItemsPane, 0, miRows, 1, 1, 1, 0);
		miRows++;
	}

	pLabel = AddLabel(mpItems, (int32)muiItems, szItem);
	muiItems++;

	//As tall as its items, up to the most shown.
	iRowHeight = pLabel->GetRequiredSize().y;
	uiShown = (muiItems < MESSAGE_DIALOG_MAX_ITEMS) ? muiItems : MESSAGE_DIALOG_MAX_ITEMS;
	mpItems->SetRequiredSize();
	mpItemsPane->SetDesiredSize(mpItems->GetRequiredSize().x, mpDialog->GetLook()->pcTheme->GetMetric(TM_MessageItemsGap) + iRowHeight * (int32)uiShown);
}


//////////////////////////////////////////////////////////////////////////
//
// A label on row iRow of pGrid, in the dialog's font.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CLabel> CMessageDialog::AddLabel(Ptr<CGridContainer> pGrid, int32 iRow, const char* szText)
{
	Ptr<CLabel>	pLabel;

	pLabel = OMalloc<CLabel>(mpDialog->GetWindow(), mpDialog->GetLook()->pcTheme->GetFont(), szText);
	pLabel->SetAlignment(CSH_Left, CSV_Top);
	pGrid->AddComponent(pLabel, 0, iRow, 1, 1, 1, 0);
	return pLabel;
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the dialog, sized to what it holds, until a button closes it.
// True if OK did; GetClicked says which did.
//
//////////////////////////////////////////////////////////////////////////
bool CMessageDialog::Show(void)
{
	return mpDialog->Show();
}


//////////////////////////////////////////////////////////////////////////
//
// The caption of the button that closed the dialog ("OK" for OK whatever
// its caption), empty if the window manager closed it.
//
//////////////////////////////////////////////////////////////////////////
char* CMessageDialog::GetClicked(void)
{
	return mpDialog->GetClicked();
}


//////////////////////////////////////////////////////////////////////////
//
// The dialog, for its buttons' captions and its extra button.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDialog> CMessageDialog::GetDialog(void)
{
	return mpDialog;
}

