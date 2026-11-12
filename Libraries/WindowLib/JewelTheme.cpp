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


//In EThemeMetric's order.
static int32	gaiJewelThemeMetrics[] =
{
	8,		//TM_ButtonPaddingX
	0,		//TM_ButtonPaddingY, so buttons, fields and combo boxes are 29 high, Jewel's are 28.
	4,		//TM_ButtonIconGap
	6,		//TM_FieldPaddingX
	0,		//TM_FieldPaddingY
	6,		//TM_CheckBoxGap
	16,		//TM_ComboChevronWidth
	8,		//TM_ComboChevronGap
	16,		//TM_TreeChevronWidth
	20,		//TM_TreeIconWidth
	16,		//TM_TreeIndent
	4,		//TM_TreeRowPadding
	10,		//TM_TabSideGap
	6,		//TM_TabGap
	7,		//TM_TabTopGap
	4,		//TM_TabBottomGap
	3,		//TM_TabUnderline
	16,		//TM_TabIconSize
	1,		//TM_MenuInsetX
	6,		//TM_MenuInsetY
	32,		//TM_MenuIconWidth
	13,		//TM_ComboMenuIconWidth, so a combo box's items line up with its text.
	24,		//TM_MenuChevronWidth
	5,		//TM_MenuItemPadding
	9,		//TM_MenuSeparatorHeight
	1,		//TM_MenuBarPaddingX
	1,		//TM_MenuBarPaddingY
	4,		//TM_MenuSelectionRadius
	6,		//TM_MenuSelectionInset
	8,		//TM_MenuSeparatorInset
	16,		//TM_DialogInset
	12,		//TM_DialogLabelGap
	4,		//TM_DialogRowGap
	10,		//TM_DialogDividerGap
	16,		//TM_DialogButtonGap
	72,		//TM_DialogButtonWidth
	360,	//TM_DialogMinWidth
	8,		//TM_MessageItemsGap
	16,		//TM_MessageItemIndent
	620,	//TM_FileChooserWidth
	440,	//TM_FileChooserHeight
	420,	//TM_FileChooserMinWidth
	300,	//TM_FileChooserMinHeight
	24,		//TM_FileChooserRowHeight
	30,		//TM_FileChooserIconWidth
	8,		//TM_FileChooserGap
	720,	//TM_GoToWidth
	480,	//TM_GoToHeight
	22,		//TM_GoToRowHeight
	22,		//TM_GoToIconWidth
	8,		//TM_GoToGap
	900,	//TM_FindWidth
	680,	//TM_FindHeight
	4,		//TM_DockFrameBorder
	30,		//TM_DockGuideSize
	32,		//TM_DockGuideStep
	8,		//TM_DockGuideInset
	6,		//TM_DockGuideArrowLength
	3,		//TM_DockGuideArrowTip
	8,		//TM_DockGuideIconInset
	3,		//TM_DockGuideIconTitle
	5,		//TM_DockGuideBarInset
	2,		//TM_DockGuideBarWidth
	7,		//TM_DockGuideBarEnd
	4,		//TM_DockSashSize
	26,		//TM_DockTitleBarHeight
	8,		//TM_DockTitleGap
	22,		//TM_DockTitleButtonWidth
	24,		//TM_AutoHideStripSize
	10,		//TM_AutoHideTabGap
	4,		//TM_AutoHideTabSpacing
	3,		//TM_AutoHideLine
	4,		//TM_AutoHideEdge
	34,		//TM_ToolbarThickness
	4,		//TM_ToolbarInset
	13,		//TM_ToolbarSeparatorSpace
	22,		//TM_ToolbarSeparatorLength
	24,		//TM_StatusBarHeight
	8,		//TM_StatusFieldGap
	16,		//TM_StatusSeparatorLength
	8,		//TM_ToolTipPaddingX
	5,		//TM_ToolTipPaddingY
	20,		//TM_ToolTipOffset
};
static_assert(sizeof(gaiJewelThemeMetrics) / sizeof(gaiJewelThemeMetrics[0]) == TM_NumMetrics, "gaiJewelThemeMetrics needs a row for each EThemeMetric.");


//In EThemeColour's order.
static ARGB32	gasJewelThemeColours[] =
{
	JEWEL_TEXT,				//TC_Text
	JEWEL_LINE_NUMBER,		//TC_DimText
	JEWEL_WHITE,			//TC_BrightText
	JEWEL_ICON,				//TC_Icon
	JEWEL_PANEL,			//TC_Panel
	JEWEL_EDITOR,			//TC_Editor
	JEWEL_SEPARATOR,		//TC_Separator
	JEWEL_BORDER,			//TC_Border
	JEWEL_BORDER_HOVER,		//TC_BorderHover
	JEWEL_BLUE,				//TC_Accent
	JEWEL_BLUE_HOVER,		//TC_AccentHover
	JEWEL_GREEN,			//TC_Success
	JEWEL_RED,				//TC_Failure
	JEWEL_SELECTION,		//TC_TextSelection
	JEWEL_OCCURRENCE,		//TC_TextOccurrence
	JEWEL_WRITE_OCCURRENCE,	//TC_TextWriteOccurrence
	JEWEL_ICON,				//TC_Caret
	JEWEL_CARET_LINE,		//TC_CaretLine
	JEWEL_SEARCH_MATCH,		//TC_SearchMatch
	JEWEL_SELECTION,		//TC_ListSelected
	JEWEL_HOVER,			//TC_ListHover
	JEWEL_EDITOR,			//TC_TabBar
	JEWEL_HOVER,			//TC_TabHover
	JEWEL_BLUE,				//TC_TabUnderline
	JEWEL_SEPARATOR,		//TC_TabSeparator
	JEWEL_BAR_BACKGROUND,	//TC_MenuBar
	JEWEL_PANEL,			//TC_MenuBackground
	JEWEL_SEPARATOR,		//TC_MenuBorder
	JEWEL_SELECTION,		//TC_MenuSelected
	JEWEL_HOVER,			//TC_MenuHover
	JEWEL_SEPARATOR,		//TC_MenuSeparator
	JEWEL_PANEL,			//TC_DialogBackground
	JEWEL_SEPARATOR,		//TC_DialogDivider
	JEWEL_LINE_NUMBER,		//TC_LineNumber
	JEWEL_LINE_NUMBER_ACTIVE,	//TC_LineNumberActive
	JEWEL_PANEL,			//TC_Gutter
	Set32BitColour((uint8)0x29, (uint8)0x44, (uint8)0x36),  //TC_DiffInsertedLines
	Set32BitColour((uint8)0x48, (uint8)0x4A, (uint8)0x4A),  //TC_DiffDeletedLines
	Set32BitColour((uint8)0x38, (uint8)0x55, (uint8)0x70),  //TC_DiffModifiedLines
	Set32BitColour((uint8)0x44, (uint8)0x71, (uint8)0x52),  //TC_DiffInsertedChanges
	Set32BitColour((uint8)0x65, (uint8)0x6E, (uint8)0x76),  //TC_DiffDeletedChanges
	Set32BitColour((uint8)0x43, (uint8)0x69, (uint8)0x8D),  //TC_DiffModifiedChanges
	Set32BitColour((uint8)0x1E, (uint8)0x1F, (uint8)0x22),  //TC_DiffDivider
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
	JEWEL_PANEL,			//TC_ToolTip
	JEWEL_SEPARATOR,		//TC_ToolTipBorder
};
static_assert(sizeof(gasJewelThemeColours) / sizeof(gasJewelThemeColours[0]) == TC_NumColours, "gasJewelThemeColours needs a row for each EThemeColour.");


//////////////////////////////////////////////////////////////////////////
//
// The theme's font is pWhiteColours' in the theme's text colour, so an
// application needn't know the colour.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CWindowTheme> CJewelTheme::CreateDarkTheme(Ptr<CWindow> pWindow, Ptr<CFontColours> pWhiteColours)
{
	return CreateDarkTheme(pWindow, pWhiteColours->GetFont(gasJewelThemeColours[TC_Text]));
}


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

	pTheme->SetScrollBarImages(OMalloc<CScrollBarImages>(8, Set32BitColour((uint8)200, (uint8)200, (uint8)200, (uint8)32), Set32BitColour((uint8)200, (uint8)200, (uint8)200, (uint8)128), &cFormat));
	SetColours(pTheme);
	SetMetrics(pTheme);
	SetImages(pTheme, pWindow);

	//The menus read the theme.  The combo box's items have no icons, so their text lines up with the combo box's.
	pTheme->SetMenuStyle(OMalloc<CMenuStyle>(pTheme));
	pComboStyle = OMalloc<CMenuStyle>(pTheme);
	pComboStyle->SetIconWidth(pTheme->GetMetric(TM_ComboMenuIconWidth));
	pTheme->SetComboMenuStyle(pComboStyle);
	return pTheme;
}


//////////////////////////////////////////////////////////////////////////
//
// Every size of the theme, in pixels.
//
//////////////////////////////////////////////////////////////////////////
void CJewelTheme::SetMetrics(Ptr<CWindowTheme> pTheme)
{
	size	i;

	for (i = 0; i < TM_NumMetrics; i++)
	{
		pTheme->SetMetric((EThemeMetric)i, gaiJewelThemeMetrics[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every icon of the theme, and the menu bar's flat button images, drawn
// in the window's colour format.  IDEA's
// editor tabs have a grey cross that whitens under the pointer.
//
//////////////////////////////////////////////////////////////////////////
void CJewelTheme::SetImages(Ptr<CWindowTheme> pTheme, Ptr<CWindow> pWindow)
{
	CColourFormatHelper		cFormat;

	pWindow->GetColourFormat(&cFormat);
	pTheme->SetImage(TI_Chevron, CJewelImageGenerator::CreateChevron(JEWEL_ICON, &cFormat));
	pTheme->SetImage(TI_DownChevron, CJewelImageGenerator::CreateDownChevron(JEWEL_ICON, &cFormat));
	pTheme->SetImage(TI_Pin, CJewelImageGenerator::CreatePin(JEWEL_ICON, true, &cFormat));
	pTheme->SetImage(TI_Unpin, CJewelImageGenerator::CreatePin(JEWEL_ICON, false, &cFormat));
	pTheme->SetImage(TI_Close, CJewelImageGenerator::CreateCross(JEWEL_ICON, &cFormat));
	pTheme->SetImage(TI_TabClose, CJewelImageGenerator::CreateCross(JEWEL_BORDER_PUSHED, &cFormat));
	pTheme->SetImage(TI_TabCloseHover, CJewelImageGenerator::CreateCross(JEWEL_TEXT, &cFormat));
	pTheme->SetImage(TI_PreviousArrow, CJewelImageGenerator::CreateArrow(JEWEL_ICON, true, &cFormat));
	pTheme->SetImage(TI_NextArrow, CJewelImageGenerator::CreateArrow(JEWEL_ICON, false, &cFormat));
	pTheme->SetMenuBarButtonImages(CJewelImageGenerator::CreateFlatButtonImages(JEWEL_BAR_BACKGROUND, JEWEL_HOVER, JEWEL_HOVER, &cFormat));
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

