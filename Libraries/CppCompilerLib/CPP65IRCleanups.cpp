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
// An entry of eNewKind with nothing in it yet.
//
//////////////////////////////////////////////////////////////////////////
void SCPP65IRCleanup::Clear(ECPP65IRCleanup eNewKind)
{
	size	ui;

	eKind = eNewKind;
	sPlace.sAddress = IRNone();
	sPlace.psType = NULL;
	sPlace.psBitField = NULL;
	bConditional = false;
	sMade = IRNone();
	pcUndo = NULL;
	ePart = CPP65IRP_Destroy;
	pcBaseObject = NULL;
	pcFunction = NULL;
	pcFor = NULL;
	szRuntime = NULL;
	sCount = IRNone();
	for (ui = 0; ui < CPP65_IR_PART_VALUES; ui++)
	{
		asValues[ui] = IRNone();
		apsValueTypes[ui] = NULL;
	}
	uiValues = 0;
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
	psEntry->Clear(CPP65IRC_Local);
	psEntry->sPlace = *psPlace;
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
	psEntry->Clear(CPP65IRC_Temporary);
	psEntry->sPlace = *psPlace;
	psEntry->bConditional = bConditional;
	psEntry->sMade = sMade;
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
	psEntry->Clear(CPP65IRC_Catch);
	muiLocals++;
}


//////////////////////////////////////////////////////////////////////////
//
// Something only an exception undoes (a part, ECPP65IRPart), from psPart.
// It is counted with neither the locals nor the temporaries, so a normal
// way out never sees it; it is taken out when what it is for is done
// (RemovePartsFrom, RemoveNewestPart).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::PushPart(SCPP65IRCleanup* psPart)
{
	SCPP65IRCleanup*	psEntry;

	psEntry = masEntries.Add();
	*psEntry = *psPart;
	psEntry->eKind = CPP65IRC_Part;
	psEntry->pcUndo = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A part put in at uiIndex, under what was registered since: what it is
// for was finished there (a member made, whose initialiser's temporaries
// are above it).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::InsertPart(size uiIndex, SCPP65IRCleanup* psPart)
{
	SCPP65IRCleanup*	psEntry;

	psEntry = masEntries.InsertAt(uiIndex);
	*psEntry = *psPart;
	psEntry->eKind = CPP65IRC_Part;
	ForgetUndoFrom(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out the parts from uiIndex up (what they were for is done); the
// locals and temporaries there stay, in their order.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65IRCleanups::RemovePartsFrom(size uiIndex)
{
	size	ui;

	for (ui = masEntries.NumElements(); ui > uiIndex; ui--)
	{
		if (masEntries.Get(ui - 1)->eKind == CPP65IRC_Part)
		{
			masEntries.RemoveAt(ui - 1, true);
			ForgetUndoFrom(ui - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out the newest part; false if there is none.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRCleanups::RemoveNewestPart(void)
{
	size	ui;

	for (ui = masEntries.NumElements(); ui > 0; ui--)
	{
		if (masEntries.Get(ui - 1)->eKind == CPP65IRC_Part)
		{
			masEntries.RemoveAt(ui - 1, true);
			ForgetUndoFrom(ui - 1);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the uiOrdinal'th local (GetLocal), or temporary, is among all the
// entries; MAX_SIZE if there aren't so many.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65IRCleanups::IndexOfLocal(size uiOrdinal)
{
	size	uiIndex;

	return Get(false, uiOrdinal, &uiIndex) ? uiIndex : MAX_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPP65IRCleanups::IndexOfTemporary(size uiOrdinal)
{
	size	uiIndex;

	return Get(true, uiOrdinal, &uiIndex) ? uiIndex : MAX_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an entry is one of the temporaries (bTemporary) or of the locals,
// which a handler's is among.  A part is neither.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65IRCleanups::IsCounted(SCPP65IRCleanup* psEntry, bool bTemporary)
{
	if (bTemporary)
	{
		return psEntry->eKind == CPP65IRC_Temporary;
	}
	return (psEntry->eKind == CPP65IRC_Local) || (psEntry->eKind == CPP65IRC_Catch);
}


//////////////////////////////////////////////////////////////////////////
//
// The uiOrdinal'th temporary, or (a handler's entry is one of them) local,
// the oldest 0; NULL if there aren't so many.  The entry is only good until the next one is registered.
// *puiIndex (if it is wanted) is where it is among all the entries.
//
//////////////////////////////////////////////////////////////////////////
SCPP65IRCleanup* CCPP65IRCleanups::Get(bool bTemporary, size uiOrdinal, size* puiIndex)
{
	SCPP65IRCleanup*	psEntry;
	size				ui;
	size				uiCount;

	//From the top: what is asked for is nearly always the newest.
	uiCount = bTemporary ? muiTemporaries : muiLocals;
	for (ui = masEntries.NumElements(); (ui > 0) && (uiCount > uiOrdinal); ui--)
	{
		psEntry = masEntries.Get(ui - 1);
		if (IsCounted(psEntry, bTemporary))
		{
			uiCount--;
			if (uiCount == uiOrdinal)
			{
				if (puiIndex)
				{
					*puiIndex = ui - 1;
				}
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
		if (IsCounted(masEntries.Get(ui - 1), bTemporary))
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

