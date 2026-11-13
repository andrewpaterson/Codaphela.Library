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
#include "ArrayElementNotFound.h"
#include "StringHelper.h"
#include "UndoLog.h"


//////////////////////////////////////////////////////////////////////////
//
// Group numbers start at 0, a step's uiUndone at 1.
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::Init(size uiMaximum)
{
	masSteps.Init();
	muiNextGroup = 0;
	muiGroupDepth = 0;
	muiOpenGroup = 0;
	muiUndoCount = 0;
	muiMaximum = uiMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::Kill(void)
{
	Clear();
	masSteps.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets every step.  An open group stays open.
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::Clear(void)
{
	size	i;

	for (i = 0; i < masSteps.NumElements(); i++)
	{
		masSteps.Get(i)->szRenamed.Kill();
		masSteps.Get(i)->szDocument.Kill();
	}
	masSteps.RemoveRange(0, masSteps.NumElements());
	muiUndoCount = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The edits until the matching EndGroup are one group.  Groups nest: the
// outermost is the group.
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::BeginGroup(void)
{
	if (muiGroupDepth == 0)
	{
		muiOpenGroup = muiNextGroup;
		muiNextGroup++;
	}
	muiGroupDepth++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::EndGroup(void)
{
	if (muiGroupDepth > 0)
	{
		muiGroupDepth--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// szDocument was edited from state uiIdBefore to uiIdAfter.  Its undone
// steps can't be redone any more.  The oldest step is dropped past the
// maximum.
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::Kept(char* szDocument, size uiIdBefore, size uiIdAfter)
{
	DropUndone(szDocument);
	AddStep(szDocument, uiIdBefore, uiIdAfter);
}


//////////////////////////////////////////////////////////////////////////
//
// szOldDocument was renamed szNewDocument (its file was): a step of its
// own, in the open group if there is one, which the log's user undoes by
// renaming it back.  The documents' edits are kept under the new name by
// RenameDocument, which the user calls too.  Undone steps of either
// name can't be redone any more.
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::Renamed(char* szOldDocument, char* szNewDocument)
{
	SUndoLogStep*	psStep;

	DropUndone(szOldDocument);
	DropUndone(szNewDocument);
	psStep = AddStep(szOldDocument, 0, 0);
	psStep->szRenamed.Set(szNewDocument);
	psStep->bRename = true;
}


//////////////////////////////////////////////////////////////////////////
//
// szDocument's undone steps, and undone renames from or to it, dropped:
// a new step of it means they can't be redone.
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::DropUndone(char* szDocument)
{
	SUndoLogStep*	psStep;
	size			i;

	for (i = masSteps.NumElements(); i > 0; i--)
	{
		psStep = masSteps.Get(i - 1);
		if ((psStep->uiUndone != 0) && ((StringCompare(psStep->szDocument.Text(), szDocument) == 0) || (psStep->bRename && (StringCompare(psStep->szRenamed.Text(), szDocument) == 0))))
		{
			RemoveAt(i - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A step added, in the open group or a group of its own.  The oldest
// step is dropped past the maximum (the step added is never dropped: the
// maximum is at least one).
//
//////////////////////////////////////////////////////////////////////////
SUndoLogStep* CUndoLog::AddStep(char* szDocument, size uiIdBefore, size uiIdAfter)
{
	SUndoLogStep*	psStep;

	psStep = masSteps.Add();
	psStep->szDocument.Init(szDocument);
	psStep->szRenamed.Init();
	psStep->uiIdBefore = uiIdBefore;
	psStep->uiIdAfter = uiIdAfter;
	psStep->uiUndone = 0;
	psStep->bRename = false;
	if (muiGroupDepth > 0)
	{
		psStep->uiGroup = muiOpenGroup;
	}
	else
	{
		psStep->uiGroup = muiNextGroup;
		muiNextGroup++;
	}

	while (masSteps.NumElements() > muiMaximum)
	{
		RemoveAt(0);
	}
	return masSteps.Tail();
}


//////////////////////////////////////////////////////////////////////////
//
// szDocument's last edit, which left it in state uiIdBefore, was merged
// with the next (typing), leaving it in state uiIdAfter.  If it isn't
// logged (it was dropped) the merged edit is kept as a new one.
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::Merged(char* szDocument, size uiIdBefore, size uiIdAfter)
{
	size	uiIndex;

	uiIndex = FindStep(szDocument, MAX_SIZE, uiIdBefore, false);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		Kept(szDocument, uiIdBefore, uiIdAfter);
		return;
	}
	masSteps.Get(uiIndex)->uiIdAfter = uiIdAfter;
}


//////////////////////////////////////////////////////////////////////////
//
// szDocument's own undo went from state uiIdBefore back to uiIdAfter.
// The step is marked undone.  Returns its group, MAX_SIZE if it isn't
// logged.
//
//////////////////////////////////////////////////////////////////////////
size CUndoLog::Undone(char* szDocument, size uiIdBefore, size uiIdAfter)
{
	size	uiIndex;

	uiIndex = FindStep(szDocument, uiIdAfter, uiIdBefore, false);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return MAX_SIZE;
	}
	MarkUndone(uiIndex);
	return masSteps.Get(uiIndex)->uiGroup;
}


//////////////////////////////////////////////////////////////////////////
//
// szDocument's own redo went from state uiIdBefore forward to uiIdAfter.
// The step is marked done again.  Returns its group, MAX_SIZE if it isn't
// logged.
//
//////////////////////////////////////////////////////////////////////////
size CUndoLog::Redone(char* szDocument, size uiIdBefore, size uiIdAfter)
{
	size	uiIndex;

	uiIndex = FindStep(szDocument, uiIdBefore, uiIdAfter, true);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return MAX_SIZE;
	}
	MarkRedone(uiIndex);
	return masSteps.Get(uiIndex)->uiGroup;
}


//////////////////////////////////////////////////////////////////////////
//
// The document is called szNewDocument now (its file was renamed): its
// edits are kept under its new name.  Renames are left as they were.
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::RenameDocument(char* szOldDocument, char* szNewDocument)
{
	size	i;

	for (i = 0; i < masSteps.NumElements(); i++)
	{
		if (!masSteps.Get(i)->bRename && (StringCompare(masSteps.Get(i)->szDocument.Text(), szOldDocument) == 0))
		{
			masSteps.Get(i)->szDocument.Set(szNewDocument);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The document closed, its undo stack with it: its edits are forgotten.
// A rename isn't the document's, it is undone on the disk: it stays.
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::RemoveDocument(char* szDocument)
{
	size	i;

	for (i = masSteps.NumElements(); i > 0; i--)
	{
		if (!masSteps.Get(i - 1)->bRename && (StringCompare(masSteps.Get(i - 1)->szDocument.Text(), szDocument) == 0))
		{
			RemoveAt(i - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The group of the newest step not undone, MAX_SIZE if every step is.
// Its steps not undone are undone newest first.
//
//////////////////////////////////////////////////////////////////////////
size CUndoLog::FindUndoGroup(void)
{
	size	i;

	for (i = masSteps.NumElements(); i > 0; i--)
	{
		if (masSteps.Get(i - 1)->uiUndone == 0)
		{
			return masSteps.Get(i - 1)->uiGroup;
		}
	}
	return MAX_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
// The group of the step undone last, MAX_SIZE if none is undone.  Its
// undone steps are redone oldest first.
//
//////////////////////////////////////////////////////////////////////////
size CUndoLog::FindRedoGroup(void)
{
	size			i;
	size			uiLatest;
	SUndoLogStep*	psStep;
	size			uiGroup;

	uiLatest = 0;
	uiGroup = MAX_SIZE;
	for (i = 0; i < masSteps.NumElements(); i++)
	{
		psStep = masSteps.Get(i);
		if (psStep->uiUndone > uiLatest)
		{
			uiLatest = psStep->uiUndone;
			uiGroup = psStep->uiGroup;
		}
	}
	return uiGroup;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::MarkUndone(size uiIndex)
{
	muiUndoCount++;
	masSteps.Get(uiIndex)->uiUndone = muiUndoCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::MarkRedone(size uiIndex)
{
	masSteps.Get(uiIndex)->uiUndone = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The newest step of szDocument from uiIdBefore (any, MAX_SIZE) to
// uiIdAfter, undone or not.
//
//////////////////////////////////////////////////////////////////////////
size CUndoLog::FindStep(char* szDocument, size uiIdBefore, size uiIdAfter, bool bUndone)
{
	size			i;
	SUndoLogStep*	psStep;

	for (i = masSteps.NumElements(); i > 0; i--)
	{
		psStep = masSteps.Get(i - 1);
		if (!psStep->bRename && (psStep->uiIdAfter == uiIdAfter) && ((uiIdBefore == MAX_SIZE) || (psStep->uiIdBefore == uiIdBefore)) &&
			((psStep->uiUndone != 0) == bUndone) && (StringCompare(psStep->szDocument.Text(), szDocument) == 0))
		{
			return i - 1;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoLog::RemoveAt(size uiIndex)
{
	masSteps.Get(uiIndex)->szRenamed.Kill();
	masSteps.Get(uiIndex)->szDocument.Kill();
	masSteps.RemoveAt(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CUndoLog::NumSteps(void) { return masSteps.NumElements(); }
SUndoLogStep* CUndoLog::Get(size uiIndex) { return masSteps.Get(uiIndex); }
bool CUndoLog::IsUndone(size uiIndex) { return masSteps.Get(uiIndex)->uiUndone != 0; }
bool CUndoLog::IsRename(size uiIndex) { return masSteps.Get(uiIndex)->bRename; }

