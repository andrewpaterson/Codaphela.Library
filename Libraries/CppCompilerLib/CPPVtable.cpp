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
	maiVirtualOffsets.Init();
	mbWritten = false;
	mpcComplete = NULL;
	miCompleteOffset = 0;
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
		macSecondaries.Get(i)->maiVirtualOffsets.Kill();
		macSecondaries.Get(i)->maiAdjust.Kill();
		macSecondaries.Get(i)->mapcFunctions.Kill();
	}
	macSecondaries.Kill();
	maiVirtualOffsets.Kill();
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
	return FindOverridden(pcFunction) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The function in the vtables that pcFunction overrides, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPVtable::FindOverridden(CCPPFunction* pcFunction)
{
	SCPPSecondaryVtable*	psVtable;
	size					i;
	size					j;

	for (i = 0; i < mapcFunctions.NumElements(); i++)
	{
		if (pcFunction->Overrides(mapcFunctions.GetPtr(i)))
		{
			return mapcFunctions.GetPtr(i);
		}
	}
	for (i = 0; i < macSecondaries.NumElements(); i++)
	{
		psVtable = macSecondaries.Get(i);
		for (j = 0; j < psVtable->mapcFunctions.NumElements(); j++)
		{
			if (pcFunction->Overrides(psVtable->mapcFunctions.GetPtr(j)))
			{
				return psVtable->mapcFunctions.GetPtr(j);
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's own vtable starts as the first base's (not a virtual base's,
// which doesn't share its pointer).  A function with the name and
// parameters of a virtual function in it overrides it (and is virtual);
// other virtual functions are added on the end.  A destructor
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

	if (pcClass->mpcBase && !pcClass->macBases.Get(0)->bVirtual)
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
// vtable pointer: the non-virtual bases' own (except the first base's when the
// class shares it) and their secondary vtables that aren't in their virtual
// bases, with the class's functions overriding; then each virtual base's own
// and secondary vtables, once, where the complete object has them.  Each part
// has the offsets of its class's virtual bases.  The class is laid out.
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::LayoutSecondaries(CCPPClass* pcClass)
{
	SCPPBase*				psBase;
	SCPPVirtualBase*		psVirtual;
	CCPPVtable*				pcBase;
	SCPPSecondaryVtable*	psVtable;
	bool					bShared;
	size					i;
	size					j;
	int						iOffset;

	SetVirtualOffsets(pcClass, 0, pcClass, &maiVirtualOffsets);
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (psBase->bVirtual)
		{
			continue;
		}
		pcBase = &psBase->pcClass->mcVtable;
		bShared = (i == 0) && (pcClass->mpcBase == psBase->pcClass) && pcBase->HasPointer() && (miPointerOffset == pcBase->miPointerOffset);
		if (!bShared && pcBase->HasPointer())
		{
			AddSecondary(pcClass, psBase->iOffset, psBase->pcClass, &pcBase->mapcFunctions, NULL);
		}
		for (j = 0; j < pcBase->macSecondaries.NumElements(); j++)
		{
			psVtable = pcBase->macSecondaries.Get(j);
			if (!psVtable->pcVirtualBase)
			{
				AddSecondary(pcClass, psBase->iOffset + psVtable->iOffset, psVtable->pcOwner, &psVtable->mapcFunctions, &psVtable->maiAdjust);
			}
		}
	}
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		psVirtual = pcClass->macVirtualBases.Get(i);
		pcBase = &psVirtual->pcClass->mcVtable;
		iOffset = VirtualBaseOffset(pcClass, psVirtual->pcClass);
		if (pcBase->HasPointer())
		{
			AddVirtualPart(pcClass, psVirtual->pcClass, iOffset, psVirtual->pcClass, &pcBase->mapcFunctions, NULL);
		}
		for (j = 0; j < pcBase->macSecondaries.NumElements(); j++)
		{
			psVtable = pcBase->macSecondaries.Get(j);
			if (!psVtable->pcVirtualBase)
			{
				AddVirtualPart(pcClass, psVirtual->pcClass, iOffset + psVtable->iOffset, psVtable->pcOwner, &psVtable->mapcFunctions, &psVtable->maiAdjust);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The construction vtable of pcClass (laid out) for its subobject at iOffset
// in a complete object of pcComplete: pcClass's own part and its non-virtual
// bases' as pcClass has them, its virtual bases' parts where pcComplete puts
// them, with pcClass's overriders.  In the order and of the sizes of
// pcClass's vtable, so the IR finds its parts the same way.
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::LayoutConstruction(CCPPClass* pcClass, CCPPClass* pcComplete, int iOffset)
{
	size	i;

	mpcComplete = pcComplete;
	miCompleteOffset = iOffset;
	miPointerOffset = pcClass->mcVtable.miPointerOffset;
	for (i = 0; i < pcClass->mcVtable.mapcFunctions.NumElements(); i++)
	{
		mapcFunctions.Add(pcClass->mcVtable.mapcFunctions.GetPtr(i));
	}
	LayoutSecondaries(pcClass);
}

//////////////////////////////////////////////////////////////////////////
//
// A secondary vtable at iOffset, starting as a base's.  The class's own functions
// override; they need this moved back from the subobject to the class.
//
//////////////////////////////////////////////////////////////////////////
SCPPSecondaryVtable* CCPPVtable::AddSecondary(CCPPClass* pcClass, int iOffset, CCPPClass* pcOwner, CArrayCPPFunctions* papcFunctions, CArrayInt* paiAdjust)
{
	SCPPSecondaryVtable*	psVtable;
	CCPPFunction*			pcFunction;
	size					i;
	size					j;

	psVtable = macSecondaries.Add();
	psVtable->iOffset = iOffset;
	psVtable->pcOwner = pcOwner;
	psVtable->pcVirtualBase = NULL;
	psVtable->iVirtualBaseOffset = 0;
	psVtable->mapcFunctions.Init();
	psVtable->maiAdjust.Init();
	psVtable->maiVirtualOffsets.Init();
	SetVirtualOffsets(pcClass, iOffset, pcOwner, &psVtable->maiVirtualOffsets);
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
	return psVtable;
}


//////////////////////////////////////////////////////////////////////////
//
// The part of a subobject in the virtual base pcVirtualBase (at iOffset in
// the complete object), starting as its class's: each class of the complete object that
// derives from pcOwner overrides it, a base's before the classes derived
// from it, so the part has the final overriders from every path to it.  An
// overrider's this is moved from the subobject to its class's.
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::AddVirtualPart(CCPPClass* pcClass, CCPPClass* pcVirtualBase, int iOffset, CCPPClass* pcOwner, CArrayCPPFunctions* papcFunctions, CArrayInt* paiAdjust)
{
	SCPPSecondaryVtable*		psVtable;
	CArrayTemplatePtr<CCPPClass>	apcClasses;
	CArrayInt					aiOffsets;
	CCPPClass*					pcOverrider;
	CCPPFunction*				pcFunction;
	size						i;
	size						j;
	size						k;

	psVtable = macSecondaries.Add();
	psVtable->iOffset = iOffset;
	psVtable->pcOwner = pcOwner;
	psVtable->pcVirtualBase = pcVirtualBase;
	psVtable->iVirtualBaseOffset = VirtualBaseOffset(pcClass, pcVirtualBase);
	psVtable->mapcFunctions.Init();
	psVtable->maiAdjust.Init();
	psVtable->maiVirtualOffsets.Init();
	SetVirtualOffsets(pcClass, iOffset, pcOwner, &psVtable->maiVirtualOffsets);
	for (i = 0; i < papcFunctions->NumElements(); i++)
	{
		psVtable->mapcFunctions.Add(papcFunctions->GetPtr(i));
		psVtable->maiAdjust.Add(paiAdjust ? paiAdjust->GetValue(i) : 0);
	}

	apcClasses.Init();
	aiOffsets.Init();
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		AddOverriders(pcClass->macVirtualBases.Get(i)->pcClass, VirtualBaseOffset(pcClass, pcClass->macVirtualBases.Get(i)->pcClass), &apcClasses, &aiOffsets);
	}
	AddOverriders(pcClass, 0, &apcClasses, &aiOffsets);
	for (k = 0; k < apcClasses.NumElements(); k++)
	{
		pcOverrider = apcClasses.GetPtr(k);
		if ((pcOverrider == pcOwner) || !pcOverrider->HasBase(pcOwner))
		{
			continue;
		}
		for (i = 0; i < pcOverrider->mapcMembers.NumElements(); i++)
		{
			pcFunction = pcOverrider->mapcMembers.GetPtr(i);
			if (pcFunction->mbStatic || (pcFunction->meKind == CPPFK_Constructor))
			{
				continue;
			}
			for (j = 0; j < psVtable->mapcFunctions.NumElements(); j++)
			{
				if (pcFunction->Overrides(psVtable->mapcFunctions.GetPtr(j)))
				{
					psVtable->mapcFunctions.SetPtr(j, pcFunction);
					psVtable->maiAdjust.SetValue(j, aiOffsets.GetValue(k) - iOffset);
				}
			}
		}
	}
	aiOffsets.Kill();
	apcClasses.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The classes of pcClass's subobject at iOffset and its non-virtual bases'
// (each base before the class), with where each is, for AddVirtualPart.
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::AddOverriders(CCPPClass* pcClass, int iOffset, CArrayTemplatePtr<CCPPClass>* papcClasses, CArrayInt* paiOffsets)
{
	SCPPBase*	psBase;
	size		i;

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual)
		{
			AddOverriders(psBase->pcClass, iOffset + psBase->iOffset, papcClasses, paiOffsets);
		}
	}
	papcClasses->Add(pcClass);
	paiOffsets->Add(iOffset);
}


//////////////////////////////////////////////////////////////////////////
//
// The distances from a part's subobject (at iOffset in an object of
// pcClass) to each of the part's class's (pcOwner's) virtual bases, which a
// complete object of pcClass puts where it lays them out.
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::SetVirtualOffsets(CCPPClass* pcClass, int iOffset, CCPPClass* pcOwner, CArrayInt* paiOffsets)
{
	size	i;

	for (i = 0; i < pcOwner->macVirtualBases.NumElements(); i++)
	{
		paiOffsets->Add(VirtualBaseOffset(pcClass, pcOwner->macVirtualBases.Get(i)->pcClass) - iOffset);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where pcClass's virtual base pcVirtualBase is, from the start of pcClass's
// object: where pcClass puts it, or for a construction vtable where the
// complete object does.
//
//////////////////////////////////////////////////////////////////////////
int CCPPVtable::VirtualBaseOffset(CCPPClass* pcClass, CCPPClass* pcVirtualBase)
{
	SCPPVirtualBase*	psVirtual;

	psVirtual = (mpcComplete ? mpcComplete : pcClass)->GetVirtualBase(pcVirtualBase);
	if (!psVirtual)
	{
		return 0;
	}
	return mpcComplete ? (psVirtual->iOffset - miCompleteOffset) : psVirtual->iOffset;
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
	SCPPSecondaryVtable*	psVtable;
	size					i;


	AddEntries(pasEntries, &maiVirtualOffsets, (miPointerOffset == -1) ? 0 : miPointerOffset, NULL, &mapcFunctions, NULL);
	for (i = 0; i < macSecondaries.NumElements(); i++)
	{
		psVtable = macSecondaries.Get(i);
		AddEntries(pasEntries, &psVtable->maiVirtualOffsets, psVtable->iOffset + psVtable->pcOwner->mcVtable.miPointerOffset, psVtable, &psVtable->mapcFunctions, &psVtable->maiAdjust);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A part's entries: its virtual offsets (the last first), its header, then
// its functions.  psVirtual is the part's when it is in a virtual base.
//
//////////////////////////////////////////////////////////////////////////
void CCPPVtable::AddEntries(CArrayCPPVtableEntries* pasEntries, CArrayInt* paiVirtualOffsets, int iVtablePointer, SCPPSecondaryVtable* psVirtual, CArrayCPPFunctions* papcFunctions, CArrayInt* paiAdjust)
{
	SCPPVtableEntry*	psEntry;
	size				i;
	CCPPClass*			pcVirtualBase;
	int					iInVirtual;

	pcVirtualBase = (psVirtual && psVirtual->pcVirtualBase) ? psVirtual->pcVirtualBase : NULL;
	iInVirtual = pcVirtualBase ? (iVtablePointer - psVirtual->iVirtualBaseOffset) : 0;

	for (i = paiVirtualOffsets->NumElements(); i > 0; i--)
	{
		psEntry = pasEntries->Add();
		psEntry->eKind = CPPVE_Offset;
		psEntry->iVtablePointer = 0;
		psEntry->pcFunction = NULL;
		psEntry->iAdjust = paiVirtualOffsets->GetValue(i - 1);
		psEntry->pcVirtualBase = pcVirtualBase;
		psEntry->iInVirtual = iInVirtual;
	}
	psEntry = pasEntries->Add();
	psEntry->eKind = CPPVE_Header;
	psEntry->iVtablePointer = iVtablePointer;
	psEntry->pcFunction = NULL;
	psEntry->iAdjust = 0;
	psEntry->pcVirtualBase = pcVirtualBase;
	psEntry->iInVirtual = iInVirtual;
	for (i = 0; i < papcFunctions->NumElements(); i++)
	{
		psEntry = pasEntries->Add();
		psEntry->eKind = CPPVE_Function;
		psEntry->iVtablePointer = 0;
		psEntry->pcFunction = papcFunctions->GetPtr(i);
		psEntry->iAdjust = paiAdjust ? paiAdjust->GetValue(i) : 0;
		psEntry->pcVirtualBase = pcVirtualBase;
		psEntry->iInVirtual = iInVirtual;
	}
}
