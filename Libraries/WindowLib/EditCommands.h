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
#ifndef __EDIT_COMMANDS_H__
#define __EDIT_COMMANDS_H__
#include "StandardLib/Object.h"
#include "CommandSet.h"


class CWindow;
class CComponent;
class CFocus;
//An Edit menu's commands (Undo, Redo, Cut, Copy, Paste and Select All, named Edit.Undo and so on) for a window's
//command set.  They act on the component the keys last went to that can take them (a text area or field, say), not
//on the menu holding the keys while it is open: running one gives that component the keys again and the standard
//command (CIC_Undo ...), and each can run while it would do something there (CComponent::CanDoCommand).  Once that
//component has gone from the window, they act on GetDefaultTarget's (an IDE's active document, say).  Their keys are
//the standard ones, Control+Z and so on, shown but not bound again.
class CEditCommands : public CObject
{
CONSTRUCTABLE(CEditCommands);
DESTRUCTABLE(CEditCommands);
protected:
	Ptr<CWindow>		mpWindow;
	Ptr<CComponent>		mpTarget;  //The component the keys last went to that takes an edit command, NULL for none.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CCommandSet> pCommands);
	void				Class(void) override;
	void				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CComponent>				GetTarget(void);
	virtual Ptr<CComponent>		GetDefaultTarget(void);
	void						FocusChanged(CFocus* pcFocus);

	void				Undo(void);
	void				Redo(void);
	void				Cut(void);
	void				Copy(void);
	void				Paste(void);
	void				SelectAll(void);
	bool				CanUndo(void);
	bool				CanRedo(void);
	bool				CanCut(void);
	bool				CanCopy(void);
	bool				CanPaste(void);
	bool				CanSelectAll(void);

protected:
	void				AddCommand(Ptr<CCommandSet> pCommands, const char* szName, const char* szCaption, EComponentInputCommand eStandard, void(CEditCommands::*fRun)(void), bool(CEditCommands::*fCan)(void));
	void				Run(EComponentInputCommand eCommand);
	bool				Can(EComponentInputCommand eCommand);
	bool				IsTakingEdits(Ptr<CComponent> pComponent);
};


#endif // __EDIT_COMMANDS_H__

