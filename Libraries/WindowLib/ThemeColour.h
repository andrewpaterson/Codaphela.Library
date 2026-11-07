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
#ifndef __THEME_COLOUR_H__
#define __THEME_COLOUR_H__


//The colours a window theme keeps (CWindowTheme::GetColour), each set by whoever makes the theme (CJewelTheme).
enum EThemeColour
{
	TC_TextSelection,			//Behind selected text.
	TC_Caret,
	TC_ListSelected,			//A list's selected row.
	TC_ListHover,				//A list's row under the pointer.
	TC_TabBar,					//Behind a tabbed container's tabs.
	TC_TabHover,				//Behind a tab under the pointer.
	TC_TabUnderline,			//Under the selected tab.
	TC_TabSeparator,			//The line between the tabs and the page.
	TC_MenuBar,					//Behind the menu bar.
	TC_MenuBackground,			//Behind a menu's items.
	TC_MenuBorder,				//A menu's edge.
	TC_MenuSelected,			//Behind a menu's selected item.
	TC_MenuHover,				//Behind a menu bar button under the pointer.
	TC_MenuSeparator,			//A menu's separator line.
	TC_DialogBackground,		//Behind a dialog's form.
	TC_DialogDivider,			//A dialog's divider line.
	TC_LineNumber,				//A code editor's line numbers, and a listing out of date.
	TC_LineNumberActive,		//Line numbers on a change band.
	TC_Gutter,					//Behind a code editor's line numbers.
	TC_DiffInsertedLines,		//Behind a diff's inserted lines.
	TC_DiffDeletedLines,
	TC_DiffModifiedLines,
	TC_DiffInsertedChanges,		//Over the characters that changed in a diff's inserted block.
	TC_DiffDeletedChanges,
	TC_DiffModifiedChanges,
	TC_DiffDivider,				//Behind a diff's divider shapes.

	//Docking's (see CDockManager).
	TC_DockTitleBar,			//A tool group's title bar.
	TC_DockActiveTitleBar,		//The active pane's group's title bar.
	TC_DockSash,				//Between a split's nodes.
	TC_DockBackground,			//The empty document well, and behind the dock area.
	TC_DockStrip,				//An auto hide strip.
	TC_DockStripTab,			//A tab on an auto hide strip.
	TC_DockStripTabHover,
	TC_DockStripTabLine,		//Along the strip side of a strip's tab.
	TC_DockGuide,				//A docking guide's face.
	TC_DockGuideBorder,
	TC_DockGuideHover,			//The guide under the pointer.
	TC_DockGuideArrow,
	TC_DockPreview,				//Where a pane would go, drawn over the window (with its opacity).
	TC_DockPreviewBorder,
	TC_DockFrameBorder,			//A floating window's edge.

	TC_NumColours,
};


#endif // __THEME_COLOUR_H__
