/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __UNDO_STACK_H__
#define __UNDO_STACK_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"


//The most snapshots an undo stack keeps; the oldest are dropped.
#define UNDO_STACK_MAXIMUM	100


//A saved project, and whether the change that made it only changed the selection.
struct SUndoSnapshot
{
	CChars	szXML;
	bool	bSelectionOnly;
};


typedef CArrayTemplate<SUndoSnapshot>	CArrayUndoSnapshot;


//Logicim's UndoStack: the projects saved after each change, oldest first, and the one the editor has now (the current
//snapshot).  Pushing a snapshot drops those after the current one and makes it the current one.
//
//A snapshot that only changed the selection can be skipped.  FindUndo finds the snapshot before the last change that
//wasn't only to the selection, at or before the current one; FindRedo finds the next change after the current one that
//wasn't only to the selection, or the last snapshot if they all were.  Stepping finds the one before or after the
//current one, as Logicim's does.  Each returns ARRAY_ELEMENT_NOT_FOUND if there's nothing to go to; the editor loads
//the snapshot found and then makes it the current one with MoveTo.
class CUndoStack
{
protected:
	CArrayUndoSnapshot	masSnapshots;
	size				muiCurrent;
	size				muiMaximum;

public:
	void	Init(size uiMaximum = UNDO_STACK_MAXIMUM);
	void	Kill(void);

	void	Clear(void);
	void	Push(char* szXML, bool bSelectionOnly);
	size	FindUndo(bool bStep);
	size	FindRedo(bool bStep);
	char*	Get(size uiIndex);
	void	MoveTo(size uiIndex);

	size	NumSnapshots(void);
	size	GetCurrent(void);
	bool	IsSelectionOnly(size uiIndex);

protected:
	void	RemoveFrom(size uiIndex);
	void	RemoveFirst(void);
};


#endif // __UNDO_STACK_H__

