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
#include <string.h>
#include "ArrayElementNotFound.h"
#include "UndoStack.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::Init(size uiMaximum)
{
	masSnapshots.Init();
	mszNewest.Init();
	mszGot.Init();
	muiCurrent = 0;
	muiMaximum = uiMaximum;
	muiNextId = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::Kill(void)
{
	Clear();
	masSnapshots.Kill();
	mszGot.Kill();
	mszNewest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::Clear(void)
{
	RemoveFrom(0);
	mszNewest.Clear();
	mszGot.Clear();
	muiCurrent = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The snapshots after the current one can't be redone any more.
// pauiMarks (UNDO_STACK_MARKS of them, NULL for zeros) are kept with it.
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::Push(char* szData, bool bSelectionOnly, size* pauiMarks)
{
	SUndoSnapshot*	psSnapshot;

	if (masSnapshots.NumElements() > 0)
	{
		MakeCurrentNewest();

		//The state that was newest is kept as how it differs from the new one.
		SetDifference(masSnapshots.Tail(), mszNewest.Text(), szData);
	}
	psSnapshot = masSnapshots.Add();
	psSnapshot->szMiddle.Init();
	psSnapshot->uiPrefix = 0;
	psSnapshot->uiSuffix = 0;
	psSnapshot->bSelectionOnly = bSelectionOnly;
	psSnapshot->uiId = muiNextId;
	muiNextId++;
	SetMarks(psSnapshot, pauiMarks);
	mszNewest.Set(szData);
	while (masSnapshots.NumElements() > muiMaximum)
	{
		RemoveFirst();
	}
	muiCurrent = masSnapshots.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The current snapshot becomes szData, as a change merged into the one
// that made it (typing a word, say).  The snapshots after it can't be
// redone any more, and it has a new id: it is another state.  pauiMarks
// are its marks, NULL to keep those it has.  With no snapshots it is
// pushed.
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::ReplaceCurrent(char* szData, size* pauiMarks)
{
	SUndoSnapshot*	psSnapshot;
	CChars			szBefore;

	if (masSnapshots.NumElements() == 0)
	{
		Push(szData, false, pauiMarks);
		return;
	}
	MakeCurrentNewest();

	//The state before it is kept as how it differs from the new current state.
	if (muiCurrent > 0)
	{
		szBefore.Init(Get(muiCurrent - 1));
		SetDifference(masSnapshots.Get(muiCurrent - 1), szBefore.Text(), szData);
		szBefore.Kill();
	}
	psSnapshot = masSnapshots.Get(muiCurrent);
	psSnapshot->uiId = muiNextId;
	muiNextId++;
	if (pauiMarks)
	{
		SetMarks(psSnapshot, pauiMarks);
	}
	mszNewest.Set(szData);
}


//////////////////////////////////////////////////////////////////////////
//
// The snapshots after the current one go (they can't be redone any more)
// and the current one is kept whole, as the newest.
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::MakeCurrentNewest(void)
{
	SUndoSnapshot*	psSnapshot;

	if (muiCurrent + 1 >= masSnapshots.NumElements())
	{
		return;
	}
	mszNewest.Set(Get(muiCurrent));
	RemoveFrom(muiCurrent + 1);
	psSnapshot = masSnapshots.Get(muiCurrent);
	psSnapshot->szMiddle.Clear();
	psSnapshot->uiPrefix = 0;
	psSnapshot->uiSuffix = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// psSnapshot, whose state is szState, keeps it as how it differs from
// szNext, the state after it: the bytes they start and end with, and what
// szState has between them.
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::SetDifference(SUndoSnapshot* psSnapshot, char* szState, char* szNext)
{
	size	uiStateLength;
	size	uiNextLength;
	size	uiPrefix;
	size	uiSuffix;

	uiStateLength = strlen(szState);
	uiNextLength = strlen(szNext);
	uiPrefix = 0;
	while ((uiPrefix < uiStateLength) && (uiPrefix < uiNextLength) && (szState[uiPrefix] == szNext[uiPrefix]))
	{
		uiPrefix++;
	}
	uiSuffix = 0;
	while ((uiSuffix < uiStateLength - uiPrefix) && (uiSuffix < uiNextLength - uiPrefix) && (szState[uiStateLength - 1 - uiSuffix] == szNext[uiNextLength - 1 - uiSuffix]))
	{
		uiSuffix++;
	}
	psSnapshot->uiPrefix = uiPrefix;
	psSnapshot->uiSuffix = uiSuffix;
	psSnapshot->szMiddle.Clear();
	psSnapshot->szMiddle.Append(&szState[uiPrefix], uiStateLength - uiSuffix - uiPrefix);
}


//////////////////////////////////////////////////////////////////////////
//
// The first snapshot is where the editor started; what made it isn't
// undone.
//
//////////////////////////////////////////////////////////////////////////
size CUndoStack::FindUndo(bool bStep)
{
	size	i;

	if ((masSnapshots.NumElements() == 0) || (muiCurrent == 0))
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	if (bStep)
	{
		return muiCurrent - 1;
	}

	for (i = muiCurrent; i > 0; i--)
	{
		if (!masSnapshots.Get(i)->bSelectionOnly)
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
size CUndoStack::FindRedo(bool bStep)
{
	size	i;
	size	uiLast;

	if (masSnapshots.NumElements() == 0)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	uiLast = masSnapshots.NumElements() - 1;
	if (muiCurrent == uiLast)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	if (bStep)
	{
		return muiCurrent + 1;
	}

	for (i = muiCurrent + 1; i < uiLast; i++)
	{
		if (!masSnapshots.Get(i)->bSelectionOnly)
		{
			return i;
		}
	}
	return uiLast;
}


//////////////////////////////////////////////////////////////////////////
//
// The state of the snapshot at uiIndex: made from the newest's, back
// through the differences, so it is only good until the next call.
//
//////////////////////////////////////////////////////////////////////////
char* CUndoStack::Get(size uiIndex)
{
	size			i;
	SUndoSnapshot*	psSnapshot;
	CChars			szState;
	size			uiLength;

	if (uiIndex + 1 >= masSnapshots.NumElements())
	{
		return mszNewest.Text();
	}

	mszGot.Set(mszNewest.Text());
	for (i = masSnapshots.NumElements() - 1; i > uiIndex; i--)
	{
		psSnapshot = masSnapshots.Get(i - 1);
		uiLength = mszGot.Length();
		szState.Init();
		szState.Append(mszGot.Text(), psSnapshot->uiPrefix);
		szState.Append(psSnapshot->szMiddle);
		szState.Append(&mszGot.Text()[uiLength - psSnapshot->uiSuffix], psSnapshot->uiSuffix);
		mszGot.Set(szState.Text());
		szState.Kill();
	}
	return mszGot.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::MoveTo(size uiIndex)
{
	muiCurrent = uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the snapshot with id uiId, ARRAY_ELEMENT_NOT_FOUND if it
// isn't kept (any more).
//
//////////////////////////////////////////////////////////////////////////
size CUndoStack::FindId(size uiId)
{
	size	i;

	for (i = 0; i < masSnapshots.NumElements(); i++)
	{
		if (masSnapshots.Get(i)->uiId == uiId)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// The snapshots from uiIndex on.
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::RemoveFrom(size uiIndex)
{
	size	i;

	for (i = uiIndex; i < masSnapshots.NumElements(); i++)
	{
		masSnapshots.Get(i)->szMiddle.Kill();
	}
	if (uiIndex < masSnapshots.NumElements())
	{
		masSnapshots.RemoveRange(uiIndex, masSnapshots.NumElements());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The oldest snapshot: it is kept as how it differs from the next, so it
// goes without changing any other.
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::RemoveFirst(void)
{
	masSnapshots.Get(0)->szMiddle.Kill();
	masSnapshots.RemoveAt(0);
	if (muiCurrent > 0)
	{
		muiCurrent--;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::SetMarks(SUndoSnapshot* psSnapshot, size* pauiMarks)
{
	size	i;

	for (i = 0; i < UNDO_STACK_MARKS; i++)
	{
		psSnapshot->auiMarks[i] = pauiMarks ? pauiMarks[i] : 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CUndoStack::NumSnapshots(void) { return masSnapshots.NumElements(); }
size CUndoStack::GetCurrent(void) { return muiCurrent; }
bool CUndoStack::IsSelectionOnly(size uiIndex) { return masSnapshots.Get(uiIndex)->bSelectionOnly; }
size CUndoStack::GetId(size uiIndex) { return masSnapshots.Get(uiIndex)->uiId; }
size* CUndoStack::GetMarks(size uiIndex) { return masSnapshots.Get(uiIndex)->auiMarks; }


//////////////////////////////////////////////////////////////////////////
//
// The id of the current snapshot, 0 if there are none.
//
//////////////////////////////////////////////////////////////////////////
size CUndoStack::GetCurrentId(void)
{
	return (muiCurrent < masSnapshots.NumElements()) ? masSnapshots.Get(muiCurrent)->uiId : 0;
}

