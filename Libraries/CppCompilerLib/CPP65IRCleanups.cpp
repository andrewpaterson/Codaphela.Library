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
// The uiOrdinal'th entry of the kind, the oldest 0; NULL if there aren't so
// many.  The entry is only good until the next one is registered.
//
//////////////////////////////////////////////////////////////////////////
SCPP65IRCleanup* CCPP65IRCleanups::Get(ECPP65IRCleanup eKind, size uiOrdinal)
{
	SCPP65IRCleanup*	psEntry;
	size				ui;
	size				uiCount;

	//From the top: what is asked for is nearly always the newest.
	uiCount = (eKind == CPP65IRC_Local) ? muiLocals : muiTemporaries;
	for (ui = masEntries.NumElements(); (ui > 0) && (uiCount > uiOrdinal); ui--)
	{
		psEntry = masEntries.Get(ui - 1);
		if (psEntry->eKind == eKind)
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
	return Get(CPP65IRC_Local, uiOrdinal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65IRCleanup* CCPP65IRCleanups::GetTemporary(size uiOrdinal)
{
	return Get(CPP65IRC_Temporary, uiOrdinal);
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets the newest entries of the kind until uiCount of it are left.  The
// others keep their order.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::Cut(ECPP65IRCleanup eKind, size uiCount)
{
	size*	puiCount;
	size	ui;

	puiCount = (eKind == CPP65IRC_Local) ? &muiLocals : &muiTemporaries;
	for (ui = masEntries.NumElements(); (ui > 0) && (*puiCount > uiCount); ui--)
	{
		if (masEntries.Get(ui - 1)->eKind == eKind)
		{
			masEntries.RemoveAt(ui - 1, true);
			(*puiCount)--;
		}
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
	Cut(CPP65IRC_Local, uiCount);
}


//////////////////////////////////////////////////////////////////////////
//
// The end of a full expression: the temporaries registered since there were
// uiCount are forgotten.  A local registered since stays.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::CutTemporaries(size uiCount)
{
	Cut(CPP65IRC_Temporary, uiCount);
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
			return true;
		}
	}
	return false;
}

