/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __UNDO_STACK_H__
#define __UNDO_STACK_H__
#include "Chars.h"
#include "ArrayTemplate.h"


//The most snapshots an undo stack keeps; the oldest are dropped.
#define UNDO_STACK_MAXIMUM	100

//The numbers kept with each snapshot for whoever pushed it (a text's caret and selection before and after the edit).
#define UNDO_STACK_MARKS	4


//A saved state (a project's XML, a text), whether the change that made it only changed the selection, an id no
//other snapshot of the stack has had and its owner's marks.  The newest snapshot's state is kept whole (the stack's
//mszNewest); each other's is kept as how it differs from the one after it: its uiPrefix bytes and its uiSuffix bytes
//are the next one's, and szMiddle is what is between them.
struct SUndoSnapshot
{
	CChars	szMiddle;
	size	uiPrefix;
	size	uiSuffix;
	bool	bSelectionOnly;
	size	uiId;
	size	auiMarks[UNDO_STACK_MARKS];
};


typedef CArrayTemplate<SUndoSnapshot>	CArrayUndoSnapshot;


//Logicim's UndoStack: the states saved after each change, oldest first, and the one the editor has now (the current
//snapshot).  Pushing a snapshot drops those after the current one and makes it the current one.
//
//A snapshot that only changed the selection can be skipped.  FindUndo finds the snapshot before the last change that
//wasn't only to the selection, at or before the current one; FindRedo finds the next change after the current one that
//wasn't only to the selection, or the last snapshot if they all were.  Stepping finds the one before or after the
//current one, as Logicim's does.  Each returns ARRAY_ELEMENT_NOT_FOUND if there's nothing to go to; the editor loads
//the snapshot found (Get) and then makes it the current one with MoveTo.
//
//Each snapshot has its own id (ReplaceCurrent gives the replacement a new one), so an editor that keeps the id of the
//state it saved can tell when undo or redo comes back to it.  Only the newest state is kept whole, the others as
//their differences from the next, so a long history of small changes to a big text costs the changes, not the text.
class CUndoStack
{
protected:
	CArrayUndoSnapshot	masSnapshots;
	CChars				mszNewest;	//The newest snapshot's state, whole.
	CChars				mszGot;		//The state Get last made.
	size				muiCurrent;
	size				muiMaximum;
	size				muiNextId;

public:
	void	Init(size uiMaximum = UNDO_STACK_MAXIMUM);
	void	Kill(void);

	void	Clear(void);
	void	Push(char* szData, bool bSelectionOnly, size* pauiMarks = NULL);
	void	ReplaceCurrent(char* szData, size* pauiMarks = NULL);
	size	FindUndo(bool bStep);
	size	FindRedo(bool bStep);
	char*	Get(size uiIndex);
	void	MoveTo(size uiIndex);
	size	FindId(size uiId);

	size	NumSnapshots(void);
	size	GetCurrent(void);
	size	GetCurrentId(void);
	size	GetId(size uiIndex);
	size*	GetMarks(size uiIndex);
	bool	IsSelectionOnly(size uiIndex);

protected:
	void	MakeCurrentNewest(void);
	void	SetDifference(SUndoSnapshot* psSnapshot, char* szState, char* szNext);
	void	RemoveFrom(size uiIndex);
	void	RemoveFirst(void);
	void	SetMarks(SUndoSnapshot* psSnapshot, size* pauiMarks);
};


#endif // __UNDO_STACK_H__

