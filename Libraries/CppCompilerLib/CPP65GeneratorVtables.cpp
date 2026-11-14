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

	if (pcClass->mcVtable.mbWritten)
	{
		return;
	}
	pcClass->mcVtable.mbWritten = true;

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
	pcClass->mcVtable.GetEntries(&asEntries);
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
			AppendThunkLabel(&szLabel, !pcFunction->IsUnitFunction());
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
	szLabel.Append(pcFunction->mpcClass->mcVtable.miPointerOffset);
	szLabel.Append('_');
	szLabel.Append(pcFunction->miSlot);
	if (Emit(&szLabel))
	{
		AppendThunkLabel(&szLabel, true);
		mpcTarget->AppendVirtualCallThunk(&mszThunks, pcFunction->mpcClass->mcVtable.miPointerOffset, CPP65_VTABLE_ENTRY_BYTES * pcFunction->miSlot);
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


