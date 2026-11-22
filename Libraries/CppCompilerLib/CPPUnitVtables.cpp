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
#include "CPPUnit.h"


//The unit's virtual functions: laying them out, the vtables and type info, and the thunks.


//////////////////////////////////////////////////////////////////////////
//
// _Class__2: the addresses of the virtual functions, written the first time an
// object of the class is made.  Its functions are generated.  Pure virtual
// functions are R_Pure, which stops.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::EnsureVtable(CCPPClass* pcClass)
{
	CArrayCPPVtableEntries	asEntries;
	CChars					szLabel;

	if (pcClass->mcVtable.mbWritten)
	{
		return;
	}
	pcClass->mcVtable.mbWritten = true;

	//Each unit that makes one of a class's objects writes its vtable in a COMDAT section
	//(the linker keeps one); a class without a name has its own.
	szLabel.Init();
	mcMangler.AppendVtableLabel(&szLabel, pcClass);
	if (!Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	if (pcClass->IsUnnamed())
	{
		CCPPUnitData::AppendSection(mcData.GetText(), "rodata", &szLabel, NULL);
	}
	else
	{
		CCPPUnitData::AppendSection(mcData.GetText(), "rodata", &szLabel, ", comdat");
		CCPPUnitData::AppendGlobal(mcData.GetText(), &szLabel);
	}
	mcData.GetText()->Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
	mcData.GetText()->AppendNewLine();

	asEntries.Init();
	pcClass->mcVtable.GetEntries(&asEntries);
	AppendVtableParts(pcClass, &szLabel, &asEntries);
	asEntries.Kill();
	szLabel.Kill();

	EnsureTypeInfo(pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable's words (CCPPVtable::GetEntries), with the label after the first
// part's header.  The headers have pcClass's type info.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendVtableParts(CCPPClass* pcClass, CChars* pszLabel, CArrayCPPVtableEntries* pasEntries)
{
	SCPPVtableEntry*	psEntry;
	size				i;
	bool				bLabelled;

	bLabelled = false;
	for (i = 0; i < pasEntries->NumElements(); i++)
	{
		psEntry = pasEntries->Get(i);
		if (psEntry->eKind == CPPVE_Offset)
		{
			mcData.GetText()->Append("\t\t.long ")->Append(psEntry->iAdjust);
			mcData.GetText()->AppendNewLine();
			continue;
		}
		if (psEntry->eKind == CPPVE_Header)
		{
			AppendVtableHeader(pcClass, psEntry->iVtablePointer);
		}
		else
		{
			AppendVtableEntry(psEntry->pcFunction, psEntry->iAdjust);
		}
		if (!bLabelled)
		{
			mcData.GetText()->Append(*pszLabel);
			mcData.GetText()->Append(':');
			mcData.GetText()->AppendNewLine();
			bLabelled = true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__4, the VTT of a class with virtual bases (CPP65_VTT_LABEL): the
// class's vtable, then the entries of each base its constructors and
// destructor make with a base version, the non-virtual bases' then the
// virtual bases' (VTTIndex).  A base subobject's entries are its
// construction vtable then its own non-virtual bases' (VTTEntries), which is
// what the base version is passed.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::EnsureVTT(CCPPClass* pcClass)
{
	CChars				szLabel;
	CChars				szWords;
	SCPPBase*			psBase;
	SCPPVirtualBase*	psVirtual;
	size				i;

	szLabel.Init();
	mcMangler.AppendVTTLabel(&szLabel, pcClass);
	if (!Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	EnsureVtable(pcClass);

	//The construction vtables are written first, each in its own section.
	szWords.Init();
	szWords.Append("\t\t.long ");
	mcMangler.AppendVtableLabel(&szWords, pcClass);
	szWords.AppendNewLine();
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual && psBase->pcClass->HasVirtualBases())
		{
			AppendVTTEntries(pcClass, psBase->pcClass, psBase->iOffset, &szWords);
		}
	}
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		psVirtual = pcClass->macVirtualBases.Get(i);
		if (psVirtual->pcClass->HasVirtualBases())
		{
			AppendVTTEntries(pcClass, psVirtual->pcClass, psVirtual->iOffset, &szWords);
		}
	}

	if (pcClass->IsUnnamed())
	{
		CCPPUnitData::AppendSection(mcData.GetText(), "rodata", &szLabel, NULL);
	}
	else
	{
		CCPPUnitData::AppendSection(mcData.GetText(), "rodata", &szLabel, ", comdat");
		CCPPUnitData::AppendGlobal(mcData.GetText(), &szLabel);
	}
	mcData.GetText()->Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
	mcData.GetText()->AppendNewLine();
	mcData.GetText()->Append(szLabel);
	mcData.GetText()->Append(':');
	mcData.GetText()->AppendNewLine();
	mcData.GetText()->Append(szWords);
	szWords.Kill();
	szLabel.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The VTT entries (in pszWords) of the pcClass subobject at iOffset in a
// pcComplete: its construction vtable, which is written, then its non-
// virtual bases' entries.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendVTTEntries(CCPPClass* pcComplete, CCPPClass* pcClass, int iOffset, CChars* pszWords)
{
	CCPPVtable				cVtable;
	CArrayCPPVtableEntries	asEntries;
	CChars					szLabel;
	SCPPBase*				psBase;
	size					i;

	szLabel.Init();
	mcMangler.AppendConstructionVtableLabel(&szLabel, pcComplete, pcClass, iOffset);
	pszWords->Append("\t\t.long ");
	pszWords->Append(szLabel);
	pszWords->AppendNewLine();
	if (Emit(&szLabel))
	{
		if (pcComplete->IsUnnamed())
		{
			CCPPUnitData::AppendSection(mcData.GetText(), "rodata", &szLabel, NULL);
		}
		else
		{
			CCPPUnitData::AppendSection(mcData.GetText(), "rodata", &szLabel, ", comdat");
			CCPPUnitData::AppendGlobal(mcData.GetText(), &szLabel);
		}
		mcData.GetText()->Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
		mcData.GetText()->AppendNewLine();
		cVtable.Init();
		cVtable.LayoutConstruction(pcClass, pcComplete, iOffset);
		asEntries.Init();
		cVtable.GetEntries(&asEntries);
		AppendVtableParts(pcClass, &szLabel, &asEntries);
		asEntries.Kill();
		cVtable.Kill();
		EnsureTypeInfo(pcClass);
	}
	szLabel.Kill();

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual && psBase->pcClass->HasVirtualBases())
		{
			AppendVTTEntries(pcComplete, psBase->pcClass, iOffset + psBase->iOffset, pszWords);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many words of VTT a base version of a constructor or destructor of
// pcClass is given: its construction vtable, then its non-virtual bases'.
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnit::VTTEntries(CCPPClass* pcClass)
{
	SCPPBase*	psBase;
	size		i;
	size		uiEntries;

	uiEntries = 1;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual && psBase->pcClass->HasVirtualBases())
		{
			uiEntries += VTTEntries(psBase->pcClass);
		}
	}
	return uiEntries;
}


//////////////////////////////////////////////////////////////////////////
//
// Where in pcClass's VTT (in words) the entries for its base pcBase are, a
// direct non-virtual base or a virtual base that has virtual bases of its
// own; -1 if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
int CCPPUnit::VTTIndex(CCPPClass* pcClass, CCPPClass* pcBase)
{
	SCPPBase*			psBase;
	SCPPVirtualBase*	psVirtual;
	size				i;
	size				uiIndex;

	uiIndex = 1;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual && psBase->pcClass->HasVirtualBases())
		{
			if (psBase->pcClass == pcBase)
			{
				return (int)uiIndex;
			}
			uiIndex += VTTEntries(psBase->pcClass);
		}
	}
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		psVirtual = pcClass->macVirtualBases.Get(i);
		if (psVirtual->pcClass->HasVirtualBases())
		{
			if (psVirtual->pcClass == pcBase)
			{
				return (int)uiIndex;
			}
			uiIndex += VTTEntries(psVirtual->pcClass);
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The two words before each part of a vtable, for dynamic_cast: how far the
// object's start is from the vtable pointer using that part (iVtablePointer is
// where that pointer is in the object), then the class's type info (a
// construction vtable's is its subobject's class's, which typeid and
// dynamic_cast give while it is made).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendVtableHeader(CCPPClass* pcClass, int iVtablePointer)
{
	mcData.GetText()->Append("\t\t.long ");
	mcData.GetText()->Append(-iVtablePointer);
	mcData.GetText()->AppendNewLine();
	mcData.GetText()->Append("\t\t.long ");
	mcMangler.AppendTypeInfoLabel(mcData.GetText(), pcClass);
	mcData.GetText()->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__3: the class's type info, for typeid and dynamic_cast: its name and
// kind (CPP65_TYPE_INFO_NAME_OFFSET), then every subobject of an object of the
// class, the object itself first: the subobject class's type info, its
// offset in the object and its flags (CPP65_TYPE_INFO_PUBLIC,
// CPP65_TYPE_INFO_VIRTUAL, which virtual base it is or is in, and in the
// class's own entry where its vtable pointer is).  A zero ends it.  Each unit that needs it writes
// it in a COMDAT section (the linker keeps one) so its address identifies the
// class.  The type infos of the bases are written too.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::EnsureTypeInfo(CCPPClass* pcClass)
{
	CChars		szLabel;
	CChars		szName;
	CCPPClass*	pcVirtual;
	size		i;
	uint32		uiVtable;

	if (pcClass->mbTypeInfo)
	{
		return;
	}
	pcClass->mbTypeInfo = true;

	szLabel.Init();
	mcMangler.AppendTypeInfoLabel(&szLabel, pcClass);
	if (!Emit(&szLabel))
	{
		szLabel.Kill();
		return;
	}
	szName.Init();
	AppendTypeInfoName(&szLabel, pcClass->mpsType, !pcClass->IsUnnamed(), &szName);
	if (pcClass->IsUnnamed())
	{
		CCPPUnitData::AppendSection(mcData.GetText(), "rodata", &szLabel, NULL);
	}
	else
	{
		CCPPUnitData::AppendSection(mcData.GetText(), "rodata", &szLabel, ", comdat");
		CCPPUnitData::AppendGlobal(mcData.GetText(), &szLabel);
	}
	szLabel.Kill();
	mcData.GetText()->Append("\t\t.align ")->Append(CPP65_VTABLE_ENTRY_BYTES);
	mcData.GetText()->AppendNewLine();
	mcMangler.AppendTypeInfoLabel(mcData.GetText(), pcClass);
	mcData.GetText()->Append(':');
	mcData.GetText()->AppendNewLine();
	AppendTypeInfoHeader(&szName, CPP65TIK_Class);
	uiVtable = 0;
	if (pcClass->IsPolymorphic() || (pcClass->macVirtualBases.NumElements() > 0))
	{
		uiVtable = ((uint32)pcClass->GetVtablePointerOffset() + 1) << CPP65_TYPE_INFO_VTABLE_SHIFT;
	}
	AppendTypeInfoBases(pcClass, 0, CPP65_TYPE_INFO_PUBLIC, uiVtable);
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		pcVirtual = pcClass->macVirtualBases.Get(i)->pcClass;
		AppendTypeInfoBases(pcVirtual, pcClass->macVirtualBases.Get(i)->iOffset, CPP65_TYPE_INFO_VIRTUAL | (IsPublicBase(pcClass, pcVirtual) ? CPP65_TYPE_INFO_PUBLIC : 0),
							((uint32)pcClass->GetVirtualBaseIndex(pcVirtual) + 1) << CPP65_TYPE_INFO_IN_VIRTUAL_SHIFT);
	}
	mcData.GetText()->Append("\t\t.long 0");
	mcData.GetText()->AppendNewLine();
	szName.Kill();

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		EnsureTypeInfo(pcClass->macBases.Get(i)->pcClass);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type info entries for the class's subobject at iOffset and its non-
// virtual bases' (a virtual base's are the complete object's, once), with
// its flags (uiFlags): whether only public bases lead to it, and whether it
// is a virtual base.  uiHigh is the flags word's bits from 8 up: which
// virtual base the subobject is or is in, which its bases have too, and for
// the class's own entry where its vtable pointer is, which they don't.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendTypeInfoBases(CCPPClass* pcClass, int iOffset, uint32 uiFlags, uint32 uiHigh)
{
	SCPPBase*	psBase;
	size		i;

	mcData.GetText()->Append("\t\t.long ");
	mcMangler.AppendTypeInfoLabel(mcData.GetText(), pcClass);
	mcData.GetText()->AppendNewLine();
	mcData.GetText()->Append("\t\t.long ");
	mcData.GetText()->Append(iOffset);
	mcData.GetText()->AppendNewLine();
	mcData.GetText()->Append("\t\t.long ");
	mcData.GetText()->Append((int)(uiFlags | uiHigh));
	mcData.GetText()->AppendNewLine();
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual)
		{
			AppendTypeInfoBases(psBase->pcClass, iOffset + psBase->iOffset, (psBase->eAccess == CPPA_Public) ? (uiFlags & CPP65_TYPE_INFO_PUBLIC) : 0, uiHigh & CPP65_TYPE_INFO_IN_VIRTUAL_MASK);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcBase is a base of pcClass in one way only, which passes no
// virtual base and is public at every step: a pcBase is then the base of at
// most one pcClass object, *piOffset bytes into it.  (pcClass itself isn't.)
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsOnePublicBase(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset)
{
	CArrayCPPBasePaths*	pasPaths;
	SCPPBasePath*		psPath;
	SCPPBasePath*		psOne;
	size				i;

	*piOffset = 0;
	if (pcClass == pcBase)
	{
		return false;
	}
	mpcStatistics->Count(CPPCC_BaseWalks);
	pasPaths = pcClass->BasePaths();
	psOne = NULL;
	for (i = 1; i < pasPaths->NumElements(); i++)
	{
		psPath = pasPaths->Get(i);
		if (psPath->pcClass == pcBase)
		{
			if (psOne || psPath->pcVirtual)
			{
				return false;
			}
			psOne = psPath;
		}
	}
	if (psOne)
	{
		*piOffset = psOne->iOffset;
	}
	return psOne && psOne->bPublic;
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcBase is reached from pcClass through public bases only, by one
// path at least (a virtual base has many).  [class.access.base] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsPublicBase(CCPPClass* pcClass, CCPPClass* pcBase)
{
	CArrayCPPBasePaths*	pasPaths;
	size				i;

	pasPaths = pcClass->BasePaths();
	for (i = 0; i < pasPaths->NumElements(); i++)
	{
		if ((pasPaths->Get(i)->pcClass == pcBase) && pasPaths->Get(i)->bPublic)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A type info's header, in the section being written: its name's label
// (pszName, AppendTypeInfoName) and its kind's word (an ECPP65TypeInfoKind,
// and for a pointer CPP65_TYPE_INFO_POINTEE_CONST and _VOLATILE).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendTypeInfoHeader(CChars* pszName, uint32 uiKind)
{
	mcData.GetText()->Append("\t\t.long ");
	mcData.GetText()->Append(pszName);
	mcData.GetText()->AppendNewLine();
	mcData.GetText()->Append("\t\t.long ");
	mcData.GetText()->Append((int)uiKind);
	mcData.GetText()->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The name a type info points to (pszInfo's, with __Name: *pszName), a
// string of its own written before it: the type as the program writes it
// (CCPPTypeWriter, shown).  COMDAT and global with a type info that is (bShared).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendTypeInfoName(CChars* pszInfo, SCPP65Type* psType, bool bShared, CChars* pszName)
{
	CChars			szText;
	CCPPTypeWriter	cWriter;
	size			i;

	pszName->Append(pszInfo);
	pszName->Append("__Name");
	szText.Init();
	cWriter.Init(this, CPPTW_Shown);
	cWriter.AppendType(psType, &szText);
	CCPPUnitData::AppendSection(mcData.GetText(), "rodata", pszName, bShared ? ", comdat" : NULL);
	if (bShared)
	{
		CCPPUnitData::AppendGlobal(mcData.GetText(), pszName);
	}
	mcData.GetText()->Append(pszName);
	mcData.GetText()->Append(':');
	mcData.GetText()->AppendNewLine();
	mcData.GetText()->Append("\t\t.byte ");
	for (i = 0; i < szText.Length(); i++)
	{
		mcData.GetText()->Append((int)(uint8)szText.GetChar(i));
		mcData.GetText()->Append(", ");
	}
	mcData.GetText()->Append('0');
	mcData.GetText()->AppendNewLine();
	szText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The routine an asm statement is: its text in a routine of the unit's own
// (local to its object), which the statement calls.  Its label is in
// *pszLabel.  [dcl.asm]
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AsmRoutine(const char* szText, CChars* pszLabel)
{
	pszLabel->Append("__asm");
	pszLabel->Append(miLabels++);
	mszThunks.Append(pszLabel)->Append(":\n");
	mpcTarget->AppendAsmRoutine(&mszThunks, szText);
}


//////////////////////////////////////////////////////////////////////////
//
// A vtable entry: the function, or a thunk that adds iAdjust to this and jumps to
// it.  Each thunk is written once.  Pure virtual functions are R_Pure.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::AppendVtableEntry(CCPPFunction* pcFunction, int iAdjust)
{
	CChars	szLabel;
	CChars	szFunction;

	mcData.GetText()->Append("\t\t.long ");
	if (pcFunction->mbPure)
	{
		mcMangler.AppendVtableEntryLabel(mcData.GetText(), pcFunction, iAdjust);
		mbPureVirtual = true;
		mcData.GetText()->AppendNewLine();
		return;
	}

	RequireFunction(pcFunction);
	szLabel.Init();
	mcMangler.AppendVtableEntryLabel(&szLabel, pcFunction, iAdjust);
	if (iAdjust != 0)
	{
		if (Emit(&szLabel))
		{
			AppendThunkLabel(&szLabel, !pcFunction->IsUnitFunction());
			szFunction.Init();
			mcMangler.AppendFunctionLabel(&szFunction, pcFunction);
			mpcTarget->AppendAdjustThunk(&mszThunks, iAdjust, szFunction.Text());
			szFunction.Kill();
		}
	}
	mcData.GetText()->Append(szLabel);
	mcData.GetText()->AppendNewLine();
	szLabel.Kill();
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
void CCPPUnit::AppendVirtualCallThunk(CChars* psz, CCPPFunction* pcFunction)
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
void CCPPUnit::AppendThunkLabel(CChars* pszLabel, bool bComdat)
{
	if (bComdat)
	{
		CCPPUnitData::AppendSection(&mszThunks, "text", pszLabel, ", comdat");
		CCPPUnitData::AppendGlobal(&mszThunks, pszLabel);
	}
	else
	{
		CCPPUnitData::AppendSection(&mszThunks, "text", pszLabel, NULL);
	}
	mszThunks.Append(*pszLabel);
	mszThunks.Append(':');
	mszThunks.AppendNewLine();
}


