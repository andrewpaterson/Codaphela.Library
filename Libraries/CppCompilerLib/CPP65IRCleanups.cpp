/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "CPP65IRCleanups.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::Init(void)
{
	masEntries.Init();
	muiLocals = 0;
	muiTemporaries = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::Kill(void)
{
	masEntries.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Empty, for the next function.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::ReInit(void)
{
	masEntries.ReInit();
	muiLocals = 0;
	muiTemporaries = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// How many locals are registered: a mark for a scope, a loop's body or a
// jump out (CutLocals, GetLocal).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65IRCleanups::NumLocals(void) const
{
	return muiLocals;
}


//////////////////////////////////////////////////////////////////////////
//
// How many temporaries are registered: a mark for a full expression
// (CutTemporaries, GetTemporary).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65IRCleanups::NumTemporaries(void) const
{
	return muiTemporaries;
}


//////////////////////////////////////////////////////////////////////////
//
// Everything registered, in the order it was (GetEntry: the oldest is 0).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65IRCleanups::NumEntries(void) const
{
	return masEntries.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65IRCleanup* CCPP65IRCleanups::GetEntry(size uiIndex)
{
	return masEntries.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// An object (or array of them) destroyed when its scope is left.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::PushLocal(SCPP65IRPlace* psPlace)
{
	SCPP65IRCleanup*	psEntry;

	psEntry = masEntries.Add();
	psEntry->eKind = CPP65IRC_Local;
	psEntry->sPlace = *psPlace;
	psEntry->bConditional = false;
	psEntry->sMade = IRNone();
	psEntry->pcUndo = NULL;
	muiLocals++;
}


//////////////////////////////////////////////////////////////////////////
//
// A temporary destroyed at the end of its full expression; bConditional
// with its flag sMade (SCPP65IRCleanup).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::PushTemporary(SCPP65IRPlace* psPlace, bool bConditional, SIRReference sMade)
{
	SCPP65IRCleanup*	psEntry;

	psEntry = masEntries.Add();
	psEntry->eKind = CPP65IRC_Temporary;
	psEntry->sPlace = *psPlace;
	psEntry->bConditional = bConditional;
	psEntry->sMade = sMade;
	psEntry->pcUndo = NULL;
	muiTemporaries++;
}


//////////////////////////////////////////////////////////////////////////
//
// A handler has begun: its entry ends it when its scope is left.  It is
// counted with the locals, so that it is undone in its turn among them.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::PushCatch(void)
{
	SCPP65IRCleanup*	psEntry;

	psEntry = masEntries.Add();
	psEntry->eKind = CPP65IRC_Catch;
	psEntry->sPlace.sAddress = IRNone();
	psEntry->sPlace.psType = NULL;
	psEntry->sPlace.psBitField = NULL;
	psEntry->bConditional = false;
	psEntry->sMade = IRNone();
	psEntry->pcUndo = NULL;
	muiLocals++;
}


//////////////////////////////////////////////////////////////////////////
//
// The uiOrdinal'th temporary, or (a handler's entry is one of them) local,
// the oldest 0; NULL if there aren't so many.  The entry is only good until the next one is registered.
//
//////////////////////////////////////////////////////////////////////////
SCPP65IRCleanup* CCPP65IRCleanups::Get(bool bTemporary, size uiOrdinal)
{
	SCPP65IRCleanup*	psEntry;
	size				ui;
	size				uiCount;

	//From the top: what is asked for is nearly always the newest.
	uiCount = bTemporary ? muiTemporaries : muiLocals;
	for (ui = masEntries.NumElements(); (ui > 0) && (uiCount > uiOrdinal); ui--)
	{
		psEntry = masEntries.Get(ui - 1);
		if ((psEntry->eKind == CPP65IRC_Temporary) == bTemporary)
		{
			uiCount--;
			if (uiCount == uiOrdinal)
			{
				return psEntry;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65IRCleanup* CCPP65IRCleanups::GetLocal(size uiOrdinal)
{
	return Get(false, uiOrdinal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65IRCleanup* CCPP65IRCleanups::GetTemporary(size uiOrdinal)
{
	return Get(true, uiOrdinal);
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets the newest temporaries, or locals, until uiCount of them are left.
// The others keep their order; one that had something taken from under it
// forgets its block (ForgetUndoFrom).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::Cut(bool bTemporary, size uiCount)
{
	size*	puiCount;
	size	ui;

	puiCount = bTemporary ? &muiTemporaries : &muiLocals;
	for (ui = masEntries.NumElements(); (ui > 0) && (*puiCount > uiCount); ui--)
	{
		if ((masEntries.Get(ui - 1)->eKind == CPP65IRC_Temporary) == bTemporary)
		{
			masEntries.RemoveAt(ui - 1, true);
			(*puiCount)--;
			ForgetUndoFrom(ui - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The entries from uiIndex up forget their blocks (pcUndo): each goes on to
// the block of the entry below it, and what is below them has changed.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::ForgetUndoFrom(size uiIndex)
{
	size	ui;

	for (ui = uiIndex; ui < masEntries.NumElements(); ui++)
	{
		masEntries.Get(ui)->pcUndo = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The end of a scope: the locals registered since there were uiCount are
// forgotten (their destructors have been written).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::CutLocals(size uiCount)
{
	Cut(false, uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
// The end of a full expression: the temporaries registered since there were
// uiCount are forgotten.  A local registered since stays.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::CutTemporaries(size uiCount)
{
	Cut(true, uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out the newest temporary at sAddress that isn't conditional, giving
// its place: a reference is to keep it alive, and it is registered again as
// a local.  False if there is none.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRCleanups::TakeTemporary(SIRReference sAddress, SCPP65IRPlace* psPlace)
{
	SCPP65IRCleanup*	psEntry;
	size				ui;

	for (ui = masEntries.NumElements(); ui > 0; ui--)
	{
		psEntry = masEntries.Get(ui - 1);
		if ((psEntry->eKind == CPP65IRC_Temporary) && !psEntry->bConditional && psEntry->sPlace.sAddress.Equals(sAddress))
		{
			*psPlace = psEntry->sPlace;
			masEntries.RemoveAt(ui - 1, true);
			muiTemporaries--;
			ForgetUndoFrom(ui - 1);
			return true;
		}
	}
	return false;
}

