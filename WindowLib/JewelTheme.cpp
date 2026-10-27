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
#include "JewelButtonImages.h"
#include "Window.h"
#include "JewelTheme.h"


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
	pTheme->SetButtonImages(CJewelButtonImages::CreateButtonImages(&cFormat), CJewelButtonImages::CreateDefaultButtonImages(&cFormat));
	pTheme->SetToolButtonImages(CJewelButtonImages::CreateFlatButtonImages(JEWEL_PANEL, JEWEL_HOVER, JEWEL_SEPARATOR, &cFormat));
	pTheme->SetCheckBoxImages(CJewelButtonImages::CreateCheckBoxImages(&cFormat));
	pTheme->SetMenuStyle(OMalloc<CMenuStyle>(pWindow, pFont));

	//The combo box's items have no icons, so their text lines up with the combo box's.
	pComboStyle = OMalloc<CMenuStyle>(pWindow, pFont);
	pComboStyle->SetIconWidth(13);
	pTheme->SetComboBoxLook(pComboStyle, CJewelButtonImages::CreateDownChevron(JEWEL_ICON, &cFormat));

	pTheme->SetScrollBarImages(OMalloc<CScrollBarImages>(8, Set32BitColour((uint8)200, (uint8)200, (uint8)200, (uint8)128), &cFormat));
	pTheme->SetTextColours(JEWEL_SELECTION, JEWEL_ICON);
	pTheme->SetListColours(JEWEL_SELECTION, JEWEL_HOVER);

	//IDEA's editor tabs: flat on the editor's colour, the selected one underlined in blue, a grey cross that whitens.
	pTheme->SetTabLook(JEWEL_EDITOR, JEWEL_HOVER, JEWEL_BLUE, JEWEL_SEPARATOR, CJewelButtonImages::CreateCross(JEWEL_BORDER_PUSHED, &cFormat), CJewelButtonImages::CreateCross(JEWEL_TEXT, &cFormat));
	return pTheme;
}

