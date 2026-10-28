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
#ifndef __JEWEL_THEME_H__
#define __JEWEL_THEME_H__
#include "WindowTheme.h"


class CWindow;


//Makes the theme of IntelliJ IDEA's new UI in its dark (Jewel) colours, drawn at runtime by CJewelImageGenerator: outlined
//buttons with a blue default button, flat tool buttons, check boxes, the main menu's style, a combo box's narrower menu
//style and chevron, translucent scroll bar thumbs and the selection colours.  The font, Inter 13 in IntelliJ, is given
//in the colour the text is drawn in.
class CJewelTheme
{
public:
	static Ptr<CWindowTheme>	CreateDarkTheme(Ptr<CWindow> pWindow, Ptr<CFont> pFont);
};


#endif // __JEWEL_THEME_H__

