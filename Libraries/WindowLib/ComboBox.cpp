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
#include "Window.h"
#include "ComboBox.h"


#define COMBO_BOX_CHEVRON_GAP	8  //The least space between the widest item's text and the chevron.


//////////////////////////////////////////////////////////////////////////
//
// The list is a menu in the theme's combo box menu style and the theme's
// chevron is drawn on the right of the face.
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	PreInit();

	CImageBorderContainer::Init(pWindow, pTheme->GetButtonImages(BL_Normal), 6, 2);
	mpStyle = pTheme->GetComboMenuStyle();
	mpFont = pTheme->GetFont();
	mcSelectionChanged.Init();
	miSelected = -1;
	SetCanGetFocus(true);

	mpContent = OMalloc<CGridContainer>(pWindow);
	mpContent->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpPadded->AddComponent(mpContent);

	mpText = OMalloc<CLabel>(pWindow, mpFont, "");
	mpText->SetAlignment(CSH_Left, CSV_Center);
	mpContent->AddComponent(mpText, 0, 0, 1, 1, 1, 0);
	mpChevron = OMalloc<CIconComponent>(pWindow, pTheme->GetImage(TI_DownChevron), 16);
	mpChevron->SetCelAlignment(CSH_Right, CSV_Center);
	mpChevron->SetAlignment(CSH_Right, CSV_Center);
	mpContent->AddComponent(mpChevron, 1, 0);

	mpMenu = OMalloc<CMenuContainer>(pWindow, pTheme, mpStyle);

	UpdateFaceSize();
	AddInputAction(CIC_ActivateComponent, this, &CComboBox::ActivateComponent);
	AddInputAction(CIC_Space, this, &CComboBox::Open);
	AddInputAction(CIC_Up, this, &CComboBox::Up);
	AddInputAction(CIC_Down, this, &CComboBox::Down);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::Class(void)
{
	CImageBorderContainer::Class();

	M_Pointer(mpStyle);
	M_Pointer(mpFont);
	M_Pointer(mpContent);
	M_Pointer(mpText);
	M_Pointer(mpChevron);
	M_Pointer(mpMenu);
	U_Unknown(CComponentEvent<CComboBox>, mcSelectionChanged);
	U_Int32(miSelected);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::Free(void)
{
	mcSelectionChanged.Kill();
	CImageBorderContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComboBox::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComboBox::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The first item added is chosen.
//
//////////////////////////////////////////////////////////////////////////
size CComboBox::AddItem(const char* szText)
{
	mpMenu->AddItem(szText);
	if (miSelected == -1)
	{
		Select(0);
	}
	UpdateFaceSize();
	return mpMenu->NumItems() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves nothing chosen.  The selection event isn't called.  It can be
// called from the selection event, to show other items.
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::RemoveAllItems(void)
{
	mpMenu->RemoveAllItems();
	Select(-1);
	UpdateFaceSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComboBox::NumItems(void)
{
	return mpMenu->NumItems();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CComboBox::GetItemText(size uiIndex)
{
	if (uiIndex >= mpMenu->NumItems())
	{
		return NULL;
	}
	return mpMenu->GetItem(uiIndex)->GetText();
}


//////////////////////////////////////////////////////////////////////////
//
// -1 chooses nothing.  The selection event isn't called.
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::Select(int32 iIndex)
{
	if ((iIndex < -1) || (iIndex >= (int32)mpMenu->NumItems()))
	{
		return;
	}
	miSelected = iIndex;
	mpText->SetText(iIndex == -1 ? "" : GetItemText(iIndex));

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Chooses the item and calls the selection event if it is another one.
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::Choose(int32 iIndex)
{
	if (iIndex != miSelected)
	{
		Select(iIndex);
		mcSelectionChanged.Call(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CComboBox::GetSelectedText(void)
{
	if (miSelected == -1)
	{
		return NULL;
	}
	return GetItemText(miSelected);
}


//////////////////////////////////////////////////////////////////////////
//
// The face is wide enough for the widest item, a gap and the chevron, and
// a line high, plus the padding.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CComboBox::GetContentSize(void)
{
	size		i;
	int32		iWidth;
	int32		iMaxWidth;

	iMaxWidth = 0;
	for (i = 0; i < mpMenu->NumItems(); i++)
	{
		iWidth = (int32)mpFont->CalculateWidth(GetItemText(i));
		if (iWidth > iMaxWidth)
		{
			iMaxWidth = iWidth;
		}
	}
	return SInt32Vec2(iMaxWidth + COMBO_BOX_CHEVRON_GAP + mpChevron->GetDesiredSize().x, mpFont->GetLineHeight());
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the list below the combo box, as wide as it, with the chosen item
// selected.  The list takes the keys.
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::Open(void)
{
	SInt32Vec2	sPosition;
	int32		iWidth;

	if (IsOpen() || (mpMenu->NumItems() == 0))
	{
		return;
	}

	iWidth = GetActualSize().x - mpStyle->GetMenuInsets()->GetLeftAndRight();
	mpMenu->SetDesiredSize(iWidth, -1);
	sPosition.Init(GetPosition().x, GetPosition().y + GetActualSize().y);
	mpMenu->Open(sPosition, this, true);
	if (miSelected != -1)
	{
		mpMenu->Select(mpMenu->GetItemRow(mpMenu->GetItem(miSelected)));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::Close(void)
{
	if (IsOpen())
	{
		mpMenu->Close();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComboBox::IsOpen(void)
{
	return mpMenu->IsOpen();
}


//////////////////////////////////////////////////////////////////////////
//
// An item in the list was clicked, the list has already closed.
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::PopupItemClicked(Ptr<CComponent> pContent, Ptr<CComponent> pItem)
{
	size	i;

	if (pContent != mpMenu)
	{
		return;
	}

	for (i = 0; i < mpMenu->NumItems(); i++)
	{
		if (&mpMenu->GetItem(i) == &pItem)
		{
			Choose((int32)i);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The keys come back to the combo box when its list closes.
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::PopupClosed(Ptr<CComponent> pContent)
{
	CImageBorderContainer::PopupClosed(pContent);
	if (pContent == mpMenu)
	{
		mpWindow->GetFocus()->SetFocus(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Called when another item is chosen, from the list or with up and down,
// with the combo box: pCombo->GetSelectionEvent()->Add(this,
// &CMyContent::TypeChosen).  Select doesn't call it.
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CComboBox>* CComboBox::GetSelectionEvent(void)
{
	return &mcSelectionChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Focus goes to the list while it is open.
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::FocusOrMouseOverChanged(void)
{
	UpdateLook();
}


//////////////////////////////////////////////////////////////////////////
//
// Enter clicks the window's default button.
//
//////////////////////////////////////////////////////////////////////////
bool CComboBox::WantsEnter(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Focussed, while the list is open too, has the focus ring.  Otherwise
// mouse over or normal.
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::UpdateLook(void)
{
	UpdateBorderState(HasFocus() || IsOpen());
}


//////////////////////////////////////////////////////////////////////////
//
// A press on the combo box, its owner, doesn't close its list so it does
// that itself.
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::ActivateComponent(int x, int y)
{
	if (IsOpen())
	{
		Close();
	}
	else
	{
		Open();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::Up(void)
{
	if (miSelected > 0)
	{
		Choose(miSelected - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComboBox::Down(void)
{
	if (miSelected + 1 < (int32)mpMenu->NumItems())
	{
		Choose(miSelected + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CComboBox::GetSelected(void) { return miSelected; }
Ptr<CMenuContainer> CComboBox::GetMenu(void) { return mpMenu; }
Ptr<CLabel> CComboBox::GetTextLabel(void) { return mpText; }

