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
#include "StandardLib/Objects.h"
#include "JewelColours.h"
#include "JewelImageGenerator.h"
#include "Window.h"
#include "JewelTheme.h"


//In EThemeColour's order.
static ARGB32	gasJewelThemeColours[TC_NumColours] =
{
	JEWEL_SELECTION,		//TC_TextSelection
	JEWEL_ICON,				//TC_Caret
	JEWEL_SELECTION,		//TC_ListSelected
	JEWEL_HOVER,			//TC_ListHover
	JEWEL_EDITOR,			//TC_TabBar
	JEWEL_HOVER,			//TC_TabHover
	JEWEL_BLUE,				//TC_TabUnderline
	JEWEL_SEPARATOR,		//TC_TabSeparator
	JEWEL_PANEL,			//TC_DockTitleBar
	JEWEL_SELECTION,		//TC_DockActiveTitleBar
	JEWEL_EDITOR,			//TC_DockSash
	JEWEL_EDITOR,			//TC_DockBackground
	JEWEL_BAR_BACKGROUND,	//TC_DockStrip
	JEWEL_PANEL,			//TC_DockStripTab
	JEWEL_HOVER,			//TC_DockStripTabHover
	JEWEL_BORDER,			//TC_DockStripTabLine
	JEWEL_PANEL,			//TC_DockGuide
	JEWEL_BORDER_HOVER,		//TC_DockGuideBorder
	JEWEL_SELECTION,		//TC_DockGuideHover
	JEWEL_BLUE,				//TC_DockGuideArrow
	Set32BitColour((uint8)0x35, (uint8)0x74, (uint8)0xF0, (uint8)0x50),  //TC_DockPreview, see-through blue.
	JEWEL_BLUE,				//TC_DockPreviewBorder
	JEWEL_SEPARATOR,		//TC_DockFrameBorder
};


//////////////////////////////////////////////////////////////////////////
//
// The images are drawn in the window's colour format.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CWindowTheme> CJewelTheme::CreateDarkTheme(Ptr<CWindow> pWindow, Ptr<CFont> pFont)
{
	CColourFormatHelper		cFormat;
	Ptr<CWindowTheme>		pTheme;
	Ptr<CMenuStyle>			pComboStyle;

	pWindow->GetColourFormat(&cFormat);

	pTheme = OMalloc<CWindowTheme>(pFont);
	pTheme->SetButtonImages(CJewelImageGenerator::CreateButtonImages(&cFormat), CJewelImageGenerator::CreateDefaultButtonImages(&cFormat));
	pTheme->SetSmallButtonImages(CJewelImageGenerator::CreateSmallButtonImages(&cFormat));
	pTheme->SetToolButtonImages(CJewelImageGenerator::CreateFlatButtonImages(JEWEL_PANEL, JEWEL_HOVER, JEWEL_SEPARATOR, &cFormat));
	pTheme->SetCheckBoxImages(CJewelImageGenerator::CreateCheckBoxImages(&cFormat));
	pTheme->SetMenuStyle(OMalloc<CMenuStyle>(pWindow, pFont));

	//The combo box's items have no icons, so their text lines up with the combo box's.
	pComboStyle = OMalloc<CMenuStyle>(pWindow, pFont);
	pComboStyle->SetIconWidth(13);
	pTheme->SetComboBoxLook(pComboStyle, CJewelImageGenerator::CreateDownChevron(JEWEL_ICON, &cFormat));

	pTheme->SetScrollBarImages(OMalloc<CScrollBarImages>(8, Set32BitColour((uint8)200, (uint8)200, (uint8)200, (uint8)32), Set32BitColour((uint8)200, (uint8)200, (uint8)200, (uint8)128), &cFormat));
	SetColours(pTheme);

	//IDEA's editor tabs: a grey cross that whitens.
	pTheme->SetTabCloseImages(CJewelImageGenerator::CreateCross(JEWEL_BORDER_PUSHED, &cFormat), CJewelImageGenerator::CreateCross(JEWEL_TEXT, &cFormat));
	return pTheme;
}


//////////////////////////////////////////////////////////////////////////
//
// Every colour of the theme, as IntelliJ IDEA's dark theme's.  The editor
// tabs are flat on the editor's colour, the selected one underlined in
// blue.
//
//////////////////////////////////////////////////////////////////////////
void CJewelTheme::SetColours(Ptr<CWindowTheme> pTheme)
{
	size	i;

	for (i = 0; i < TC_NumColours; i++)
	{
		pTheme->SetColour((EThemeColour)i, gasJewelThemeColours[i]);
	}
}

