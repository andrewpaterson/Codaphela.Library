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
#include "Toolbar.h"


//////////////////////////////////////////////////////////////////////////
//
// An empty toolbar, along the top of a window, or down its side if
// bVertical.  Its inset is at both ends.
//
//////////////////////////////////////////////////////////////////////////
void CToolbar::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, bool bVertical)
{
	PreInit();

	CBarContainer::Init(pWindow, bVertical, pTheme->GetMetric(TM_ToolbarThickness), pTheme->GetMetric(TM_ToolbarInset));
	mpTheme = pTheme;
	maButtons.Init();
	maDisabledIcons.Init();
	AddGap(pTheme->GetMetric(TM_ToolbarInset));

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CToolbar::Class(void)
{
	CBarContainer::Class();
	M_Pointer(mpTheme);
	M_Embedded(maButtons);
	M_Embedded(maDisabledIcons);
}


//////////////////////////////////////////////////////////////////////////
//
// An object's memory is freed without its destructor running so the
// toolbar takes itself off its buttons.
//
//////////////////////////////////////////////////////////////////////////
void CToolbar::Free(void)
{
	size	i;

	for (i = 0; i < maButtons.NumElements(); i++)
	{
		if (maButtons.Get(i).IsNotNull())
		{
			maButtons.Get(i)->GetClickEvent()->Remove(this);
		}
	}
	CBarContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// A button that runs pCommand: flat with the command's icon, or a normal
// button with its caption if it has none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButton> CToolbar::AddCommand(Ptr<CCommand> pCommand, Ptr<CImageCel> pDisabledIcon)
{
	Ptr<CButton>	pButton;

	if (pCommand->GetIcon().IsNotNull())
	{
		pButton = OMalloc<CButton>(mpWindow, mpTheme, BL_Flat, pCommand->GetIcon(), (const char*)NULL);
		pButton->SetPadding(0, 0);
		pButton->SetShowsPushed(true);
	}
	else
	{
		pButton = OMalloc<CButton>(mpWindow, mpTheme, pCommand->GetCaption());
	}
	pButton->SetCommand(pCommand);
	pButton->SetAlignment(CSH_Center, CSV_Center);
	pButton->GetClickEvent()->Add(this, &CToolbar::ButtonClicked);
	Append(pButton);
	maButtons.Add(pButton);
	maDisabledIcons.Add(pDisabledIcon);
	Refresh();
	return pButton;
}


//////////////////////////////////////////////////////////////////////////
//
// Each command's button is as its command is now: on (it has the look
// of one under the pointer) while a command that is on or off is on, and
// with its disabled icon while the command can't run.
//
//////////////////////////////////////////////////////////////////////////
void CToolbar::Refresh(void)
{
	Ptr<CButton>	pButton;
	Ptr<CCommand>	pCommand;
	Ptr<CImageCel>	pIcon;
	size			i;

	for (i = 0; i < maButtons.NumElements(); i++)
	{
		pButton = maButtons.Get(i);
		pCommand = pButton->GetCommand();
		if (pButton->IsHighlighted() != pCommand->IsChecked())
		{
			pButton->SetHighlighted(pCommand->IsChecked());
		}
		if (maDisabledIcons.Get(i).IsNotNull())
		{
			pIcon = pCommand->IsEnabled() ? pCommand->GetIcon() : maDisabledIcons.Get(i);
			if (pButton->GetIcon() != pIcon)
			{
				pButton->SetIcon(pIcon);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A button ran its command, which may have changed what the others'
// commands are.
//
//////////////////////////////////////////////////////////////////////////
void CToolbar::ButtonClicked(CButton* pcButton)
{
	Refresh();
}


size CToolbar::NumCommandButtons(void) { return maButtons.NumElements(); }
Ptr<CButton> CToolbar::GetCommandButton(size uiIndex) { return maButtons.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// Whether a command's button has its disabled icon.
//
//////////////////////////////////////////////////////////////////////////
bool CToolbar::IsShownDisabled(size uiIndex)
{
	return maDisabledIcons.Get(uiIndex).IsNotNull() && (maButtons.Get(uiIndex)->GetIcon() == maDisabledIcons.Get(uiIndex));
}


//////////////////////////////////////////////////////////////////////////
//
// A line across the toolbar between the groups of its buttons.
//
//////////////////////////////////////////////////////////////////////////
void CToolbar::AddSeparator(void)
{
	AppendLine(mpTheme->GetMetric(TM_ToolbarSeparatorSpace), mpTheme->GetMetric(TM_ToolbarSeparatorLength), mpTheme->GetColour(TC_Separator));
}

