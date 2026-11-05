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
#ifndef __DOCK_COLOUR_H__
#define __DOCK_COLOUR_H__


//The colours of docking (see CDockManager), kept by the window theme (CWindowTheme::GetDockColour).
enum EDockColour
{
	DC_TitleBar,			//A tool group's title bar.
	DC_ActiveTitleBar,		//The active pane's group's title bar.
	DC_Sash,				//Between a split's nodes.
	DC_Background,			//The empty document well, and behind the dock area.
	DC_Strip,				//An auto hide strip.
	DC_StripTab,			//A tab on an auto hide strip.
	DC_StripTabHover,
	DC_StripTabLine,		//Along the strip side of a strip's tab.
	DC_Guide,				//A docking guide's face.
	DC_GuideBorder,
	DC_GuideHover,			//The guide under the pointer.
	DC_GuideArrow,
	DC_Preview,				//Where a pane would go, drawn over the window (with its opacity).
	DC_PreviewBorder,
	DC_FrameBorder,			//A floating window's edge.
	DC_NumColours,
};


#endif // __DOCK_COLOUR_H__

