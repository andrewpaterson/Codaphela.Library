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
	TM_DialogInset,				//Between a dialog's edge and its form and buttons.
	TM_DialogLabelGap,			//Between a dialog's label and its editor.
	TM_DialogRowGap,			//Between a dialog's rows.
	TM_DialogDividerGap,		//Above a dialog's divider.
	TM_DialogButtonGap,			//Between a dialog's form and its buttons.
	TM_DialogButtonWidth,		//A dialog's buttons, unless a caption needs more.
	TM_DialogMinWidth,			//The narrowest a dialog sized to what it holds is.
	TM_MessageItemsGap,			//Between a message dialog's message and its items.
	TM_MessageItemIndent,		//Of each of a message dialog's items.
	TM_FileChooserWidth,		//A file chooser as it opens.
	TM_FileChooserHeight,
	TM_FileChooserMinWidth,		//The smallest a file chooser can be resized to.
	TM_FileChooserMinHeight,
	TM_FileChooserRowHeight,	//A file chooser's list rows.
	TM_FileChooserIconWidth,	//The column of a file chooser's icons; a name starts after it.
	TM_FileChooserGap,			//Between a file chooser's rows, and between its buttons along the top.
	TM_GoToWidth,				//A Go To dialog as it opens; it can be resized to half this.
	TM_GoToHeight,
	TM_GoToRowHeight,			//A Go To dialog's result rows.
	TM_GoToIconWidth,			//The column of a Go To dialog's icons.
	TM_GoToGap,					//Between a Go To dialog's field and results, and after a result's name.
	TM_DockFrameBorder,			//A floating window's edge, dragged to resize it.
	TM_DockGuideSize,			//A docking guide's square.
	TM_DockGuideStep,			//From one guide of a compass to the next.
	TM_DockGuideInset,			//An edge guide from the dock area's edge.
	TM_DockGuideArrowLength,	//A guide's arrow, from its tip to its widest.
	TM_DockGuideArrowTip,		//From a guide's middle to its arrow's tip.
	TM_DockGuideIconInset,		//A centre guide's window from the guide's edge.
	TM_DockGuideIconTitle,		//That window's title bar.
	TM_DockGuideBarInset,		//An outer guide's bar from the edge it is along.
	TM_DockGuideBarWidth,
	TM_DockGuideBarEnd,			//The bar's ends from the guide's other edges.
	TM_DockSashSize,			//Between a split's children.
	TM_DockTitleBarHeight,		//A tool group's title bar.
	TM_DockTitleGap,			//Before a title bar's title.
	TM_DockTitleButtonWidth,	//A title bar's buttons.
	TM_AutoHideStripSize,		//An auto hide strip's thickness.
	TM_AutoHideTabGap,			//Before and after a strip tab's text.
	TM_AutoHideTabSpacing,		//Between a strip's tabs.
	TM_AutoHideLine,			//A strip tab's line along the strip's outer side.
	TM_AutoHideEdge,			//A flyout's inner edge, dragged to resize it.
	TM_ToolbarThickness,		//A toolbar's height, or a vertical one's width.
	TM_ToolbarInset,			//Before a toolbar's first item, and after its last.
	TM_ToolbarSeparatorSpace,	//Along a toolbar, the space a separator takes.
	TM_ToolbarSeparatorLength,	//Across a toolbar, a separator's line.
	TM_StatusBarHeight,
	TM_StatusFieldGap,			//Before each of a status bar's fields, and after the last.
	TM_StatusSeparatorLength,	//A status bar separator's line.
	TM_ToolTipPaddingX,			//Between a tool tip's border and its text.
	TM_ToolTipPaddingY,
	TM_ToolTipOffset,			//From the pointer down to a tool tip's top, clear of the pointer's shape.

	TM_NumMetrics,
};


#endif // __THEME_METRIC_H__
