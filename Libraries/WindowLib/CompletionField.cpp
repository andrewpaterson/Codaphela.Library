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
#include <string.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "Window.h"
#include "MenuItem.h"
#include "MenuStyle.h"
#include "CompletionField.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szText)
{
	PreInit();

	CTextField::Init(pWindow, pTheme, szText);
	mpStyle = pTheme->GetComboMenuStyle();
	mpMenu = OMalloc<CMenuContainer>(pWindow, pTheme, mpStyle);
	mbTaking = false;
	AddInputAction(CIC_Up, this, &CCompletionField::Up);
	AddInputAction(CIC_Down, this, &CCompletionField::Down);
	AddInputAction(CIC_Enter, this, &CCompletionField::Enter);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::Class(void)
{
	CTextField::Class();

	M_Pointer(mpStyle);
	M_Pointer(mpMenu);
	U_Bool(mbTaking);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::Free(void)
{
	CTextField::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCompletionField::Save(CObjectWriter* pcFile) { return false; }
bool CCompletionField::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// What the field's text could be completed to, in order, and a note for
// each (paszNotes can be NULL, and a note empty).  The list opens under
// the field if it has the keys; with none it closes.
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::SetCompletions(CArrayChars* paszTexts, CArrayChars* paszNotes)
{
	Ptr<CMenuItem>	pItem;
	size			i;

	if (mbTaking)
	{
		return;
	}
	mpMenu->Close();
	mpMenu->RemoveAllItems();
	for (i = 0; paszTexts && (i < paszTexts->NumElements()); i++)
	{
		pItem = mpMenu->AddItem(paszTexts->GetText(i));
		if (paszNotes && (i < paszNotes->NumElements()) && !paszNotes->Get(i)->Empty())
		{
			pItem->SetShortcut(paszNotes->GetText(i));
		}
	}
	pItem = NULL;
	if ((mpMenu->NumItems() > 0) && HasFocus())
	{
		Open();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::Open(void)
{
	SInt32Vec2	sPosition;
	int32		iWidth;

	iWidth = GetActualSize().x - mpStyle->GetMenuInsets()->GetLeftAndRight();
	mpMenu->SetDesiredSize(iWidth, -1);
	sPosition.Init(GetPosition().x, GetPosition().y + GetActualSize().y);
	mpMenu->Open(sPosition, this, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::CloseCompletions(void) { mpMenu->Close(); }
bool CCompletionField::IsCompleting(void) { return mpMenu->IsOpen(); }
size CCompletionField::NumCompletions(void) { return mpMenu->NumItems(); }
const char* CCompletionField::GetCompletion(size uiIndex) { return mpMenu->GetItem(uiIndex)->GetText(); }
const char* CCompletionField::GetCompletionNote(size uiIndex) { return mpMenu->GetItem(uiIndex)->GetShortcut(); }
Ptr<CMenuContainer> CCompletionField::GetMenu(void) const { return mpMenu; }


//////////////////////////////////////////////////////////////////////////
//
// A completion made the field's text; the list closes.  False if there
// is none at the index.
//
//////////////////////////////////////////////////////////////////////////
bool CCompletionField::TakeCompletion(size uiIndex)
{
	CChars	szText;

	if (uiIndex >= mpMenu->NumItems())
	{
		return false;
	}
	szText.Init(mpMenu->GetItem(uiIndex)->GetText());
	mpMenu->Close();
	Take(szText.Text());
	szText.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::Take(const char* szText)
{
	mbTaking = true;
	SetText(szText);
	SetCaretPosition(strlen(szText));
	mbTaking = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Enter is the field's while a row of the list is chosen.
//
//////////////////////////////////////////////////////////////////////////
bool CCompletionField::WantsEnter(void)
{
	return mpMenu->IsOpen() && (mpMenu->GetSelected() >= 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A row of the list was pressed, or entered: it is taken.  The list has
// closed.
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::PopupItemClicked(Ptr<CComponent> pContent, Ptr<CComponent> pItem)
{
	CChars	szText;
	size	i;

	if (pContent != mpMenu)
	{
		return;
	}
	for (i = 0; i < mpMenu->NumItems(); i++)
	{
		if (&mpMenu->GetItem(i) == &pItem)
		{
			szText.Init(mpMenu->GetItem(i)->GetText());
			Take(szText.Text());
			szText.Kill();
			mpWindow->GetFocus()->SetFocus(this);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The list closes when the keys go anywhere but to it.
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::FocusOrMouseOverChanged(void)
{
	Ptr<CComponent>	pFocussed;

	CTextField::FocusOrMouseOverChanged();
	if (!HasFocus() && mpMenu->IsOpen())
	{
		pFocussed = mpWindow->GetFocus()->GetFocussedComponent();
		if (pFocussed.IsNull() || !pFocussed->IsWithin(mpMenu))
		{
			mpMenu->Close();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::Up(void)
{
	if (mpMenu->IsOpen())
	{
		mpMenu->Up();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::Down(void)
{
	if (mpMenu->IsOpen())
	{
		mpMenu->Down();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCompletionField::Enter(void)
{
	if (mpMenu->IsOpen() && (mpMenu->GetSelected() >= 0))
	{
		mpMenu->Activate();
	}
}

