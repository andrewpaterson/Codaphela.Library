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


//In EThemeMetric's order, for the error reading one that wasn't set.
static const char*	gaszThemeMetricNames[TM_NumMetrics] =
{
	"ButtonPaddingX", "ButtonPaddingY", "ButtonIconGap", "FieldPaddingX", "FieldPaddingY", "CheckBoxGap",
	"ComboChevronWidth", "ComboChevronGap", "TreeChevronWidth", "TreeIconWidth", "TreeIndent", "TreeRowPadding",
	"TabSideGap", "TabGap", "TabTopGap", "TabBottomGap", "TabUnderline", "TabIconSize", "MenuInsetX", "MenuInsetY",
	"MenuIconWidth", "ComboMenuIconWidth", "MenuChevronWidth", "MenuItemPadding", "MenuSeparatorHeight",
	"MenuBarPaddingX", "MenuBarPaddingY", "MenuSelectionRadius", "MenuSelectionInset", "MenuSeparatorInset",
};


//In EThemeImage's order, for the error reading one that wasn't set.
static const char*	gaszThemeImageNames[TI_NumImages] =
{
	"Chevron", "DownChevron", "Pin", "Unpin", "Close", "TabClose", "TabCloseHover", "PreviousArrow", "NextArrow",
};


//In EThemeColour's order, for the error reading one that wasn't set.
static const char*	gaszThemeColourNames[TC_NumColours] =
{
	"TextSelection", "Caret", "ListSelected", "ListHover", "TabBar", "TabHover", "TabUnderline", "TabSeparator",
	"MenuBar", "MenuBackground", "MenuBorder", "MenuSelected", "MenuHover", "MenuSeparator",
	"DockTitleBar", "DockActiveTitleBar", "DockSash", "DockBackground", "DockStrip", "DockStripTab", "DockStripTabHover",
	"DockStripTabLine", "DockGuide", "DockGuideBorder", "DockGuideHover", "DockGuideArrow", "DockPreview",
	"DockPreviewBorder", "DockFrameBorder",
};


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
// Its users aren't told until Changed, so a theme can be set up a colour
// at a time.
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
		gcLogger.Error2(__METHOD__, " Theme metric [", gaszThemeMetricNames[eMetric], "] isn't set.", NULL);
	}
	return maiMetrics[eMetric];
}


//////////////////////////////////////////////////////////////////////////
//
// Tells the theme's users (its change event) that its colours, sizes or
// images changed, so they lay out and draw again.
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

