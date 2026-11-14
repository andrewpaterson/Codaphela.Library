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
#ifndef __REVEALING_UNDO_H__
#define __REVEALING_UNDO_H__
#include "BaseLib/Chars.h"


//What a revealing undo asks of the editor it undoes for.  bRedo is for redo rather than undo.
class CRevealingUndoHost
{
public:
	virtual bool	IsUndoShown(bool bRedo) = 0;  //Whether what the next undo changes can be seen now.
	virtual bool	ShowUndo(bool bRedo) = 0;  //Brings it into view; false if there's nothing to undo.
	virtual bool	DoUndo(bool bRedo) = 0;
	virtual void	AppendUndoState(CChars* pszDest) = 0;  //Where the editor and its history are, to tell a second press from a new first one.
	virtual void	AppendUndoWhere(bool bRedo, CChars* pszDest) = 0;  //Where the change is, "in Board in U2".
};


//Undo that never undoes what can't be seen, as an editor that bounced somewhere and undid something there would: if
//what the next undo changes is in view, a press undoes it; if not, the first press only shows it, saying so ("Ctrl+Z
//again to undo the change in Board in U2"), and a second press, with nothing changed in between, undoes it.  The same
//for redo.  The editor (its CRevealingUndoHost) says what's in view, shows it and undoes; this only decides which.
class CRevealingUndo
{
protected:
	CRevealingUndoHost*	mpcHost;  //Not held.
	CChars				mszShown;  //The host's state after the last first press, empty if there was none since.
	bool				mbShownRedo;

public:
	void	Init(CRevealingUndoHost* pcHost);
	void	Kill(void);

	bool	Press(bool bRedo, const char* szKeys, CChars* pszMessage);
	void	Forget(void);
	bool	IsShowing(void);
};


#endif // __REVEALING_UNDO_H__

