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
#ifndef __THEME_METRIC_H__
#define __THEME_METRIC_H__


//The sizes in pixels a window theme keeps (CWindowTheme::GetMetric), each set by whoever makes the theme (CJewelTheme).
enum EThemeMetric
{
	TM_ButtonPaddingX,			//Between a button's border and its content, each side.
	TM_ButtonPaddingY,
	TM_ButtonIconGap,			//Between a button's icon and its caption.
	TM_FieldPaddingX,			//Between a text field's or combo box's border and its text.
	TM_FieldPaddingY,
	TM_CheckBoxGap,				//Between a check box's box and its caption.
	TM_ComboChevronWidth,		//A combo box's chevron.
	TM_ComboChevronGap,			//The least space between a combo box's widest item and its chevron.
	TM_TreeChevronWidth,		//A tree node's chevron.
	TM_TreeIconWidth,			//A tree node's icon.
	TM_TreeIndent,				//How far each depth of a tree is in.
	TM_TreeRowPadding,			//Added to the font's line height for a tree's row.
	TM_TabSideGap,				//From a tab's sides to its icon, name or cross.
	TM_TabGap,					//Between a tab's icon, name and cross.
	TM_TabTopGap,				//Above a tab's name.
	TM_TabBottomGap,			//Between a tab's name and its underline.
	TM_TabUnderline,			//The selected tab's underline.
	TM_TabIconSize,				//A tab's icon and close cross, square.
	TM_MenuInsetX,				//Between a menu's popup and its items, the border included, at the sides.
	TM_MenuInsetY,				//And at the top and bottom.
	TM_MenuIconWidth,			//A menu's icon column.
	TM_ComboMenuIconWidth,		//A combo box's list's icon column.
	TM_MenuChevronWidth,		//A menu's chevron column.
	TM_MenuItemPadding,			//Above and below a menu item's label.
	TM_MenuSeparatorHeight,
	TM_MenuBarPaddingX,			//Around a menu bar button's caption, inside its border.
	TM_MenuBarPaddingY,
	TM_MenuSelectionRadius,		//Of the rounded block behind a menu's selected item.
	TM_MenuSelectionInset,		//Between the selected item's block and the menu's sides.
	TM_MenuSeparatorInset,		//Between a separator's line and the menu's sides.

	TM_NumMetrics,
};


#endif // __THEME_METRIC_H__
