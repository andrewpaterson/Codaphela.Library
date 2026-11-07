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
#ifndef __THEME_IMAGE_H__
#define __THEME_IMAGE_H__


//The icons a window theme keeps (CWindowTheme::GetImage), each set by whoever makes the theme (CJewelTheme).
enum EThemeImage
{
	TI_Chevron,				//Pointing right: a menu item's submenu, a collapsed tree node.
	TI_DownChevron,			//An expanded tree node, a combo box, a tool window's menu.
	TI_Pin,					//A tool window pinned open.
	TI_Unpin,				//A tool window that hides itself.
	TI_Close,				//A tool window's close cross.
	TI_TabClose,			//A tab's close cross.
	TI_TabCloseHover,		//A tab's close cross under the pointer.
	TI_PreviousArrow,		//Up: the previous difference.
	TI_NextArrow,			//Down: the next difference.

	TI_NumImages,
};


#endif // __THEME_IMAGE_H__
