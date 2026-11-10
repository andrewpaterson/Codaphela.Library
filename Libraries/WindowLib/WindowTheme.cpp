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
#include "WindowTheme.h"


//How many times a colour, size or icon that wasn't set has been read, by any theme (see GetTotalUnsetReads).
static size	gsuiThemeUnsetReads = 0;


//In EThemeMetric's order, for the error reading one that wasn't set.
static const char*	gaszThemeMetricNames[] =
{
	"ButtonPaddingX", "ButtonPaddingY", "ButtonIconGap", "FieldPaddingX", "FieldPaddingY", "CheckBoxGap",
	"ComboChevronWidth", "ComboChevronGap", "TreeChevronWidth", "TreeIconWidth", "TreeIndent", "TreeRowPadding",
	"TabSideGap", "TabGap", "TabTopGap", "TabBottomGap", "TabUnderline", "TabIconSize", "MenuInsetX", "MenuInsetY",
	"MenuIconWidth", "ComboMenuIconWidth", "MenuChevronWidth", "MenuItemPadding", "MenuSeparatorHeight",
	"MenuBarPaddingX", "MenuBarPaddingY", "MenuSelectionRadius", "MenuSelectionInset", "MenuSeparatorInset",
	"DialogInset", "DialogLabelGap", "DialogRowGap", "DialogDividerGap", "DialogButtonGap", "DialogButtonWidth",
	"DialogMinWidth", "MessageItemsGap", "MessageItemIndent", "FileChooserWidth", "FileChooserHeight",
	"FileChooserMinWidth", "FileChooserMinHeight", "FileChooserRowHeight", "FileChooserIconWidth", "FileChooserGap",
	"GoToWidth", "GoToHeight", "GoToRowHeight", "GoToIconWidth", "GoToGap",
	"DockFrameBorder", "DockGuideSize", "DockGuideStep", "DockGuideInset", "DockGuideArrowLength", "DockGuideArrowTip",
	"DockGuideIconInset", "DockGuideIconTitle", "DockGuideBarInset", "DockGuideBarWidth", "DockGuideBarEnd",
	"DockSashSize", "DockTitleBarHeight", "DockTitleGap", "DockTitleButtonWidth",
	"AutoHideStripSize", "AutoHideTabGap", "AutoHideTabSpacing", "AutoHideLine", "AutoHideEdge",
	"ToolbarThickness", "ToolbarInset", "ToolbarSeparatorSpace", "ToolbarSeparatorLength",
	"StatusBarHeight", "StatusFieldGap", "StatusSeparatorLength",
	"ToolTipPaddingX", "ToolTipPaddingY", "ToolTipOffset",
};
static_assert(sizeof(gaszThemeMetricNames) / sizeof(gaszThemeMetricNames[0]) == TM_NumMetrics, "gaszThemeMetricNames needs a row for each EThemeMetric.");


//In EThemeImage's order, for the error reading one that wasn't set.
static const char*	gaszThemeImageNames[] =
{
	"Chevron", "DownChevron", "Pin", "Unpin", "Close", "TabClose", "TabCloseHover", "PreviousArrow", "NextArrow",
};
static_assert(sizeof(gaszThemeImageNames) / sizeof(gaszThemeImageNames[0]) == TI_NumImages, "gaszThemeImageNames needs a row for each EThemeImage.");


//In EThemeColour's order, for the error reading one that wasn't set.
static const char*	gaszThemeColourNames[] =
{
	"Text", "DimText", "BrightText", "Icon", "Panel", "Editor", "Separator", "Border", "BorderHover", "Accent",
	"AccentHover", "Success", "Failure",
	"TextSelection", "Caret", "CaretLine", "ListSelected", "ListHover", "TabBar", "TabHover", "TabUnderline", "TabSeparator",
	"MenuBar", "MenuBackground", "MenuBorder", "MenuSelected", "MenuHover", "MenuSeparator", "DialogBackground",
	"DialogDivider", "LineNumber", "LineNumberActive", "Gutter", "DiffInsertedLines", "DiffDeletedLines",
	"DiffModifiedLines", "DiffInsertedChanges", "DiffDeletedChanges", "DiffModifiedChanges", "DiffDivider",
	"DockTitleBar", "DockActiveTitleBar", "DockSash", "DockBackground", "DockStrip", "DockStripTab", "DockStripTabHover",
	"DockStripTabLine", "DockGuide", "DockGuideBorder", "DockGuideHover", "DockGuideArrow", "DockPreview",
	"DockPreviewBorder", "DockFrameBorder",
	"ToolTip", "ToolTipBorder",
};
static_assert(sizeof(gaszThemeColourNames) / sizeof(gaszThemeColourNames[0]) == TC_NumColours, "gaszThemeColourNames needs a row for each EThemeColour.");


//////////////////////////////////////////////////////////////////////////
//
// pFont can be NULL for a theme whose widgets have no text.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::Init(Ptr<CFont> pFont)
{
	size	i;

	PreInit();

	mpFont = pFont;
	mpButtonImages = NULL;
	mpDefaultButtonImages = NULL;
	mpSmallButtonImages = NULL;
	mpToolButtonImages = NULL;
	mpMenuBarButtonImages = NULL;
	mpCheckBoxImages = NULL;
	mpMenuStyle = NULL;
	mpComboMenuStyle = NULL;
	mpScrollBarImages = NULL;
	maImages.Init();
	for (i = 0; i < TI_NumImages; i++)
	{
		maImages.Add(NULL);
	}
	memset(masColours, 0, sizeof(masColours));
	memset(mabColoursSet, 0, sizeof(mabColoursSet));
	memset(maiMetrics, 0, sizeof(maiMetrics));
	memset(mabMetricsSet, 0, sizeof(mabMetricsSet));
	mcChangeEvent.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::Class(void)
{
	M_Pointer(mpFont);
	M_Pointer(mpButtonImages);
	M_Pointer(mpDefaultButtonImages);
	M_Pointer(mpSmallButtonImages);
	M_Pointer(mpToolButtonImages);
	M_Pointer(mpMenuBarButtonImages);
	M_Pointer(mpCheckBoxImages);
	M_Pointer(mpMenuStyle);
	M_Pointer(mpComboMenuStyle);
	M_Pointer(mpScrollBarImages);
	M_Embedded(maImages);
	U_UInt32Array(masColours, TC_NumColours);
	U_BoolArray(mabColoursSet, TC_NumColours);
	U_Int32Array(maiMetrics, TM_NumMetrics);
	U_BoolArray(mabMetricsSet, TM_NumMetrics);
	U_Unknown(CComponentEvent<CWindowTheme>, mcChangeEvent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::Free(void)
{
	mcChangeEvent.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindowTheme::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWindowTheme::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// pDefaultImages can be NULL, a default button then looks like any other.
// It must have the same tile size as pImages.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetButtonImages(Ptr<CButtonImages> pImages, Ptr<CButtonImages> pDefaultImages)
{
	mpButtonImages = pImages;
	mpDefaultButtonImages = pDefaultImages;
}


//////////////////////////////////////////////////////////////////////////
//
// A combo box's list is a menu, it has its own style so that it can have
// a narrower icon column than the main menus.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetComboMenuStyle(Ptr<CMenuStyle> pStyle)
{
	mpComboMenuStyle = pStyle;
}


//////////////////////////////////////////////////////////////////////////
//
// Set before the widgets that use it are made.  Nothing is told until
// Changed, so a theme can be set up a colour at a time.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetColour(EThemeColour eColour, ARGB32 sColour)
{
	masColours[eColour] = sColour;
	mabColoursSet[eColour] = true;
}


//////////////////////////////////////////////////////////////////////////
//
// A colour that wasn't set is an error: the theme's maker must set every
// colour its widgets use.  It is transparent black.
//
//////////////////////////////////////////////////////////////////////////
ARGB32 CWindowTheme::GetColour(EThemeColour eColour)
{
	if (!mabColoursSet[eColour])
	{
		gsuiThemeUnsetReads++;
		gcLogger.Error2(__METHOD__, " Theme colour [", gaszThemeColourNames[eColour], "] isn't set.", NULL);
	}
	return masColours[eColour];
}


//////////////////////////////////////////////////////////////////////////
//
// As SetColour.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetImage(EThemeImage eImage, Ptr<CImageCel> pImage)
{
	maImages.Set(eImage, pImage);
}


//////////////////////////////////////////////////////////////////////////
//
// An icon that wasn't set is an error, as a colour is.  NULL.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CWindowTheme::GetImage(EThemeImage eImage)
{
	Ptr<CImageCel>	pImage;

	pImage = maImages.Get(eImage);
	if (pImage.IsNull())
	{
		gsuiThemeUnsetReads++;
		gcLogger.Error2(__METHOD__, " Theme image [", gaszThemeImageNames[eImage], "] isn't set.", NULL);
	}
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// As SetColour.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetMetric(EThemeMetric eMetric, int32 iPixels)
{
	maiMetrics[eMetric] = iPixels;
	mabMetricsSet[eMetric] = true;
}


//////////////////////////////////////////////////////////////////////////
//
// A size that wasn't set is an error, as a colour is.  Zero.
//
//////////////////////////////////////////////////////////////////////////
int32 CWindowTheme::GetMetric(EThemeMetric eMetric)
{
	if (!mabMetricsSet[eMetric])
	{
		gsuiThemeUnsetReads++;
		gcLogger.Error2(__METHOD__, " Theme metric [", gaszThemeMetricNames[eMetric], "] isn't set.", NULL);
	}
	return maiMetrics[eMetric];
}


//////////////////////////////////////////////////////////////////////////
//
// Tells whoever listens to the theme's change event that its colours,
// sizes or images changed.  Widgets already made don't change: they read
// the theme as they were made.
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::Changed(void)
{
	mcChangeEvent.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// A small button is a normal one if the theme hasn't small images.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> CWindowTheme::GetButtonImages(EButtonLook eLook)
{
	if (eLook == BL_Flat)
	{
		return mpToolButtonImages;
	}
	if ((eLook == BL_Small) && mpSmallButtonImages.IsNotNull())
	{
		return mpSmallButtonImages;
	}
	return mpButtonImages;
}


//////////////////////////////////////////////////////////////////////////
//
// Only a normal button is drawn as a default button: small and tool
// buttons' images haven't the default images' tile size.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButtonImages> CWindowTheme::GetDefaultButtonImages(EButtonLook eLook)
{
	if (GetButtonImages(eLook) != mpButtonImages)
	{
		return NULL;
	}
	return mpDefaultButtonImages;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWindowTheme::SetFont(Ptr<CFont> pFont) { mpFont = pFont; }
void CWindowTheme::SetSmallButtonImages(Ptr<CButtonImages> pImages) { mpSmallButtonImages = pImages; }
void CWindowTheme::SetToolButtonImages(Ptr<CButtonImages> pImages) { mpToolButtonImages = pImages; }
void CWindowTheme::SetMenuBarButtonImages(Ptr<CButtonImages> pImages) { mpMenuBarButtonImages = pImages; }
Ptr<CButtonImages> CWindowTheme::GetMenuBarButtonImages(void) { return mpMenuBarButtonImages; }
void CWindowTheme::SetCheckBoxImages(Ptr<CCheckBoxImages> pImages) { mpCheckBoxImages = pImages; }
void CWindowTheme::SetMenuStyle(Ptr<CMenuStyle> pStyle) { mpMenuStyle = pStyle; }
void CWindowTheme::SetScrollBarImages(Ptr<CScrollBarImages> pImages) { mpScrollBarImages = pImages; }
Ptr<CFont> CWindowTheme::GetFont(void) { return mpFont; }
Ptr<CCheckBoxImages> CWindowTheme::GetCheckBoxImages(void) { return mpCheckBoxImages; }
Ptr<CMenuStyle> CWindowTheme::GetMenuStyle(void) { return mpMenuStyle; }
Ptr<CMenuStyle> CWindowTheme::GetComboMenuStyle(void) { return mpComboMenuStyle; }
Ptr<CScrollBarImages> CWindowTheme::GetScrollBarImages(void) { return mpScrollBarImages; }
bool CWindowTheme::IsColourSet(EThemeColour eColour) { return mabColoursSet[eColour]; }
bool CWindowTheme::IsImageSet(EThemeImage eImage) { return maImages.Get(eImage).IsNotNull(); }
bool CWindowTheme::IsMetricSet(EThemeMetric eMetric) { return mabMetricsSet[eMetric]; }
CComponentEvent<CWindowTheme>* CWindowTheme::GetChangeEvent(void) { return &mcChangeEvent; }


//////////////////////////////////////////////////////////////////////////
//
// The names a colour, size or icon is reported by when it isn't set.
//
//////////////////////////////////////////////////////////////////////////
const char* CWindowTheme::GetColourName(EThemeColour eColour) { return gaszThemeColourNames[eColour]; }
const char* CWindowTheme::GetMetricName(EThemeMetric eMetric) { return gaszThemeMetricNames[eMetric]; }
const char* CWindowTheme::GetImageName(EThemeImage eImage) { return gaszThemeImageNames[eImage]; }


//////////////////////////////////////////////////////////////////////////
//
// How many times any theme's colour, size or icon has been read without
// being set: an error each time, which tests check didn't happen.
//
//////////////////////////////////////////////////////////////////////////
size CWindowTheme::GetTotalUnsetReads(void) { return gsuiThemeUnsetReads; }
