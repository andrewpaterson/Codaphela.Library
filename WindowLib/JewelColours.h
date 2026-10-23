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
#ifndef __JEWEL_COLOURS_H__
#define __JEWEL_COLOURS_H__
#include "SupportLib/ColourARGB32.h"


//The colours of JetBrains' Jewel dark theme, IntelliJ IDEA's new UI, that WindowLib's Jewel looks are drawn in.
#define JEWEL_BAR_BACKGROUND	Set32BitColour((uint8)0x27, (uint8)0x28, (uint8)0x2E)  //The main menu bar.
#define JEWEL_PANEL				Set32BitColour((uint8)0x2B, (uint8)0x2D, (uint8)0x30)  //Panels, menus, dialogs and button faces.
#define JEWEL_EDITOR			Set32BitColour((uint8)0x1E, (uint8)0x1F, (uint8)0x22)  //The editor, and the lines between panels.
#define JEWEL_BORDER			Set32BitColour((uint8)0x4E, (uint8)0x51, (uint8)0x57)  //A control's border.
#define JEWEL_BORDER_HOVER		Set32BitColour((uint8)0x6F, (uint8)0x73, (uint8)0x7A)  //A control's border under the pointer.
#define JEWEL_BORDER_PUSHED		Set32BitColour((uint8)0x86, (uint8)0x8A, (uint8)0x91)  //A control's border while pushed.
#define JEWEL_SEPARATOR			Set32BitColour((uint8)0x43, (uint8)0x45, (uint8)0x4A)  //Separators and a menu's border.
#define JEWEL_HOVER				Set32BitColour((uint8)0x39, (uint8)0x3B, (uint8)0x40)  //A row or flat button under the pointer.
#define JEWEL_SELECTION			Set32BitColour((uint8)0x2E, (uint8)0x43, (uint8)0x6E)  //Selected rows and text.
#define JEWEL_BLUE				Set32BitColour((uint8)0x35, (uint8)0x74, (uint8)0xF0)  //Focus rings, default buttons and ticked boxes.
#define JEWEL_BLUE_HOVER		Set32BitColour((uint8)0x46, (uint8)0x82, (uint8)0xFA)
#define JEWEL_BLUE_PUSHED		Set32BitColour((uint8)0x36, (uint8)0x6A, (uint8)0xCE)
#define JEWEL_ICON				Set32BitColour((uint8)0xCE, (uint8)0xD0, (uint8)0xD6)  //Icons, chevrons and the caret.
#define JEWEL_TEXT				Set32BitColour((uint8)0xDF, (uint8)0xE1, (uint8)0xE5)
#define JEWEL_WHITE				Set32BitColour((uint8)0xFF, (uint8)0xFF, (uint8)0xFF)  //A tick.


#endif // __JEWEL_COLOURS_H__

