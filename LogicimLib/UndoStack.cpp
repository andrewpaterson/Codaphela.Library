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
#include "BaseLib/ArrayElementNotFound.h"
#include "UndoStack.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::Init(size uiMaximum)
{
	masSnapshots.Init();
	muiCurrent = 0;
	muiMaximum = uiMaximum;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::Kill(void)
{
	Clear();
	masSnapshots.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::Clear(void)
{
	RemoveFrom(0);
	muiCurrent = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The snapshots after the current one can't be redone any more.
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::Push(char* szXML, bool bSelectionOnly)
{
	SUndoSnapshot*	psSnapshot;

	if (masSnapshots.NumElements() > 0)
	{
		RemoveFrom(muiCurrent + 1);
	}
	psSnapshot = masSnapshots.Add();
	psSnapshot->szXML.Init(szXML);
	psSnapshot->bSelectionOnly = bSelectionOnly;
	while (masSnapshots.NumElements() > muiMaximum)
	{
		RemoveFirst();
	}
	muiCurrent = masSnapshots.NumElements() - 1;
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
//
//////////////////////////////////////////////////////////////////////////
char* CUndoStack::Get(size uiIndex)
{
	return masSnapshots.Get(uiIndex)->szXML.Text();
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
// The snapshots from uiIndex on.
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::RemoveFrom(size uiIndex)
{
	size	i;

	for (i = uiIndex; i < masSnapshots.NumElements(); i++)
	{
		masSnapshots.Get(i)->szXML.Kill();
	}
	if (uiIndex < masSnapshots.NumElements())
	{
		masSnapshots.RemoveRange(uiIndex, masSnapshots.NumElements());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CUndoStack::RemoveFirst(void)
{
	masSnapshots.Get(0)->szXML.Kill();
	masSnapshots.RemoveAt(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CUndoStack::NumSnapshots(void) { return masSnapshots.NumElements(); }
size CUndoStack::GetCurrent(void) { return muiCurrent; }
bool CUndoStack::IsSelectionOnly(size uiIndex) { return masSnapshots.Get(uiIndex)->bSelectionOnly; }

