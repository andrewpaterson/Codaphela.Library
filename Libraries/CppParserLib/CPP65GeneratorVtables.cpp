/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

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
bool CCPP65Generator::LayoutVirtuals(SCPP65Class* psClass, CCPPSyntax* pcNode)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psVirtual;
	size				i;
	size				j;
	bool				bOverride;

	if (psClass->psBase)
	{
		for (i = 0; i < psClass->psBase->mapsVirtuals.NumElements(); i++)
		{
			psClass->mapsVirtuals.Add(psClass->psBase->mapsVirtuals.GetPtr(i));
		}
	}

	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass != psClass) || psFunction->bStatic || (psFunction->eKind == CPP65FK_Constructor))
		{
			continue;
		}

		bOverride = false;
		for (j = 0; j < psClass->mapsVirtuals.NumElements(); j++)
		{
			psVirtual = psClass->mapsVirtuals.GetPtr(j);
			if (Overrides(psVirtual, psFunction))
			{
				psClass->mapsVirtuals.SetPtr(j, psFunction);
				psFunction->bVirtual = true;
				psFunction->iSlot = (int)j;
				bOverride = true;
				break;
			}
		}
		//A function that overrides one in a secondary vtable also gets a place in the class's own.
		if (!bOverride && (psFunction->bVirtual || OverridesBase(psClass, psFunction)))
		{
			psFunction->bVirtual = true;
			psFunction->iSlot = (int)psClass->mapsVirtuals.NumElements();
			psClass->mapsVirtuals.Add(psFunction);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if psFunction overrides psVirtual: destructors override destructors, other
// functions need the same name and parameters.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Overrides(SCPP65Function* psVirtual, SCPP65Function* psFunction)
{
	return (psVirtual->eKind == psFunction->eKind) && ((psFunction->eKind == CPP65FK_Destructor) || ((psVirtual->lliName == psFunction->lliName) && SameParameters(psVirtual, psFunction)));
}


//////////////////////////////////////////////////////////////////////////
//
// True if psFunction overrides a virtual function in any of psClass's bases'
// vtables.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::OverridesBase(SCPP65Class* psClass, SCPP65Function* psFunction)
{
	SCPP65Class*	psBase;
	SCPP65Vtable*	psVtable;
	size			i;
	size			j;
	size			k;

	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i)->psClass;
		for (j = 0; j < psBase->mapsVirtuals.NumElements(); j++)
		{
			if (Overrides(psBase->mapsVirtuals.GetPtr(j), psFunction))
			{
				return true;
			}
		}
		for (j = 0; j < psBase->macVtables.NumElements(); j++)
		{
			psVtable = psBase->macVtables.Get(j);
			for (k = 0; k < psVtable->mapsFunctions.NumElements(); k++)
			{
				if (Overrides(psVtable->mapsFunctions.GetPtr(k), psFunction))
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
void CCPP65Generator::LayoutSecondaryVtables(SCPP65Class* psClass)
{
	SCPP65Base*		psBase;
	SCPP65Vtable*	psVtable;
	bool			bShared;
	size			i;
	size			j;

	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		bShared = (i == 0) && (psClass->psBase == psBase->psClass) && (psBase->psClass->iVtableOffset != -1) && (psClass->iVtableOffset == psBase->psClass->iVtableOffset);
		if (!bShared && (psBase->psClass->iVtableOffset != -1))
		{
			AddSecondaryVtable(psClass, psBase->iOffset, psBase->psClass, &psBase->psClass->mapsVirtuals, NULL);
		}
		for (j = 0; j < psBase->psClass->macVtables.NumElements(); j++)
		{
			psVtable = psBase->psClass->macVtables.Get(j);
			AddSecondaryVtable(psClass, psBase->iOffset + psVtable->iOffset, psVtable->psOwner, &psVtable->mapsFunctions, &psVtable->maiAdjust);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A secondary vtable at iOffset, starting as a base's.  The class's own functions
// override; they need this moved back from the subobject to the class.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddSecondaryVtable(SCPP65Class* psClass, int iOffset, SCPP65Class* psOwner, CArrayCPP65FunctionPtrs* papsFunctions, CArrayInt* paiAdjust)
{
	SCPP65Vtable*		psVtable;
	SCPP65Function*		psFunction;
	size				i;
	size				j;

	psVtable = psClass->macVtables.Add();
	psVtable->iOffset = iOffset;
	psVtable->psOwner = psOwner;
	psVtable->mapsFunctions.Init();
	psVtable->maiAdjust.Init();
	for (i = 0; i < papsFunctions->NumElements(); i++)
	{
		psVtable->mapsFunctions.Add(papsFunctions->GetPtr(i));
		psVtable->maiAdjust.Add(paiAdjust ? paiAdjust->GetValue(i) : 0);
	}

	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass != psClass) || psFunction->bStatic || (psFunction->eKind == CPP65FK_Constructor))
		{
			continue;
		}
		for (j = 0; j < psVtable->mapsFunctions.NumElements(); j++)
		{
			if (Overrides(psVtable->mapsFunctions.GetPtr(j), psFunction))
			{
				psVtable->mapsFunctions.SetPtr(j, psFunction);
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
bool CCPP65Generator::IsPolymorphic(SCPP65Class* psClass)
{
	return (psClass->iVtableOffset != -1) || (psClass->macVtables.NumElements() > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function in the class's vtables (its final overriders) is pure
// virtual: no object of the class can be made.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsAbstract(SCPP65Class* psClass)
{
	SCPP65Vtable*	psVtable;
	size			i;
	size			j;

	for (i = 0; i < psClass->mapsVirtuals.NumElements(); i++)
	{
		if (psClass->mapsVirtuals.GetPtr(i)->bPure)
		{
			return true;
		}
	}
	for (i = 0; i < psClass->macVtables.NumElements(); i++)
	{
		psVtable = psClass->macVtables.Get(i);
		for (j = 0; j < psVtable->mapsFunctions.NumElements(); j++)
		{
			if (psVtable->mapsFunctions.GetPtr(j)->bPure)
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
void CCPP65Generator::EnsureVtable(SCPP65Class* psClass)
{
	CArrayCPP65VtableEntries	asEntries;
	SCPP65VtableEntry*			psEntry;
	size						i;
	CChars						szLabel;

	if (psClass->bVtable)
	{
		return;
	}
	psClass->bVtable = true;

	//Each unit that makes one of a class's objects writes its vtable in a COMDAT section
	//(the linker keeps one); a class without a name has its own.
	szLabel.Init();
	AppendVtableLabel(&szLabel, psClass);
	if (!Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	if (psClass->lliName < 0)
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
	VtableEntries(psClass, &asEntries);
	for (i = 0; i < asEntries.NumElements(); i++)
	{
		psEntry = asEntries.Get(i);
		if (psEntry->eKind == CPP65VE_Header)
		{
			AppendVtableHeader(psClass, psEntry->iVtablePointer);
		}
		else
		{
			AppendVtableEntry(psEntry->psFunction, psEntry->iAdjust);
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

	EnsureTypeInfo(psClass);
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
void CCPP65Generator::VtableEntries(SCPP65Class* psClass, CArrayCPP65VtableEntries* pasEntries)
{
	SCPP65VtableEntry*	psEntry;
	SCPP65Vtable*		psVtable;
	size				i;
	size				j;

	psEntry = pasEntries->Add();
	psEntry->eKind = CPP65VE_Header;
	psEntry->iVtablePointer = (psClass->iVtableOffset == -1) ? 0 : psClass->iVtableOffset;
	psEntry->psFunction = NULL;
	psEntry->iAdjust = 0;
	for (i = 0; i < psClass->mapsVirtuals.NumElements(); i++)
	{
		psEntry = pasEntries->Add();
		psEntry->eKind = CPP65VE_Function;
		psEntry->iVtablePointer = 0;
		psEntry->psFunction = psClass->mapsVirtuals.GetPtr(i);
		psEntry->iAdjust = 0;
	}
	for (i = 0; i < psClass->macVtables.NumElements(); i++)
	{
		psVtable = psClass->macVtables.Get(i);
		psEntry = pasEntries->Add();
		psEntry->eKind = CPP65VE_Header;
		psEntry->iVtablePointer = psVtable->iOffset + psVtable->psOwner->iVtableOffset;
		psEntry->psFunction = NULL;
		psEntry->iAdjust = 0;
		for (j = 0; j < psVtable->mapsFunctions.NumElements(); j++)
		{
			psEntry = pasEntries->Add();
			psEntry->eKind = CPP65VE_Function;
			psEntry->iVtablePointer = 0;
			psEntry->psFunction = psVtable->mapsFunctions.GetPtr(j);
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
void CCPP65Generator::AppendVtableHeader(SCPP65Class* psClass, int iVtablePointer)
{
	mszData.Append("\t\t.long ");
	mszData.Append(-iVtablePointer);
	mszData.AppendNewLine();
	mszData.Append("\t\t.long ");
	AppendTypeInfoLabel(&mszData, psClass);
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
void CCPP65Generator::EnsureTypeInfo(SCPP65Class* psClass)
{
	CChars	szLabel;
	size	i;

	if (psClass->bTypeInfo)
	{
		return;
	}
	psClass->bTypeInfo = true;

	szLabel.Init();
	AppendTypeInfoLabel(&szLabel, psClass);
	if (!Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	if (psClass->lliName < 0)
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
	AppendTypeInfoLabel(&mszData, psClass);
	mszData.Append(':');
	mszData.AppendNewLine();
	AppendTypeInfoBases(psClass, 0);
	mszData.Append("\t\t.long 0");
	mszData.AppendNewLine();

	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		EnsureTypeInfo(psClass->macBases.Get(i)->psClass);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type info entries for the class's subobject at iOffset and all its bases'.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendTypeInfoBases(SCPP65Class* psClass, int iOffset)
{
	SCPP65Base*	psBase;
	size		i;

	mszData.Append("\t\t.long ");
	AppendTypeInfoLabel(&mszData, psClass);
	mszData.AppendNewLine();
	mszData.Append("\t\t.long ");
	mszData.Append(iOffset);
	mszData.AppendNewLine();
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		AppendTypeInfoBases(psBase->psClass, iOffset + psBase->iOffset);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where a vtable pointer is in an object of a polymorphic class: its own, else
// the first secondary one's.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::VtablePointerOffset(SCPP65Class* psClass)
{
	SCPP65Vtable*	psVtable;

	if (psClass->iVtableOffset != -1)
	{
		return psClass->iVtableOffset;
	}
	psVtable = psClass->macVtables.Get(0);
	return psVtable->iOffset + psVtable->psOwner->iVtableOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable entry: the function, or a thunk that adds iAdjust to this and jumps to
// it.  Each thunk is written once.  Pure virtual functions are R_Pure.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVtableEntry(SCPP65Function* psFunction, int iAdjust)
{
	CChars	szLabel;

	mszData.Append("\t\t.long ");
	if (psFunction->bPure)
	{
		AppendVtableEntryLabel(&mszData, psFunction, iAdjust);
		mbPureVirtual = true;
		mszData.AppendNewLine();
		return;
	}

	RequireFunction(psFunction);
	szLabel.Init();
	AppendVtableEntryLabel(&szLabel, psFunction, iAdjust);
	if (iAdjust != 0)
	{
		if (Emit(&szLabel))
		{
			AppendThunkLabel(&szLabel, !IsUnitFunction(psFunction));
			if (mpsTarget->eTarget == CPPT_W65C816)
			{
				AppendAdjustThunk816(iAdjust);
				mszThunks.Append("\t\tjml ");
			}
			else
			{
				mszThunks.Append("\t\tadd X, ");
				mszThunks.Append(iAdjust);
				mszThunks.AppendNewLine();
				mszThunks.Append("\t\tjmp ");
			}
			AppendFunctionLabel(&mszThunks, psFunction);
			mszThunks.AppendNewLine();
			mszThunks.AppendNewLine();
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
void CCPP65Generator::AppendVtableEntryLabel(CChars* psz, SCPP65Function* psFunction, int iAdjust)
{
	if (psFunction->bPure)
	{
		psz->Append("R_Pure");
		return;
	}
	AppendFunctionLabel(psz, psFunction);
	if (iAdjust != 0)
	{
		psz->Append("__t");
		psz->Append(-iAdjust);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The label of the thunk a pointer to a virtual member function points to (as
// MSVC's vcall thunks): with the object in X it jumps to the function in the
// object's vtable.  Thunks are named by the vtable pointer's offset and the slot,
// __vcall_8_3, so every class and unit shares them (comdat).
//
//		__vcall_8_3:
//				cpy A, [8 + X]
//				jmp [12 + A]
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVirtualCallThunk(CChars* psz, SCPP65Function* psFunction)
{
	CChars	szLabel;

	szLabel.Init("__vcall_");
	szLabel.Append(psFunction->psClass->iVtableOffset);
	szLabel.Append('_');
	szLabel.Append(psFunction->iSlot);
	if (Emit(&szLabel))
	{
		AppendThunkLabel(&szLabel, true);
		if (mpsTarget->eTarget == CPPT_W65C816)
		{
			AppendVirtualCallThunk816(psFunction);
		}
		else
		{
			mszThunks.Append("\t\tcpy A, [");
			mszThunks.Append(psFunction->psClass->iVtableOffset);
			mszThunks.Append(" + X]");
			mszThunks.AppendNewLine();
			mszThunks.Append("\t\tjmp [");
			mszThunks.Append(CPP65_VTABLE_ENTRY_BYTES * psFunction->iSlot);
			mszThunks.Append(" + A]");
			mszThunks.AppendNewLine();
			mszThunks.AppendNewLine();
		}
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


//////////////////////////////////////////////////////////////////////////
//
// szOpcode of a word on the stack or the direct page, as lda [S + 4].
//
//////////////////////////////////////////////////////////////////////////
static void AppendThunkWord(CChars* pszThunks, const char* szOpcode, const char* szBase, int iOffset)
{
	pszThunks->Append("\t\t")->Append(szOpcode)->Append(" [")->Append(szBase)->Append(" + ")->Append(iOffset)->Append("]\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Loads into A the word at Y from the long pointer on the direct page at
// iPointer, Y being iOffset.
//
//////////////////////////////////////////////////////////////////////////
static void AppendThunkPointerLoad(CChars* pszThunks, int iPointer, int iOffset)
{
	pszThunks->Append("\t\tldy ")->Append(iOffset)->AppendNewLine();
	pszThunks->Append("\t\tlda [[D + ")->Append(iPointer)->Append("]:24 + Y]\n");
}


//////////////////////////////////////////////////////////////////////////
//
// A W65C816 this adjusting thunk's body before its jml: this is above the
// return address (CPP65_816_ADJUST_THIS).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendAdjustThunk816(int iAdjust)
{
	mszThunks.Append("\t\t.width (A, X), 16\n");
	mszThunks.Append("\t\tclc\n");
	AppendThunkWord(&mszThunks, "lda", "S", CPP65_816_ADJUST_THIS);
	mszThunks.Append("\t\tadc 0x")->Append((uint32)iAdjust & 0xFFFF, 16)->AppendNewLine();
	AppendThunkWord(&mszThunks, "sta", "S", CPP65_816_ADJUST_THIS);
	AppendThunkWord(&mszThunks, "lda", "S", CPP65_816_ADJUST_THIS + CPP65_816_WORD_BYTES);
	mszThunks.Append("\t\tadc 0x")->Append(((uint32)iAdjust >> 16) & 0xFFFF, 16)->AppendNewLine();
	AppendThunkWord(&mszThunks, "sta", "S", CPP65_816_ADJUST_THIS + CPP65_816_WORD_BYTES);
}


//////////////////////////////////////////////////////////////////////////
//
// A W65C816 virtual call thunk's body, on the frame CPP65_816_VCALL_ names.
// The function's address - 1 is put where the caller's D was, D restored,
// the bytes below it dropped and rtl jumps to the function with the stack as
// the caller left it.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVirtualCallThunk816(SCPP65Function* psFunction)
{
	int		iVtable;
	int		iEntry;

	iVtable = psFunction->psClass->iVtableOffset;
	iEntry = CPP65_VTABLE_ENTRY_BYTES * psFunction->iSlot;
	mszThunks.Append("\t\t.width (A, X), 16\n");
	mszThunks.Append("\t\tphd\n");
	mszThunks.Append("\t\tpha\n");
	mszThunks.Append("\t\tpha\n");
	mszThunks.Append("\t\ttsc\n");
	mszThunks.Append("\t\ttcd\n");
	AppendThunkPointerLoad(&mszThunks, CPP65_816_VCALL_THIS, iVtable);
	AppendThunkWord(&mszThunks, "sta", "D", CPP65_816_VCALL_VTABLE);
	AppendThunkPointerLoad(&mszThunks, CPP65_816_VCALL_THIS, iVtable + CPP65_816_WORD_BYTES);
	AppendThunkWord(&mszThunks, "sta", "D", CPP65_816_VCALL_VTABLE + CPP65_816_WORD_BYTES);
	AppendThunkPointerLoad(&mszThunks, CPP65_816_VCALL_VTABLE, iEntry + CPP65_816_WORD_BYTES);
	mszThunks.Append("\t\ttax\n");
	AppendThunkPointerLoad(&mszThunks, CPP65_816_VCALL_VTABLE, iEntry);
	mszThunks.Append("\t\tdec A\n");
	AppendThunkWord(&mszThunks, "sta", "D", CPP65_816_VCALL_VTABLE);
	AppendThunkWord(&mszThunks, "lda", "D", CPP65_816_VCALL_CALLER_D);
	mszThunks.Append("\t\ttay\n");
	AppendThunkWord(&mszThunks, "lda", "D", CPP65_816_VCALL_VTABLE);
	AppendThunkWord(&mszThunks, "sta", "D", CPP65_816_VCALL_TARGET);
	mszThunks.Append("\t\ttxa\n");
	mszThunks.Append("\t\tsep m\n");
	AppendThunkWord(&mszThunks, "sta", "D", CPP65_816_VCALL_TARGET_BANK);
	mszThunks.Append("\t\tpla\n");
	mszThunks.Append("\t\trep m\n");
	mszThunks.Append("\t\tpla\n");
	mszThunks.Append("\t\ttya\n");
	mszThunks.Append("\t\ttcd\n");
	mszThunks.Append("\t\trtl\n");
	mszThunks.AppendNewLine();
}
