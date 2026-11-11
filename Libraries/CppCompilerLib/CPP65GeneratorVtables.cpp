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
#include "CPP65Generator.h"


//The generator's virtual functions: laying them out, the vtables and type info, and the thunks.


//////////////////////////////////////////////////////////////////////////
//
// The vtable starts as the first base's.  A function with the name and parameters
// of a virtual function in it overrides it (and is virtual); other virtual functions
// are added on the end.  A destructor overrides a virtual base destructor.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::LayoutVirtuals(CCPPClass* pcClass, CCPPSyntax* pcNode)
{
	CCPPFunction*		pcFunction;
	CCPPFunction*		pcVirtual;
	size				i;
	size				j;
	bool				bOverride;

	if (pcClass->mpcBase)
	{
		for (i = 0; i < pcClass->mpcBase->mapcVirtuals.NumElements(); i++)
		{
			pcClass->mapcVirtuals.Add(pcClass->mpcBase->mapcVirtuals.GetPtr(i));
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
		for (j = 0; j < pcClass->mapcVirtuals.NumElements(); j++)
		{
			pcVirtual = pcClass->mapcVirtuals.GetPtr(j);
			if (Overrides(pcVirtual, pcFunction))
			{
				pcClass->mapcVirtuals.SetPtr(j, pcFunction);
				pcFunction->mbVirtual = true;
				pcFunction->miSlot = (int)j;
				bOverride = true;
				break;
			}
		}
		//A function that overrides one in a secondary vtable also gets a place in the class's own.
		if (!bOverride && (pcFunction->mbVirtual || OverridesBase(pcClass, pcFunction)))
		{
			pcFunction->mbVirtual = true;
			pcFunction->miSlot = (int)pcClass->mapcVirtuals.NumElements();
			pcClass->mapcVirtuals.Add(pcFunction);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcFunction overrides pcVirtual: destructors override destructors, other
// functions need the same name and parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Overrides(CCPPFunction* pcVirtual, CCPPFunction* pcFunction)
{
	return (pcVirtual->meKind == pcFunction->meKind) && ((pcFunction->meKind == CPPFK_Destructor) || ((pcVirtual->mlliName == pcFunction->mlliName) && SameParameters(pcVirtual, pcFunction)));
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcFunction overrides a virtual function in any of pcClass's bases'
// vtables.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::OverridesBase(CCPPClass* pcClass, CCPPFunction* pcFunction)
{
	CCPPClass*	pcBase;
	SCPPSecondaryVtable*	psVtable;
	size			i;
	size			j;
	size			k;

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		pcBase = pcClass->macBases.Get(i)->pcClass;
		for (j = 0; j < pcBase->mapcVirtuals.NumElements(); j++)
		{
			if (Overrides(pcBase->mapcVirtuals.GetPtr(j), pcFunction))
			{
				return true;
			}
		}
		for (j = 0; j < pcBase->macVtables.NumElements(); j++)
		{
			psVtable = pcBase->macVtables.Get(j);
			for (k = 0; k < psVtable->mapcFunctions.NumElements(); k++)
			{
				if (Overrides(psVtable->mapcFunctions.GetPtr(k), pcFunction))
				{
					return true;
				}
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The vtables of the polymorphic base subobjects that do not share the class's
// vtable pointer: the bases' own (except the first base's when the class shares it)
// and their secondary vtables, with the class's functions overriding.  The base
// offsets must be known.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::LayoutSecondaryVtables(CCPPClass* pcClass)
{
	SCPPBase*		psBase;
	SCPPSecondaryVtable*	psVtable;
	bool			bShared;
	size			i;
	size			j;

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		bShared = (i == 0) && (pcClass->mpcBase == psBase->pcClass) && (psBase->pcClass->miVtableOffset != -1) && (pcClass->miVtableOffset == psBase->pcClass->miVtableOffset);
		if (!bShared && (psBase->pcClass->miVtableOffset != -1))
		{
			AddSecondaryVtable(pcClass, psBase->iOffset, psBase->pcClass, &psBase->pcClass->mapcVirtuals, NULL);
		}
		for (j = 0; j < psBase->pcClass->macVtables.NumElements(); j++)
		{
			psVtable = psBase->pcClass->macVtables.Get(j);
			AddSecondaryVtable(pcClass, psBase->iOffset + psVtable->iOffset, psVtable->pcOwner, &psVtable->mapcFunctions, &psVtable->maiAdjust);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A secondary vtable at iOffset, starting as a base's.  The class's own functions
// override; they need this moved back from the subobject to the class.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddSecondaryVtable(CCPPClass* pcClass, int iOffset, CCPPClass* pcOwner, CArrayCPPFunctions* papcFunctions, CArrayInt* paiAdjust)
{
	SCPPSecondaryVtable*		psVtable;
	CCPPFunction*		pcFunction;
	size				i;
	size				j;

	psVtable = pcClass->macVtables.Add();
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
			if (Overrides(psVtable->mapcFunctions.GetPtr(j), pcFunction))
			{
				psVtable->mapcFunctions.SetPtr(j, pcFunction);
				psVtable->maiAdjust.SetValue(j, -iOffset);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if objects of the class have a vtable pointer, its own or a base's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsPolymorphic(CCPPClass* pcClass)
{
	return (pcClass->miVtableOffset != -1) || (pcClass->macVtables.NumElements() > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function in the class's vtables (its final overriders) is pure
// virtual: no object of the class can be made.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsAbstract(CCPPClass* pcClass)
{
	SCPPSecondaryVtable*	psVtable;
	size			i;
	size			j;

	for (i = 0; i < pcClass->mapcVirtuals.NumElements(); i++)
	{
		if (pcClass->mapcVirtuals.GetPtr(i)->mbPure)
		{
			return true;
		}
	}
	for (i = 0; i < pcClass->macVtables.NumElements(); i++)
	{
		psVtable = pcClass->macVtables.Get(i);
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
// _Class__2: the addresses of the virtual functions, written the first time an
// object of the class is made.  Its functions are generated.  Pure virtual
// functions are R_Pure, which stops.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::EnsureVtable(CCPPClass* pcClass)
{
	CArrayCPPVtableEntries	asEntries;
	SCPPVtableEntry*			psEntry;
	size						i;
	CChars						szLabel;

	if (pcClass->mbVtable)
	{
		return;
	}
	pcClass->mbVtable = true;

	//Each unit that makes one of a class's objects writes its vtable in a COMDAT section
	//(the linker keeps one); a class without a name has its own.
	szLabel.Init();
	AppendVtableLabel(&szLabel, pcClass);
	if (!Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	if (pcClass->mlliName < 0)
	{
		AppendSection(&mszData, "rodata", &szLabel, NULL);
	}
	else
	{
		AppendSection(&mszData, "rodata", &szLabel, ", comdat");
		AppendGlobal(&mszData, &szLabel);
	}
	mszData.Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
	mszData.AppendNewLine();

	//The label is after the first part's header.
	asEntries.Init();
	VtableEntries(pcClass, &asEntries);
	for (i = 0; i < asEntries.NumElements(); i++)
	{
		psEntry = asEntries.Get(i);
		if (psEntry->eKind == CPPVE_Header)
		{
			AppendVtableHeader(pcClass, psEntry->iVtablePointer);
		}
		else
		{
			AppendVtableEntry(psEntry->pcFunction, psEntry->iAdjust);
		}
		if (i == 0)
		{
			mszData.Append(szLabel);
			mszData.Append(':');
			mszData.AppendNewLine();
		}
	}
	asEntries.Kill();
	szLabel.Kill();

	EnsureTypeInfo(pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// The class's vtable, in order (see CPP65_VTABLE_ENTRY_BYTES): its own
// part's header (before the label) and functions, then each secondary
// vtable's header and functions.  The generator writes it (EnsureVtable)
// and the IR front end describes it and stores its parts' addresses from
// the same list.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::VtableEntries(CCPPClass* pcClass, CArrayCPPVtableEntries* pasEntries)
{
	SCPPVtableEntry*	psEntry;
	SCPPSecondaryVtable*		psVtable;
	size				i;
	size				j;

	psEntry = pasEntries->Add();
	psEntry->eKind = CPPVE_Header;
	psEntry->iVtablePointer = (pcClass->miVtableOffset == -1) ? 0 : pcClass->miVtableOffset;
	psEntry->pcFunction = NULL;
	psEntry->iAdjust = 0;
	for (i = 0; i < pcClass->mapcVirtuals.NumElements(); i++)
	{
		psEntry = pasEntries->Add();
		psEntry->eKind = CPPVE_Function;
		psEntry->iVtablePointer = 0;
		psEntry->pcFunction = pcClass->mapcVirtuals.GetPtr(i);
		psEntry->iAdjust = 0;
	}
	for (i = 0; i < pcClass->macVtables.NumElements(); i++)
	{
		psVtable = pcClass->macVtables.Get(i);
		psEntry = pasEntries->Add();
		psEntry->eKind = CPPVE_Header;
		psEntry->iVtablePointer = psVtable->iOffset + psVtable->pcOwner->miVtableOffset;
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


//////////////////////////////////////////////////////////////////////////
//
// The two words before each part of a vtable, for dynamic_cast: how far the
// object's start is from the vtable pointer using that part (iVtablePointer is
// where that pointer is in the object), then the class's type info.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVtableHeader(CCPPClass* pcClass, int iVtablePointer)
{
	mszData.Append("\t\t.long ");
	mszData.Append(-iVtablePointer);
	mszData.AppendNewLine();
	mszData.Append("\t\t.long ");
	AppendTypeInfoLabel(&mszData, pcClass);
	mszData.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__3: the class's type info, for dynamic_cast.  Every subobject of an
// object of the class, the object itself first: the subobject class's type info
// and its offset in the object.  A zero ends it.  Each unit that needs it writes
// it in a COMDAT section (the linker keeps one) so its address identifies the
// class.  The type infos of the bases are written too.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::EnsureTypeInfo(CCPPClass* pcClass)
{
	CChars	szLabel;
	size	i;

	if (pcClass->mbTypeInfo)
	{
		return;
	}
	pcClass->mbTypeInfo = true;

	szLabel.Init();
	AppendTypeInfoLabel(&szLabel, pcClass);
	if (!Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	if (pcClass->mlliName < 0)
	{
		AppendSection(&mszData, "rodata", &szLabel, NULL);
	}
	else
	{
		AppendSection(&mszData, "rodata", &szLabel, ", comdat");
		AppendGlobal(&mszData, &szLabel);
	}
	szLabel.Kill();
	mszData.Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
	mszData.AppendNewLine();
	AppendTypeInfoLabel(&mszData, pcClass);
	mszData.Append(':');
	mszData.AppendNewLine();
	AppendTypeInfoBases(pcClass, 0);
	mszData.Append("\t\t.long 0");
	mszData.AppendNewLine();

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		EnsureTypeInfo(pcClass->macBases.Get(i)->pcClass);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type info entries for the class's subobject at iOffset and all its bases'.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendTypeInfoBases(CCPPClass* pcClass, int iOffset)
{
	SCPPBase*	psBase;
	size		i;

	mszData.Append("\t\t.long ");
	AppendTypeInfoLabel(&mszData, pcClass);
	mszData.AppendNewLine();
	mszData.Append("\t\t.long ");
	mszData.Append(iOffset);
	mszData.AppendNewLine();
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		AppendTypeInfoBases(psBase->pcClass, iOffset + psBase->iOffset);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where a vtable pointer is in an object of a polymorphic class: its own, else
// the first secondary one's.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::VtablePointerOffset(CCPPClass* pcClass)
{
	SCPPSecondaryVtable*	psVtable;

	if (pcClass->miVtableOffset != -1)
	{
		return pcClass->miVtableOffset;
	}
	psVtable = pcClass->macVtables.Get(0);
	return psVtable->iOffset + psVtable->pcOwner->miVtableOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable entry: the function, or a thunk that adds iAdjust to this and jumps to
// it.  Each thunk is written once.  Pure virtual functions are R_Pure.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVtableEntry(CCPPFunction* pcFunction, int iAdjust)
{
	CChars	szLabel;
	CChars	szFunction;

	mszData.Append("\t\t.long ");
	if (pcFunction->mbPure)
	{
		AppendVtableEntryLabel(&mszData, pcFunction, iAdjust);
		mbPureVirtual = true;
		mszData.AppendNewLine();
		return;
	}

	RequireFunction(pcFunction);
	szLabel.Init();
	AppendVtableEntryLabel(&szLabel, pcFunction, iAdjust);
	if (iAdjust != 0)
	{
		if (Emit(&szLabel))
		{
			AppendThunkLabel(&szLabel, !IsUnitFunction(pcFunction));
			szFunction.Init();
			AppendFunctionLabel(&szFunction, pcFunction);
			mpcTarget->AppendAdjustThunk(&mszThunks, iAdjust, szFunction.Text());
			szFunction.Kill();
		}
	}
	mszData.Append(szLabel);
	mszData.AppendNewLine();
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable entry's label: R_Pure for a pure virtual function, else the
// function's, or its thunk's (__t and the adjustment's negation) when this
// is adjusted by iAdjust first.  AppendVtableEntry writes the thunk.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVtableEntryLabel(CChars* psz, CCPPFunction* pcFunction, int iAdjust)
{
	if (pcFunction->mbPure)
	{
		psz->Append("R_Pure");
		return;
	}
	AppendFunctionLabel(psz, pcFunction);
	if (iAdjust != 0)
	{
		psz->Append("__t");
		psz->Append(-iAdjust);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The label of the thunk a pointer to a virtual member function points to (as
// MSVC's vcall thunks): it jumps to the function in the object's vtable
// (CCPPTarget::AppendVirtualCallThunk).  Thunks are named by the vtable
// pointer's offset and the slot, __vcall_8_3, so every class and unit shares
// them (comdat).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVirtualCallThunk(CChars* psz, CCPPFunction* pcFunction)
{
	CChars	szLabel;

	szLabel.Init("__vcall_");
	szLabel.Append(pcFunction->mpcClass->miVtableOffset);
	szLabel.Append('_');
	szLabel.Append(pcFunction->miSlot);
	if (Emit(&szLabel))
	{
		AppendThunkLabel(&szLabel, true);
		mpcTarget->AppendVirtualCallThunk(&mszThunks, pcFunction->mpcClass->miVtableOffset, CPP65_VTABLE_ENTRY_BYTES * pcFunction->miSlot);
	}
	psz->Append(szLabel);
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A thunk's section and label, to be followed by its code.  One every unit
// may write is global, and comdat so the linker keeps one.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendThunkLabel(CChars* pszLabel, bool bComdat)
{
	if (bComdat)
	{
		AppendSection(&mszThunks, "text", pszLabel, ", comdat");
		AppendGlobal(&mszThunks, pszLabel);
	}
	else
	{
		AppendSection(&mszThunks, "text", pszLabel, NULL);
	}
	mszThunks.Append(*pszLabel);
	mszThunks.Append(':');
	mszThunks.AppendNewLine();
}


