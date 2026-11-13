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
#ifndef __UNDO_LOG_H__
#define __UNDO_LOG_H__
#include "Chars.h"
#include "ArrayTemplate.h"


//The most steps an undo log keeps; the oldest are dropped.
#define UNDO_LOG_MAXIMUM	1000


//An edit of one document, its undo stack's snapshot ids before and after it, the group it was made in and, if it has
//been undone, when (the log's count of undos when it was, 0 if it hasn't been).
struct SUndoLogStep
{
	CChars	szDocument;
	size	uiIdBefore;
	size	uiIdAfter;
	size	uiGroup;
	size	uiUndone;
};


typedef CArrayTemplate<SUndoLogStep>	CArrayUndoLogStep;


//The edits of many documents, each with its own undo stack (CUndoStack), in the order they were made: an undo across
//them all, like IntelliJ's global undo.  The log only records; the editor undoes each document's step with its own
//undo stack (going to the step's uiIdBefore, or back to its uiIdAfter for a redo) and tells the log.
//
//The edits made between BeginGroup and EndGroup, of any number of documents (a reload of every changed file, a rename),
//are one group: undoing or redoing one of them undoes or redoes them all.  Every other edit is a group of its own.
//
//FindUndoGroup is the group of the newest edit not undone; FindRedoGroup the group undone last.  Undoing a document's
//edit with its own undo is told with Undone, which finds the step and returns its group, so the rest of the group can
//be undone too.  A new edit of a document drops its undone steps, they can't be redone any more.
class CUndoLog
{
protected:
	CArrayUndoLogStep	masSteps;
	size				muiNextGroup;
	size				muiGroupDepth;  //BeginGroup less EndGroup calls.
	size				muiOpenGroup;  //The group edits go into while muiGroupDepth isn't 0.
	size				muiUndoCount;
	size				muiMaximum;

public:
	void			Init(size uiMaximum = UNDO_LOG_MAXIMUM);
	void			Kill(void);

	void			Clear(void);
	void			BeginGroup(void);
	void			EndGroup(void);

	void			Kept(char* szDocument, size uiIdBefore, size uiIdAfter);
	void			Merged(char* szDocument, size uiIdBefore, size uiIdAfter);
	size			Undone(char* szDocument, size uiIdBefore, size uiIdAfter);
	size			Redone(char* szDocument, size uiIdBefore, size uiIdAfter);
	void			RemoveDocument(char* szDocument);
	void			RenameDocument(char* szOldDocument, char* szNewDocument);

	size			FindUndoGroup(void);
	size			FindRedoGroup(void);
	void			MarkUndone(size uiIndex);
	void			MarkRedone(size uiIndex);

	size			NumSteps(void);
	SUndoLogStep*	Get(size uiIndex);
	bool			IsUndone(size uiIndex);

protected:
	size			FindStep(char* szDocument, size uiIdBefore, size uiIdAfter, bool bUndone);
	void			RemoveAt(size uiIndex);
};


#endif // __UNDO_LOG_H__

