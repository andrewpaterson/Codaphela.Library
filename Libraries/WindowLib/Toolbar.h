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
#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__
#include "BarContainer.h"
#include "WindowTheme.h"
#include "Button.h"
#include "Command.h"


//A toolbar, or a column of tool buttons like IntelliJ's tool window stripes: the buttons of commands and any other
//components (a combo box, a label) in the order they are added, with separator lines between groups of them.  A
//command's button shows its icon, or its caption if it has none, runs it, and has its caption and keys as its tool tip.
//A command's button looks as its command is (Refresh, called by whoever knows it changed, and after each click of one
//of the toolbar's buttons): on while a command that is on or off is on, and with its disabled icon, if it was given
//one, while the command can't run.
//The sizes are the theme's TM_Toolbar metrics; see CBarContainer for the stretch.
class CToolbar : public CBarContainer
{
CONSTRUCTABLE(CToolbar);
DESTRUCTABLE(CToolbar);
protected:
	Ptr<CWindowTheme>	mpTheme;
	CArray<CButton>		maButtons;  //The commands' buttons.
	CArray<CImageCel>	maDisabledIcons;  //Each one's icon while its command can't run, NULL for the same.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, bool bVertical);
	void				Class(void) override;

	void				Free(void) override;
	Ptr<CButton>		AddCommand(Ptr<CCommand> pCommand, Ptr<CImageCel> pDisabledIcon = NULL);
	void				Refresh(void);
	size				NumCommandButtons(void);
	Ptr<CButton>		GetCommandButton(size uiIndex);
	bool				IsShownDisabled(size uiIndex);
	void				ButtonClicked(CButton* pcButton);
	void				AddSeparator(void);
};


#endif // __TOOLBAR_H__

