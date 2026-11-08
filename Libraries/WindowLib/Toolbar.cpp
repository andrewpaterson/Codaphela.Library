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
// bVertical.
//
//////////////////////////////////////////////////////////////////////////
void CToolbar::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, bool bVertical)
{
	PreInit();

	CBarContainer::Init(pWindow, bVertical, pTheme->GetMetric(TM_ToolbarThickness), 0);
	mpTheme = pTheme;
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
}


//////////////////////////////////////////////////////////////////////////
//
// A button that runs pCommand: flat with the command's icon, or a normal
// button with its caption if it has none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButton> CToolbar::AddCommand(Ptr<CCommand> pCommand)
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
	Append(pButton);
	return pButton;
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

