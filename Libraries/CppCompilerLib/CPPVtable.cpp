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
#include "CPPClass.h"
#include "CPPVtable.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::Init(void)
{
	mapcFunctions.Init();
	miPointerOffset = -1;
	macSecondaries.Init();
	mbWritten = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::Kill(void)
{
	size	i;

	for (i = 0; i < macSecondaries.NumElements(); i++)
	{
		macSecondaries.Get(i)->maiAdjust.Kill();
		macSecondaries.Get(i)->mapcFunctions.Kill();
	}
	macSecondaries.Kill();
	mapcFunctions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Where a vtable pointer is in an object of a polymorphic class: its own, else
// the first secondary one's.
//
//////////////////////////////////////////////////////////////////////////
int CCPPVtable::GetPointerOffset(void)
{
	SCPPSecondaryVtable*	psVtable;

	if (miPointerOffset != -1)
	{
		return miPointerOffset;
	}
	psVtable = macSecondaries.Get(0);
	return psVtable->iOffset + psVtable->pcOwner->mcVtable.miPointerOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function in the vtables (the final overriders) is pure virtual.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPVtable::HasPure(void)
{
	SCPPSecondaryVtable*	psVtable;
	size					i;
	size					j;

	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		if (mapcFunctions.GetPtr(i)->mbPure)
		{
			return true;
		}
	}
	for (i = 0; i < macSecondaries.NumElements(); i++)
	{
		psVtable = macSecondaries.Get(i);
		for (j = 0; j < psVtable->mapcFunctions.NumElements(); j++)
		{
			if (psVtable->mapcFunctions.GetPtr(j)->mbPure)
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcFunction overrides a virtual function in any of the vtables.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPVtable::IsOverriddenBy(CCPPFunction* pcFunction)
{
	SCPPSecondaryVtable*	psVtable;
	size					i;
	size					j;

	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		if (pcFunction->Overrides(mapcFunctions.GetPtr(i)))
		{
			return true;
		}
	}
	for (i = 0; i < macSecondaries.NumElements(); i++)
	{
		psVtable = macSecondaries.Get(i);
		for (j = 0; j < psVtable->mapcFunctions.NumElements(); j++)
		{
			if (pcFunction->Overrides(psVtable->mapcFunctions.GetPtr(j)))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's own vtable starts as the first base's.  A function with the
// name and parameters of a virtual function in it overrides it (and is
// virtual); other virtual functions are added on the end.  A destructor
// overrides a virtual base destructor.
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::LayoutOwn(CCPPClass* pcClass)
{
	CArrayCPPFunctions*	papcBase;
	CCPPFunction*		pcFunction;
	CCPPFunction*		pcVirtual;
	size				i;
	size				j;
	bool				bOverride;

	if (pcClass->mpcBase)
	{
		papcBase = &pcClass->mpcBase->mcVtable.mapcFunctions;
		for (i = 0; i < papcBase->NumElements(); i++)
		{
			mapcFunctions.Add(papcBase->GetPtr(i));
		}
	}

	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if (pcFunction->mbStatic || (pcFunction->meKind == CPPFK_Constructor))
		{
			continue;
		}

		bOverride = false;
		for (j = 0; j < mapcFunctions.NumElements(); j++)
		{
			pcVirtual = mapcFunctions.GetPtr(j);
			if (pcFunction->Overrides(pcVirtual))
			{
				mapcFunctions.SetPtr(j, pcFunction);
				pcFunction->mbVirtual = true;
				pcFunction->miSlot = (int)j;
				bOverride = true;
				break;
			}
		}
		//A function that overrides one in a secondary vtable also gets a place in the class's own.
		if (!bOverride && (pcFunction->mbVirtual || pcClass->OverridesBase(pcFunction)))
		{
			pcFunction->mbVirtual = true;
			pcFunction->miSlot = (int)mapcFunctions.NumElements();
			mapcFunctions.Add(pcFunction);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The vtables of the polymorphic base subobjects that do not share the class's
// vtable pointer: the bases' own (except the first base's when the class shares it)
// and their secondary vtables, with the class's functions overriding.  The base
// offsets and the class's own vtable pointer must be known.
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::LayoutSecondaries(CCPPClass* pcClass)
{
	SCPPBase*				psBase;
	CCPPVtable*				pcBase;
	SCPPSecondaryVtable*	psVtable;
	bool					bShared;
	size					i;
	size					j;

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		pcBase = &psBase->pcClass->mcVtable;
		bShared = (i == 0) && (pcClass->mpcBase == psBase->pcClass) && pcBase->HasPointer() && (miPointerOffset == pcBase->miPointerOffset);
		if (!bShared && pcBase->HasPointer())
		{
			AddSecondary(pcClass, psBase->iOffset, psBase->pcClass, &pcBase->mapcFunctions, NULL);
		}
		for (j = 0; j < pcBase->macSecondaries.NumElements(); j++)
		{
			psVtable = pcBase->macSecondaries.Get(j);
			AddSecondary(pcClass, psBase->iOffset + psVtable->iOffset, psVtable->pcOwner, &psVtable->mapcFunctions, &psVtable->maiAdjust);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A secondary vtable at iOffset, starting as a base's.  The class's own functions
// override; they need this moved back from the subobject to the class.
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::AddSecondary(CCPPClass* pcClass, int iOffset, CCPPClass* pcOwner, CArrayCPPFunctions* papcFunctions, CArrayInt* paiAdjust)
{
	SCPPSecondaryVtable*	psVtable;
	CCPPFunction*			pcFunction;
	size					i;
	size					j;

	psVtable = macSecondaries.Add();
	psVtable->iOffset = iOffset;
	psVtable->pcOwner = pcOwner;
	psVtable->mapcFunctions.Init();
	psVtable->maiAdjust.Init();
	for (i = 0; i < papcFunctions->NumElements(); i++)
	{
		psVtable->mapcFunctions.Add(papcFunctions->GetPtr(i));
		psVtable->maiAdjust.Add(paiAdjust ? paiAdjust->GetValue(i) : 0);
	}

	for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		if (pcFunction->mbStatic || (pcFunction->meKind == CPPFK_Constructor))
		{
			continue;
		}
		for (j = 0; j < psVtable->mapcFunctions.NumElements(); j++)
		{
			if (pcFunction->Overrides(psVtable->mapcFunctions.GetPtr(j)))
			{
				psVtable->mapcFunctions.SetPtr(j, pcFunction);
				psVtable->maiAdjust.SetValue(j, -iOffset);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The vtable as it is written, in order (see CPP65_VTABLE_ENTRY_BYTES): the
// class's own part's header (before the label) and functions, then each
// secondary vtable's header and functions.  The generator writes it
// (EnsureVtable) and the IR front end describes it and stores its parts'
// addresses from the same list.
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::GetEntries(CArrayCPPVtableEntries* pasEntries)
{
	SCPPVtableEntry*		psEntry;
	SCPPSecondaryVtable*	psVtable;
	size					i;
	size					j;

	psEntry = pasEntries->Add();
	psEntry->eKind = CPPVE_Header;
	psEntry->iVtablePointer = (miPointerOffset == -1) ? 0 : miPointerOffset;
	psEntry->pcFunction = NULL;
	psEntry->iAdjust = 0;
	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		psEntry = pasEntries->Add();
		psEntry->eKind = CPPVE_Function;
		psEntry->iVtablePointer = 0;
		psEntry->pcFunction = mapcFunctions.GetPtr(i);
		psEntry->iAdjust = 0;
	}
	for (i = 0; i < macSecondaries.NumElements(); i++)
	{
		psVtable = macSecondaries.Get(i);
		psEntry = pasEntries->Add();
		psEntry->eKind = CPPVE_Header;
		psEntry->iVtablePointer = psVtable->iOffset + psVtable->pcOwner->mcVtable.miPointerOffset;
		psEntry->pcFunction = NULL;
		psEntry->iAdjust = 0;
		for (j = 0; j < psVtable->mapcFunctions.NumElements(); j++)
		{
			psEntry = pasEntries->Add();
			psEntry->eKind = CPPVE_Function;
			psEntry->iVtablePointer = 0;
			psEntry->pcFunction = psVtable->mapcFunctions.GetPtr(j);
			psEntry->iAdjust = psVtable->maiAdjust.GetValue(j);
		}
	}
}

